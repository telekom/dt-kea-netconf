// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <asiolink/io_error.h>
#include <exceptions/exceptions.h>
#include <netconf/netconf_cfg_mgr.h>
#include <netconf/netconf_log.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/foreach.hpp>

#include <sstream>
#include <string>

using namespace std;
using namespace isc::process;
using namespace isc::data;
using namespace isc::http;

using isc::dhcp::DhcpSpaceType;

namespace isc {
namespace netconf {

// *********************** CfgControlSocket  *************************

CfgControlSocket::Type CfgControlSocket::stringToType(const string& type) {
    if (type == "http") {
        return CfgControlSocket::Type::HTTP;
    } else if (type == "kea-config-tool") {
        return CfgControlSocket::Type::KEA_CONFIG_TOOL;
    } else if (type == "stdout") {
        return CfgControlSocket::Type::STDOUT;
    } else if (type == "unix") {
        return CfgControlSocket::Type::UNIX;
    }

    isc_throw(BadValue, "Unknown control socket type: " << type);
}

const string CfgControlSocket::typeToString(CfgControlSocket::Type type) {
    switch (type) {
    case CfgControlSocket::Type::HTTP:
        return "http";
    case CfgControlSocket::Type::KEA_CONFIG_TOOL:
        return "kea-config-tool";
    case CfgControlSocket::Type::STDOUT:
        return "stdout";
    case CfgControlSocket::Type::UNIX:
        return "unix";
    default:
        isc_throw(BadValue, "Unknown control socket type: " << type);
    }
}

ElementPtr CfgControlSocket::toElement() const {
    ElementPtr result = Element::createMap();
    // Set user-context
    contextToElement(result);
    // Set name
    result->set("socket-name", Element::create(name_));
    // Set type
    result->set("socket-type", Element::create(typeToString(type_)));
    // Set url
    result->set("socket-url", Element::create(url_.toText()));
    return result;
}

// *********************** CfgModel  *************************

DhcpSpaceType CfgModel::determineDhcpSpace() const {
    if (model_.find("dhcp6") != string::npos || model_.find("dhcpv6") != string::npos) {
        return isc::dhcp::DHCP_SPACE_V6;
    } else if (model_.find("dhcp4") != string::npos || model_.find("dhcpv4") != string::npos) {
        return isc::dhcp::DHCP_SPACE_V4;
    }
    return isc::dhcp::DHCP_SPACE_UNKNOWN;
}

ElementPtr CfgModel::toElement() const {
    ElementPtr const& result(Element::createMap());
    contextToElement(result);
    result->set("model", Element::create(model_));
    for (auto [str, subscription] :
         array<pair<string, CfgSubscription>, 4>{{{"configuration", configuration_},
                                                  {"notifications", notifications_},
                                                  {"operational", operational_},
                                                  {"rpc", rpc_}}}) {
        result->set(str, subscription.toElement());
    }
    return result;
}

// *************************** PARSERS ***********************************

// *********************** ControlSocketConfigParser  *************************

CfgControlSocketPtr ControlSocketConfigParser::parse(ElementPtr const& ctrl_sock_config) {
    CfgControlSocketPtr result;
    string type_str = getString(ctrl_sock_config, "socket-type");
    string name = getString(ctrl_sock_config, "socket-name");
    string url_str = getString(ctrl_sock_config, "socket-url");
    ElementPtr user_context = ctrl_sock_config->get("user-context");

    // Type must be valid.
    CfgControlSocket::Type type;
    try {
        type = CfgControlSocket::stringToType(type_str);
    } catch (const exception& ex) {
        isc_throw(ConfigError, ex.what() << " '" << type_str << "' ("
                                         << getPosition("socket-type", ctrl_sock_config) << ")");
    }

    // Url must be valid.
    Url url(url_str);
    if (!url.isValid()) {
        isc_throw(ConfigError, "invalid control socket url: "
                                   << url.getErrorMessage() << " '" << url_str << "' ("
                                   << getPosition("socket-url", ctrl_sock_config) << ")");
    }

    // Create the control socket.
    try {
        result.reset(new CfgControlSocket(type, name, url));
    } catch (const exception& ex) {
        isc_throw(ConfigError, ex.what() << " (" << ctrl_sock_config->getPosition() << ")");
    }

    // Add user-context.
    if (user_context) {
        result->setContext(user_context);
    }

    return result;
}

// *********************** ModelConfigParser  *************************

CfgModelPtr ModelConfigParser::parse(ElementPtr const& model_config) {
    try {
        return make_shared<CfgModel>(model_config->get("model")->stringValue(),
                                     CfgSubscription(model_config->get("configuration")),
                                     CfgSubscription(model_config->get("notifications")),
                                     CfgSubscription(model_config->get("operational")),
                                     CfgSubscription(model_config->get("rpc")),
                                     model_config->get("user-context"));
    } catch (const exception& ex) {
        isc_throw(ConfigError, ex.what() << " (" << model_config->getPosition() << ")");
    }
}

}  // namespace netconf
}  // namespace isc

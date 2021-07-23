// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef NETCONF_CONFIG_H
#define NETCONF_CONFIG_H

#include <cc/cfg_to_element.h>
#include <cc/data.h>
#include <cc/simple_parser.h>
#include <cc/user_context.h>
#include <exceptions/exceptions.h>
#include <http/url.h>
#include <util/dhcp.h>

#include <boost/foreach.hpp>

#include <stdint.h>
#include <string>

namespace isc {
namespace netconf {

/// @file netconf_config.h
/// @brief A collection of classes for housing and parsing the application
/// configuration necessary for the Netconf application.
///
/// @note NetconfConfig is not here: this file contains component of
/// this class but not the class itself.
///
/// This file contains the class declarations for the class hierarchy created
/// from the Netconf configuration and the parser classes used to create it.
/// The application configuration consists of a list of models.
///
/// The parsing class hierarchy reflects this same scheme.  Working top down:
///
/// A ServerMapParser handles the model map invoking a
/// ModelConfigParser to parse each server.
///
/// A ModelConfigParser handles the scalars which belong to the server as well
/// as creating and invoking a CtrlSocketParser to parse its control socket.
///
/// A CtrlSocketParser handles the scalars which belong to the control socket.

/// @brief Represents a Control Socket.
///
/// Acts as a storage class containing the basic attributes which
/// describe a Control Socket.
class CfgControlSocket : public isc::data::UserContext, isc::data::CfgToElement {
public:
    /// @brief Defines the list of possible control socket types.
    enum Type {
        HTTP,  //< HTTP socket
        KEA_CONFIG_TOOL,  //< kea-config-tool
        STDOUT,  //< standard output
        UNIX,  //< Unix socket
    };

    /// @brief Constructor.
    ///
    /// @param type The socket type.
    /// @param name The Unix socket name.
    /// @param url The HTTP server URL.
    CfgControlSocket(Type type, const std::string& name, const isc::http::Url& url)
        : type_(type), name_(name), url_(url) {
    }

    /// @brief Destructor (doing nothing).
    virtual ~CfgControlSocket() = default;

    /// @brief Getter which returns the socket type.
    ///
    /// @return returns the socket type as a CfgControlSocket::Type.
    Type getType() const {
        return type_;
    }

    /// @brief Getter which returns the Unix socket name.
    ///
    /// @return returns the Unix socket name as a std::string.
    const std::string getName() const {
        return name_;
    }

    /// @brief Getter which returns the HTTP server URL.
    ///
    /// @return returns the HTTP server URL as an isc::http::Url.
    const isc::http::Url getUrl() const {
        return url_;
    }

    /// @brief Converts socket type name to CfgControlSocket::Type.
    ///
    /// @param type The type name.
    /// Currently supported values are "unix", "http" and "stdout".
    ///
    /// @return The CfgControlSocket::Type corresponding to the type name.
    /// @throw BadValue if the type name isn't recognized.
    static Type stringToType(const std::string& type);

    /// @brief Converts CfgControlSocket::Type to string.
    ///
    /// @param type The CfgControlSocket::Type type.
    /// @return The type name corresponding to the enumeration element.
    static const std::string typeToString(CfgControlSocket::Type type);

    /// @brief Unparse a configuration object
    ///
    /// @return a pointer to a configuration
    virtual isc::data::ElementPtr toElement() const;

private:
    /// @brief The socket type.
    Type type_;

    /// @brief The UNIX socket name.
    const std::string name_;

    /// @brief The HTTP server URL.
    const isc::http::Url url_;
};

/// @brief Defines a pointer for CfgControlSocket instances.
using CfgControlSocketPtr = std::shared_ptr<CfgControlSocket>;

/// @brief Parser for CfgControlSocket.
///
/// This class parses the configuration element "control-socket"
/// and creates an instance of a CfgControlSocket.
struct ControlSocketConfigParser : public isc::data::SimpleParser {
    /// @brief Performs the actual parsing of the given "control-socket" element.
    ///
    /// Parses a configuration for the elements needed to instantiate a
    /// CfgControlSocket, validates those entries, creates a CfgControlSocket
    /// instance.
    ///
    /// @param ctrl_sock_config is the "control-socket" configuration to parse.
    ///
    /// @return pointer to the new CfgControlSocket instance.
    CfgControlSocketPtr parse(isc::data::ElementPtr const& ctrl_sock_config);
};

struct CfgSubscription {
    CfgSubscription(isc::data::ElementPtr const& config) {
        if (config) {
            isc::data::ElementPtr const& control_socket(config->get("control-socket"));
            if (control_socket) {
                ControlSocketConfigParser parser;
                control_socket_ = parser.parse(control_socket);
            }
            on_boot_update_socket_ = config->get("on-boot-update-socket")->boolValue();
            on_boot_update_sysrepo_ = config->get("on-boot-update-sysrepo")->boolValue();
            subscribe_ = config->get("subscribe")->boolValue();
            validate_ = config->get("validate")->boolValue();
        }
    }

    isc::data::ElementPtr toElement() {
        isc::data::ElementPtr const& result(isc::data::Element::createMap());
        result->checkAndSet("control-socket", control_socket_->toElement());
        result->set("on-boot-update-socket", on_boot_update_socket_);
        result->set("on-boot-update-socket", on_boot_update_sysrepo_);
        result->set("subscribe", subscribe_);
        result->set("validate", validate_);
        return result;
    }

    CfgControlSocketPtr control_socket_;
    bool on_boot_update_socket_;
    bool on_boot_update_sysrepo_;
    bool subscribe_;
    bool validate_;
};

/// @brief Represents a managed CfgModel.
///
/// Acts as a storage class containing the basic attributes and
/// the Control Socket which describe a managed CfgModel.
class CfgModel : isc::data::UserContext, isc::data::CfgToElement {
public:
    /// @brief Constructor.
    ///
    /// @param model The model name.
    /// @param ctrl_sock The control socket.
    CfgModel(std::string const& model,
             CfgSubscription const& configuration,
             CfgSubscription const& notifications,
             CfgSubscription const& operational,
             CfgSubscription const& rpc,
             isc::data::ElementPtr const& user_context)
        : model_(model), dhcp_space_(determineDhcpSpace()), configuration_(configuration),
          notifications_(notifications), operational_(operational), rpc_(rpc) {
        if (user_context) {
            setContext(user_context);
        }
    }

    /// @brief Destructor (doing nothing).
    virtual ~CfgModel() = default;

    /// @brief Getter which returns the model name.
    ///
    /// @return returns the model name as a std::string
    std::string const& getModel() const {
        return model_;
    }

    CfgSubscription const& getConfiguration() const {
        return configuration_;
    }

    CfgSubscription const& getNotifications() const {
        return notifications_;
    }

    CfgSubscription const& getOperational() const {
        return operational_;
    }

    CfgSubscription const& getRpc() const {
        return rpc_;
    }

    /// @brief Returns a DhcpSpaceType enum from the server model name
    isc::dhcp::DhcpSpaceType getDhcpSpace() const {
        return dhcp_space_;
    }

    /// @brief Unparse a configuration object
    ///
    /// @return a pointer to a configuration
    virtual isc::data::ElementPtr toElement() const;

private:
    isc::dhcp::DhcpSpaceType determineDhcpSpace() const;

    /// @brief The model name.
    std::string const model_;

    isc::dhcp::DhcpSpaceType const dhcp_space_;

    CfgSubscription configuration_;

    CfgSubscription notifications_;

    CfgSubscription operational_;

    CfgSubscription rpc_;
};

/// @brief Defines a pointer for CfgModel instances.
using CfgModelPtr = std::shared_ptr<CfgModel>;

/// @brief Defines a map of CfgModels, keyed by the name.
using CfgModelsList = std::vector<CfgModelPtr>;

/// @brief Defines a pointer to map of CfgModels.
using CfgModelsListPtr = std::shared_ptr<CfgModelsList>;

/// @brief Parser for CfgModel.
///
/// This class parses the configuration value from the "models" map
/// and creates an instance of a CfgModel.
class ModelConfigParser : public isc::data::SimpleParser {
public:
    /// @brief Performs the actual parsing of the given value from
    /// the "models" map.
    ///
    /// Parses a configuration for the elements needed to instantiate a
    /// CfgModel, validates those entries, creates a CfgModel instance.
    ///
    /// @param model_config is the value from the "models" map to parse.
    /// @return pointer to the new CfgModel instance.
    CfgModelPtr parse(isc::data::ElementPtr const& model_config);
};

}  // namespace netconf
}  // namespace isc

#endif  // NETCONF_CONFIG_H

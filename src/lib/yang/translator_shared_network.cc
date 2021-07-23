// Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <sstream>
#include <yang/adaptor.h>
#include <yang/translator_shared_network.h>
#include <yang/yang_models.h>

using namespace std;
using namespace isc::data;
using namespace sysrepo;

namespace isc {
namespace yang {

TranslatorSharedNetwork::TranslatorSharedNetwork(S_Session session, const string& model)
    : TranslatorBasic(session, model), TranslatorOptionData(session, model),
      TranslatorOptionDataList(session, model), TranslatorPool(session, model),
      TranslatorPools(session, model), TranslatorPdPool(session, model),
      TranslatorPdPools(session, model), TranslatorHost(session, model),
      TranslatorHosts(session, model), TranslatorSubnet(session, model),
      TranslatorSubnets(session, model) {
}

ElementPtr TranslatorSharedNetwork::getSharedNetwork(const string& xpath) {
    try {
        if ((model_ == KEA_DHCP4_SERVER)) {
            return (getSharedNetworkKea(xpath, "subnet4"));
        } else if ((model_ == KEA_DHCP6_SERVER)) {
            return (getSharedNetworkKea(xpath, "subnet6"));
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError,
                  "sysrepo error getting shared network at '" << xpath << "': " << ex.what());
    }
    isc_throw(NotImplemented, "getSharedNetwork not implemented for the model: " << model_);
}

ElementPtr TranslatorSharedNetwork::getSharedNetworkKea(const string& xpath,
                                                        const std::string& subsel) {
    ElementPtr result = Element::createMap();
    ElementPtr name = getItem(xpath + "/name");
    if (!name) {
        // Can't happen as the name is the key.
        isc_throw(Unexpected, "getSharedNetworkKea requires name: " << xpath);
    }
    result->set("name", name);
    ElementPtr subnets = getSubnets(xpath);
    if (subnets && (subnets->size() > 0)) {
        result->set(subsel, subnets);
    }
    if (subsel == "subnet6") {
        ElementPtr preferred = getItem(xpath + "/preferred-lifetime");
        if (preferred) {
            result->set("preferred-lifetime", preferred);
        }
        ElementPtr min_pref = getItem(xpath + "/min-preferred-lifetime");
        if (min_pref) {
            result->set("min-preferred-lifetime", min_pref);
        }
        ElementPtr max_pref = getItem(xpath + "/max-preferred-lifetime");
        if (max_pref) {
            result->set("max-preferred-lifetime", max_pref);
        }
    }
    ElementPtr valid = getItem(xpath + "/valid-lifetime");
    if (valid) {
        result->set("valid-lifetime", valid);
    }
    ElementPtr min_valid = getItem(xpath + "/min-valid-lifetime");
    if (min_valid) {
        result->set("min-valid-lifetime", min_valid);
    }
    ElementPtr max_valid = getItem(xpath + "/max-valid-lifetime");
    if (max_valid) {
        result->set("max-valid-lifetime", max_valid);
    }
    ElementPtr renew = getItem(xpath + "/renew-timer");
    if (renew) {
        result->set("renew-timer", renew);
    }
    ElementPtr rebind = getItem(xpath + "/rebind-timer");
    if (rebind) {
        result->set("rebind-timer", rebind);
    }
    ElementPtr calculate = getItem(xpath + "/calculate-tee-times");
    if (calculate) {
        result->set("calculate-tee-times", calculate);
    }
    ElementPtr t1_percent = getItem(xpath + "/t1-percent");
    if (t1_percent) {
        result->set("t1-percent", t1_percent);
    }
    ElementPtr t2_percent = getItem(xpath + "/t2-percent");
    if (t2_percent) {
        result->set("t2-percent", t2_percent);
    }
    ElementPtr options = getOptionDataList(xpath);
    if (options && (options->size() > 0)) {
        result->set("option-data", options);
    }
    ElementPtr interface = getItem(xpath + "/interface");
    if (interface) {
        result->set("interface", interface);
    }
    if (subsel == "subnet6") {
        ElementPtr interface_id = getItem(xpath + "/interface-id");
        if (interface_id) {
            result->set("interface-id", interface_id);
        }
        ElementPtr rapid_commit = getItem(xpath + "/rapid-commit");
        if (rapid_commit) {
            result->set("rapid-commit", rapid_commit);
        }
    }
    ElementPtr guard = getItem(xpath + "/client-class");
    if (guard) {
        result->set("client-class", guard);
    }
    ElementPtr required = getItems(xpath + "/require-client-classes");
    if (required && (required->size() > 0)) {
        result->set("require-client-classes", required);
    }
    ElementPtr mode = getItem(xpath + "/reservation-mode");
    if (mode) {
        result->set("reservation-mode", mode);
    }
    ElementPtr relay = getItems(xpath + "/relay/ip-addresses");
    if (relay && (relay->size() > 0)) {
        ElementPtr relay_map = Element::createMap();
        relay_map->set("ip-addresses", relay);
        result->set("relay", relay_map);
    }
    if (subsel == "subnet4") {
        ElementPtr match = getItem(xpath + "/match-client-id");
        if (match) {
            result->set("match-client-id", match);
        }
        ElementPtr auth = getItem(xpath + "/authoritative");
        if (auth) {
            result->set("authoritative", auth);
        }
        ElementPtr next = getItem(xpath + "/next-server");
        if (next) {
            result->set("next-server", next);
        }
        ElementPtr hostname = getItem(xpath + "/server-hostname");
        if (hostname) {
            result->set("server-hostname", hostname);
        }
        ElementPtr boot = getItem(xpath + "/boot-file-name");
        if (boot) {
            result->set("boot-file-name", boot);
        }
    }
    ElementPtr context = getItem(xpath + "/user-context");
    if (context) {
        result->set("user-context", Element::fromJSON(context->stringValue()));
    }
    return (result);
}

void TranslatorSharedNetwork::setSharedNetwork(const string& xpath, ElementPtr elem) {
    try {
        if ((model_ == KEA_DHCP4_SERVER)) {
            setSharedNetworkKea(xpath, elem, "subnet4");
        } else if ((model_ == KEA_DHCP6_SERVER)) {
            setSharedNetworkKea(xpath, elem, "subnet6");
        } else {
            isc_throw(NotImplemented, "setSharedNetwork not implemented for the model: " << model_);
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error setting shared network '"
                                    << elem->str() << "' at '" << xpath << "': " << ex.what());
    }
}

void TranslatorSharedNetwork::setSharedNetworkKea(const string& xpath,
                                                  ElementPtr elem,
                                                  const std::string& subsel) {
    // Skip name which is the key.
    ElementPtr subnets = elem->get(subsel);
    if (subnets && (subnets->size() > 0)) {
        setSubnets(xpath, subnets);
    }
    if (subsel == "subnet6") {
        ElementPtr preferred = elem->get("preferred-lifetime");
        if (preferred) {
            setItem(xpath + "/preferred-lifetime", preferred, SR_UINT32_T);
        }
        ElementPtr min_pref = elem->get("min-preferred-lifetime");
        if (min_pref) {
            setItem(xpath + "/min-preferred-lifetime", min_pref, SR_UINT32_T);
        }
        ElementPtr max_pref = elem->get("max-preferred-lifetime");
        if (max_pref) {
            setItem(xpath + "/max-preferred-lifetime", max_pref, SR_UINT32_T);
        }
    }
    ElementPtr valid = elem->get("valid-lifetime");
    if (valid) {
        setItem(xpath + "/valid-lifetime", valid, SR_UINT32_T);
    }
    ElementPtr min_valid = elem->get("min-valid-lifetime");
    if (min_valid) {
        setItem(xpath + "/min-valid-lifetime", min_valid, SR_UINT32_T);
    }
    ElementPtr max_valid = elem->get("max-valid-lifetime");
    if (max_valid) {
        setItem(xpath + "/max-valid-lifetime", max_valid, SR_UINT32_T);
    }
    ElementPtr renew = elem->get("renew-timer");
    if (renew) {
        setItem(xpath + "/renew-timer", renew, SR_UINT32_T);
    }
    ElementPtr rebind = elem->get("rebind-timer");
    if (rebind) {
        setItem(xpath + "/rebind-timer", rebind, SR_UINT32_T);
    }
    ElementPtr calculate = elem->get("calculate-tee-times");
    if (calculate) {
        setItem(xpath + "/calculate-tee-times", calculate, SR_BOOL_T);
    }
    ElementPtr t1_percent = elem->get("t1-percent");
    if (t1_percent) {
        setItem(xpath + "/t1-percent", t1_percent, SR_DECIMAL64_T);
    }
    ElementPtr t2_percent = elem->get("t2-percent");
    if (t2_percent) {
        setItem(xpath + "/t2-percent", t2_percent, SR_DECIMAL64_T);
    }
    ElementPtr options = elem->get("option-data");
    if (options && (options->size() > 0)) {
        setOptionDataList(xpath, options);
    }
    ElementPtr interface = elem->get("interface");
    if (interface) {
        setItem(xpath + "/interface", interface, SR_STRING_T);
    }
    if (subsel == "subnet6") {
        ElementPtr interface_id = elem->get("interface-id");
        if (interface_id) {
            setItem(xpath + "/interface-id", interface_id, SR_STRING_T);
        }
        ElementPtr rapid_commit = elem->get("rapid-commit");
        if (rapid_commit) {
            setItem(xpath + "/rapid-commit", rapid_commit, SR_BOOL_T);
        }
    }
    ElementPtr guard = elem->get("client-class");
    if (guard) {
        setItem(xpath + "/client-class", guard, SR_STRING_T);
    }
    ElementPtr required = elem->get("require-client-classes");
    if (required && (required->size() > 0)) {
        for (ElementPtr rclass : required->listValue()) {
            setItem(xpath + "/require-client-classes", rclass, SR_STRING_T);
        }
    }
    ElementPtr mode = elem->get("reservation-mode");
    if (mode) {
        setItem(xpath + "/reservation-mode", mode, SR_ENUM_T);
    }
    ElementPtr relay = elem->get("relay");
    if (relay) {
        ElementPtr address = relay->get("ip-address");
        ElementPtr addresses = relay->get("ip-addresses");
        if (address) {
            setItem(xpath + "/relay/ip-addresses", address, SR_STRING_T);
        } else if (addresses && (addresses->size() > 0)) {
            for (ElementPtr addr : addresses->listValue()) {
                setItem(xpath + "/relay/ip-addresses", addr, SR_STRING_T);
            }
        }
    }
    if (subsel == "subnet4") {
        ElementPtr match = elem->get("match-client-id");
        if (match) {
            setItem(xpath + "/match-client-id", match, SR_BOOL_T);
        }
        ElementPtr auth = elem->get("authoritative");
        if (auth) {
            setItem(xpath + "/authoritative", auth, SR_BOOL_T);
        }
        ElementPtr next = elem->get("next-server");
        if (next) {
            setItem(xpath + "/next-server", next, SR_STRING_T);
        }
        ElementPtr hostname = elem->get("server-hostname");
        if (hostname) {
            setItem(xpath + "/server-hostname", hostname, SR_STRING_T);
        }
        ElementPtr boot = elem->get("boot-file-name");
        if (boot) {
            setItem(xpath + "/boot-file-name", boot, SR_STRING_T);
        }
    }
    ElementPtr context = Adaptor::getContext(elem);
    if (context) {
        ElementPtr repr = Element::create(context->str());
        setItem(xpath + "/user-context", repr, SR_STRING_T);
    }
}

TranslatorSharedNetworks::TranslatorSharedNetworks(S_Session session, const string& model)
    : TranslatorBasic(session, model), TranslatorOptionData(session, model),
      TranslatorOptionDataList(session, model), TranslatorPool(session, model),
      TranslatorPools(session, model), TranslatorPdPool(session, model),
      TranslatorPdPools(session, model), TranslatorHost(session, model),
      TranslatorHosts(session, model), TranslatorSubnet(session, model),
      TranslatorSubnets(session, model), TranslatorSharedNetwork(session, model) {
}

ElementPtr TranslatorSharedNetworks::getSharedNetworks(const string& xpath) {
    return getList<TranslatorSharedNetwork>(xpath + "/shared-network", *this,
                                            &TranslatorSharedNetwork::getSharedNetwork);
}

void TranslatorSharedNetworks::setSharedNetworks(const string& xpath, ElementPtr elem) {
    try {
        if ((model_ == KEA_DHCP4_SERVER) || (model_ == KEA_DHCP6_SERVER)) {
            setSharedNetworksKea(xpath, elem);
        } else {
            isc_throw(NotImplemented,
                      "setSharedNetworks not implemented for the model: " << model_);
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error setting shared networks '"
                                    << elem->str() << "' at '" << xpath << "': " << ex.what());
    }
}

void TranslatorSharedNetworks::setSharedNetworksKea(const string& xpath, ElementPtr elem) {
    for (size_t i = 0; i < elem->size(); ++i) {
        ElementPtr network = elem->get(i);
        if (!network->contains("name")) {
            isc_throw(BadValue, "setSharedNetworksKea requires name: " << network->str());
        }
        string name = network->get("name")->stringValue();
        ostringstream key;
        key << xpath << "/shared-network[name='" << name << "']";
        setSharedNetwork(key.str(), network);
    }
}

}  // namespace yang
}  // namespace isc

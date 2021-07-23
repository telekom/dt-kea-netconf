// Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <dhcpsrv/parsers/simple_parser4.h>
#include <dhcpsrv/parsers/simple_parser6.h>
#include <util/defer.h>
#include <yang/adaptor_config.h>
#include <yang/translator_config.h>
#include <yang/translator_universal.h>

#include <sstream>

using namespace std;
using namespace isc::data;
using namespace sysrepo;

using isc::dhcp::SimpleParser4;
using isc::dhcp::SimpleParser6;

namespace isc {
namespace yang {

TranslatorConfig::TranslatorConfig(S_Session session, const string& model)
    : TranslatorBasic(session, model), TranslatorDatabase(session, model),
      TranslatorDatabases(session, model), TranslatorOptionData(session, model),
      TranslatorOptionDataList(session, model), TranslatorOptionDef(session, model),
      TranslatorOptionDefList(session, model), TranslatorClass(session, model),
      TranslatorClasses(session, model), TranslatorPool(session, model),
      TranslatorPools(session, model), TranslatorPdPool(session, model),
      TranslatorPdPools(session, model), TranslatorHost(session, model),
      TranslatorHosts(session, model), TranslatorSubnet(session, model),
      TranslatorSubnets(session, model), TranslatorSharedNetwork(session, model),
      TranslatorSharedNetworks(session, model), TranslatorLogger(session, model),
      TranslatorLoggers(session, model) {
}

ElementPtr TranslatorConfig::getConfig() {
    try {
        if (model_ == IETF_DHCPV6_SERVER) {
            return (getConfigIetf6());
        } else if (model_ == KEA_DHCP4_SERVER) {
            return (getConfigKea4("/kea-dhcp4-server:config"));
        } else if (model_ == KEA_DHCP6_SERVER) {
            return (getConfigKea6("/kea-dhcp6-server:config"));
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error getting config: " << ex.what());
    }
    isc_throw(NotImplemented, "getConfig not implemented for the model: " << model_);
}

ElementPtr TranslatorConfig::getConfigIetf6() {
    ElementPtr result = Element::createMap();
    ElementPtr dhcp6 = Element::createMap();
    result->set("Dhcp6", dhcp6);
    string xpath = "/" + model_ + ":server/server-config";
    ElementPtr ranges = getSubnets(xpath + "/network-ranges");
    if (ranges && !ranges->empty()) {
        dhcp6->set("subnet6", ranges);
    }
    // Skip everything else.
    return (result);
}

ElementPtr TranslatorConfig::getConfigKea4(string const& xpath) {
    ElementPtr result = Element::createMap();
    ElementPtr dhcp = getServerKeaDhcp4(xpath + "/Dhcp4");
    result->set("Dhcp4", dhcp);
    return (result);
}

ElementPtr TranslatorConfig::getConfigKea6(string const& xpath) {
    ElementPtr result = Element::createMap();
    ElementPtr dhcp = getServerKeaDhcp6(xpath + "/Dhcp6");
    result->set("Dhcp6", dhcp);
    return (result);
}

ElementPtr TranslatorConfig::getHooksKea(string const& xpath) {
    return getList(xpath + "/hooks-libraries", *this, &TranslatorConfig::getHook);
}

ElementPtr TranslatorConfig::getHook(string const& xpath) {
    ElementPtr const& hook_library(Element::createMap());
    ElementPtr const& name(getItem(xpath + "/library"));
    if (name) {
        hook_library->set("library", name);
        for (string const& p : {"lawful-interception-parameters", "policy-engine-parameters",
                                "sign-up-portal-parameters", "parameters"}) {
            ElementPtr const& parameters(getHooksParameters(xpath + "/" + p, p));
            if (parameters && !parameters->empty()) {
                hook_library->set(p, parameters);
                break;
            }
        }
    }
    return hook_library;
}

ElementPtr TranslatorConfig::getHooksParameters(string const& xpath, string const& parameters_key) {
    if (parameters_key == "lawful-interception-parameters") {
        return getHooksParametersForLawfulInterception(xpath);
    } else if (parameters_key == "policy-engine-parameters") {
        return getHooksParametersForPolicyEngine(xpath);
    } else if (parameters_key == "sign-up-portal-parameters") {
        return getHooksParametersForSignUpPortal(xpath);
    }
    return getItem(xpath + "/" + parameters_key);
}

ElementPtr TranslatorConfig::getHooksParametersForLawfulInterception(string const& xpath) {
    ElementPtr parameters(Element::createMap());
    checkAndGet(parameters, xpath, "operations-database", *static_cast<TranslatorDatabase*>(this),
                &TranslatorDatabase::getDatabase);
    checkAndGetLeaf(parameters, xpath, "lawful-interception-classes");
    return parameters;
}

ElementPtr TranslatorConfig::getHooksParametersForPolicyEngine(string const& xpath) {
    ElementPtr const& parameters(Element::createMap());
    parameters->set("config", Element::createMap());
    ElementPtr const& config(parameters->get("config"));
    config->set("classification-group", Element::createMap());
    ElementPtr const& classification_group(config->get("classification-group"));
    string const& xxpath(xpath + "/config");
    string const& classification_group_xpath(xxpath + "/classification-group");
    checkAndGetLeafList(classification_group, classification_group_xpath, "encoded-options");
    checkAndGetList(classification_group, classification_group_xpath, "groups", *this,
                    &TranslatorConfig::getGroup);
    checkAndGetLeaf(classification_group, classification_group_xpath, "tag");

    config->set("ia_na-synthesis", Element::createMap());
    ElementPtr const& iana_synthesis(config->get("ia_na-synthesis"));
    string const& iana_synthesis_xpath(xxpath + "/ia_na-synthesis");
    checkAndGetLeaf(iana_synthesis, iana_synthesis_xpath, "enable-filtering");
    checkAndGetList(iana_synthesis, iana_synthesis_xpath, "client-classes",
                    *static_cast<TranslatorClass*>(this), &TranslatorClass::getClass);

    checkAndGetLeafList(config, xxpath, "library-classes");
    checkAndGetLeaf(config, xxpath, "logging");
    checkAndGet(config, xxpath, "config-database", *static_cast<TranslatorDatabase*>(this),
                &TranslatorDatabase::getDatabase);
    ElementPtr network_topology(Element::createMap());
    config->set("network-topology", network_topology);
    checkAndGetList(network_topology, xxpath + "/network-topology", "zones", *this,
                    &TranslatorConfig::getZone);
    checkAndGetList(network_topology, xxpath + "/network-topology", "subnets", *this,
                    &TranslatorConfig::getSubnetsNT);
    return parameters;
}

ElementPtr TranslatorConfig::getGroup(string const& xpath) {
    ElementPtr group(Element::createMap());
    checkAndGetLeaf(group, xpath, "class");
    checkAndGetLeaf(group, xpath, "tag");
    return group;
}

ElementPtr TranslatorConfig::getZone(string const& xpath) {
    ElementPtr zone(Element::createMap());
    zone->set("address-format-group", Element::createMap());
    ElementPtr address_format_group(zone->get("address-format-group"));
    checkAndGetList(address_format_group, xpath + "/address-format-group", "groups", *this,
                    &TranslatorConfig::getAddressFormatGroup);
    zone->set("allocation-group", Element::createMap());
    ElementPtr allocation_group(zone->get("allocation-group"));
    string const& xxpath(xpath + "/allocation-group");
    checkAndGetList(allocation_group, xxpath, "groups", *this,
                    &TranslatorConfig::getAllocationGroup);
    checkAndGetLeaf(allocation_group, xxpath, "mask");
    checkAndGetLeaf(zone, xpath, "id");
    return zone;
}

ElementPtr TranslatorConfig::getAddressFormatGroup(string const& xpath) {
    ElementPtr groups(Element::createMap());
    checkAndGetLeaf(groups, xpath, "mask");
    checkAndGetLeaf(groups, xpath, "type");
    return groups;
}

ElementPtr TranslatorConfig::getAllocationGroup(string const& xpath) {
    ElementPtr groups(Element::createMap());
    checkAndGetLeaf(groups, xpath, "allocation");
    checkAndGetLeaf(groups, xpath, "bits");
    checkAndGetLeaf(groups, xpath, "signature");
    checkAndGetLeaf(groups, xpath, "tag");
    checkAndGetLeaf(groups, xpath, "type");
    checkAndGetLeaf(groups, xpath, "mask");
    checkAndGetLeaf(groups, xpath, "prefix-len");
    return groups;
}

ElementPtr TranslatorConfig::getSubnetsNT(string const& xpath) {
    ElementPtr subnets(Element::createMap());
    checkAndGetLeaf(subnets, xpath, "subnet");
    checkAndGetList(subnets, xpath, "network-topology", *this,
                    &TranslatorConfig::getNetworkTopology);
    return subnets;
}

ElementPtr TranslatorConfig::getNetworkTopology(string const& xpath) {
    ElementPtr network_topology(Element::createMap());
    checkAndGetLeaf(network_topology, xpath, "zone");
    checkAndGetLeaf(network_topology, xpath, "router");
    checkAndGetLeaf(network_topology, xpath, "port");
    checkAndGetLeaf(network_topology, xpath, "user-port");
    return network_topology;
}

ElementPtr TranslatorConfig::getHooksParametersForSignUpPortal(string const& xpath) {
    return getHooksParametersForPolicyEngine(xpath);
}

ElementPtr TranslatorConfig::getExpiredKea(const string& xpath) {
    ElementPtr expired = Element::createMap();

    checkAndGetLeaf(expired, xpath, "reclaim-timer-wait-time");
    checkAndGetLeaf(expired, xpath, "flush-reclaimed-timer-wait-time");
    checkAndGetLeaf(expired, xpath, "hold-reclaimed-time");
    checkAndGetLeaf(expired, xpath, "max-reclaim-leases");
    checkAndGetLeaf(expired, xpath, "max-reclaim-time");
    checkAndGetLeaf(expired, xpath, "unwarned-reclaim-cycles");

    if (!expired->empty()) {
        return (expired);
    }

    return (ElementPtr());
}

ElementPtr TranslatorConfig::getDdnsKea(const string& xpath) {
    ElementPtr ddns = Element::createMap();
    checkAndGetLeaf(ddns, xpath, "enable-updates");
    checkAndGetLeaf(ddns, xpath, "qualifying-suffix");
    checkAndGetLeaf(ddns, xpath, "server-ip");
    checkAndGetLeaf(ddns, xpath, "server-port");
    checkAndGetLeaf(ddns, xpath, "sender-ip");
    checkAndGetLeaf(ddns, xpath, "sender-port");
    checkAndGetLeaf(ddns, xpath, "max-queue-size");
    checkAndGetLeaf(ddns, xpath, "ncr-protocol");
    checkAndGetLeaf(ddns, xpath, "ncr-format");
    checkAndGetLeaf(ddns, xpath, "override-no-update");
    checkAndGetLeaf(ddns, xpath, "override-client-update");
    checkAndGetLeaf(ddns, xpath, "replace-client-name");
    checkAndGetLeaf(ddns, xpath, "generated-prefix");
    checkAndGetLeaf(ddns, xpath, "hostname-char-set");
    checkAndGetLeaf(ddns, xpath, "hostname-char-replacement");

    ElementPtr context = getItem(xpath + "/user-context");
    if (context) {
        ddns->set("user-context", Element::fromJSON(context->stringValue()));
    }

    if (!ddns->empty()) {
        // If there's something to return, use it.
        return (ddns);
    }

    // If not, return null.
    return (ElementPtr());
}

ElementPtr TranslatorConfig::getDhcpQueueControl(string const& xpath) {
    ElementPtr dhcp_queue_control = Element::createMap();

    checkAndGetLeaf(dhcp_queue_control, xpath, "capacity");
    checkAndGetLeaf(dhcp_queue_control, xpath, "enable-queue");
    checkAndGetLeaf(dhcp_queue_control, xpath, "queue-type");

    if (!dhcp_queue_control->empty()) {
        return dhcp_queue_control;
    }

    return ElementPtr();
}

ElementPtr TranslatorConfig::getConfigControlKea(const string& xpath) {
    ElementPtr config_ctrl = Element::createMap();
    checkAndGetLeaf(config_ctrl, xpath, "config-fetch-wait-time");
    ElementPtr databases = getDatabases(xpath + "/config-database");
    if (databases && !databases->empty()) {
        config_ctrl->set("config-databases", databases);
    }
    if (!config_ctrl->empty()) {
        // If there's something to return, use it.
        return (config_ctrl);
    }

    // If not, return null.
    return (ElementPtr());
}

ElementPtr TranslatorConfig::getServerKeaDhcpCommon(const string& xpath) {
    ElementPtr result = Element::createMap();

    checkAndGetLeaf(result, xpath, "valid-lifetime");
    checkAndGetLeaf(result, xpath, "min-valid-lifetime");
    checkAndGetLeaf(result, xpath, "max-valid-lifetime");
    checkAndGetLeaf(result, xpath, "renew-timer");
    checkAndGetLeaf(result, xpath, "rebind-timer");
    checkAndGetLeaf(result, xpath, "calculate-tee-times");
    checkAndGetLeaf(result, xpath, "t1-percent");
    checkAndGetLeaf(result, xpath, "t2-percent");
    checkAndGetLeaf(result, xpath, "decline-probation-period");
    checkAndGetLeaf(result, xpath, "hostname-char-set");
    checkAndGetLeaf(result, xpath, "hostname-char-replacement");

    ElementPtr networks = getSharedNetworks(xpath);
    if (networks && !networks->empty()) {
        result->set("shared-networks", networks);
    }
    ElementPtr classes = getClasses(xpath);
    if (classes && !classes->empty()) {
        result->set("client-classes", classes);
    }
    ElementPtr database = getDatabase(xpath + "/lease-database");
    if (database) {
        result->set("lease-database", database);
    }
    ElementPtr databases = getDatabases(xpath + "/hosts-database");
    if (databases && !databases->empty()) {
        result->set("hosts-databases", databases);
    }
    ElementPtr host_ids = getItems(xpath + "/host-reservation-identifiers");
    if (host_ids) {
        result->set("host-reservation-identifiers", host_ids);
    }
    ElementPtr defs = getOptionDefList(xpath);
    if (defs && !defs->empty()) {
        result->set("option-def", defs);
    }
    ElementPtr options = getOptionDataList(xpath);
    if (options && !options->empty()) {
        result->set("option-data", options);
    }
    ElementPtr hooks = getHooksKea(xpath);
    if (hooks && !hooks->empty()) {
        result->set("hooks-libraries", hooks);
    }
    ElementPtr expired = getExpiredKea(xpath + "/expired-leases-processing");
    if (expired) {
        result->set("expired-leases-processing", expired);
    }
    checkAndGetLeaf(result, xpath, "dhcp4o6-port");
    ElementPtr ddns = getDdnsKea(xpath + "/dhcp-ddns");
    if (ddns) {
        result->set("dhcp-ddns", ddns);
    }
    ElementPtr context = getItem(xpath + "/user-context");
    if (context) {
        result->set("user-context", Element::fromJSON(context->stringValue()));
    }
    ElementPtr checks = getItem(xpath + "/sanity-checks/lease-checks");
    if (checks) {
        ElementPtr sanity = Element::createMap();
        sanity->set("lease-checks", checks);
        result->set("sanity-checks", sanity);
    }
    checkAndGetLeaf(result, xpath, "reservation-mode");
    ElementPtr hosts = getHosts(xpath);
    if (hosts && !hosts->empty()) {
        result->set("reservations", hosts);
    }
    ElementPtr config_ctrl = getConfigControlKea(xpath + "/config-control");
    if (config_ctrl) {
        result->set("config-control", config_ctrl);
    }
    checkAndGetLeaf(result, xpath, "server-tag");
    ElementPtr queue_ctrl = getDhcpQueueControl(xpath + "/dhcp-queue-control");
    if (queue_ctrl) {
        result->set("dhcp-queue-control", queue_ctrl);
    }
    ElementPtr loggers = getLoggers(xpath);
    if (loggers && !loggers->empty()) {
        result->set("loggers", loggers);
    }
    checkAndGetLeaf(result, xpath, "multi-threading/enable-multi-threading");
    checkAndGetLeaf(result, xpath, "multi-threading/packet-queue-size");
    checkAndGetLeaf(result, xpath, "multi-threading/thread-pool-size");
    checkAndGetLeaf(result, xpath, "privacy-history-size");
    checkAndGetLeaf(result, xpath, "privacy-valid-lifetime");
    return (result);
}

ElementPtr TranslatorConfig::getInterfacesConfig(string const& xpath) {
    ElementPtr if_config = Element::createMap();
    ElementPtr ifs = getItems(xpath + "/interfaces-config/interfaces");
    if (ifs && !ifs->empty()) {
        if_config->set("interfaces", ifs);
    }
    if ((model_ == KEA_DHCP4_SERVER)) {
        checkAndGetLeaf(if_config, xpath + "/interfaces-config", "dhcp-socket-type");
        checkAndGetLeaf(if_config, xpath + "/interfaces-config", "outbound-interface");
    }
    checkAndGetLeaf(if_config, xpath + "/interfaces-config", "re-detect");
    ElementPtr context = getItem(xpath + "/interfaces-config/user-context");
    if (context) {
        if_config->set("user-context", Element::fromJSON(context->stringValue()));
    }
    return if_config;
}

ElementPtr TranslatorConfig::getServerKeaDhcp4(string xpath) {
    ElementPtr result = getServerKeaDhcpCommon(xpath);
    // Handle subnets.
    ElementPtr subnets = getSubnets(xpath);
    if (subnets && !subnets->empty()) {
        result->set("subnet4", subnets);
    }
    // Handle interfaces.
    ElementPtr if_config = getInterfacesConfig(xpath);
    if (!if_config->empty()) {
        result->set("interfaces-config", if_config);
    }
    // Handle DHCPv4 specific global parameters.
    checkAndGetLeaf(result, xpath, "echo-client-id");
    checkAndGetLeaf(result, xpath, "match-client-id");
    checkAndGetLeaf(result, xpath, "next-server");
    checkAndGetLeaf(result, xpath, "server-hostname");
    checkAndGetLeaf(result, xpath, "boot-file-name");
    checkAndGetLeaf(result, xpath, "authoritative");
    SimpleParser4::setAllDefaults(result);
    return (result);
}

ElementPtr TranslatorConfig::getServerKeaDhcp6(string xpath) {
    ElementPtr result = getServerKeaDhcpCommon(xpath);
    // Handle DHCPv6 specific global parameters.
    checkAndGetLeaf(result, xpath, "data-directory");
    checkAndGetLeaf(result, xpath, "preferred-lifetime");
    checkAndGetLeaf(result, xpath, "min-preferred-lifetime");
    checkAndGetLeaf(result, xpath, "max-preferred-lifetime");
    // Handle subnets.
    ElementPtr subnets = getSubnets(xpath);
    if (subnets && !subnets->empty()) {
        result->set("subnet6", subnets);
    }
    // Handle interfaces.
    ElementPtr if_config = getInterfacesConfig(xpath);
    if (!if_config->empty()) {
        result->set("interfaces-config", if_config);
    }
    // Handle DHCPv6 specific global entries.
    ElementPtr relay = getItems(xpath + "/relay-supplied-options");
    if (relay) {
        result->set("relay-supplied-options", relay);
    }
    ElementPtr macs = getItems(xpath + "/mac-sources");
    if (macs) {
        result->set("mac-sources", macs);
    }
    // Handle server-id.
    // @todo: move to a DUID translator.
    ElementPtr server_id = Element::createMap();
    checkAndGetLeaf(server_id, xpath + "/server-id", "type");
    checkAndGetLeaf(server_id, xpath + "/server-id", "identifier");
    checkAndGetLeaf(server_id, xpath + "/server-id", "time");
    checkAndGetLeaf(server_id, xpath + "/server-id", "htype");
    checkAndGetLeaf(server_id, xpath + "/server-id", "enterprise-id");
    checkAndGetLeaf(server_id, xpath + "/server-id", "persist");
    ElementPtr context = getItem(xpath + "/server-id/user-context");
    if (context) {
        server_id->set("user-context", Element::fromJSON(context->stringValue()));
    }
    if (!server_id->empty()) {
        result->set("server-id", server_id);
    }
    SimpleParser6::setAllDefaults(result);
    return (result);
}

ElementPtr TranslatorConfig::getMasterConfig(string const& xpath) {
    ElementPtr result(Element::createMap());

    // .["instance-id"]
    checkAndGetLeaf(result, xpath, "instance-id");

    // .["server-config"]
    ElementPtr server_config;
    if (server_config && !server_config->empty()) {
        result->set("server-config", server_config);
    }

    if (result->empty()) {
        result.reset();
    }

    return result;
}

ElementPtr TranslatorConfig::getShard(string const& xpath) {
    ElementPtr result = Element::createMap();

    // .name
    checkAndGetLeaf(result, xpath, "name");

    // .["config-database"]
    ElementPtr config_database = getDatabase(xpath + "/config-database");
    if (config_database) {
        result->set("config-database", config_database);
    }

    // .["admin-credentials"]
    ElementPtr admin_credentials = getDatabase(xpath + "/admin-credentials");
    if (admin_credentials) {
        result->set("admin-credentials", admin_credentials);
    }

    // .["master-config"]
    ElementPtr master_config =
        getList(xpath + "/master-config", *this, &TranslatorConfig::getMasterConfig);
    if (master_config) {
        result->set("master-config", master_config);
    }

    // .["shard-config"]
    ElementPtr shard_config;
    if (shard_config) {
        result->set("shard-config", shard_config);
    }

    if (result->empty()) {
        result.reset();
    }

    return result;
}

ElementPtr TranslatorConfig::getMaster(string const& xpath) {
    ElementPtr result = Element::createMap();

    // .name
    checkAndGetLeaf(result, xpath, "name");

    // .["admin-credentials"]
    ElementPtr admin_credentials = getDatabase(xpath + "/admin-credentials");
    if (admin_credentials) {
        result->set("admin-credentials", admin_credentials);
    }

    // .shards[]
    ElementPtr shards = getList(xpath + "/shards", *this, &TranslatorConfig::getShard);
    if (shards) {
        result->set("shards", shards);
    }

    if (result->empty()) {
        result.reset();
    }

    return result;
}

ElementPtr TranslatorConfig::getDistributed(string const& xpath) {
    ElementPtr result = Element::createMap();

    checkAndGetLeaf(result, xpath, "skip-validation");

    // .masters
    ElementPtr masters = getList(xpath + "/masters", *this, &TranslatorConfig::getMaster);
    if (masters) {
        result->set("masters", masters);
    }

    if (result->empty()) {
        result.reset();
    }

    return result;
}

void TranslatorConfig::setConfig(ElementPtr const& elem) {
    try {
        if (model_ == IETF_DHCPV6_SERVER) {
            if (elem) {
                AdaptorConfig::preProcess6(elem);
                setConfigIetf6(elem);
            } else {
                delConfigIetf6();
            }
        } else if (model_ == KEA_DHCP4_SERVER) {
            if (elem) {
                AdaptorConfig::preProcess4(elem);
                setConfigKea4("/kea-dhcp4-server:config", elem);
            } else {
                delConfigKea("/kea-dhcp4-server:config");
            }
        } else if (model_ == KEA_DHCP6_SERVER) {
            if (elem) {
                AdaptorConfig::preProcess6(elem);
                setConfigKea6("/kea-dhcp6-server:config", elem);
            } else {
                delConfigKea("/kea-dhcp6-server:config");
            }
        } else {
            isc_throw(NotImplemented, "setConfig not implemented for the model: " << model_);
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError,
                  "sysrepo error setting config '" << elem->str() << "': " << ex.what());
    }
}

void TranslatorConfig::delConfigIetf6() {
    delItem("/" + model_ + ":server");
}

void TranslatorConfig::setConfigIetf6(ElementPtr const& elem) {
    string xpath = "/" + model_ + ":server/server-config";
    ElementPtr dhcp6 = elem->get("Dhcp6");
    if (!dhcp6) {
        isc_throw(BadValue, "no Dhcp6 entry in " << elem->str());
    }
    ElementPtr ranges = dhcp6->get("subnet6");
    if (ranges && !ranges->empty()) {
        setSubnets(xpath + "/network-ranges", ranges);
    }
    // Skip everything else.
}

void TranslatorConfig::delConfigKea(const string& xpath) {
    delItem(xpath);
}

void TranslatorConfig::setConfigKea4(const string& xpath, ElementPtr const& elem) {
    ElementPtr dhcp = elem->get("Dhcp4");
    if (dhcp) {
        setServerKeaDhcp4(xpath + "/Dhcp4", dhcp);
    }
}

void TranslatorConfig::setConfigKea6(const string& xpath, ElementPtr const& elem) {
    ElementPtr dhcp = elem->get("Dhcp6");
    if (dhcp) {
        setServerKeaDhcp6(xpath + "/Dhcp6", dhcp);
    }
}

void TranslatorConfig::setServerKeaDhcpCommon(const string& xpath, ElementPtr const& elem) {
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
    ElementPtr period = elem->get("decline-probation-period");
    if (period) {
        setItem(xpath + "/decline-probation-period", period, SR_UINT32_T);
    }
    ElementPtr networks = elem->get("shared-networks");
    if (networks) {
        setSharedNetworks(xpath, networks);
    }
    ElementPtr classes = elem->get("client-classes");
    if (classes && !classes->empty()) {
        setClasses(xpath, classes);
    }
    ElementPtr database = elem->get("lease-database");
    if (database) {
        setDatabase(xpath + "/lease-database", database);
    }
    ElementPtr databases = elem->get("hosts-databases");
    if (databases && !databases->empty()) {
        setDatabases(xpath + "/hosts-database", databases);
    } else {
        // Reuse of database from lease-database.
        database = elem->get("hosts-database");
        if (database) {
            ElementPtr list = Element::createList();
            list->add(copy(database));
            setDatabases(xpath + "/hosts-database", list);
        }
    }
    ElementPtr host_ids = elem->get("host-reservation-identifiers");
    if (host_ids) {
        for (ElementPtr id : host_ids->listValue()) {
            setItem(xpath + "/host-reservation-identifiers", id, SR_ENUM_T);
        }
    }
    ElementPtr defs = elem->get("option-def");
    if (defs && !defs->empty()) {
        setOptionDefList(xpath, defs);
    }
    ElementPtr options = elem->get("option-data");
    if (options && !options->empty()) {
        setOptionDataList(xpath, options);
    }
    ElementPtr hook_libs = elem->get("hooks-libraries");
    if (hook_libs) {
        for (ElementPtr lib : hook_libs->listValue()) {
            ElementPtr name = lib->get("library");
            if (!name) {
                continue;
            }
            string const& hook_library(xpath + "/hooks-libraries[library='" + name->stringValue() +
                                       "']");
            ElementPtr parameters;
            for (string const& p : {"lawful-interception-parameters", "policy-engine-parameters",
                                    "sign-up-portal-parameters", "parameters"}) {
                parameters = lib->get(p);
                if (parameters && !parameters->empty()) {
                    setHooksParameters(hook_library, p, parameters);
                    break;
                }
            }
            if (!parameters) {
                ElementPtr list = Element::createList();
                setItem(hook_library + "/parameters", list, SR_LIST_T);
            }
        }
    }
    ElementPtr expired = elem->get("expired-leases-processing");
    if (expired) {
        ElementPtr reclaim = expired->get("reclaim-timer-wait-time");
        if (reclaim) {
            setItem(xpath + "/expired-leases-processing/reclaim-timer-wait-time", reclaim,
                    SR_UINT32_T);
        }
        ElementPtr flush = expired->get("flush-reclaimed-timer-wait-time");
        if (flush) {
            setItem(xpath + "/expired-leases-processing/flush-reclaimed-timer-wait-time", flush,
                    SR_UINT32_T);
        }
        ElementPtr hold = expired->get("hold-reclaimed-time");
        if (hold) {
            setItem(xpath + "/expired-leases-processing/hold-reclaimed-time", hold, SR_UINT32_T);
        }
        ElementPtr max_leases = expired->get("max-reclaim-leases");
        if (max_leases) {
            setItem(xpath + "/expired-leases-processing/max-reclaim-leases", max_leases,
                    SR_UINT32_T);
        }
        ElementPtr max_time = expired->get("max-reclaim-time");
        if (max_time) {
            setItem(xpath + "/expired-leases-processing/max-reclaim-time", max_time, SR_UINT32_T);
        }
        ElementPtr unwarned = expired->get("unwarned-reclaim-cycles");
        if (unwarned) {
            setItem(xpath + "/expired-leases-processing/unwarned-reclaim-cycles", unwarned,
                    SR_UINT32_T);
        }
    }
    ElementPtr port = elem->get("dhcp4o6-port");
    if (port) {
        setItem(xpath + "/dhcp4o6-port", port, SR_UINT16_T);
    }
    ElementPtr char_set = elem->get("hostname-char-set");
    if (char_set) {
        setItem(xpath + "/hostname-char-set", char_set, SR_STRING_T);
    }
    ElementPtr char_repl = elem->get("hostname-char-replacement");
    if (char_repl) {
        setItem(xpath + "/hostname-char-replacement", char_repl, SR_STRING_T);
    }
    ElementPtr ddns = elem->get("dhcp-ddns");
    if (ddns) {
        ElementPtr enable = ddns->get("enable-updates");
        if (enable) {
            setItem(xpath + "/dhcp-ddns/enable-updates", enable, SR_BOOL_T);
        }
        ElementPtr suffix = ddns->get("qualifying-suffix");
        if (suffix) {
            setItem(xpath + "/dhcp-ddns/qualifying-suffix", suffix, SR_STRING_T);
        }
        ElementPtr server_ip = ddns->get("server-ip");
        if (server_ip) {
            setItem(xpath + "/dhcp-ddns/server-ip", server_ip, SR_STRING_T);
        }
        ElementPtr server_port = ddns->get("server-port");
        if (server_port) {
            setItem(xpath + "/dhcp-ddns/server-port", server_port, SR_UINT16_T);
        }
        ElementPtr sender_ip = ddns->get("sender-ip");
        if (sender_ip) {
            setItem(xpath + "/dhcp-ddns/sender-ip", sender_ip, SR_STRING_T);
        }
        ElementPtr sender_port = ddns->get("sender-port");
        if (sender_port) {
            setItem(xpath + "/dhcp-ddns/sender-port", sender_port, SR_UINT16_T);
        }
        ElementPtr queue = ddns->get("max-queue-size");
        if (queue) {
            setItem(xpath + "/dhcp-ddns/max-queue-size", queue, SR_UINT32_T);
        }
        ElementPtr protocol = ddns->get("ncr-protocol");
        if (protocol) {
            setItem(xpath + "/dhcp-ddns/ncr-protocol", protocol, SR_ENUM_T);
        }
        ElementPtr format = ddns->get("ncr-format");
        if (format) {
            setItem(xpath + "/dhcp-ddns/ncr-format", format, SR_ENUM_T);
        }
        ElementPtr no_up = ddns->get("override-no-update");
        if (no_up) {
            setItem(xpath + "/dhcp-ddns/override-no-update", no_up, SR_BOOL_T);
        }
        ElementPtr client = ddns->get("override-client-update");
        if (client) {
            setItem(xpath + "/dhcp-ddns/override-client-update", client, SR_BOOL_T);
        }
        ElementPtr replace = ddns->get("replace-client-name");
        if (replace) {
            setItem(xpath + "/dhcp-ddns/replace-client-name", replace, SR_ENUM_T);
        }
        ElementPtr generated = ddns->get("generated-prefix");
        if (generated) {
            setItem(xpath + "/dhcp-ddns/generated-prefix", generated, SR_STRING_T);
        }
        ElementPtr char_set = ddns->get("hostname-char-set");
        if (char_set) {
            setItem(xpath + "/dhcp-ddns/hostname-char-set", char_set, SR_STRING_T);
        }
        ElementPtr char_repl = ddns->get("hostname-char-replacement");
        if (char_repl) {
            setItem(xpath + "/dhcp-ddns/hostname-char-replacement", char_repl, SR_STRING_T);
        }
        ElementPtr context = Adaptor::getContext(ddns);
        if (context) {
            ElementPtr repr = Element::create(context->str());
            setItem(xpath + "/dhcp-ddns/user-context", repr, SR_STRING_T);
        }
    }
    ElementPtr context = Adaptor::getContext(elem);
    if (context) {
        ElementPtr repr = Element::create(context->str());
        setItem(xpath + "/user-context", repr, SR_STRING_T);
    }
    ElementPtr sanity = elem->get("sanity-checks");
    if (sanity) {
        ElementPtr checks = sanity->get("lease-checks");
        if (checks) {
            setItem(xpath + "/sanity-checks/lease-checks", checks, SR_ENUM_T);
        }
    }
    ElementPtr hr_mode = elem->get("reservation-mode");
    if (hr_mode) {
        setItem(xpath + "/reservation-mode", hr_mode, SR_ENUM_T);
    }
    ElementPtr hosts = elem->get("reservations");
    if (hosts && !hosts->empty()) {
        setHosts(xpath, hosts);
    }
    ElementPtr config_ctrl = elem->get("config-control");
    if (config_ctrl && !config_ctrl->empty()) {
        ElementPtr cfwt = config_ctrl->get("config-fetch-wait-time");
        if (cfwt) {
            setItem(xpath + "/config-control/config-fetch-wait-time", cfwt, SR_UINT32_T);
        }
        databases = config_ctrl->get("config-databases");
        if (databases && !databases->empty()) {
            setDatabases(xpath + "/config-control/config-database", databases);
        }
    }
    ElementPtr server_tag = elem->get("server-tag");
    if (server_tag) {
        setItem(xpath + "/server-tag", server_tag, SR_STRING_T);
    }
    ElementPtr dhcp_queue_control = elem->get("dhcp-queue-control");
    if (dhcp_queue_control) {
        setDhcpQueueControl(xpath + "/dhcp-queue-control", dhcp_queue_control);
    }
    checkAndSetLeaf(elem, xpath, "multi-threading/enable-multi-threading", SR_BOOL_T);
    checkAndSetLeaf(elem, xpath, "multi-threading/packet-queue-size", SR_UINT32_T);
    checkAndSetLeaf(elem, xpath, "multi-threading/thread-pool-size", SR_UINT32_T);
    checkAndSetLeaf(elem, xpath, "privacy-history-size", SR_UINT32_T);
    checkAndSetLeaf(elem, xpath, "privacy-valid-lifetime", SR_UINT32_T);
}

void TranslatorConfig::setInterfacesConfig(const string& xpath, ElementPtr const& elem) {
    ElementPtr ifs(elem->get("interfaces"));
    if (ifs && !ifs->empty()) {
        for (ElementPtr intf : ifs->listValue()) {
            setItem(xpath + "/interfaces", intf, SR_STRING_T);
        }
    }
    if ((model_ == KEA_DHCP4_SERVER)) {
        checkAndSetLeaf(elem, xpath, "dhcp-socket-type", SR_ENUM_T);
        checkAndSetLeaf(elem, xpath, "outbound-interface", SR_ENUM_T);
    }
    checkAndSetLeaf(elem, xpath, "re-detect", SR_BOOL_T);
    ElementPtr context = Adaptor::getContext(elem);
    if (context) {
        setItem(xpath + "/user-context", Element::create(context->str()), SR_STRING_T);
    }
}

void TranslatorConfig::setServerKeaDhcp4(const string& xpath, ElementPtr const& elem) {
    SimpleParser4::setAllDefaults(elem);
    setServerKeaDhcpCommon(xpath, elem);
    ElementPtr subnets = elem->get("subnet4");
    if (subnets) {
        setSubnets(xpath, subnets);
    }
    ElementPtr if_config = elem->get("interfaces-config");
    if (if_config) {
        setInterfacesConfig(xpath + "/interfaces-config", if_config);
    }
    ElementPtr echo = elem->get("echo-client-id");
    if (echo) {
        setItem(xpath + "/echo-client-id", echo, SR_BOOL_T);
    }
    ElementPtr match = elem->get("match-client-id");
    if (match) {
        setItem(xpath + "/match-client-id", match, SR_BOOL_T);
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
    ElementPtr auth = elem->get("authoritative");
    if (auth) {
        setItem(xpath + "/authoritative", auth, SR_BOOL_T);
    }
}

void TranslatorConfig::setServerKeaDhcp6(const string& xpath, ElementPtr const& elem) {
    SimpleParser6::setAllDefaults(elem);
    setServerKeaDhcpCommon(xpath, elem);
    ElementPtr data_dir = elem->get("data-directory");
    if (data_dir) {
        setItem(xpath + "/data-directory", data_dir, SR_STRING_T);
    }
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
    ElementPtr subnets = elem->get("subnet6");
    if (subnets) {
        setSubnets(xpath, subnets);
    }
    ElementPtr if_config = elem->get("interfaces-config");
    if (if_config) {
        setInterfacesConfig(xpath + "/interfaces-config", if_config);
    }
    ElementPtr relay = elem->get("relay-supplied-options");
    if (relay) {
        for (ElementPtr addr : relay->listValue()) {
            setItem(xpath + "/relay-supplied-options", addr, SR_STRING_T);
        }
    }
    ElementPtr macs = elem->get("mac-sources");
    if (macs) {
        for (ElementPtr source : macs->listValue()) {
            setItem(xpath + "/mac-sources", source, SR_STRING_T);
        }
    }
    ElementPtr server_id = elem->get("server-id");
    if (server_id) {
        ElementPtr id_type = server_id->get("type");
        if (id_type) {
            setItem(xpath + "/server-id/type", id_type, SR_ENUM_T);
        }
        ElementPtr id_id = server_id->get("identifier");
        if (id_id) {
            setItem(xpath + "/server-id/identifier", id_id, SR_STRING_T);
        }
        ElementPtr id_time = server_id->get("time");
        if (id_time) {
            setItem(xpath + "/server-id/time", id_time, SR_UINT32_T);
        }
        ElementPtr id_htype = server_id->get("htype");
        if (id_htype) {
            setItem(xpath + "/server-id/htype", id_htype, SR_UINT16_T);
        }
        ElementPtr id_ent_id = server_id->get("enterprise-id");
        if (id_ent_id) {
            setItem(xpath + "/server-id/enterprise-id", id_ent_id, SR_UINT32_T);
        }
        ElementPtr id_persist = server_id->get("persist");
        if (id_persist) {
            setItem(xpath + "/server-id/persist", id_persist, SR_BOOL_T);
        }
        ElementPtr context = Adaptor::getContext(server_id);
        if (context) {
            ElementPtr repr = Element::create(context->str());
            setItem(xpath + "/server-id/user-context", repr, SR_STRING_T);
        }
    }
}

void TranslatorConfig::setDhcpQueueControl(string const& xpath, ElementPtr const& elem) {
    try {
        if ((model_ == KEA_DHCP4_SERVER) || (model_ == KEA_DHCP6_SERVER)) {
            setDhcpQueueControlKea(xpath, elem);
        } else {
            isc_throw(NotImplemented,
                      "setDhcpQueueControl() not implemented for the model: " << model_);
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error setting dhcp-queue-control '"
                                    << elem->str() << "' at '" << xpath << "': " << ex.what());
    }
}

void TranslatorConfig::setDhcpQueueControlKea(string const& xpath, ElementPtr const& elem) {
    checkAndSetLeaf(elem, xpath, "capacity", SR_UINT32_T);
    checkAndSetLeaf(elem, xpath, "enable-queue", SR_BOOL_T);
    checkAndSetLeaf(elem, xpath, "queue-type", SR_STRING_T);
}

void TranslatorConfig::setHooksParameters(string const& xpath,
                                          string const& parameters_key,
                                          ElementPtr const& parameters) {
    if (parameters_key == "lawful-interception-parameters") {
        setHooksParametersForLawfulInterception(xpath + "/" + parameters_key, parameters);
    } else if (parameters_key == "policy-engine-parameters") {
        setHooksParametersForPolicyEngine(xpath + "/" + parameters_key, parameters);
    } else if (parameters_key == "sign-up-portal-parameters") {
        setHooksParametersForSignUpPortal(xpath + "/" + parameters_key, parameters);
    } else if (parameters_key == "parameters") {
        setItem(xpath + "/" + parameters_key, parameters, SR_STRING_T);
    }
}

void TranslatorConfig::setHooksParametersForLawfulInterception(string const& xpath,
                                                               ElementPtr const& parameters) {
    ElementPtr config(parameters->get("config"));
    if (!config) {
        return;
    }
    string xxpath(xpath + "/config");

    checkAndSet(config, xxpath, "operations-database", *static_cast<TranslatorDatabase*>(this),
                &TranslatorDatabase::setDatabase);
    checkAndSetLeaf(config, xxpath, "lawful-interception-classes", SR_STRING_T);
}

void TranslatorConfig::setHooksParametersForPolicyEngine(string const& xpath,
                                                         ElementPtr const& parameters) {
    ElementPtr const& config(parameters->get("config"));
    if (!config) {
        return;
    }
    string xxpath(xpath + "/config");

    ElementPtr const& classification_group(config->get("classification-group"));
    if (classification_group) {
        string const classification_group_xpath(xxpath + "/classification-group");
        checkAndSetLeafList(classification_group, classification_group_xpath, "encoded-options",
                            SR_UINT16_T);
        checkAndSetList(classification_group, classification_group_xpath, "groups", "class", *this,
                        &TranslatorConfig::setGroup);
        checkAndSetLeaf(classification_group, classification_group_xpath, "tag", SR_STRING_T);
    }
    checkAndSet(config, xxpath, "config-database", *static_cast<TranslatorDatabase*>(this),
                &TranslatorDatabase::setDatabase);

    ElementPtr const& iana_synthesis(config->get("ia_na-synthesis"));
    if (iana_synthesis) {
        std::string const& iana_synthesis_xpath(xxpath + "/ia_na-synthesis");
        checkAndSetLeaf(config, iana_synthesis_xpath, "enable-filtering", SR_BOOL_T);
        checkAndSetList(iana_synthesis, iana_synthesis_xpath, "client-classes", "name",
                        *static_cast<TranslatorClass*>(this), &TranslatorClass::setClassKea);
    }

    checkAndSetLeafList(config, xxpath, "library-classes", SR_STRING_T);
    checkAndSetLeaf(config, xxpath, "logging", SR_STRING_T);

    ElementPtr const& network_topology(config->get("network-topology"));
    if (!network_topology) {
        return;
    }

    checkAndSetList(network_topology, xxpath + "/network-topology", "zones", "id", *this,
                    &TranslatorConfig::setZone);
    checkAndSetList(network_topology, xxpath + "/network-topology", "subnets", "subnet", *this,
                    &TranslatorConfig::setSubnetNT);
}

void TranslatorConfig::setHooksParametersForSignUpPortal(string const& xpath,
                                                         ElementPtr const& parameters) {
    setHooksParametersForPolicyEngine(xpath, parameters);
}

void TranslatorConfig::setGroup(string const& xpath,
                                ElementPtr const& groups,
                                [[maybe_unused]] bool const skip /* = false */) {
    checkAndSetLeaf(groups, xpath, "tag", SR_STRING_T);
}

void TranslatorConfig::setZone(string const& xpath,
                               ElementPtr const& zones,
                               [[maybe_unused]] bool const skip /* = false */) {
    ElementPtr address_format_group(zones->get("address-format-group"));
    if (address_format_group) {
        checkAndSetList(address_format_group, xpath + "/address-format-group", "groups", "mask",
                        *this, &TranslatorConfig::setAddressFormatGroups);
    }
    ElementPtr allocation_group(zones->get("allocation-group"));
    if (allocation_group) {
        string const& xxpath(xpath + "/allocation-group");
        checkAndSetList(allocation_group, xxpath, "groups", "tag", *this,
                        &TranslatorConfig::setAllocationGroups);
        checkAndSetLeaf(allocation_group, xxpath, "mask", SR_STRING_T);
    }
}

void TranslatorConfig::setAddressFormatGroups(string const& xpath,
                                              ElementPtr const& groups,
                                              [[maybe_unused]] bool const skip /* = false */) {
    checkAndSetLeaf(groups, xpath, "type", SR_STRING_T);
}

void TranslatorConfig::setAllocationGroups(string const& xpath,
                                           ElementPtr const& groups,
                                           [[maybe_unused]] bool const skip /* = false */) {
    checkAndSetLeaf(groups, xpath, "allocation", SR_STRING_T);
    checkAndSetLeaf(groups, xpath, "bits", SR_STRING_T);
    checkAndSetLeaf(groups, xpath, "signature", SR_STRING_T);
    checkAndSetLeaf(groups, xpath, "type", SR_STRING_T);
    checkAndSetLeaf(groups, xpath, "prefix-len", SR_INT32_T);
}

void TranslatorConfig::setSubnetNT(string const& xpath,
                                   ElementPtr const& subnets,
                                   [[maybe_unused]] bool const skip /* = false */) {
    checkAndSetList(subnets, xpath, "network-topology", "zone router port user-port", *this,
                    &TranslatorConfig::setNetworkTopology, FORCE_CREATION);
}

void TranslatorConfig::setNetworkTopology([[maybe_unused]] string const& xpath,
                                          [[maybe_unused]] ElementPtr const& network_topology,
                                          [[maybe_unused]] bool const skip /* = false */) {
    // There are no mandatory fields outside the key.
}

}  // namespace yang
}  // namespace isc

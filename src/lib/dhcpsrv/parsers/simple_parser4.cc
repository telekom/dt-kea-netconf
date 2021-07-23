// Copyright (C) 2016-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <dhcpsrv/parsers/simple_parser4.h>
#include <cc/data.h>
#include <boost/foreach.hpp>
#include <iostream>

using namespace isc::data;

namespace isc {
namespace dhcp {
/// @brief This sets of arrays define the default values and
///        values inherited (derived) between various scopes.
///
/// Each of those is documented in @file simple_parser4.cc. This
/// is different than most other comments in Kea code. The reason
/// for placing those in .cc rather than .h file is that it
/// is expected to be one centralized place to look at for
/// the default values. This is expected to be looked at also by
/// people who are not skilled in C or C++, so they may be
/// confused with the differences between declaration and definition.
/// As such, there's one file to look at that hopefully is readable
/// without any C or C++ skills.
///
/// @{

/// @brief This table defines all global parameters in DHCPv4.
///
/// Boolean, integer, real and string types are for scalar parameters,
/// list and map types for entries.
/// Order follows global_param rule in bison grammar.
const SimpleKeywords SimpleParser4::GLOBAL4_PARAMETERS = {
#ifdef TERASTREAM
    { "privacy-history-size",           Element::integer },
    { "privacy-valid-lifetime",         Element::integer },
#endif  // TERASTREAM
    { "valid-lifetime",                 Element::integer },
    { "min-valid-lifetime",             Element::integer },
    { "max-valid-lifetime",             Element::integer },
    { "renew-timer",                    Element::integer },
    { "rebind-timer",                   Element::integer },
    { "decline-probation-period",       Element::integer },
    { "subnet4",                        Element::list },
    { "shared-networks",                Element::list },
    { "interfaces-config",              Element::map },
    { "lease-database",                 Element::map },
    { "hosts-database",                 Element::map },
    { "hosts-databases",                Element::list },
    { "host-reservation-identifiers",   Element::list },
    { "client-classes",                 Element::list },
    { "option-def",                     Element::list },
    { "option-data",                    Element::list },
    { "hooks-libraries",                Element::list },
    { "expired-leases-processing",      Element::map },
    { "dhcp4o6-port",                   Element::integer },
    { "control-socket",                 Element::map },
    { "dhcp-queue-control",             Element::map },
    { "dhcp-ddns",                      Element::map },
    { "echo-client-id",                 Element::boolean },
    { "match-client-id",                Element::boolean },
    { "authoritative",                  Element::boolean },
    { "next-server",                    Element::string },
    { "server-hostname",                Element::string },
    { "boot-file-name",                 Element::string },
    { "user-context",                   Element::map },
    { "comment",                        Element::string },
    { "sanity-checks",                  Element::map },
    { "reservations",                   Element::list },
    { "config-control",                 Element::map },
    { "server-tag",                     Element::string },
    { "reservation-mode",               Element::string },
    { "calculate-tee-times",            Element::boolean },
    { "t1-percent",                     Element::real },
    { "t2-percent",                     Element::real },
    { "loggers",                        Element::list },
    { "hostname-char-set",              Element::string },
    { "hostname-char-replacement",      Element::string },
    { "ddns-send-updates",              Element::boolean },
    { "ddns-override-no-update",        Element::boolean },
    { "ddns-override-client-update",    Element::boolean },
    { "ddns-replace-client-name",       Element::string },
    { "ddns-generated-prefix",          Element::string },
    { "ddns-qualifying-suffix",         Element::string },
    { "store-extended-info",            Element::boolean },
    { "statistic-default-sample-count", Element::integer },
    { "statistic-default-sample-age",   Element::integer },
    { "multi-threading",                Element::map },
};

/// @brief This table defines default global values for DHCPv4
///
/// Some of the global parameters defined in the global scope (i.e. directly
/// in Dhcp4) are optional. If not defined, the following values will be
/// used.
const SimpleDefaults SimpleParser4::GLOBAL4_DEFAULTS = {
#ifdef TERASTREAM
    { "privacy-history-size",           Element::integer, "0" },
    { "privacy-valid-lifetime",         Element::integer, "0" },
#endif  // TERASTREAM
    { "valid-lifetime",                 Element::integer, "7200" },
    { "decline-probation-period",       Element::integer, "86400" }, // 24h
    { "dhcp4o6-port",                   Element::integer, "0" },
    { "echo-client-id",                 Element::boolean, "true" },
    { "match-client-id",                Element::boolean, "true" },
    { "authoritative",                  Element::boolean, "false" },
    { "next-server",                    Element::string,  "0.0.0.0" },
    { "server-hostname",                Element::string,  "" },
    { "boot-file-name",                 Element::string,  "" },
    { "server-tag",                     Element::string,  "" },
    { "reservation-mode",               Element::string,  "all" },
    { "calculate-tee-times",            Element::boolean, "false" },
    { "t1-percent",                     Element::real,    ".50" },
    { "t2-percent",                     Element::real,    ".875" },
    { "ddns-send-updates",              Element::boolean, "true" },
    { "ddns-override-no-update",        Element::boolean, "false" },
    { "ddns-override-client-update",    Element::boolean, "false" },
    { "ddns-replace-client-name",       Element::string,  "never" },
    { "ddns-generated-prefix",          Element::string,  "myhost" },
    { "ddns-qualifying-suffix",         Element::string,  "" },
    { "hostname-char-set",              Element::string,  "[^A-Za-z0-9.-]" },
    { "hostname-char-replacement",      Element::string,  "" },
    { "store-extended-info",            Element::boolean, "false" },
    { "statistic-default-sample-count", Element::integer, "20" },
    { "statistic-default-sample-age",   Element::integer, "0" },
};

/// @brief This table defines all option definition parameters.
///
/// Boolean, integer, real and string types are for scalar parameters,
/// list and map types for entries.
/// Order follows option_def_param rules in bison grammar.
const SimpleKeywords SimpleParser4::OPTION4_DEF_PARAMETERS = {
    { "name",         Element::string },
    { "code",         Element::integer },
    { "type",         Element::string },
    { "record-types", Element::string },
    { "space",        Element::string },
    { "encapsulate",  Element::string },
    { "array",        Element::boolean, },
    { "user-context", Element::map },
    { "comment",      Element::string },
    { "metadata",     Element::map },
};

/// @brief This table defines default values for option definitions in DHCPv4.
///
/// Dhcp4 may contain an array called option-def that enumerates new option
/// definitions. This array lists default values for those option definitions.
const SimpleDefaults SimpleParser4::OPTION4_DEF_DEFAULTS = {
    { "record-types", Element::string,  ""},
    { "space",        Element::string,  "dhcp4"}, // DHCP4_OPTION_SPACE
    { "array",        Element::boolean, "false"},
    { "encapsulate",  Element::string,  "" },
};

/// @brief This table defines all option parameters.
///
/// Boolean, integer, real and string types are for scalar parameters,
/// list and map types for entries.
/// Order follows option_param rules in bison grammar.
const SimpleKeywords SimpleParser4::OPTION4_PARAMETERS = {
    { "name",         Element::string },
    { "data",         Element::string },
    { "code",         Element::integer },
    { "space",        Element::string },
    { "csv-format",   Element::boolean },
    { "always-send",  Element::boolean },
    { "user-context", Element::map },
    { "comment",      Element::string },
    { "metadata",     Element::map },
};

/// @brief This table defines default values for options in DHCPv4.
///
/// Dhcp4 usually contains option values (option-data) defined in global,
/// subnet, class or host reservations scopes. This array lists default values
/// for those option-data declarations.
const SimpleDefaults SimpleParser4::OPTION4_DEFAULTS = {
    { "space",        Element::string,  "dhcp4"}, // DHCP4_OPTION_SPACE
    { "csv-format",   Element::boolean, "true"},
    { "always-send",  Element::boolean, "false"},
};

/// @brief This table defines all subnet parameters for DHCPv4.
///
/// Boolean, integer, real and string types are for scalar parameters,
/// list and map types for entries.
/// Order follows subnet4_param rule in bison grammar.
const SimpleKeywords SimpleParser4::SUBNET4_PARAMETERS = {
    { "allocation-type",                Element::string },
    { "v4-psid-offset",                 Element::integer },
    { "v4-psid-len",                    Element::integer },
    { "valid-lifetime",                 Element::integer },
    { "min-valid-lifetime",             Element::integer },
    { "max-valid-lifetime",             Element::integer },
    { "renew-timer",                    Element::integer },
    { "rebind-timer",                   Element::integer },
    { "option-data",                    Element::list },
    { "pools",                          Element::list },
    { "subnet",                         Element::string },
    { "interface",                      Element::string },
    { "id",                             Element::integer },
    { "client-class",                   Element::string },
    { "require-client-classes",         Element::list },
    { "reservations",                   Element::list },
    { "reservation-mode",               Element::string },
    { "relay",                          Element::map },
    { "match-client-id",                Element::boolean },
    { "authoritative",                  Element::boolean },
    { "next-server",                    Element::string },
    { "server-hostname",                Element::string },
    { "boot-file-name",                 Element::string },
    { "4o6-interface",                  Element::string },
    { "4o6-interface-id",               Element::string },
    { "4o6-subnet",                     Element::string },
    { "user-context",                   Element::map },
    { "comment",                        Element::string },
    { "calculate-tee-times",            Element::boolean },
    { "t1-percent",                     Element::real },
    { "t2-percent",                     Element::real },
    { "ddns-send-updates",              Element::boolean },
    { "ddns-override-no-update",        Element::boolean },
    { "ddns-override-client-update",    Element::boolean },
    { "ddns-replace-client-name",       Element::string },
    { "ddns-generated-prefix",          Element::string },
    { "ddns-qualifying-suffix",         Element::string },
    { "hostname-char-set",              Element::string },
    { "hostname-char-replacement",      Element::string },
    { "store-extended-info",            Element::boolean },
    { "metadata",                       Element::map },
};

/// @brief This table defines default values for each IPv4 subnet.
///
/// Note: When updating this array, please also update SHARED_SUBNET4_DEFAULTS
/// below. In most cases, those two should be kept in sync, except cases
/// where a parameter can be derived from shared-networks, but is not
/// defined on global level. Currently there are two such parameters:
/// interface and reservation-mode
const SimpleDefaults SimpleParser4::SUBNET4_DEFAULTS = {
    { "id",               Element::integer, "0" }, // 0 means autogenerate
    { "interface",        Element::string,  "" },
    { "client-class",     Element::string,  "" },
    { "allocation-type",  Element::string,  "default" },
    { "4o6-interface",    Element::string,  "" },
    { "4o6-interface-id", Element::string,  "" },
    { "4o6-subnet",       Element::string,  "" },
    { "v4-psid-offset",   Element::integer, "0" },
    { "v4-psid-len",      Element::integer, "0" },
};

/// @brief This table defines default values for each IPv4 subnet that is
///        part of a shared network
///
/// This is mostly the same as @ref SUBNET4_DEFAULTS, except two parameters
/// that can be derived from shared-network, but cannot from global scope.
/// Those are: interface and reservation-mode.
const SimpleDefaults SimpleParser4::SHARED_SUBNET4_DEFAULTS = {
    { "id",               Element::integer, "0" }, // 0 means autogenerate
    { "allocation-type",  Element::string,  "default" },
    { "4o6-interface",    Element::string,  "" },
    { "4o6-interface-id", Element::string,  "" },
    { "4o6-subnet",       Element::string,  "" },
    { "v4-psid-offset",   Element::integer, "0" },
    { "v4-psid-len",      Element::integer, "0" },
};

/// @brief List of parameters that can be inherited to subnet4 scope.
///
/// Some parameters may be defined on both global (directly in Dhcp4) and
/// subnet (Dhcp4/subnet4/...) scope. If not defined in the subnet scope,
/// the value is being inherited (derived) from the global scope. This
/// array lists all of such parameters.
///
/// This list is also used for inheriting from global to shared networks
/// and from shared networks to subnets within it.
const ParamsList SimpleParser4::INHERIT_TO_SUBNET4 = {
    "rebind-timer",
    "relay",
    "renew-timer",
    "valid-lifetime",
    "min-valid-lifetime",
    "max-valid-lifetime",
    "calculate-tee-times",
    "t1-percent",
    "t2-percent",
    "store-extended-info",
};

/// @brief This table defines all pool parameters.
///
/// Boolean, integer, real and string types are for scalar parameters,
/// list and map types for entries.
/// Order follows pool_param rules in bison grammar.
const SimpleKeywords SimpleParser4::POOL4_PARAMETERS = {
    { "pool",                    Element::string },
    { "option-data",             Element::list },
    { "client-class",            Element::string },
    { "require-client-classes",  Element::list },
    { "user-context",            Element::map },
    { "comment",                 Element::string },
    { "metadata",                Element::map },
    { "id",                      Element::integer },
    { "max-address-utilization", Element::integer },
};

/// @brief This table defines all shared network parameters for DHCPv4.
///
/// Boolean, integer, real and string types are for scalar parameters,
/// list and map types for entries.
/// Order follows shared_network_param rule in bison grammar.
const SimpleKeywords SimpleParser4::SHARED_NETWORK4_PARAMETERS = {
    { "name",                           Element::string },
    { "subnet4",                        Element::list },
    { "interface",                      Element::string },
    { "renew-timer",                    Element::integer },
    { "rebind-timer",                   Element::integer },
    { "option-data",                    Element::list },
    { "match-client-id",                Element::boolean },
    { "authoritative",                  Element::boolean },
    { "next-server",                    Element::string },
    { "server-hostname",                Element::string },
    { "boot-file-name",                 Element::string },
    { "relay",                          Element::map },
    { "reservation-mode",               Element::string },
    { "client-class",                   Element::string },
    { "require-client-classes",         Element::list },
    { "valid-lifetime",                 Element::integer },
    { "min-valid-lifetime",             Element::integer },
    { "max-valid-lifetime",             Element::integer },
    { "user-context",                   Element::map },
    { "comment",                        Element::string },
    { "calculate-tee-times",            Element::boolean },
    { "t1-percent",                     Element::real },
    { "t2-percent",                     Element::real },
    { "ddns-send-updates",              Element::boolean },
    { "ddns-override-no-update",        Element::boolean },
    { "ddns-override-client-update",    Element::boolean },
    { "ddns-replace-client-name",       Element::string },
    { "ddns-generated-prefix",          Element::string },
    { "ddns-qualifying-suffix",         Element::string },
    { "hostname-char-set",              Element::string },
    { "hostname-char-replacement",      Element::string },
    { "store-extended-info",            Element::boolean },
    { "metadata",                       Element::map },
};

/// @brief This table defines default values for each IPv4 shared network.
const SimpleDefaults SimpleParser4::SHARED_NETWORK4_DEFAULTS = {
    { "client-class",     Element::string, "" },
    { "interface",        Element::string, "" },
};

/// @brief This table defines default values for interfaces for DHCPv4.
const SimpleDefaults SimpleParser4::IFACE4_DEFAULTS = {
    { "re-detect", Element::boolean, "true" },
};

/// @brief This table defines default values for dhcp-queue-control in DHCPv4.
const SimpleDefaults SimpleParser4::DHCP_QUEUE_CONTROL4_DEFAULTS = {
    { "enable-queue",   Element::boolean, "false"},
    { "queue-type",     Element::string,  "kea-ring4"},
    { "capacity",       Element::integer, "64"},
};

/// @brief This table defines default values for multi-threading in DHCPv4.
const SimpleDefaults SimpleParser4::DHCP_MULTI_THREADING4_DEFAULTS = {
    { "enable-multi-threading", Element::boolean, "false" },
    { "thread-pool-size",       Element::integer, "0" },
    { "packet-queue-size",      Element::integer, "64" },
};

/// @brief This defines default values for sanity checking for DHCPv4.
const SimpleDefaults SimpleParser4::SANITY_CHECKS4_DEFAULTS = {
    { "lease-checks", Element::string, "warn" },
};

/// @}

/// ---------------------------------------------------------------------------
/// --- end of default values -------------------------------------------------
/// ---------------------------------------------------------------------------

size_t SimpleParser4::setAllDefaults(ElementPtr global) {
    size_t cnt = 0;

    // Set global defaults first.
    cnt = setDefaults(global, GLOBAL4_DEFAULTS);

    // Now set option definition defaults for each specified option definition
    ElementPtr option_defs = global->get("option-def");
    if (option_defs) {
        BOOST_FOREACH(ElementPtr option_def, option_defs->listValue()) {
            cnt += SimpleParser::setDefaults(option_def, OPTION4_DEF_DEFAULTS);
        }
    }

    // Set the defaults for option data
    ElementPtr options = global->get("option-data");
    if (options) {
        cnt += setListDefaults(options, OPTION4_DEFAULTS);
    }

    // Now set the defaults for defined subnets
    ElementPtr subnets = global->get("subnet4");
    if (subnets) {
        cnt += setListDefaults(subnets, SUBNET4_DEFAULTS);
    }

    // Set the defaults for interfaces config
    ElementPtr ifaces_cfg = global->get("interfaces-config");
    if (ifaces_cfg) {
        cnt += setDefaults(ifaces_cfg, IFACE4_DEFAULTS);
    }

    // Set defaults for shared networks
    ElementPtr shared = global->get("shared-networks");
    if (shared) {
        BOOST_FOREACH(ElementPtr net, shared->listValue()) {

            cnt += setDefaults(net, SHARED_NETWORK4_DEFAULTS);

            ElementPtr subs = net->get("subnet4");
            if (subs) {
                cnt += setListDefaults(subs, SHARED_SUBNET4_DEFAULTS);
            }
        }
    }

    // Set the defaults for dhcp-queue-control.  If the element isn't
    // there we'll add it.
    ElementPtr queue_control = global->get("dhcp-queue-control");
    ElementPtr mutable_cfg;
    if (queue_control) {
        mutable_cfg = queue_control;
    } else {
        mutable_cfg = Element::createMap();
        global->set("dhcp-queue-control", mutable_cfg);
    }

    cnt += setDefaults(mutable_cfg, DHCP_QUEUE_CONTROL4_DEFAULTS);

    // Set the defaults for multi-threading.  If the element isn't there
    // we'll add it.
    ElementPtr multi_threading = global->get("multi-threading");
    if (multi_threading) {
        mutable_cfg = std::const_pointer_cast<Element>(multi_threading);
    } else {
        mutable_cfg = Element::createMap();
        global->set("multi-threading", mutable_cfg);
    }

    cnt += setDefaults(mutable_cfg, DHCP_MULTI_THREADING4_DEFAULTS);

    // Set the defaults for sanity-checks.  If the element isn't
    // there we'll add it.
    ElementPtr sanity_checks = global->get("sanity-checks");
    if (sanity_checks) {
        mutable_cfg = sanity_checks;
    } else {
        mutable_cfg = Element::createMap();
        global->set("sanity-checks", mutable_cfg);
    }

    cnt += setDefaults(mutable_cfg, SANITY_CHECKS4_DEFAULTS);

    return (cnt);
}

size_t SimpleParser4::deriveParameters(ElementPtr global) {
    size_t cnt = 0;

    // Now derive global parameters into subnets.
    ElementPtr subnets = global->get("subnet4");
    if (subnets) {
        BOOST_FOREACH(ElementPtr single_subnet, subnets->listValue()) {
            cnt += SimpleParser::deriveParams(global, single_subnet,
                                              INHERIT_TO_SUBNET4);
        }
    }

    // Deriving parameters for shared networks is a bit more involved.
    // First, the shared-network level derives from global, and then
    // subnets within derive from it.
    ElementPtr shared = global->get("shared-networks");
    if (shared) {
        BOOST_FOREACH(ElementPtr net, shared->listValue()) {
            // First try to inherit the parameters from shared network,
            // if defined there.
            // Then try to inherit them from global.
            cnt += SimpleParser::deriveParams(global, net,
                                              INHERIT_TO_SUBNET4);

            // Now we need to go thrugh all the subnets in this net.
            subnets = net->get("subnet4");
            if (subnets) {
                BOOST_FOREACH(ElementPtr single_subnet, subnets->listValue()) {
                    cnt += SimpleParser::deriveParams(net, single_subnet,
                                                      INHERIT_TO_SUBNET4);
                }
            }
        }
    }

    return (cnt);
}

}  // namespace dhcp
}  // namespace isc

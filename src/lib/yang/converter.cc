// (C) 2020 Deutsche Telekom AG.
//
// Deutsche Telekom AG and all other contributors /
// copyright owners license this file to you under the Apache
// License, Version 2.0 (the "License"); you may not use this
// file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the License for the
// specific language governing permissions and limitations
// under the License.

#include <config.h>

#include <asiolink/io_address.h>
#include <dhcp/duid.h>
#include <dhcp/option6_auth.h>
#include <dhcpsrv/subnet_mgr.h>
#include <util/dhcp.h>
#include <util/encode/hex.h>
#include <util/io_utilities.h>
#include <yang/converter.h>

namespace isc {
namespace yang {

using namespace isc::util;

using isc::asiolink::IOAddress;
using isc::data::Element;
using isc::data::ElementPtr;
using isc::dhcp::Lease;
using isc::dhcp::Option6Auth;
using isc::dhcp::Subnet4;
using isc::log::Spdlog;
using std::string;
using std::to_string;

template <>
void Converter<IETF_to_Kea>::ascertain(isc::data::ElementPtr const& config) {
    if (config->size() > 0 && !config->get("dhcpv6-node-type") && !config->get("dhcpv6-server")) {
        isc_throw(Unexpected,
                  PRETTY_FUNCTION_NAME()
                      << ": dhcpv6-node-type or dhcpv6-server expected as toplevel node");
    }
}

template <>
void Converter<Kea_to_IETF>::ascertain(isc::data::ElementPtr const& config) {
    if (config->size() > 0 && !config->get("Dhcp6")) {
        isc_throw(Unexpected, PRETTY_FUNCTION_NAME() << ": Dhcp6 expected as toplevel node");
    }
}

template <>
Converter<IETF_to_Kea>::option_set_table_t& Converter<IETF_to_Kea>::transformOptionSets() {
    static option_set_table_t _{
        // OPTION_PREFERENCE (7) Preference Option
        {"preference-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "preference");
             option_data->set("code", 7);
             std::string data;
             if (input->contains("pref-value")) {
                 data += std::to_string(input->get("pref-value")->intValue());
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_AUTH (11) Authentication Option
        {"auth-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", false);
             option_data->set("space", "dhcp6");
             option_data->set("name", "auth");
             option_data->set("code", 11);

             std::string auth_info;
             uint8_t proto, algo, method;
             uint64_t replay_detection;
             if (input->contains("protocol")) {
                 proto = input->get("protocol")->intValue();
             } else {
                 isc_throw(BadValue, PRETTY_FUNCTION_NAME()
                                         << ": protocol missing from auth-option");
             }
             if (input->contains("algorithm")) {
                 algo = input->get("algorithm")->intValue();
             } else {
                 isc_throw(BadValue, PRETTY_FUNCTION_NAME()
                                         << ": algorithm missing from auth-option");
             }
             if (input->contains("rdm")) {
                 method = input->get("rdm")->intValue();
             } else {
                 isc_throw(BadValue, PRETTY_FUNCTION_NAME() << ": rdm missing from auth-option");
             }
             if (input->contains("replay-detection")) {
                 replay_detection = input->get("replay-detection")->intValue();
             } else {
                 isc_throw(BadValue, PRETTY_FUNCTION_NAME()
                                         << ": replay-detection missing from auth-option");
             }
             if (input->contains("auth-information")) {
                 auth_info = input->get("auth-information")->stringValue();
             } else {
                 isc_throw(BadValue, PRETTY_FUNCTION_NAME()
                                         << ": auth-information missing from auth-option");
             }

             Option6Auth opt(proto, algo, method, replay_detection,
                             {auth_info.begin(), auth_info.end()});
             option_data->set("data", opt.toHexString());
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_UNICAST (12) Server Unicast Option
        {"server-unicast-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "unicast");
             option_data->set("code", 12);
             std::string data;
             if (input->contains("server-address")) {
                 data += input->get("server-address")->stringValue();
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_STATUS_CODE (13) Status Code Option
        {"status-code-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "status-code");
             option_data->set("code", 13);
             std::string data;
             if (input->contains("status-code")) {
                 data = std::to_string(input->get("status-code")->intValue());
             }
             if (input->contains("status-message")) {
                 if (!data.empty()) {
                     data += ", " + input->get("status-message")->stringValue();
                 } else {
                     data += input->get("status-message")->stringValue();
                 }
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_RAPID_COMMIT (14) Rapid Commit Option
        {"rapid-commit-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "rapid-commit");
             option_data->set("code", 14);
             option_data->set("data", "");
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_VENDOR_OPTS (17) Vendor-specific Option
        {"vendor-specific-information-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             ElementPtr const result(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "vendor-opts");
             option_data->set("code", 17);
             std::string data;
             ElementPtr const list_wrap_data(Element::createList());
             ElementPtr const list_wrap_def(Element::createList());
             if (input->contains("vendor-specific-information-option-instances")) {
                 auto const& instances =
                     input->get("vendor-specific-information-option-instances")->listValue();
                 for (auto const& instance : instances) {
                     int64_t enterprise_no = instance->get("enterprise-number")->intValue();
                     if (data.empty()) {
                         data += std::to_string(enterprise_no);
                     } else {
                         data += ", " + std::to_string(enterprise_no);
                     }
                     if (instance->contains("vendor-option-data")) {
                         for (auto const& vendor_data :
                              instance->get("vendor-option-data")->listValue()) {
                             ElementPtr const cur_vendor_data(Element::createMap());
                             ElementPtr const cur_vendor_def(Element::createMap());
                             cur_vendor_data->set("name",
                                                  "vendor-" + std::to_string(enterprise_no));
                             cur_vendor_data->set("code",
                                                  vendor_data->get("sub-option-code")->intValue());
                             cur_vendor_data->set("data", vendor_data->get("sub-option-data"));
                             cur_vendor_data->set("space", "vendor-opts-space");
                             cur_vendor_def->set("name", "vendor-" + std::to_string(enterprise_no));
                             cur_vendor_def->set("code",
                                                 vendor_data->get("sub-option-code")->intValue());
                             cur_vendor_def->set("array", false);
                             cur_vendor_def->set("space", "vendor-opts-space");
                             cur_vendor_def->set("type", "string");
                             cur_vendor_def->set("encapsulate", "");
                             list_wrap_data->add(cur_vendor_data);
                             list_wrap_def->add(cur_vendor_def);
                         }
                     }
                 }
             }
             option_data->set("data", data);
             list_wrap_data->add(option_data);
             result->set("option-data", list_wrap_data);
             result->set("option-def", list_wrap_def);
             return result;
         }},
        // OPTION_RECONF_MSG (19) Reconfigure Message Option
        {"reconfigure-message-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "reconf-msg");
             option_data->set("code", 19);
             std::string data;
             if (input->contains("msg-type")) {
                 data += std::to_string(input->get("msg-type")->intValue());
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_RECONF_ACCEPT (20) Reconfigure Accept Option
        {"reconfigure-accept-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "reconf-accept");
             option_data->set("code", 20);
             option_data->set("data", "");
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_SIP_SERVER_D (21) SIP Servers Domain Name
        {"ietf-dhcpv6-options-rfc3319:sip-server-domain-name-list-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "sip-server-dns");
             option_data->set("code", 21);
             std::string data;
             if (input->contains("sip-server")) {
                 auto const& sip_servers = input->get("sip-server")->listValue();
                 for (auto const& sip_server : sip_servers) {
                     if (!sip_server->contains("sip-serv-domain-name")) {
                         continue;
                     }
                     if (data.empty()) {
                         data += sip_server->get("sip-serv-domain-name")->stringValue();
                     } else {
                         data += ", " + sip_server->get("sip-serv-domain-name")->stringValue();
                     }
                 }
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_SIP_SERVER_A (22) SIP Servers IPv6 Address List
        {"ietf-dhcpv6-options-rfc3319:sip-server-address-list-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "sip-server-addr");
             option_data->set("code", 22);
             std::string data;
             if (input->contains("sip-server")) {
                 auto const& sip_servers = input->get("sip-server")->listValue();
                 for (auto const& sip_server : sip_servers) {
                     if (!sip_server->contains("sip-serv-addr")) {
                         continue;
                     }
                     if (data.empty()) {
                         data += sip_server->get("sip-serv-addr")->stringValue();
                     } else {
                         data += ", " + sip_server->get("sip-serv-addr")->stringValue();
                     }
                 }
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_DNS_SERVERS (23) DNS recursive Name Server option
        {"ietf-dhcpv6-options-rfc3646:dns-servers-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "dns-servers");
             option_data->set("code", 23);
             std::string data;
             if (input->contains("dns-server")) {
                 auto const& dns_servers = input->get("dns-server")->listValue();
                 for (auto const& dns_server : dns_servers) {
                     if (!dns_server->contains("dns-serv-addr")) {
                         continue;
                     }
                     if (data.empty()) {
                         data += dns_server->get("dns-serv-addr")->stringValue();
                     } else {
                         data += ", " + dns_server->get("dns-serv-addr")->stringValue();
                     }
                 }
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_DOMAIN_LIST (24) Domain Search List Option
        {"ietf-dhcpv6-options-rfc3646:domain-searchlist-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "domain-search");
             option_data->set("code", 24);
             std::string data;
             if (input->contains("domain-searchlist")) {
                 auto const& domains = input->get("domain-searchlist")->listValue();
                 for (auto const& domain : domains) {
                     if (!domain->contains("domain-search-list-entry")) {
                         continue;
                     }
                     if (data.empty()) {
                         data += domain->get("domain-search-list-entry")->stringValue();
                     } else {
                         data += ", " + domain->get("domain-search-list-entry")->stringValue();
                     }
                 }
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_NIS_SERVERS (27) Network Information Service (NIS) Servers Option
        {"ietf-dhcpv6-options-rfc3898:nis-config-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "nis-servers");
             option_data->set("code", 27);
             std::string data;
             if (input->contains("nis-server")) {
                 auto const& niss = input->get("nis-server")->listValue();
                 for (auto const& nis : niss) {
                     if (!nis->contains("nis-serv-addr")) {
                         continue;
                     }
                     if (data.empty()) {
                         data += nis->get("nis-serv-addr")->stringValue();
                     } else {
                         data += ", " + nis->get("nis-serv-addr")->stringValue();
                     }
                 }
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_NISP_SERVERS (28): Network Information Service V2 (NIS+) Servers Option.
        {"ietf-dhcpv6-options-rfc3898:nis-plus-config-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "nisp-servers");
             option_data->set("code", 28);
             std::string data;
             if (input->contains("nis-plus-server")) {
                 auto const& niss = input->get("nis-plus-server")->listValue();
                 for (auto const& nis : niss) {
                     if (!nis->contains("nis-plus-serv-addr")) {
                         continue;
                     }
                     if (data.empty()) {
                         data += nis->get("nis-plus-serv-addr")->stringValue();
                     } else {
                         data += ", " + nis->get("nis-plus-serv-addr")->stringValue();
                     }
                 }
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_NIS_DOMAIN_NAME (29) Network Information Service (NIS) Domain Name Option
        {"ietf-dhcpv6-options-rfc3898:nis-domain-name-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "nis-domain-name");
             option_data->set("code", 29);
             std::string data;
             if (input->contains("nis-domain-name")) {
                 data += input->get("nis-domain-name")->stringValue();
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_NISP_DOMAIN_NAME (30) Network Information Service V2 (NIS+) Domain Name Option
        {"ietf-dhcpv6-options-rfc3898:nis-plus-domain-name-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "nisp-domain-name");
             option_data->set("code", 30);
             std::string data;
             if (input->contains("nis-plus-domain-name")) {
                 data += input->get("nis-plus-domain-name")->stringValue();
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_SNTP_SERVERS (31) Simple Network Time Protocol (SNTP) Servers Option
        {"ietf-dhcpv6-options-rfc4075:sntp-server-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "sntp-servers");
             option_data->set("code", 31);
             std::string data;
             if (input->contains("sntp-server")) {
                 auto const& sntps = input->get("sntp-server")->listValue();
                 for (auto const& sntp : sntps) {
                     if (!sntp->contains("sntp-serv-addr")) {
                         continue;
                     }
                     if (data.empty()) {
                         data += sntp->get("sntp-serv-addr")->stringValue();
                     } else {
                         data += ", " + sntp->get("sntp-serv-addr")->stringValue();
                     }
                 }
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_INFORMATION_REFRESH_TIME (32) Information Refresh Time option
        {"info-refresh-time-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "information-refresh-time");
             option_data->set("code", 32);
             std::string data;
             if (input->contains("info-refresh-time")) {
                 data += std::to_string(input->get("info-refresh-time")->intValue());
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_BCMCS_SERVER_D (33) Broadcast and Multicast Service Controller Domain Name List
        // Option for DHCPv6
        {"ietf-dhcpv6-options-rfc4280:bcmcs-server-d",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "bcmcs-server-dns");
             option_data->set("code", 33);
             std::string data;
             if (input->contains("bcmcs-control-server-domain-names")) {
                 auto const& names = input->get("bcmcs-control-server-domain-names")->listValue();
                 for (auto const& name : names) {
                     if (data.empty()) {
                         data += name->get("bcmcs-control-server-domain-name")->stringValue();
                     } else {
                         data +=
                             ", " + name->get("bcmcs-control-server-domain-name")->stringValue();
                     }
                 }
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_BCMCS_SERVER_A (34) Broadcast and Multicast Service Controller IPv6 Address Option
        // for DHCPv6
        {"ietf-dhcpv6-options-rfc4280:bcmcs-server-a",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "bcmcs-server-addr");
             option_data->set("code", 34);
             std::string data;
             if (input->contains("bcmcs-control-server-addressess")) {
                 auto const& names = input->get("bcmcs-control-server-addressess")->listValue();
                 for (auto const& name : names) {
                     if (data.empty()) {
                         data += name->get("bcmcs-control-server-address")->stringValue();
                     } else {
                         data += ", " + name->get("bcmcs-control-server-address")->stringValue();
                     }
                 }
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
#ifdef TODO_OPTIONS
        // TODO: This doesnt match to KEA option, unclear
        // OPTION_GEOCONF_CIVIC (36) Civic Addresses Configuration Information option
        {"ietf-dhcpv6-options-rfc4776:geoconf-civic-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "geoconf-civic");
             option_data->set("code", 36);
             std::string data;
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // TODO: This doesnt match to KEA option, domain name is missing from the model
        // OPTION_CLIENT_FQDN (39) DHCPv6 Client FQDN Option
        {"ietf-dhcpv6-options-rfc4704:client-fqdn-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", false);
             option_data->set("space", "dhcp6");
             option_data->set("name", "client-fqdn");
             option_data->set("code", 39);
             std::string data;
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
#endif
        // OPTION_NEW_POSIX_TIMEZONE (41) Posix Timezone option
        {"ietf-dhcpv6-options-rfc4833:posix-timezone-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "new-posix-timezone");
             option_data->set("code", 41);
             std::string data;
             if (input->contains("tz-posix")) {
                 data = input->get("tz-posix")->stringValue();
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_NEW_TZDB_TIMEZONE (42) Timezone Database option
        {"ietf-dhcpv6-options-rfc4833:tzdb-timezone-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "new-tzdb-timezone");
             option_data->set("code", 42);
             std::string data;
             if (input->contains("tz-database")) {
                 data = input->get("tz-database")->stringValue();
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
    // This is done internally under pools, as this option is augmented
    // under network-ranges/prefix-pools/prefix-pool
#ifdef false
        // OPTION_PD_EXCLUDE (67) PD Exclude Option
        {"ietf-dhcpv6-options-rfc6603:pd-exclude",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("name", "pd-exclude");
             option_data->set("code", 67);
             std::string data;
             if (input->contains("ipv6-subnet")) {
                 data = input->get("ipv6-subnet")->stringValue();
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
#endif
        // OPTION_SOL_MAX_RT (82) sol max rt option
        {"sol-max-rt-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "solmax-rt");
             option_data->set("code", 82);
             std::string data;
             if (input->contains("sol-max-rt-value")) {
                 data += std::to_string(input->get("sol-max-rt-value")->intValue());
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_INF_MAX_RT (83) inf max rt option"
        {"inf-max-rt-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "inf-max-rt");
             option_data->set("code", 83);
             std::string data;
             if (input->contains("inf-max-rt-value")) {
                 data += std::to_string(input->get("inf-max-rt-value")->intValue());
             }
             option_data->set("data", data);
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_S46_CONT_MAPE (94) S46 MAP-E Container Option.
        // RFC7598 allows for multiple instances of this option.
        {"ietf-dhcpv6-options-rfc7598:s46-cont-mape-option-instances",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "s46-cont-mape");
             option_data->set("code", 94);
             option_data->set("data", "");
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             auto const& list = input->listValue();
             for (auto const& mape_elem : list) {
                 if (!mape_elem->contains("s46-cont-mape-option")) {
                     continue;
                 }
                 // OPTION_S46_RULE (89) S46 rule option
                 ElementPtr const s46rule =
                     mape_elem->get("s46-cont-mape-option")->get("s46-rule-option");
                 if (s46rule) {
                     auto const& rules = s46rule->get("s46-rule")->listValue();
                     for (auto const& rule : rules) {
                         ElementPtr const s46rule_data(Element::createMap());
                         s46rule_data->set("always-send", false);
                         s46rule_data->set("csv-format", true);
                         s46rule_data->set("space", "s46-cont-mape-options");
                         s46rule_data->set("name", "s46-rule");
                         s46rule_data->set("code", 89);
                         string data;
                         if (rule->get("rule-type")->stringValue() == "FMR") {
                             data += to_string(1);
                         } else {
                             data += to_string(0);
                         }
                         data += ", " + to_string(0);
                         auto const ipv4_prefix =
                             Subnet4::parsePrefix(rule->get("ipv4-prefix")->stringValue());
                         data += ", " + to_string(ipv4_prefix.second) + ", " +
                                 ipv4_prefix.first.toText();
                         data += ", " + rule->get("ipv6-prefix")->stringValue();
                         s46rule_data->set("data", data);
                         list_wrap->add(s46rule_data);
                         // OPTION_S46_PORTPARAMS (93) S46 Port Parameters Option
                         if (rule->contains("s46-portparams-option")) {
                             ElementPtr const pp =
                                 rule->get("s46-portparams-option")->get("port-parameter");
                             if (pp) {
                                 ElementPtr const pp_data(Element::createMap());
                                 pp_data->set("always-send", false);
                                 pp_data->set("csv-format", true);
                                 pp_data->set("space", "s46-rule-options");
                                 pp_data->set("name", "s46-portparams");
                                 pp_data->set("code", 93);
                                 string data;
                                 data = to_string(pp->get("offset")->intValue()) + ", " +
                                        to_string(pp->get("psid")->intValue());
                                 data += "/" + to_string(pp->get("psid-len")->intValue());
                                 pp_data->set("data", data);
                                 list_wrap->add(pp_data);
                             }
                         }
                     }
                 }
                 // OPTION_S46_BR (90) S46 BR Option
                 ElementPtr const s46br =
                     mape_elem->get("s46-cont-mape-option")->get("s46-br-option");
                 if (s46br) {
                     auto const& list = s46br->get("br")->listValue();
                     for (auto const& br_elem : list) {
                         ElementPtr const s46br_data(Element::createMap());
                         s46br_data->set("always-send", false);
                         s46br_data->set("csv-format", true);
                         s46br_data->set("space", "s46-cont-mape-options");
                         s46br_data->set("name", "s46-br");
                         s46br_data->set("code", 90);
                         s46br_data->set("data", br_elem->get("br-ipv6-addr")->stringValue());
                         list_wrap->add(s46br_data);
                     }
                 }
             }
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_S46_CONT_MAPT (95) S46 MAP-T Container Option
        {"ietf-dhcpv6-options-rfc7598:s46-cont-mapt-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "s46-cont-mapt");
             option_data->set("code", 95);
             option_data->set("data", "");
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             // OPTION_S46_RULE (89) S46 rule option
             ElementPtr const s46rule = input->get("s46-rule-option");
             if (s46rule) {
                 auto const& rules = s46rule->get("s46-rule")->listValue();
                 for (auto const& rule : rules) {
                     ElementPtr const s46rule_data(Element::createMap());
                     s46rule_data->set("always-send", false);
                     s46rule_data->set("csv-format", true);
                     s46rule_data->set("space", "s46-cont-mapt-options");
                     s46rule_data->set("name", "s46-rule");
                     s46rule_data->set("code", 89);
                     string data;
                     if (rule->get("rule-type")->stringValue() == "FMR") {
                         data += to_string(1);
                     } else {
                         data += to_string(0);
                     }
                     data += ", " + to_string(0);
                     auto const ipv4_prefix =
                         Subnet4::parsePrefix(rule->get("ipv4-prefix")->stringValue());
                     data +=
                         ", " + to_string(ipv4_prefix.second) + ", " + ipv4_prefix.first.toText();
                     data += ", " + rule->get("ipv6-prefix")->stringValue();
                     s46rule_data->set("data", data);
                     list_wrap->add(s46rule_data);
                     // OPTION_S46_PORTPARAMS (93) S46 Port Parameters Option
                     if (rule->contains("s46-portparams-option")) {
                         ElementPtr const pp =
                             rule->get("s46-portparams-option")->get("port-parameter");
                         if (pp) {
                             ElementPtr const pp_data(Element::createMap());
                             pp_data->set("always-send", false);
                             pp_data->set("csv-format", true);
                             pp_data->set("space", "s46-rule-options");
                             pp_data->set("name", "s46-portparams");
                             pp_data->set("code", 93);
                             string data;
                             data = to_string(pp->get("offset")->intValue()) + ", " +
                                    to_string(pp->get("psid")->intValue());
                             data += "/" + to_string(pp->get("psid-len")->intValue());
                             pp_data->set("data", data);
                             list_wrap->add(pp_data);
                         }
                     }
                 }
             }
             // OPTION_S46_DMR (91) S46 DMR Option
             ElementPtr const s46dmr = input->get("s46-dmr-option");
             if (s46dmr) {
                 ElementPtr const s46dmr_data(Element::createMap());
                 s46dmr_data->set("always-send", false);
                 s46dmr_data->set("csv-format", true);
                 s46dmr_data->set("space", "s46-cont-mapt-options");
                 s46dmr_data->set("name", "s46-dmr");
                 s46dmr_data->set("code", 91);
                 s46dmr_data->set("data", s46dmr->get("dmr-ipv6-prefix")->stringValue());
                 list_wrap->add(s46dmr_data);
             }
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }},
        // OPTION_S46_CONT_LW (96) S46 Lightweight 4 over 6 Container Option
        {"ietf-dhcpv6-options-rfc7598:s46-cont-lw-option",
         []([[maybe_unused]] ElementPtr const& input) -> ElementPtr {
             ElementPtr const option_data(Element::createMap());
             // Option-set found, converting it to kea
             // Converting option-data first
             option_data->set("always-send", false);
             option_data->set("csv-format", true);
             option_data->set("space", "dhcp6");
             option_data->set("name", "s46-cont-lw");
             option_data->set("code", 96);
             option_data->set("data", "");
             ElementPtr const list_wrap(Element::createList());
             list_wrap->add(option_data);
             // OPTION_S46_BR (90) S46 BR Option
             ElementPtr const s46br = input->get("s46-br-option");
             if (s46br) {
                 auto const& list = s46br->get("br")->listValue();
                 for (auto const& br_elem : list) {
                     ElementPtr const s46br_data(Element::createMap());
                     s46br_data->set("always-send", false);
                     s46br_data->set("csv-format", true);
                     s46br_data->set("space", "s46-cont-lw-options");
                     s46br_data->set("name", "s46-br");
                     s46br_data->set("code", 90);
                     s46br_data->set("data", br_elem->get("br-ipv6-addr")->stringValue());
                     list_wrap->add(s46br_data);
                 }
             }
             // OPTION_S46_V4V6BIND (92) S46 IPv4/IPv6 Address Binding option
             ElementPtr const s46bind = input->get("s46-v4-v6-binding-option");
             if (s46bind) {
                 ElementPtr const s46bind_data(Element::createMap());
                 s46bind_data->set("always-send", false);
                 s46bind_data->set("csv-format", true);
                 s46bind_data->set("space", "s46-cont-lw-options");
                 s46bind_data->set("name", "s46-v4v6bind");
                 s46bind_data->set("code", 92);
                 string data;
                 data = s46bind->get("ipv4-addr")->stringValue();
                 data += ", " + s46bind->get("bind-ipv6-prefix")->stringValue();
                 s46bind_data->set("data", data);
                 list_wrap->add(s46bind_data);
                 // OPTION_S46_PORTPARAMS (93) S46 Port Parameters Option
                 if (s46bind->contains("s46-portparams-option")) {
                     ElementPtr const pp =
                         s46bind->get("s46-portparams-option")->get("port-parameter");
                     if (pp) {
                         ElementPtr const pp_data(Element::createMap());
                         pp_data->set("always-send", false);
                         pp_data->set("csv-format", true);
                         pp_data->set("space", "s46-v4v6bind-options");
                         pp_data->set("name", "s46-portparams");
                         pp_data->set("code", 93);
                         string data;
                         data = to_string(pp->get("offset")->intValue()) + ", " +
                                to_string(pp->get("psid")->intValue());
                         data += "/" + to_string(pp->get("psid-len")->intValue());
                         pp_data->set("data", data);
                         list_wrap->add(pp_data);
                     }
                 }
             }
             ElementPtr const result(Element::createMap());
             result->set("option-data", list_wrap);
             return result;
         }}};
    return _;
}
template <>
Converter<IETF_to_Kea>::transform_table_t& Converter<IETF_to_Kea>::transform() {
    static transform_table_t _{
        {copy,
         []([[maybe_unused]] ElementPtr const& e, [[maybe_unused]] ElementPtr const& input,
            [[maybe_unused]] ElementPtr const& output) -> ElementPtr { return e; }},
        // clang-format off
/////////////////////////////////////////////// DUID ///////////////////////////////////////////////
//  +--rw server-duid                                              |  +--rw server-id!
//  |  +--rw type-code?                           uint16           |  |  +--rw type?            enumeration
//  |  +--rw (duid-type)?                                          |  |  +--rw identifier?      string
//  |  |  +--:(duid-llt)                                           |  |  +--rw time?            uint32
//  |  |  |  +--rw duid-llt-hardware-type?        uint16           |  |  +--rw htype?           uint16
//  |  |  |  +--rw duid-llt-time?                 yang:timeticks   |  |  +--rw enterprise-id?   uint32
//  |  |  |  +--rw duid-llt-link-layer-address?   yang:mac-address |  |  +--rw persist?         boolean
//  |  |  +--:(duid-en)                                            |  |  +--rw user-context?    kea:user-context
//  |  |  |  +--rw duid-en-enterprise-number?     uint32           |
//  |  |  |  +--rw duid-en-identifier?            string           |
//  |  |  +--:(duid-ll)                                            |
//  |  |  |  +--rw duid-ll-hardware-type?         uint16           |
//  |  |  |  +--rw duid-ll-link-layer-address?    yang:mac-address |
//  |  |  +--:(duid-uuid)                                          |
//  |  |  |  +--rw uuid?                          yang:uuid        |
//  |  |  +--:(duid-unstructured)                                  |
//  |  |     +--rw data?                          binary           |
//  |  +--ro active-duid?                         binary           |
        // clang-format on
        {DUID,
         []([[maybe_unused]] ElementPtr const& e, [[maybe_unused]] ElementPtr const& input,
            [[maybe_unused]] ElementPtr const& output) -> ElementPtr {
             ElementPtr const result(Element::createMap());
             dhcp::DUID duid_raw(dhcp::DUID::fromText(e->stringValue()));
             std::vector<uint8_t> const& duid_vec = duid_raw.getDuid();
             std::vector<uint8_t> identifier;
             uint16_t htype = 0;
             uint32_t time = 0, enterprise_id = 0;
             switch (duid_raw.getType()) {
             case dhcp::DUID::DUID_LLT:
                 htype = readUint16(&duid_vec[2], duid_vec.size() - 2);
                 time = readUint32(&duid_vec[4], duid_vec.size() - 4);
                 identifier.assign(duid_vec.begin() + 8, duid_vec.end());
                 result->set("type", "LLT");
                 result->set("htype", htype);
                 result->set("time", time);
                 if (!identifier.empty()) {
                     result->set("identifier", encode::encodeHex(identifier));
                 }
                 break;
             case dhcp::DUID::DUID_EN:
                 enterprise_id = readUint32(&duid_vec[2], duid_vec.size() - 2);
                 identifier.assign(duid_vec.begin() + 6, duid_vec.end());
                 result->set("type", "EN");
                 result->set("enterprise-id", enterprise_id);
                 if (!identifier.empty()) {
                     result->set("identifier", encode::encodeHex(identifier));
                 }
                 break;
             case dhcp::DUID::DUID_LL:
                 htype = readUint16(&duid_vec[2], duid_vec.size() - 2);
                 identifier.assign(duid_vec.begin() + 4, duid_vec.end());
                 result->set("type", "LL");
                 result->set("htype", htype);
                 if (!identifier.empty()) {
                     result->set("identifier", encode::encodeHex(identifier));
                 }
                 break;
             case dhcp::DUID::DUID_UUID:
                 identifier.assign(duid_vec.begin() + 2, duid_vec.end());
                 result->set("type", "UUID");
                 if (!identifier.empty()) {
                     result->set("identifier", encode::encodeHex(identifier));
                 }
                 break;
             default:
                 return nullptr;
             }

             return result;
         }},
        {network_ranges,
         []([[maybe_unused]] ElementPtr const& e, [[maybe_unused]] ElementPtr const& input,
            [[maybe_unused]] ElementPtr const& output) -> ElementPtr {
             output->checkAndXpath("/Dhcp6/valid-lifetime", e, "valid-lifetime");
             output->checkAndXpath("/Dhcp6/renew-timer", e, "renew-time");
             output->checkAndXpath("/Dhcp6/rebind-timer", e, "rebind-time");
             output->checkAndXpath("/Dhcp6/preferred-lifetime", e, "preferred-lifetime");
             /// @todo(missing in kea) option-set-id
             /// @todo(missing in kea) rapid-commit

             ElementPtr const& network_range(e->get("network-range"));
             if (!network_range) {
                 return nullptr;
             }

             ElementPtr const result(Element::createList());
             for (ElementPtr const& i : network_range->listValue()) {
                 result->add(isc::dhcp::SubnetMgr<
                             isc::dhcp::DHCP_SPACE_V6>::convertIETFNetworkRangetoKeaSubnet(i));
             }
             return result;
         }},
        {network_range_option_sets,
         []([[maybe_unused]] ElementPtr const& e, [[maybe_unused]] ElementPtr const& input,
            [[maybe_unused]] ElementPtr const& output) -> ElementPtr {
             auto f = [&](std::string const& option_set_id, std::string const& destination_xpath,
                          bool isRootNode = false) {
                 ElementPtr const& id_list(input->xpath(option_set_id));
                 if (!id_list) {
                     return;
                 }
                 ElementPtr const option_def_node(Element::createList());
                 ElementPtr const option_data_node(Element::createList());
                 for (ElementPtr const& id_element : id_list->listValue()) {
                     std::string const id(to_string(id_element->intValue()));
                     ElementPtr const& option_data_input(input->xpath(
                         "/dhcpv6-server/option-sets/option-set[option-set-id='" + id + "']"));
                     for (auto const& [option_key, option_value] : option_data_input->mapValue()) {
                         // Traverse to the first option-set skipping description
                         if (option_key == "description") {
                             continue;
                         }
                         try {
                             ElementPtr const& converted_option =
                                 transformOptionSets().at(option_key)(option_value);
                             option_data_node->checkAndAddAllListElements(converted_option,
                                                                          "option-data");
                             option_def_node->checkAndAddAllListElements(converted_option,
                                                                         "option-def");
                         } catch (std::out_of_range&) {
                             Spdlog::debug("{}: {}", "IETF Converter error. Can't convert option",
                                           option_key);
                         }
                     }
                 }
                 if (isRootNode) {
                     output->xpathIfNotEmpty(destination_xpath + "/option-data", option_data_node);
                     output->xpathIfNotEmpty(destination_xpath + "/option-def", option_def_node);
                 } else {
                     output->xpathIfNotEmpty(destination_xpath, option_data_node);
                 }
             };

             f("/dhcpv6-server/network-ranges/option-set-id", "/Dhcp6", true);

             ElementPtr const network_range(
                 input->xpath("/dhcpv6-server/network-ranges/network-range"));
             if (!network_range) {
                 return nullptr;
             }

             for (ElementPtr const& i : network_range->listValue()) {
                 ElementPtr const& id_element_i(i->get("id"));
                 std::string const id_i(to_string(id_element_i->intValue()));
                 f("/dhcpv6-server/network-ranges/network-range[id='" + id_i + "']/option-set-id",
                   "/Dhcp6/subnet6[id='" + id_i + "']/option-data");

                 for (auto const& [ietf_pool, kea_pool] :
                      std::array<std::tuple<std::string, std::string>, 2>{
                          {{"address-pool", "pool"}, {"prefix-pool", "pd-pool"}}}) {
                     ElementPtr const address_pools(i->get(ietf_pool + "s"));
                     if (address_pools) {
                         ElementPtr const address_pool(address_pools->get(ietf_pool));
                         if (address_pool) {
                             for (ElementPtr const& j : address_pool->listValue()) {
                                 ElementPtr const& id_element_j(j->get("pool-id"));
                                 std::string const id_j(to_string(id_element_j->intValue()));
                                 f("/dhcpv6-server/network-ranges/"
                                   "network-range[id='" +
                                       id_i + "']/" + ietf_pool + "s/" + ietf_pool + "[pool-id='" +
                                       id_j + "']/option-set-id",
                                   "/Dhcp6/subnet6[id='" + id_i + "']/" + kea_pool + "s[id='" +
                                       id_j + "']/option-data");

                                 // @todo
                                 // f("/dhcpv6-server/network-ranges/network-range[*]/" + ietf_pool
                                 // + "s/" + ietf_pool +
                                 // "[*]/host-reservations/host-reservation[*]/option-set-id",
                                 // "TODO");
                             }
                         }
                     }
                 }
             }

             return nullptr;
         }},
    };
    return _;
}

template <>
Converter<Kea_to_IETF>::transform_table_t& Converter<Kea_to_IETF>::transform() {
    static transform_table_t _{
        {copy,
         []([[maybe_unused]] ElementPtr const& e, [[maybe_unused]] ElementPtr const& input,
            [[maybe_unused]] ElementPtr const& output) -> ElementPtr { return e; }},
        {DUID,
         []([[maybe_unused]] ElementPtr const& e, [[maybe_unused]] ElementPtr const& input,
            [[maybe_unused]] ElementPtr const& output) -> ElementPtr {
             if (!e->get("identifier") || e->get("identifier")->stringValue().empty()) {
                 // Don't convert since ietf-dhcpv6-server will complain about empty DUID.
                 return nullptr;
             }

             ElementPtr const result(Element::createMap());
             ElementPtr const& type(e->get("type"));
             if (type) {
                 std::string const type_string(type->stringValue());
                 if (type_string == "LLT") {
                     result->checkAndSet("duid-llt-hardware-type", e, "htype");
                     result->checkAndSet("duid-llt-time", e, "time");
                     ElementPtr identifier(e->get("identifier"));
                     if (identifier) {
                         std::string identifier_string(identifier->stringValue());
                         std::string lla;
                         int i(0);
                         for (char const c : identifier_string) {
                             if (i != 0 && (i % 2 == 0)) {
                                 lla += ':';
                             }
                             lla += c;
                             ++i;
                         }
                         result->set("duid-llt-link-layer-address", lla);
                     }
                 } else if (type_string == "EN") {
                     result->checkAndSet("duid-en-enterprise-number", e, "enterprise-id");
                     result->checkAndSet("duid-en-identifier", e, "identifier");
                 } else if (type_string == "LL") {
                     result->checkAndSet("duid-ll-hardware-type", e, "htype");
                     result->checkAndSet("duid-ll-link-layer-address", e, "identifier");
                 } else {
                     /// @todo
                     /// result->checkAndSet("uuid", e, "identifier");
                     result->checkAndSet("data", e, "identifier");
                 }
             }
             return result;
         }},
        {network_ranges,
         []([[maybe_unused]] ElementPtr const& e, [[maybe_unused]] ElementPtr const& input,
            [[maybe_unused]] ElementPtr const& output) -> ElementPtr {
             ElementPtr const result(Element::createMap());
             ElementPtr const network_range(Element::createList());
             for (ElementPtr const& i : e->listValue()) {
                 network_range->add(
                     isc::dhcp::SubnetMgr<
                         isc::dhcp::DHCP_SPACE_V6>::convertKeaSubnetToIETFNetworkRange(i));
             }
             result->set("network-range", network_range);
             return result;
         }},
        {network_range_option_sets,
         []([[maybe_unused]] ElementPtr const& e, [[maybe_unused]] ElementPtr const& input,
            [[maybe_unused]] ElementPtr const& output) -> ElementPtr { return nullptr; }},
    };
    return _;
}

template <>
std::string CounterpartModel<IETF>::get(std::string const& model) {
    if (model == KEA_DHCP6_SERVER || model == IETF_DHCPV6_SERVER) {
        return IETF_DHCPV6_SERVER;
    } else if (model == KEA_DHCP4_SERVER) {
        return KEA_DHCP4_SERVER;
    }
    isc_throw(Unexpected, PRETTY_FUNCTION_NAME() << ": " << model);
}

template <>
std::string CounterpartModel<Kea>::get(std::string const& model) {
    if (model == IETF_DHCPV6_SERVER || model == KEA_DHCP6_SERVER) {
        return KEA_DHCP6_SERVER;
    } else if (model == KEA_DHCP4_SERVER) {
        return KEA_DHCP4_SERVER;
    }
    isc_throw(Unexpected, PRETTY_FUNCTION_NAME() << ": " << model);
}

}  // namespace yang
}  // namespace isc

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

#include <asiolink/addr_utilities.h>
#include <cc/command_interpreter.h>
#include <cc/data.h>
#include <dhcpsrv/lease_mgr_factory.h>
#include <dhcpsrv/subnet_mgr_factory.h>
#include <netconf/control_socket.h>
#include <netconf/control_socket_factory.h>
#include <netconf/netconf_log.h>
#include <netconf/operational_callback.h>
#include <stats/stats_mgr.h>
#include <util/encode/hex.h>
#include <yang/translator_universal.h>

using namespace std;

using isc::config::answerToText;
using isc::config::CONTROL_RESULT_SUCCESS;
using isc::config::parseAnswer;
using isc::data::Element;
using isc::data::ElementPtr;
using isc::dhcp::DHCP_SPACE_V4;
using isc::dhcp::DHCP_SPACE_V6;
using isc::dhcp::DhcpSpaceType;
using isc::dhcp::Lease;
using isc::dhcp::Lease6;
using isc::dhcp::Lease6Collection;
using isc::dhcp::Lease6Ptr;
using isc::dhcp::Subnet6;
using isc::dhcp::SubnetInfo;
using isc::dhcp::SubnetInfoPtr;
using isc::stats::StatsMgr;
using isc::yang::TranslatorUniversal;

using libyang::Data_Node;
using libyang::S_Context;
using libyang::S_Data_Node;

using sysrepo::S_Session;

namespace isc {
namespace netconf {

OperationalCallback::OperationalCallback(sysrepo::S_Session sysrepo_get_sess, CfgModelPtr const& model_config)
    : sysrepo_get_session_(sysrepo_get_sess), model_config_(model_config) {
    // Populate cache with a cpu usage value
    isc::stats::StatsMgr::instance().getCpuUsage();
}

template <>
unordered_map<string, tuple<string, bool>> const&
OperationalCallback::getStatisticMap<DHCP_SPACE_V4>() {
    static unordered_map<string, tuple<string, bool>> const _{
        {"ack-received-count", {"pkt4-ack-received", false}},
        {"ack-sent-count", {"pkt4-ack-sent", false}},
        {"decline-count", {"pkt4-decline-received", false}},
        {"discover-count", {"pkt4-discover-received", false}},
        {"inform-count", {"pkt4-inform-received", false}},
        {"nak-received-count", {"pkt4-nak-received", false}},
        {"nak-sent-count", {"pkt4-nak-sent", false}},
        {"offer-received-count", {"pkt4-offer-received", false}},
        {"offer-sent-count", {"pkt4-offer-sent", false}},
        {"parse-failed-count", {"pkt4-parse-failed", false}},
        {"receive-drop-count", {"pkt4-receive-drop", false}},
        {"received-count", {"pkt4-received", false}},
        {"release-count", {"pkt4-release-received", false}},
        {"request-count", {"pkt4-request-received", false}},
        {"sent-count", {"pkt4-sent", false}},
        {"unknown-count", {"pkt4-unknown-received", false}},
        {"ack-mean-processing-time", {"pkt4-ack-processing-time", true}},
        {"decline-mean-processing-time", {"pkt4-decline-processing-time", true}},
        {"discover-mean-processing-time", {"pkt4-discover-processing-time", true}},
        {"inform-mean-processing-time", {"pkt4-inform-processing-time", true}},
        {"nak-mean-processing-time", {"pkt4-nak-processing-time", true}},
        {"offer-mean-processing-time", {"pkt4-offer-processing-time", true}},
        {"mean-processing-time", {"pkt4-processing-time", true}},
        {"release-mean-processing-time", {"pkt4-release-processing-time", true}},
        {"request-mean-processing-time", {"pkt4-request-processing-time", true}},
        {"unknown-mean-processing-time", {"pkt4-unknown-processing-time", true}},
        {"mean-cpu-usage-per-packet", {"pkt4-cpu-usage-per-packet", true}},
        {"mean-memory-usage-per-packet", {"pkt4-memory-usage-per-packet", true}},
        // Database stats from here onwards
        {"ALL_LEASE4_STATS-count", {"ALL_LEASE4_STATS-count", false}},
        {"SUBNET_LEASE4_STATS-count", {"SUBNET_LEASE4_STATS-count", false}},
        {"SUBNET_RANGE_LEASE4_STATS-count", {"SUBNET_RANGE_LEASE4_STATS-count", false}},
        {"ALL_LEASE4_STATS-mean-processing-time", {"ALL_LEASE4_STATS-processing-time", true}},
        {"SUBNET_LEASE4_STATS-mean-processing-time", {"SUBNET_LEASE4_STATS-processing-time", true}},
        {"SUBNET_RANGE_LEASE4_STATS-mean-processing-time",
         {"SUBNET_RANGE_LEASE4_STATS-processing-time", true}},
        {"INSERT_LEASE4-mean-processing-time", {"INSERT_LEASE4-processing-time", true}},
        {"UPDATE_LEASE4-mean-processing-time", {"UPDATE_LEASE4-processing-time", true}},
        {"DELETE_LEASE4-mean-processing-time", {"DELETE_LEASE4-processing-time", true}},
        {"GET_LEASE4_EXPIRE-mean-processing-time", {"GET_LEASE4_EXPIRE-processing-time", true}},
        {"GET_LEASE4_ADDR-mean-processing-time", {"GET_LEASE4_ADDR-processing-time", true}},
        {"GET_LEASE4-mean-processing-time", {"GET_LEASE4-processing-time", true}},
        {"GET_LEASE4_CLIENTID-mean-processing-time", {"GET_LEASE4_CLIENTID-processing-time", true}},
        {"GET_LEASE4_CLIENTID_SUBID-mean-processing-time",
         {"GET_LEASE4_CLIENTID_SUBID-processing-time", true}},
        {"GET_LEASE4_LIMIT-mean-processing-time", {"GET_LEASE4_LIMIT-processing-time", true}},
        {"GET_LEASE4_HWADDR-mean-processing-time", {"GET_LEASE4_HWADDR-processing-time", true}},
        {"GET_LEASE4_HWADDR_SUBID-mean-processing-time",
         {"GET_LEASE4_HWADDR_SUBID-processing-time", true}},
        {"GET_LEASE4_HOSTNAME-mean-processing-time", {"GET_LEASE4_HOSTNAME-processing-time", true}},
        {"GET_LEASE4_PAGE-mean-processing-time", {"GET_LEASE4_PAGE-processing-time", true}},
        {"GET_LEASE4_SUBID-mean-processing-time", {"GET_LEASE4_SUBID-processing-time", true}},
        {"INSERT_LOCK4-mean-processing-time", {"INSERT_LOCK4-processing-time", true}},
        {"DELETE_LOCK4-mean-processing-time", {"DELETE_LOCK4-processing-time", true}},
        {"INSERT_LEASE4-count", {"INSERT_LEASE4-count", false}},
        {"UPDATE_LEASE4-count", {"UPDATE_LEASE4-count", false}},
        {"DELETE_LEASE4-count", {"DELETE_LEASE4-count", false}},
        {"GET_LEASE4_EXPIRE-count", {"GET_LEASE4_EXPIRE-count", false}},
        {"GET_LEASE4_ADDR-count", {"GET_LEASE4_ADDR-count", false}},
        {"GET_LEASE4-count", {"GET_LEASE4-count", false}},
        {"GET_LEASE4_CLIENTID-count", {"GET_LEASE4_CLIENTID-count", false}},
        {"GET_LEASE4_CLIENTID_SUBID-count", {"GET_LEASE4_CLIENTID_SUBID-count", false}},
        {"GET_LEASE4_LIMIT-count", {"GET_LEASE4_LIMIT-count", false}},
        {"GET_LEASE4_HWADDR-count", {"GET_LEASE4_HWADDR-count", false}},
        {"GET_LEASE4_HWADDR_SUBID-count", {"GET_LEASE4_HWADDR_SUBID-count", false}},
        {"GET_LEASE4_HOSTNAME-count", {"GET_LEASE4_HOSTNAME-count", false}},
        {"GET_LEASE4_PAGE-count", {"GET_LEASE4_PAGE-count", false}},
        {"GET_LEASE4_SUBID-count", {"GET_LEASE4_SUBID-count", false}},
        {"INSERT_LOCK4-count", {"INSERT_LOCK4-count", false}},
        {"DELETE_LOCK4-count", {"DELETE_LOCK4-count", false}}};
    return _;
}

template <>
unordered_map<string, tuple<string, bool>> const&
OperationalCallback::getStatisticMap<DHCP_SPACE_V6>() {
    static unordered_map<string, tuple<string, bool>> const _{
        {"solicit-count", {"pkt6-solicit-received", false}},
        {"advertise-count", {"pkt6-advertise-sent", false}},
        {"request-count", {"pkt6-request-received", false}},
        {"confirm-count", {"pkt6-confirm-received", false}},
        {"renew-count", {"pkt6-renew-received", false}},
        {"rebind-count", {"pkt6-rebind-received", false}},
        {"reply-count", {"pkt6-reply-sent", false}},
        {"release-count", {"pkt6-release-received", false}},
        {"decline-count", {"pkt6-decline-received", false}},
        {"reconfigure-count", {"pkt6-reconfigure-sent", false}},
        {"information-request-count", {"pkt6-infrequest-received", false}},
        {"dhcpv6-server-stats-augment:advertise-received-count",
         {"pkt6-advertise-received", false}},
        {"dhcpv6-server-stats-augment:dhcpv4-query-received-count",
         {"pkt6-dhcpv4-query-received", false}},
        {"dhcpv6-server-stats-augment:dhcpv4-response-received-count",
         {"pkt6-dhcpv4-response-received", false}},
        {"dhcpv6-server-stats-augment:dhcpv4-response-sent-count",
         {"pkt6-dhcpv4-response-sent", false}},
        {"dhcpv6-server-stats-augment:parse-failed-count", {"pkt6-parse-failed", false}},
        {"dhcpv6-server-stats-augment:receive-drop-count", {"pkt6-receive-drop", false}},
        {"dhcpv6-server-stats-augment:received-count", {"pkt6-received", false}},
        {"dhcpv6-server-stats-augment:reply-received-count", {"pkt6-reply-received", false}},
        {"dhcpv6-server-stats-augment:sent-count", {"pkt6-sent", false}},
        {"dhcpv6-server-stats-augment:unknown-count", {"pkt6-unknown-received", false}},
        {"dhcpv6-server-stats-augment:advertise-mean-processing-time",
         {"pkt6-advertise-processing-time", true}},
        {"dhcpv6-server-stats-augment:decline-mean-processing-time",
         {"pkt6-decline-processing-time", true}},
        {"dhcpv6-server-stats-augment:dhcpv4-query-mean-processing-time",
         {"pkt6-dhcpv4-query-processing-time", true}},
        {"dhcpv6-server-stats-augment:dhcpv4-response-mean-processing-time",
         {"pkt6-dhcpv4-response-processing-time", true}},
        {"dhcpv6-server-stats-augment:infrequest-mean-processing-time",
         {"pkt6-infrequest-processing-time", true}},
        {"dhcpv6-server-stats-augment:rebind-mean-processing-time",
         {"pkt6-rebind-processing-time", true}},
        {"dhcpv6-server-stats-augment:mean-processing-time", {"pkt6-processing-time", true}},
        {"dhcpv6-server-stats-augment:release-mean-processing-time",
         {"pkt6-release-processing-time", true}},
        {"dhcpv6-server-stats-augment:renew-mean-processing-time",
         {"pkt6-renew-processing-time", true}},
        {"dhcpv6-server-stats-augment:reply-mean-processing-time",
         {"pkt6-reply-processing-time", true}},
        {"dhcpv6-server-stats-augment:request-mean-processing-time",
         {"pkt6-request-processing-time", true}},
        {"dhcpv6-server-stats-augment:solicit-mean-processing-time",
         {"pkt6-solicit-processing-time", true}},
        {"dhcpv6-server-stats-augment:unknown-mean-processing-time",
         {"pkt6-unknown-processing-time", true}},
        {"dhcpv6-server-stats-augment:mean-cpu-usage-per-packet",
         {"pkt6-cpu-usage-per-packet", true}},
        {"dhcpv6-server-stats-augment:mean-memory-usage-per-packet",
         {"pkt6-memory-usage-per-packet", true}},
        // Database stats from here onwards
        {"dhcpv6-server-stats-augment:INSERT_LEASE6-mean-processing-time",
         {"INSERT_LEASE6-processing-time", true}},
        {"dhcpv6-server-stats-augment:UPDATE_LEASE6-mean-processing-time",
         {"UPDATE_LEASE6-processing-time", true}},
        {"dhcpv6-server-stats-augment:DELETE_LEASE6-mean-processing-time",
         {"DELETE_LEASE6-processing-time", true}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_EXPIRE-mean-processing-time",
         {"GET_LEASE6_EXPIRE-processing-time", true}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_ADDR-mean-processing-time",
         {"GET_LEASE6_ADDR-processing-time", true}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_DUID-mean-processing-time",
         {"GET_LEASE6_DUID-processing-time", true}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_DUID_IAID-mean-processing-time",
         {"GET_LEASE6_DUID_IAID-processing-time", true}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_DUID_IAID_SUBID-mean-processing-time",
         {"GET_LEASE6_DUID_IAID_SUBID-processing-time", true}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_LIMIT-mean-processing-time",
         {"GET_LEASE6_LIMIT-processing-time", true}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_PAGE-mean-processing-time",
         {"GET_LEASE6_PAGE-processing-time", true}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_SUBID-mean-processing-time",
         {"GET_LEASE6_SUBID-processing-time", true}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_HOSTNAME-mean-processing-time",
         {"GET_LEASE6_HOSTNAME-processing-time", true}},
        {"dhcpv6-server-stats-augment:ALL_LEASE6_STATS-mean-processing-time",
         {"ALL_LEASE6_STATS-processing-time", true}},
        {"dhcpv6-server-stats-augment:SUBNET_LEASE6_STATS-mean-processing-time",
         {"SUBNET_LEASE6_STATS-processing-time", true}},
        {"dhcpv6-server-stats-augment:SUBNET_RANGE_LEASE6_STATS-mean-processing-time",
         {"SUBNET_RANGE_LEASE6_STATS-processing-time", true}},
        {"dhcpv6-server-stats-augment:INSERT_LOCK6-mean-processing-time",
         {"INSERT_LOCK6-processing-time", true}},
        {"dhcpv6-server-stats-augment:DELETE_LOCK6-mean-processing-time",
         {"DELETE_LOCK6-processing-time", true}},
        {"dhcpv6-server-stats-augment:INSERT_LEASE6-count", {"INSERT_LEASE6-count", false}},
        {"dhcpv6-server-stats-augment:UPDATE_LEASE6-count", {"UPDATE_LEASE6-count", false}},
        {"dhcpv6-server-stats-augment:DELETE_LEASE6-count", {"DELETE_LEASE6-count", false}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_EXPIRE-count", {"GET_LEASE6_EXPIRE-count", false}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_ADDR-count", {"GET_LEASE6_ADDR-count", false}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_DUID-count", {"GET_LEASE6_DUID-count", false}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_DUID_IAID-count",
         {"GET_LEASE6_DUID_IAID-count", false}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_DUID_IAID_SUBID-count",
         {"GET_LEASE6_DUID_IAID_SUBID-count", false}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_LIMIT-count", {"GET_LEASE6_LIMIT-count", false}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_PAGE-count", {"GET_LEASE6_PAGE-count", false}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_SUBID-count", {"GET_LEASE6_SUBID-count", false}},
        {"dhcpv6-server-stats-augment:GET_LEASE6_HOSTNAME-count",
         {"GET_LEASE6_HOSTNAME-count", false}},
        {"dhcpv6-server-stats-augment:ALL_LEASE6_STATS-count", {"ALL_LEASE6_STATS-count", false}},
        {"dhcpv6-server-stats-augment:SUBNET_LEASE6_STATS-count",
         {"SUBNET_LEASE6_STATS-count", false}},
        {"dhcpv6-server-stats-augment:SUBNET_RANGE_LEASE6_STATS-count",
         {"SUBNET_RANGE_LEASE6_STATS-count", false}},
        {"dhcpv6-server-stats-augment:INSERT_LOCK6-count", {"INSERT_LOCK6-count", false}},
        {"dhcpv6-server-stats-augment:DELETE_LOCK6-count", {"DELETE_LOCK6-count", false}}};
    return _;
}

ElementPtr OperationalCallback::getLeasesBySubnet(shared_ptr<ControlSocketInterface> comm,
                                                  uint32_t subnetId) {
    // Obtain leases by calling leases-get through the unix socket
    ElementPtr leases_elem, answer;
    int rcode;
    try {
        answer = comm->leasesGet(subnetId);
        leases_elem = parseAnswer(rcode, answer);
    } catch (exception const& exception) {
        LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
            .arg(string("leases-get command failed with ") + exception.what());
        return nullptr;
    }
    if (rcode != CONTROL_RESULT_SUCCESS) {
        LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
            .arg("leases-get command returned " + answerToText(answer));
        return nullptr;
    }
    return leases_elem;
}

ElementPtr OperationalCallback::getKEAConfig(shared_ptr<ControlSocketInterface> comm) {
    ElementPtr subnet_elem, answer;
    int rcode;
    try {
        answer = comm->configGet({}, {});
        subnet_elem = parseAnswer(rcode, answer);
    } catch (exception const& exception) {
        LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
            .arg(string("config-get command failed with ") + exception.what());
        return nullptr;
    }
    if (rcode != CONTROL_RESULT_SUCCESS) {
        LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
            .arg("config-get command returned " + answerToText(answer));
        return nullptr;
    }
    return subnet_elem;
}

bool OperationalCallback::setXpath(S_Session const& session,
                                   S_Data_Node& parent,
                                   string const& node_xpath,
                                   string const& value) {
    try {
        S_Context ctx = session->get_context();
        if (parent) {
            parent->new_path(ctx, node_xpath.c_str(), value.c_str(), LYD_ANYDATA_CONSTSTRING, 0);
        } else {
            parent = make_shared<Data_Node>(ctx, node_xpath.c_str(), value.c_str(),
                                            LYD_ANYDATA_CONSTSTRING, 0);
        }
    } catch (runtime_error const& e) {
        LOG_WARN(netconf_logger, NETCONF_WARNING)
            .arg("At path " + node_xpath + ", value " + value + " " + ", error: " + e.what());
        return false;
    }
    return true;
}

template <>
std::string OperationalCallback::getResourcePrefix<DHCP_SPACE_V4>() {
    return "";
}

template <>
std::string OperationalCallback::getResourcePrefix<DHCP_SPACE_V6>() {
    return "dhcpv6-server-stats-augment:";
}

ElementPtr OperationalCallback::getSysrepoConfig(const char* module_name) {
    ElementPtr config;
    const string model(module_name);

    try {
        // Retrieve configuration from Sysrepo.
        TranslatorUniversal translator(sysrepo_get_session_, model);
        config = translator.get();
        if (!config) {
            LOG_ERROR(netconf_logger, NETCONF_GET_CONFIG_FAILED)
                .arg(model)
                .arg("configuration is empty");
            return nullptr;
        } else {
            LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_GET_CONFIG)
                .arg(model)
                .arg("\n" + prettyPrint(config));
        }
    } catch (exception const& exception) {
        LOG_ERROR(netconf_logger, NETCONF_GET_CONFIG_FAILED).arg(model).arg(exception.what());
        return nullptr;
    }

    return config;
}

template <DhcpSpaceType D>
void OperationalCallback::setAllResources(S_Data_Node& parent,
                                          S_Session& session,
                                          string request_xpath) {
    string value;
    auto const cpu_percentage = StatsMgr::instance().getCpuUsage();
    S_Context ctx = session->get_context();
    if (cpu_percentage) {
        value = to_string(cpu_percentage.value());
        setXpath(session, parent, request_xpath + getResourcePrefix<D>() + "kea-netconf-cpu-usage",
                 value);
    }
    auto const memory_usage = StatsMgr::instance().getMemoryUsage();
    if (memory_usage) {
        value = to_string(memory_usage.value());
        setXpath(session, parent,
                 request_xpath + getResourcePrefix<D>() + "kea-netconf-memory-usage", value);
    }
    shared_ptr<ControlSocketInterface> comm;
    string const& model(model_config_->getModel());
    try {
        comm = ControlSocketFactory<DHCP_SPACE_V6>::create(model_config_->getOperational(), model);
    } catch (exception const& exception) {
        LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
            .arg(string("control socket creation failed with ") + exception.what());
        return;
    }
    ElementPtr answer, cpu_elem, memory_elem;
    int rcode;
    try {
        answer = comm->resourceGet("memory-usage");
        memory_elem = parseAnswer(rcode, answer);
        if (rcode != CONTROL_RESULT_SUCCESS) {
            LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
                .arg("resource-get command returned " + answerToText(answer));
            return;
        }
        answer = comm->resourceGet("cpu-usage");
        cpu_elem = parseAnswer(rcode, answer);
    } catch (exception const& exception) {
        LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
            .arg(string("resource-get command failed with ") + exception.what());
        return;
    }
    if (rcode != CONTROL_RESULT_SUCCESS) {
        LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
            .arg("resource-get command returned " + answerToText(answer));
        return;
    }
    if (!memory_elem->contains("memory-usage") || !cpu_elem->contains("cpu-usage")) {
        LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED).arg("no resource found");
        return;
    }
    value = memory_elem->get("memory-usage")->stringValue();
    setXpath(session, parent, request_xpath + getResourcePrefix<D>() + "kea-dhcp-memory-usage",
             value);
    value = cpu_elem->get("cpu-usage")->stringValue();
    setXpath(session, parent, request_xpath + getResourcePrefix<D>() + "kea-dhcp-cpu-usage", value);
}

template <DhcpSpaceType D>
void OperationalCallback::setAllStatistics(S_Data_Node& parent,
                                           S_Session& session,
                                           string const& request_xpath) {
    string const& model(model_config_->getModel());

    shared_ptr<ControlSocketInterface> comm;
    try {
        comm = ControlSocketFactory<D>::create(model_config_->getOperational(), model);
    } catch (const exception& exception) {
        LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
            .arg(string("control socket creation failed with ") + exception.what());
        return;
    }
    // Obtain statistics through the unix socket
    ElementPtr answer, statistic_elem;
    int rcode;
    try {
        answer = comm->statisticGet("all");
        statistic_elem = parseAnswer(rcode, answer);
    } catch (const exception& exception) {
        LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
            .arg(string("statistic-get command failed with ") + exception.what());
        return;
    }
    if (rcode != CONTROL_RESULT_SUCCESS) {
        LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
            .arg("statistic-get command returned " + answerToText(answer));
        return;
    }
    for (auto const& [moduleStatName, statOption] : getStatisticMap<D>()) {
        string value("0");
        if (statistic_elem->contains(get<0>(statOption))) {
            bool const shouldReturnMeanValue(get<1>(statOption));
            if (shouldReturnMeanValue) {
                auto const& packets = statistic_elem->get(get<0>(statOption))->listValue();
                // Avoid division by 0
                if (packets.size() == 0) {
                    break;
                }

                if (packets.front()->listValue().front()->getType() == Element::real) {
                    double double_sum = 0.0;
                    for (auto const& packet : packets) {
                        double_sum += packet->listValue().front()->doubleValue();
                    }
                    value = to_string(double_sum / static_cast<double>(packets.size()));
                } else {
                    uint64_t uint_sum = 0;
                    for (auto const& packet : packets) {
                        int64_t value_to_sum = packet->listValue().front()->intValue();
                        uint_sum += static_cast<uint64_t>(value_to_sum);
                    }
                    value = to_string(uint_sum / packets.size());
                }
            } else {
                ElementPtr const& last_statistic =
                    statistic_elem->get(get<0>(statOption))->listValue().front();
                value = to_string(last_statistic->listValue().front()->intValue());
            }
        }
        setXpath(session, parent, request_xpath + moduleStatName, value);
    }
}

void OperationalCallback::prepareAndSetActiveLeases(S_Data_Node& parent,
                                                    S_Session session,
                                                    const char* module_name,
                                                    string const& setXpath) {
    ElementPtr config = getSysrepoConfig(module_name);
    if (!config) {
        return;
    }
    config = config->xpath(setXpath);

    shared_ptr<ControlSocketInterface> comm;
    try {
        comm = ControlSocketFactory<DHCP_SPACE_V6>::create(model_config_->getOperational(),
                                                           module_name);
    } catch (exception const& exception) {
        LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
            .arg(string("control socket creation failed with ") + exception.what());
        return;
    }

    ElementPtr subnetElement = getKEAConfig(comm);
    if (!subnetElement) {
        return;
    }

    for (auto const& network_range : config->get("network-range")->listValue()) {
        uint32_t subnetId = network_range->get("id")->intValue();

        ElementPtr leasesElement = getLeasesBySubnet(comm, subnetId);

        subnetElement = subnetElement->xpath("/Dhcp6/subnet6[id='" + to_string(subnetId) + "']");
        if (!subnetElement) {
            LOG_WARN(netconf_logger, NETCONF_WARNING)
                .arg("Subnet is missing from KEA config. Sysrepo and KEA might be out of sync.");
            return;
        }
        SubnetInfoPtr<DHCP_SPACE_V6> sel_subnet = make_shared<SubnetInfo<DHCP_SPACE_V6>>();
        sel_subnet->fromElement(subnetElement);

        if (network_range->get("address-pools") &&
            network_range->get("address-pools")->get("address-pool")) {
            for (auto const& pool :
                 network_range->get("address-pools")->get("address-pool")->listValue()) {
                uint32_t poolId = pool->get("pool-id")->intValue();
                Lease6Collection leases;
                for (auto const& lease_elem : leasesElement->get("leases6")->listValue()) {
                    Lease6Ptr l = Lease6::fromElement(lease_elem);
                    bool const same_pool = (l->pool_id_ != -1) ? l->pool_id_ == poolId : false;
                    bool const same_type = l->type_ == Lease::TYPE_NA || l->type_ == Lease::TYPE_TA;
                    if (same_pool && same_type) {
                        leases.emplace_back(l);
                    }
                }
                ElementPtr const& poolElement =
                    subnetElement->xpath("/pools[id='" + to_string(poolId) + "']");
                setActiveLeasesData(parent, sel_subnet, leases, session,
                                    setXpath + "/network-range[id='" + to_string(subnetId) +
                                        "']/address-pools/address-pool[pool-id='" +
                                        to_string(poolId) + "']",
                                    true, poolElement);
            }
        }
        if (network_range->get("prefix-pools") &&
            network_range->get("prefix-pools")->get("prefix-pool")) {
            for (auto const& pool :
                 network_range->get("prefix-pools")->get("prefix-pool")->listValue()) {
                uint32_t poolId = pool->get("pool-id")->intValue();
                Lease6Collection leases;
                for (auto const& lease_elem : leasesElement->get("leases6")->listValue()) {
                    Lease6Ptr l = Lease6::fromElement(lease_elem);
                    bool const same_pool = (l->pool_id_ != -1) ? l->pool_id_ == poolId : false;
                    bool const same_type = l->type_ == Lease::TYPE_PD;
                    if (same_pool && same_type) {
                        leases.emplace_back(l);
                    }
                }
                ElementPtr const& poolElement =
                    subnetElement->xpath("/pd-pools[id='" + to_string(poolId) + "']");
                setActiveLeasesData(parent, sel_subnet, leases, session,
                                    setXpath + "/network-range[id='" + to_string(subnetId) +
                                        "']/prefix-pools/prefix-pool[pool-id='" +
                                        to_string(poolId) + "']",
                                    false, poolElement);
            }
        }
    }
}

bool OperationalCallback::setActiveLeasesData(S_Data_Node& parent,
                                              SubnetInfoPtr<DHCP_SPACE_V6>& sel_subnet,
                                              Lease6Collection& leases,
                                              S_Session const& session,
                                              string const& main_xpath,
                                              bool is_addr_pools_request,
                                              ElementPtr const& pool_elem) {
    string const lease_base_path = main_xpath + "/active-leases";
    string const total_count_path = lease_base_path + "/total-count";
    string const alloc_count_path = lease_base_path + "/allocated-count";
    string total_count("0");
    if (pool_elem) {
        if (is_addr_pools_request) {
            if (!pool_elem->get("pool")) {
                return false;
            }
            // Is this prefix notation?
            string txt = pool_elem->get("pool")->stringValue();
            size_t pos = txt.find('/');
            if (pos != string::npos) {
                [[maybe_unused]] auto [_, prefix_size] =
                    Subnet6::parsePrefix(pool_elem->get("pool")->stringValue());
                total_count = to_string(isc::asiolink::prefixesInRange(prefix_size));
            }

            // Is this min-max notation?
            pos = txt.find('-');
            if (pos != string::npos) {
                // using min-max notation
                try {
                    isc::asiolink::IOAddress min(txt.substr(0, pos));
                    isc::asiolink::IOAddress max(txt.substr(pos + 1));
                    total_count = to_string(isc::asiolink::addrsInRange(min, max));
                } catch (exception const& exception) {
                    LOG_ERROR(netconf_logger, NETCONF_GET_OPER_ITEM_FAILED)
                        .arg("invalid pool range: " + txt + " " + exception.what());
                    return false;
                }
            }
        } else {
            if (!pool_elem->get("prefix-len") || !pool_elem->get("delegated-len")) {
                return false;
            }
            total_count = to_string(
                isc::asiolink::prefixesInRange(pool_elem->get("prefix-len")->intValue(),
                                               pool_elem->get("delegated-len")->intValue()));
        }
        setXpath(session, parent, total_count_path, total_count);
        setXpath(session, parent, alloc_count_path, to_string(leases.size()));
    } else {
        LOG_WARN(netconf_logger, NETCONF_WARNING)
            .arg("Pool is missing from KEA config. Sysrepo and KEA might be out of sync.");
        return false;
    }

    for (auto const& lease : leases) {
        string lease_path;
        if (is_addr_pools_request) {
            lease_path =
                lease_base_path + "/active-lease[leased-address='" + lease->addr_.toText() + "']";
        } else {
            lease_path = lease_base_path + "/active-lease[leased-prefix='" + lease->addr_.toText() +
                         "/" + to_string(lease->prefixlen_) + "']";
        }
        string const cli_duid_path = lease_path + "/client-duid";
        string const iaid_path = lease_path + "/iaid";
        string const aloc_t_path = lease_path + "/allocation-time";
        string const cltt_path = lease_path + "/last-renew-rebind";
        string const pref_l_path = lease_path + "/preferred-lifetime";
        string const valid_l_path = lease_path + "/valid-lifetime";
        string const t1_path = lease_path + "/lease-t1";
        string const t2_path = lease_path + "/lease-t2";

        setXpath(session, parent, cli_duid_path,
                 isc::util::encode::encodeHex(lease->getDuidVector()));
        setXpath(session, parent, iaid_path, to_string(lease->iaid_));

        char buf[80];
        strftime(buf, sizeof(buf), "%FT%TZ", localtime(&(lease->alloc_time_)));
        setXpath(session, parent, aloc_t_path, buf);

        // cltt_ is the last renew rebind time
        strftime(buf, sizeof(buf), "%FT%TZ", localtime(&(lease->cltt_)));
        setXpath(session, parent, cltt_path, buf);

        setXpath(session, parent, pref_l_path, to_string(lease->preferred_lft_));
        setXpath(session, parent, valid_l_path, to_string(lease->valid_lft_));
        setXpath(session, parent, t1_path, to_string(sel_subnet->renew_timer_));
        setXpath(session, parent, t2_path, to_string(sel_subnet->rebind_timer_));
    }

    return true;
}

template <>
int OperationalCallback::setOperationalData<DHCP_SPACE_V6>(S_Session session,
                                                           const char* module_name,
                                                           const char* /* path */,
                                                           const char* request_xpath,
                                                           uint32_t /* request_id */,
                                                           S_Data_Node& parent) {

    LOG_INFO(netconf_logger, NETCONF_GET_OPER_ITEM)
        .arg(request_xpath ? string(request_xpath) : string("NULL"));

    string const setXpath(string("/") + module_name + ":" +
                          TranslatorUniversal::rootNodes().at(module_name).at(0) +
                          "/network-ranges");
    parent->insert(sysrepo_get_session_->get_data(setXpath.c_str())->child());
    setAllStatistics<DHCP_SPACE_V6>(parent, session, setXpath + "/");
    setAllResources<DHCP_SPACE_V6>(parent, session, setXpath + "/");
    prepareAndSetActiveLeases(parent, session, module_name, setXpath);

    LOG_INFO(netconf_logger, NETCONF_GET_OPER_ITEM_SUCCESS).arg(setXpath);
    return SR_ERR_OK;
}

template <>
int OperationalCallback::setOperationalData<DHCP_SPACE_V4>(S_Session session,
                                                           const char* module_name,
                                                           const char* /* path */,
                                                           const char* request_xpath,
                                                           uint32_t /* request_id */,
                                                           S_Data_Node& parent) {

    LOG_INFO(netconf_logger, NETCONF_GET_OPER_ITEM)
        .arg(request_xpath ? string(request_xpath) : string("NULL"));

    string const setXpath(string("/") + module_name + ":" +
                          TranslatorUniversal::rootNodes().at(module_name).at(0));
    parent = sysrepo_get_session_->get_data(setXpath.c_str());
    setAllStatistics<DHCP_SPACE_V4>(parent, session, setXpath + "/");
    setAllResources<DHCP_SPACE_V4>(parent, session, setXpath + "/");

    LOG_INFO(netconf_logger, NETCONF_GET_OPER_ITEM_SUCCESS).arg(setXpath);
    return SR_ERR_OK;
}

template void OperationalCallback::setAllResources<DHCP_SPACE_V6>(S_Data_Node&, S_Session&, string);

template void OperationalCallback::setAllResources<DHCP_SPACE_V4>(S_Data_Node&, S_Session&, string);

template void
OperationalCallback::setAllStatistics<DHCP_SPACE_V6>(S_Data_Node&, S_Session&, string const&);

template void
OperationalCallback::setAllStatistics<DHCP_SPACE_V4>(S_Data_Node&, S_Session&, string const&);

}  // namespace netconf
}  // namespace isc

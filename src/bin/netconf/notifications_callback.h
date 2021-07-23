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

#ifndef BIN_NETCONF_NOTIFICATIONS_CALLBACK_H
#define BIN_NETCONF_NOTIFICATIONS_CALLBACK_H

#include <cc/command_interpreter.h>
#include <config/command_mgr.h>
#include <netconf/configuration_callback.h>
#include <netconf/control_socket.h>
#include <netconf/netconf_config.h>
#include <netconf/netconf_log.h>
#include <util/dt_thread_pool.h>

#include <sysrepo-cpp/Session.hpp>

namespace isc {
namespace netconf {

template <isc::dhcp::DhcpSpaceType D>
struct NotificationsCallback {
    NotificationsCallback(CfgModelPtr const& model_config) : model_config_(model_config) {
        isc::config::CommandMgr::instance().openCommandSocket(
            model_config->getNotifications().control_socket_->toElement());
        registerCommands();
    }

private:
    isc::data::ElementPtr
    requestConfigurationHandle([[maybe_unused]] std::string const& command,
                               [[maybe_unused]] isc::data::ElementPtr const& data) {
        CfgSubscription const& configuration_socket(model_config_->getConfiguration());
        isc::dhcp::DhcpSpaceType const dhcp_space(model_config_->getDhcpSpace());
        std::string const& model(model_config_->getModel());

        // If the associated model is not specified...
        if (model.empty()) {
            // Error out.
            LOG_ERROR(netconf_logger, NETCONF_INIT_CONFIG_FAILED)
                .arg(magic_enum::enum_name(dhcp_space))
                .arg(PRETTY_METHOD_NAME() + ": empty model");
            return nullptr;
        }

        LOG_INFO(netconf_logger, NETCONF_SET_CONFIG_STARTED).arg(magic_enum::enum_name(dhcp_space));

        bool first(true);
        while (true) {
            // Don't sleep on first iteration.
            if (first) {
                first = false;
            } else {
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(4s);
            }

            // Retrieve configuration from Sysrepo.
            sysrepo::S_Connection connection(std::make_shared<sysrepo::Connection>());
            sysrepo::S_Session session(
                std::make_shared<sysrepo::Session>(connection, SR_DS_STARTUP));
            isc::yang::TranslatorUniversal translator(session, model);
            isc::data::ElementPtr config(translator.get());
            if (!config) {
                // Retry for empty configuration until we have some configuration.
                LOG_ERROR(netconf_logger, NETCONF_ERROR)
                    .arg("request-configuration: YANG configuration for " + model + " is empty");
                continue;
            }

            LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_SET_CONFIG)
                .arg(magic_enum::enum_name(dhcp_space))
                .arg("\n" + prettyPrint(config));

            // Convert it to Kea configuration. This is a bold supposition. Socket might
            // work with IETF or distributed. Support would need to be added for those.
            ConfigurationCallback<D>::template convert<isc::yang::IETF_to_Kea>(config, model);

            // Push to socket.
            ControlSocketBasePtr<D> const& comm(
                ControlSocketFactory<D>::create(configuration_socket, model));
            comm->prepareForConfigSet(config, {}, {}, {});
            isc::data::ElementPtr answer;
            int rcode(isc::config::CONTROL_RESULT_EMPTY);
            try {
                answer = comm->configSet(config, {}, {}, {});
                isc::config::parseAnswer(rcode, answer);
            } catch (std::exception const& exception) {
                // Socket end is probably offline so retry until it comes online.
                LOG_ERROR(netconf_logger, NETCONF_ERROR)
                    .arg(std::string("request-configuration: config-set command failed with ") +
                         exception.what());
                continue;
            }

            if (rcode != isc::config::CONTROL_RESULT_SUCCESS) {
                // Rettry until we get a successful answer.
                std::ostringstream msg;
                msg << "request-configuration: config-set command returned "
                    << (answer ? isc::config::answerToText(answer) : "no answer");
                LOG_ERROR(netconf_logger, NETCONF_SET_CONFIG_FAILED)
                    .arg(magic_enum::enum_name(dhcp_space))
                    .arg(msg.str());
                continue;
            }

            LOG_INFO(netconf_logger, NETCONF_INFO)
                .arg("request-configuration command succesfully handled");
            break;
        }
        return isc::data::Element::create("success");
    }

    void registerCommands() {
        isc::config::CommandMgr::instance().registerCommand(
            "request-configuration",
            std::bind(&NotificationsCallback<D>::requestConfigurationHandle, this,
                      std::placeholders::_1, std::placeholders::_2));

        // clang-format off
        //+---n address-pool-utilization-threshold-exceeded
        //|  +--ro pool-id                    -> /dhcpv6-server/network-ranges/network-range/address-pools/address-pool/pool-id
        //|  +--ro total-pool-addresses       uint64
        //|  +--ro max-allocated-addresses    uint64
        //|  +--ro allocated-address-count    uint64
        // clang-format on
        isc::config::CommandMgr::instance().registerCommand(
            "address-pool-utilization-threshold-exceeded",
            []([[maybe_unused]] std::string const& command,
               isc::data::ElementPtr const& data) -> isc::data::ElementPtr {
                constexpr size_t element_count(4);
                sysrepo::S_Vals const& values(std::make_shared<sysrepo::Vals>(element_count));

                size_t i(0);
                for (std::string s : std::array<std::string, element_count>{
                         {"pool-id", "total-pool-addresses", "max-allocated-addresses",
                          "allocated-address-count"}}) {
                    std::string const xpath(
                        "/ietf-dhcpv6-server:address-pool-utilization-threshold-exceeded/" + s);
                    values->val(i++)->set(xpath.c_str(), data->get(s)->intValue(), SR_UINT64_T);
                }
                sysrepo::S_Connection const& connection(std::make_shared<sysrepo::Connection>());
                sysrepo::Session session(connection, SR_DS_RUNNING);
                session.event_notif_send(
                    "/ietf-dhcpv6-server:address-pool-utilization-threshold-exceeded", values);
                return isc::config::createAnswer(
                    isc::config::CONTROL_RESULT_SUCCESS,
                    "address-pool-utilization-threshold-exceeded notification sent.");
            });

        // clang-format off
        //+---n prefix-pool-utilization-threshold-exceeded {prefix-delegation}?
        //|  +--ro pool-id                     -> /dhcpv6-server/network-ranges/network-range/prefix-pools/prefix-pool/pool-id
        //|  +--ro total-pool-prefixes         uint64
        //|  +--ro max-allocated-prefixes      uint64
        //|  +--ro allocated-prefixes-count    uint64

        // clang-format on
        isc::config::CommandMgr::instance().registerCommand(
            "prefix-pool-utilization-threshold-exceeded",
            []([[maybe_unused]] std::string const& command,
               isc::data::ElementPtr const& data) -> isc::data::ElementPtr {
                constexpr size_t element_count(4);
                sysrepo::S_Vals const& values(std::make_shared<sysrepo::Vals>(element_count));

                size_t i(0);
                for (std::string s : std::array<std::string, element_count>{
                         {"pool-id", "total-pool-prefixes", "max-allocated-prefixes",
                          "allocated-prefixes-count"}}) {
                    std::string const xpath(
                        "/ietf-dhcpv6-server:prefix-pool-utilization-threshold-exceeded/" + s);
                    values->val(i++)->set(xpath.c_str(), data->get(s)->intValue(), SR_UINT64_T);
                }
                sysrepo::S_Connection const& connection(std::make_shared<sysrepo::Connection>());
                sysrepo::Session session(connection, SR_DS_RUNNING);
                session.event_notif_send(
                    "/ietf-dhcpv6-server:prefix-pool-utilization-threshold-exceeded", values);
                return isc::config::createAnswer(
                    isc::config::CONTROL_RESULT_SUCCESS,
                    "prefix-pool-utilization-threshold-exceeded notification sent.");
            });

        // +---n invalid-client-detected
        // |  +--ro duid?          binary
        // |  +--ro description?   string
        isc::config::CommandMgr::instance().registerCommand(
            "invalid-client-detected",
            []([[maybe_unused]] std::string const& command,
               isc::data::ElementPtr const& data) -> isc::data::ElementPtr {
                constexpr size_t element_count(1);
                sysrepo::S_Vals const& values(std::make_shared<sysrepo::Vals>(element_count));

                values->val(0)->set("/ietf-dhcpv6-server:invalid-client-detected/duid",
                                    data->get("duid")->stringValue().c_str(), SR_BINARY_T);

                sysrepo::S_Connection const& connection(std::make_shared<sysrepo::Connection>());
                sysrepo::Session session(connection, SR_DS_RUNNING);
                session.event_notif_send("/ietf-dhcpv6-server:invalid-client-detected", values);
                return isc::config::createAnswer(isc::config::CONTROL_RESULT_SUCCESS,
                                                 "invalid-client-detected notification sent.");
            });

        // +---n decline-received
        // |  +--ro duid?                 binary
        // |  +--ro declined-resources* []
        // |     +--ro (resource-type)?
        // |        +--:(declined-address)
        // |        |  +--ro address?   inet:ipv6-address
        // |        +--:(declined-prefix)
        // |           +--ro prefix?    inet:ipv6-prefix
        isc::config::CommandMgr::instance().registerCommand(
            "decline-received",
            []([[maybe_unused]] std::string const& command,
               isc::data::ElementPtr const& data) -> isc::data::ElementPtr {
                constexpr size_t element_count(2);
                sysrepo::S_Vals const& values(std::make_shared<sysrepo::Vals>(element_count));
                values->val(0)->set("/ietf-dhcpv6-server:decline-received/duid",
                                    data->get("duid")->stringValue().c_str(), SR_BINARY_T);
                values->val(1)->set(
                    "/ietf-dhcpv6-server:decline-received/declined-resources/address",
                    data->get("declined-address")->stringValue().c_str(), SR_STRING_T);
                sysrepo::S_Connection const& connection(std::make_shared<sysrepo::Connection>());
                sysrepo::Session session(connection, SR_DS_RUNNING);
                session.event_notif_send("/ietf-dhcpv6-server:decline-received", values);
                return isc::config::createAnswer(isc::config::CONTROL_RESULT_SUCCESS,
                                                 "decline-received notification sent.");
            });

        // +---n non-success-code-sent
        //    +--ro status-code    uint16
        //    +--ro duid?          binary
        isc::config::CommandMgr::instance().registerCommand(
            "non-success-code-sent",
            []([[maybe_unused]] std::string const& command,
               isc::data::ElementPtr const& data) -> isc::data::ElementPtr {
                constexpr size_t element_count(2);
                sysrepo::S_Vals const& values(std::make_shared<sysrepo::Vals>(element_count));

                values->val(0)->set("/ietf-dhcpv6-server:non-success-code-sent/status-code",
                                    data->get("status-code")->intValue(), SR_UINT16_T);
                values->val(1)->set("/ietf-dhcpv6-server:non-success-code-sent/duid",
                                    data->get("duid")->stringValue().c_str(), SR_BINARY_T);
                sysrepo::S_Connection const& connection(std::make_shared<sysrepo::Connection>());
                sysrepo::Session session(connection, SR_DS_RUNNING);
                session.event_notif_send("/ietf-dhcpv6-server:non-success-code-sent", values);
                return isc::config::createAnswer(isc::config::CONTROL_RESULT_SUCCESS,
                                                 "non-success-code-sent notification sent.");
            });
    }

public:
    void event_notif([[maybe_unused]] sysrepo::S_Session session,
                     [[maybe_unused]] sr_ev_notif_type_t const notification_type,
                     [[maybe_unused]] char const* path,
                     sysrepo::S_Vals const vals,
                     [[maybe_unused]] time_t timestamp) noexcept {
        LOG_INFO(netconf_logger, NETCONF_INFO).arg("======== notification received ========");
        for (size_t i(0); i < vals->val_cnt(); ++i) {
            LOG_INFO(netconf_logger, NETCONF_INFO).arg(vals->val(i)->xpath());
        }
    }

    void event_notif_tree([[maybe_unused]] sysrepo::S_Session session,
                          [[maybe_unused]] sr_ev_notif_type_t const notif_type,
                          [[maybe_unused]] libyang::S_Data_Node const notification,
                          [[maybe_unused]] time_t timestamp) noexcept {
        LOG_INFO(netconf_logger, NETCONF_INFO).arg("======== notification tree received ========");
        LOG_INFO(netconf_logger, NETCONF_INFO).arg(notification->print_mem(LYD_XML, LYP_FORMAT));
    }

    CfgModelPtr model_config_;
};

}  // namespace netconf
}  // namespace isc

#endif  // BIN_NETCONF_NOTIFICATIONS_CALLBACK_H

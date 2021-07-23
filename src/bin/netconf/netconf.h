// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file netconf.h

#ifndef NETCONF_H
#define NETCONF_H

#include <cc/data.h>
#include <dhcpsrv/configuration.h>
#include <dhcpsrv/hook_library_manager.h>
#include <netconf/changes.h>
#include <netconf/configuration_callback.h>
#include <netconf/control_socket.h>
#include <netconf/control_socket_factory.h>
#include <netconf/http_control_socket.h>
#include <netconf/netconf_cfg_mgr.h>
#include <netconf/netconf_log.h>
#include <netconf/stdout_control_socket.h>
#include <netconf/unix_control_socket.h>
#include <util/dt_thread_pool.h>
#include <util/functional.h>
#include <yang/converter.h>
#include <yang/translator_universal.h>

#include <sysrepo-cpp/Session.hpp>

#include <map>
#include <memory>
#include <mutex>

namespace isc {
namespace netconf {

/// @brief Forward declaration to the @c NetconfAgent.
struct NetconfAgent;

/// @brief Netconf agent.
///
/// Service performed by the Netconf agent:
///  - at boot get and display Kea server configurations.
///  - load Kea server configurations from YANG datastore.
///  - validate YANG datastore changes using Kea configuration test.
///  - load updated Kea server configurations from YANG datastore.
///  - on shutdown close subscriptions.
struct NetconfAgent {
    using S_Connection = sysrepo::S_Connection;
    using S_Session = sysrepo::S_Session;
    using S_Subscribe = sysrepo::S_Subscribe;
    using S_Val = sysrepo::S_Val;

    /// @brief Constructor
    NetconfAgent() = default;

    /// @brief Destructor (call clear)
    virtual ~NetconfAgent() {
        clear();
    }

    /// @brief Clear.
    ///
    /// Close subscriptions and sysrepo.
    void clear();

    /// @brief Initialization.
    ///
    /// Check available modules / revisions.
    /// Get and display Kea server configurations.
    /// Load Kea server configurations from YANG datastore.
    /// Subscribe configuration changes in YANG datastore.
    ///
    /// If @c NetconfProcess::global_shut_down_flag becomes true
    /// returns as soon as possible.
    ///
    /// @param cfg_mgr The configuration manager (can be null).
    void init(NetconfCfgMgrPtr const& cfg_mgr);

    /// @brief Initialize sysrepo sessions.
    ///
    /// Must be called before init. Collect the list of available
    /// modules with their revisions.
    void initSysrepo();

protected:
    /// @brief Retrieve Kea server configuration from the YANG startup
    ///        datastore and applies it to servers.
    ///
    /// This method retrieves the configuation from sysrepo first, then
    /// established control socket connection to Kea servers (currently
    /// dhcp4 and/or dhcp6) and then attempts to send configuration
    /// using config-set.
    ///
    /// If boot-update is set to false, this operation is a no-op.
    ///
    /// @param model_config The service name and configuration pair.
    ///
    /// @{
    template <isc::dhcp::DhcpSpaceType D>
    void onBootUpdateSocket(const CfgModelPtr& model_config) {
        // The boot-update & subscribe-changes flags are prerequisites.
        if (!model_config->getConfiguration().on_boot_update_socket_ ||
            !model_config->getConfiguration().subscribe_) {
            return;
        }

        std::string const& model(model_config->getModel());
        isc::dhcp::DhcpSpaceType const dhcp_space(model_config->getDhcpSpace());

        // If the associated model is not specified...
        if (model.empty()) {
            // Error out.
            LOG_ERROR(netconf_logger, NETCONF_INIT_CONFIG_FAILED)
                .arg(magic_enum::enum_name(dhcp_space))
                .arg(PRETTY_METHOD_NAME() + ": empty model");
            return;
        }

        LOG_INFO(netconf_logger, NETCONF_SET_CONFIG_STARTED).arg(magic_enum::enum_name(dhcp_space));

        // Using a thread pool here would mean that we see an end in sight for this thread so that
        // it can join gracefully in the end. That is rather false e.g. when socket end never starts
        // and asubsequent SIGINT wants to end the current kea-netconf process, it will result in
        // hangingin the thread pool destructor followed by undefined behavior caused by objects
        // already being destructed. So rather than doing some unorthodox forced thread termination
        // in the thread pool or passing the SIGINT to the thread, let's just start a detached
        // thread in which case the thread and the process will end immediately.
        std::thread(
            exceptionLoggedBind(
                netconf_logger, NETCONF_ERROR, PRETTY_METHOD_NAME(),
                [=, this] {
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
                        isc::yang::TranslatorUniversal translator(startup_session_, model);
                        isc::data::ElementPtr config(translator.get());
                        if (!config) {
                            // Retry for empty configuration until we have some configuration.
                            LOG_ERROR(netconf_logger, NETCONF_ERROR)
                                .arg("onBootUpdateSocket(): YANG configuration for " + model +
                                     " is empty");
                            continue;
                        }

                        LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_SET_CONFIG)
                            .arg(magic_enum::enum_name(dhcp_space))
                            .arg("\n" + prettyPrint(config));

                        // Convert it to Kea configuration. This is a bold supposition. Socket might
                        // work with IETF. Support would need to be added for those.
                        ConfigurationCallback<D>::template convert<isc::yang::IETF_to_Kea>(config,
                                                                                           model);

                        // Push to socket.
                        ControlSocketBasePtr<D> const& comm(ControlSocketFactory<D>::create(
                            model_config->getConfiguration(), model));
                        comm->prepareForConfigSet(config, {}, {}, {});
                        isc::data::ElementPtr answer;
                        int rcode(isc::config::CONTROL_RESULT_EMPTY);
                        try {
                            answer = comm->configSet(config, {}, {}, {});
                            isc::config::parseAnswer(rcode, answer);
                        } catch (std::exception const& exception) {
                            // Socket end is probably offline so retry until it comes online.
                            LOG_ERROR(netconf_logger, NETCONF_ERROR)
                                .arg(std::string(
                                         "onBootUpdateSocket(): config-set command failed with ") +
                                     exception.what());
                            continue;
                        }

                        if (rcode != isc::config::CONTROL_RESULT_SUCCESS) {
                            // Rettry until we get a successful answer.
                            std::ostringstream msg;
                            msg << "onBootUpdateSocket(): config-set command returned "
                                << (answer ? isc::config::answerToText(answer) : "no answer");
                            LOG_ERROR(netconf_logger, NETCONF_SET_CONFIG_FAILED)
                                .arg(magic_enum::enum_name(dhcp_space))
                                .arg(msg.str());
                            continue;
                        }

                        LOG_INFO(netconf_logger, NETCONF_BOOT_UPDATE_COMPLETED)
                            .arg(magic_enum::enum_name(dhcp_space));
                        break;
                    }
                }))
            .detach();
    }

    template <isc::dhcp::DhcpSpaceType D>
    void onBootUpdateSysrepo(CfgModelPtr const& model_config) {
        // The boot-update & subscribe-changes flags are prerequisites.
        if (!model_config->getConfiguration().on_boot_update_sysrepo_ ||
            !model_config->getConfiguration().subscribe_) {
            return;
        }

        std::string const& model(model_config->getModel());
        isc::dhcp::DhcpSpaceType const dhcp_space(model_config->getDhcpSpace());

        // If the associated model is not specified...
        if (model.empty()) {
            // Error out.
            LOG_ERROR(netconf_logger, NETCONF_INIT_CONFIG_FAILED)
                .arg(magic_enum::enum_name(dhcp_space))
                .arg(PRETTY_METHOD_NAME() + ": empty model");
            return;
        }

        LOG_INFO(netconf_logger, NETCONF_SET_CONFIG_STARTED).arg(magic_enum::enum_name(dhcp_space));

        // Using a thread pool here would mean that we see an end in sight for this thread so that
        // it can join gracefully in the end. That is rather false e.g. when socket end never starts
        // and asubsequent SIGINT wants to end the current kea-netconf process, it will result in
        // hangingin the thread pool destructor followed by undefined behavior caused by objects
        // already being destructed. So rather than doing some unorthodox forced thread termination
        // in the thread pool or passing the SIGINT to the thread, let's just start a detached
        // thread in which case the thread and the process will end immediately.
        std::thread(
            exceptionLoggedBind(
                netconf_logger, NETCONF_ERROR, PRETTY_METHOD_NAME(),
                [=, this] {
                    bool first(true);
                    while (true) {
                        // Don't sleep on first iteration.
                        if (first) {
                            first = false;
                        } else {
                            using namespace std::chrono_literals;
                            std::this_thread::sleep_for(4s);
                        }

                        isc::data::ElementPtr comm_config;
                        int comm_rcode;

                        // Read from socket.
                        isc::data::ElementPtr config(isc::data::Element::createMap());
                        ControlSocketBasePtr<D> const& comm(ControlSocketFactory<D>::create(
                            model_config->getConfiguration(), model));
                        try {
                            std::tie(comm_rcode, comm_config) = comm->configGetParsed({}, nullptr);
                        } catch (std::exception const& exception) {
                            // Socket end is probably offline so retry until it comes online.
                            LOG_ERROR(netconf_logger, NETCONF_ERROR)
                                .arg(std::string(
                                         "onBootUpdateSysrepo(): config-get command failed with ") +
                                     exception.what());
                            continue;
                        }
                        if (comm_rcode == isc::config::CONTROL_RESULT_SUCCESS) {
                            config = comm_config;
                        } else {
                            LOG_WARN(netconf_logger, NETCONF_INIT_CONFIG_FAILED)
                                .arg(magic_enum::enum_name(dhcp_space))
                                .arg("onBootUpdateSysrepo(): configGetParsed(): return_code == " +
                                     std::to_string(comm_rcode));
                        }

                        // Push to sysrepo.
                        ConfigurationCallback<D> callback(model_config);
                        callback.pushDirectlyToSysrepoAsynchronously(
                            model, config, ConfigurationCallback<D>::FRESH_PUSH, SR_DS_STARTUP);

                        LOG_INFO(netconf_logger, NETCONF_INIT_CONFIG_COMPLETED).arg(model);
                        break;
                    }
                }))
            .detach();
    }
    /// @}

    static void setAllDefaults(isc::data::ElementPtr& element);

    /// @brief Subscribe changes for a module in YANG datastore.
    ///
    /// @param model_config subscription configuration
    template <isc::dhcp::DhcpSpaceType D>
    void subscribeConfig(CfgModelPtr const& model_config);

    /// @brief Subscribe to notifications coming from the paired kea-dhcp6.
    ///
    /// @param model_config subscription configuration
    template <isc::dhcp::DhcpSpaceType D>
    void subscribeNotifications(CfgModelPtr const& model_config);

    /// @brief Subscribe for providing operational data on request.
    ///
    /// @param model_config subscription configuration
    template <isc::dhcp::DhcpSpaceType D>
    void subscribeOperational(CfgModelPtr const& model_config);

    void subscribeRpc(CfgModelPtr const& model_config);

    /// @brief Sysrepo connection
    S_Connection connection_;

    /// @brief Sysrepo startup datastore session
    S_Session startup_session_;

    /// @brief Sysrepo running datastore session
    S_Session running_session_;

    /// @brief Available modules and revisions in Sysrepo.
    std::map<const std::string, const std::string> modules_;

    /// @brief Subscription map
    std::vector<S_Subscribe> subscriptions_;
};

}  // namespace netconf
}  // namespace isc

#endif  // NETCONF_H

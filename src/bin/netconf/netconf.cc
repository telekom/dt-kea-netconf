// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file netconf.cc
/// Contains the Netconf agent methods.

#include <config.h>

#include <cc/command_interpreter.h>
#include <database/db_exceptions.h>
#include <dhcpsrv/parsers/simple_parser4.h>
#include <dhcpsrv/parsers/simple_parser6.h>
#include <kea_config_tool/controller.h>
#include <netconf/configuration_callback.h>
#include <netconf/control_socket_factory.h>
#include <netconf/netconf.h>
#include <netconf/netconf_controller.h>
#include <netconf/netconf_log.h>
#include <netconf/notifications_callback.h>
#include <netconf/operational_callback.h>
#include <netconf/rpc_callback.h>
#include <util/func.h>
#include <util/magic_enum.hpp>
#include <util/strutil.h>

using namespace isc::config;
using namespace isc::data;
using namespace isc::netconf;
using namespace isc::util;
using namespace std;

using sysrepo::Connection;
using sysrepo::S_Session;
using sysrepo::Session;
using sysrepo::Subscribe;

using isc::Unexpected;
using isc::dhcp::DHCP_SPACE_V4;
using isc::dhcp::DHCP_SPACE_V6;
using isc::dhcp::DhcpSpaceType;
using isc::dhcp::SimpleParser4;
using isc::dhcp::SimpleParser6;
using isc::yang::TranslatorUniversal;

namespace isc {
namespace netconf {

void NetconfAgent::clear() {
    for (S_Subscribe& subscription : subscriptions_) {
        subscription.reset();
    }
    subscriptions_.clear();
    running_session_.reset();
    startup_session_.reset();
    connection_.reset();
}

void NetconfAgent::init(NetconfCfgMgrPtr const& cfg_mgr) {
    // Abort if there is no configuration manager.
    if (!cfg_mgr) {
        return;
    }

    // Initialize sysrepo interface.
    initSysrepo();

    // For all servers...
    CfgModelsListPtr const& models(cfg_mgr->getNetconfConfig()->getModelConfigs());
    for (CfgModelPtr const& model : *models) {
        DhcpSpaceType dhcp_space(model->getDhcpSpace());
        if (dhcp_space == DHCP_SPACE_V4) {
            onBootUpdateSysrepo<DHCP_SPACE_V4>(model);
            onBootUpdateSocket<DHCP_SPACE_V4>(model);
            subscribeOperational<DHCP_SPACE_V4>(model);
            subscribeConfig<DHCP_SPACE_V4>(model);
            subscribeNotifications<DHCP_SPACE_V4>(model);
        } else if (dhcp_space == DHCP_SPACE_V6) {
            onBootUpdateSysrepo<DHCP_SPACE_V6>(model);
            onBootUpdateSocket<DHCP_SPACE_V6>(model);
            subscribeOperational<DHCP_SPACE_V6>(model);
            subscribeConfig<DHCP_SPACE_V6>(model);
            subscribeNotifications<DHCP_SPACE_V6>(model);
        } else {
            isc_throw(Unexpected, PRETTY_FUNCTION_NAME()
                                      << ": stumbled upon unhandled DHCP space '"
                                      << magic_enum::enum_name(dhcp_space) << "'");
        }
        subscribeRpc(model);
    }
}

void NetconfAgent::initSysrepo() {
    try {
        connection_ = make_shared<Connection>();
    } catch (const exception& exception) {
        isc_throw(Unexpected, PRETTY_METHOD_NAME()
                                  << ": can't connect to sysrepo: " << exception.what());
    }

    try {
        startup_session_ = make_shared<Session>(connection_, SR_DS_STARTUP);
        running_session_ = make_shared<Session>(connection_, SR_DS_RUNNING);
    } catch (const exception& exception) {
        isc_throw(Unexpected, PRETTY_METHOD_NAME()
                                  << ": can't establish a sysrepo session: " << exception.what());
    }
}

void NetconfAgent::setAllDefaults(ElementPtr& element) {
    if (!element->contains("masters")) {
        return;
    }
    ElementPtr const& masters(element->xpath("/config/masters"));
    for (ElementPtr const& master : masters->listValue()) {
        if (!master->contains("shards")) {
            continue;
        }
        ElementPtr const& shards(master->get("shards"));
        for (ElementPtr const& shard : shards->listValue()) {
            if (!shard->contains("shard-config")) {
                continue;
            }
            ElementPtr const& shard_config(shard->get("shard-config"));
            if (shard_config->contains("Dhcp4")) {
                ElementPtr const& dhcp(shard_config->get("Dhcp4"));
                SimpleParser4::setAllDefaults(dhcp);
            }
            if (shard_config->contains("Dhcp6")) {
                ElementPtr const& dhcp(shard_config->get("Dhcp6"));
                SimpleParser6::setAllDefaults(dhcp);
            }
        }
    }
}

template <DhcpSpaceType D>
void NetconfAgent::subscribeConfig(CfgModelPtr const& model_config) {
    string const& model(model_config->getModel());
    DhcpSpaceType const dhcp_space(model_config->getDhcpSpace());

    // If the subscribe-changes flag is not set or the model associated with it is not
    // specified.
    if (!model_config->getConfiguration().subscribe_) {
        return;
    }

    LOG_INFO(netconf_logger, NETCONF_SUBSCRIBE_CONFIG)
        .arg(magic_enum::enum_name(dhcp_space))
        .arg(model);
    S_Subscribe subscription(make_shared<Subscribe>(running_session_));
    sr_subscr_options_t options(SR_SUBSCR_UPDATE);
    if (!model_config->getConfiguration().validate_) {
        options |= SR_SUBSCR_DONE_ONLY;
    }

    shared_ptr<ConfigurationCallback<D>> config_cb(
        make_shared<ConfigurationCallback<D>>(model_config, startup_session_));
    sysrepo::ModuleChangeCb callback =
        bind(&ConfigurationCallback<D>::module_change, config_cb, placeholders::_1,
             placeholders::_2, placeholders::_3, placeholders::_4, placeholders::_5);
    try {
        subscription->module_change_subscribe(model.c_str(), callback, nullptr, 0, options);
    } catch (exception const& exception) {
        LOG_ERROR(netconf_logger, NETCONF_SUBSCRIBE_CONFIG_FAILED)
            .arg(magic_enum::enum_name(dhcp_space))
            .arg(model)
            .arg(exception.what());
        subscription.reset();
        return;
    }

    subscriptions_.push_back(subscription);
}

template <DhcpSpaceType D>
void NetconfAgent::subscribeNotifications(CfgModelPtr const& model_config) {
    string const& model(model_config->getModel());
    DhcpSpaceType const dhcp_space(model_config->getDhcpSpace());

    // If the subscribe-changes flag is not set or the model associated with it is not
    // specified.
    if (!model_config->getNotifications().subscribe_) {
        return;
    }

    LOG_INFO(netconf_logger, NETCONF_SUBSCRIBE_NOTIFICATIONS)
        .arg(magic_enum::enum_name(dhcp_space))
        .arg(model);
    S_Subscribe subscription(make_shared<Subscribe>(running_session_));

    std::shared_ptr<NotificationsCallback<DHCP_SPACE_V4>> notif_cb(
        make_shared<NotificationsCallback<DHCP_SPACE_V4>>(model_config));
    sysrepo::EventNotifCb callback =
        bind(&NotificationsCallback<DHCP_SPACE_V4>::event_notif, notif_cb, placeholders::_1,
             placeholders::_2, placeholders::_3, placeholders::_4, placeholders::_5);
    try {
        subscription->event_notif_subscribe(model.c_str(), callback);
    } catch (exception const& exception) {
        LOG_ERROR(netconf_logger, NETCONF_SUBSCRIBE_NOTIFICATIONS_FAILED)
            .arg(magic_enum::enum_name(dhcp_space))
            .arg(model)
            .arg(exception.what());
        subscription.reset();
        return;
    }

    subscriptions_.push_back(subscription);
}

template <DhcpSpaceType D>
void NetconfAgent::subscribeOperational(CfgModelPtr const& model_config) {
    string const& model(model_config->getModel());

    /// @todo only one toplevel node is supported for now
    string const& root_node(TranslatorUniversal::rootNodes().at(model).at(0));

    string oper_xpath("/" + model + ":" + root_node);
    if (isc::yang::Is::Ietf(model)) {
        oper_xpath += "/network-ranges";
    }

    // Only ietf-dhcpv6 and kea-dhcpv4 models are supported
    if (!model_config->getOperational().subscribe_ || !isc::yang::Has::OperationalDataNodes(model))
        return;

    LOG_INFO(netconf_logger, NETCONF_SUBSCRIBE_OPER).arg(oper_xpath).arg(model);
    S_Subscribe oper_subs(make_shared<Subscribe>(running_session_));

    shared_ptr<OperationalCallback> oper_cb(
        make_shared<OperationalCallback>(running_session_, model_config));
    sysrepo::OperGetItemsCb callback =
        bind(&OperationalCallback::setOperationalData<D>, oper_cb, placeholders::_1, placeholders::_2,
             placeholders::_3, placeholders::_4, placeholders::_5, placeholders::_6);
    try {
        oper_subs->oper_get_items_subscribe(model.c_str(), callback, oper_xpath.c_str());
    } catch (exception const& exception) {
        LOG_ERROR(netconf_logger, NETCONF_SUBSCRIBE_OPER_FAILED)
            .arg(oper_xpath)
            .arg(model)
            .arg(exception.what());
        oper_subs.reset();
        return;
    }
    subscriptions_.push_back(oper_subs);
}

void NetconfAgent::subscribeRpc(CfgModelPtr const& model_config) {
    string const& model(model_config->getModel());

    static array<string, 2> const rpc_xpaths{("/" + model + ":delete-address-lease"),
                                             ("/" + model + ":delete-prefix-lease")};

    if (!model_config->getRpc().subscribe_ || !isc::yang::Is::Ietf(model)) {
        return;
    }

    S_Subscribe rpc_subscription(make_shared<Subscribe>(running_session_));
    shared_ptr<RpcCallback> rpc_cb(make_shared<RpcCallback>(model_config));
    sysrepo::RpcCb callback =
        bind(&RpcCallback::rpc, rpc_cb, placeholders::_1, placeholders::_2, placeholders::_3,
             placeholders::_4, placeholders::_5, placeholders::_6);

    for (auto const& rpc_xpath : rpc_xpaths) {
        LOG_INFO(netconf_logger, NETCONF_SUBSCRIBE_RPC).arg(rpc_xpath).arg(model);

        try {
            rpc_subscription->rpc_subscribe(rpc_xpath.c_str(), callback);
        } catch (exception const& exception) {
            LOG_ERROR(netconf_logger, NETCONF_SUBSCRIBE_RPC_FAILED)
                .arg(rpc_xpath)
                .arg(model)
                .arg(exception.what());
            rpc_subscription.reset();
            return;
        }
    }

    subscriptions_.push_back(rpc_subscription);
}

}  // namespace netconf
}  // namespace isc

// Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <cc/command_interpreter.h>
#include <cc/simple_parser.h>
#include <exceptions/exceptions.h>
#include <hooks/libinfo.h>
#include <netconf/netconf_cfg_mgr.h>
#include <netconf/netconf_log.h>
#include <netconf/simple_parser.h>

using std::ostringstream;
using std::string;

using namespace isc::config;
using namespace isc::dhcp;
using namespace isc::process;
using namespace isc::data;
using namespace isc::hooks;
using isc::Exception;

namespace isc {
namespace netconf {

NetconfConfig::NetconfConfig()
    : configured_globals_(Element::createMap()), models_list_(new CfgModelsList()) {
}

NetconfConfig::NetconfConfig(const NetconfConfig& orig)
    : ConfigBase(), configured_globals_(orig.configured_globals_), models_list_(orig.models_list_),
      hooks_config_(orig.hooks_config_) {
}

void NetconfConfig::extractConfiguredGlobals(ElementPtr config) {
    for (auto const& [key, value] : config->mapValue()) {
        if (value->getType() != Element::list && value->getType() != Element::map) {
            addConfiguredGlobal(key, value);
        }
    }
}

NetconfCfgMgr::NetconfCfgMgr() : DCfgMgrBase(ConfigPtr(new NetconfConfig())) {
}

NetconfCfgMgr::~NetconfCfgMgr() = default;

string NetconfCfgMgr::getConfigSummary(const uint32_t /*selection*/) {

    NetconfConfigPtr ctx = getNetconfConfig();

    // No globals to print.
    ostringstream s;

    // Then print models.
    for (CfgModelPtr const& model : *ctx->getModelConfigs()) {
        if (s.tellp() != 0) {
            s << " ";
        }
        s << magic_enum::enum_name(model->getDhcpSpace());
    }

    if (s.tellp() == 0) {
        s << "none";
    }

    // Finally, print the hook libraries names
    const HookLibsCollection libs = ctx->getHooksConfig().get();
    s << ", " << libs.size() << " lib(s):";
    for (auto const& lib : libs) {
        s << get<LIBRARY>(lib) << " ";
    }

    return s.str();
}

ConfigPtr NetconfCfgMgr::createNewContext() {
    return ConfigPtr(new NetconfConfig());
}

ElementPtr NetconfCfgMgr::parse(ElementPtr config_set, bool check_only) {
    // Do a sanity check first.
    if (!config_set) {
        isc_throw(DhcpConfigError, "Mandatory config parameter not provided");
    }

    NetconfConfigPtr ctx = getNetconfConfig();

    // Preserve all scalar global parameters.
    ctx->extractConfiguredGlobals(config_set);

    // Set the defaults and derive parameters.
    NetconfSimpleParser::setAllDefaults(config_set);
    NetconfSimpleParser::deriveParameters(config_set);

    // And parse the configuration.
    ElementPtr answer;
    string excuse;
    try {
        // Do the actual parsing
        NetconfSimpleParser parser;
        parser.parse(ctx, config_set, check_only);
    } catch (const Exception& ex) {
        excuse = ex.what();
        answer = createAnswer(CONTROL_RESULT_ERROR, excuse);
    } catch (...) {
        excuse = "undefined configuration parsing error";
        answer = createAnswer(CONTROL_RESULT_ERROR, excuse);
    }

    // At this stage the answer was created only in case of exception.
    if (answer) {
        if (check_only) {
            LOG_ERROR(netconf_logger, NETCONF_CONFIG_CHECK_FAIL).arg(excuse);
        } else {
            LOG_ERROR(netconf_logger, NETCONF_CONFIG_FAIL).arg(excuse);
        }
        return answer;
    }

    if (check_only) {
        answer = createAnswer(CONTROL_RESULT_SUCCESS, "Configuration check successful");
    } else {
        answer = createAnswer(CONTROL_RESULT_SUCCESS, "Configuration applied successfully.");
    }

    return answer;
}

ElementPtr NetconfConfig::toElement() const {
    ElementPtr netconf = ConfigBase::toElement();
    // Set user-context
    contextToElement(netconf);
    // Add in explicitly configured globals.
    netconf->setValue(configured_globals_->mapValue());
    // Set hooks-libraries
    netconf->set("hooks-libraries", hooks_config_.toElement());
    // Set models
    ElementPtr const& models(Element::createList());
    for (CfgModelPtr const& model : *models_list_) {
        models->add(model->toElement());
    }
    netconf->set("models", models);
    // Set Netconf
    ElementPtr const& result(Element::createMap());
    result->set("Netconf", netconf);
    return result;
}

}  // namespace netconf
}  // namespace isc

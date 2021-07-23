// Copyright (C) 2018-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <boost/foreach.hpp>
#include <cc/data.h>
#include <cc/dhcp_config_error.h>
#include <hooks/hooks_manager.h>
#include <hooks/hooks_parser.h>
#include <netconf/netconf_config.h>
#include <netconf/simple_parser.h>

using namespace isc::data;

using std::array;
using std::string;

namespace isc {
namespace netconf {
/// @brief This sets of arrays define the default values in various scopes
///        of the Netconf Configuration.
///
/// Each of those is documented in @file netconf/simple_parser.cc. This
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

/// @brief Defines defaults for control-socket elements.
const SimpleDefaults NetconfSimpleParser::CTRL_SOCK_DEFAULTS = {
    {"socket-type", Element::string, "stdout"},
    {"socket-name", Element::string, ""},
    {"process-args", Element::string, ""},
    {"socket-url", Element::string, "http://127.0.0.1:8000/"},
};

/// @brief Defines default values for subscription elements.
const SimpleDefaults NetconfSimpleParser::SUBSCRIPTION_DEFAULTS = {
    {"on-boot-update-socket", Element::boolean, "true"},
    {"on-boot-update-sysrepo", Element::boolean, "true"},
    {"subscribe", Element::boolean, "false"},
    {"validate", Element::boolean, "true"},
};
/// @brief List of parameters that can be inherited to models scope.
///
/// Some parameters may be defined on both global (directly in Netconf) and
/// servers (Netconf/models/...) scope. If not defined in the
/// models scope, the value is being inherited (derived) from
/// the global scope. This array lists all of such parameters.
const ParamsList NetconfSimpleParser::INHERIT_TO_MODELS = {};

/// @}

/// ---------------------------------------------------------------------------
/// --- end of default values -------------------------------------------------
/// ---------------------------------------------------------------------------

size_t NetconfSimpleParser::setAllDefaults(const ElementPtr& global) {
    size_t cnt = 0;

    ElementPtr models = global->get("models");
    if (models) {
        for (ElementPtr const& model : models->listValue()) {
            cnt += setModelDefaults(model);
        }
    }

    return cnt;
}

size_t NetconfSimpleParser::deriveParameters(ElementPtr global) {
    size_t cnt = 0;

    // Now derive global parameters into models.
    ElementPtr const& models(global->get("models"));
    if (models) {
        for (ElementPtr const& model : models->listValue()) {
            cnt += SimpleParser::deriveParams(global, model, INHERIT_TO_MODELS);
        }
    }

    return cnt;
}

size_t NetconfSimpleParser::setModelDefaults(ElementPtr const& model) {
    size_t cnt = 0;

    for (string const& key :
         array<string, 4>{"configuration", "notifications", "operational", "rpc"}) {
        ElementPtr const& subscription(model->get(key));
        if (subscription) {
            cnt += setDefaults(subscription, SUBSCRIPTION_DEFAULTS);
            ElementPtr const& control_socket(subscription->get("control-socket"));
            if (control_socket) {
                cnt += setDefaults(control_socket, CTRL_SOCK_DEFAULTS);
            }
        }
    }

    return cnt;
}

void NetconfSimpleParser::parse(const NetconfConfigPtr& ctx,
                                const ElementPtr& config,
                                bool check_only) {

    // User context can be done at anytime.
    ElementPtr user_context = config->get("user-context");
    if (user_context) {
        ctx->setContext(user_context);
    }

    // Get models.
    ElementPtr models = config->get("models");
    if (models) {
        for (ElementPtr const& model : models->listValue()) {
            ModelConfigParser model_parser;
            CfgModelPtr const& model_config(model_parser.parse(model));
            ctx->getModelConfigs()->push_back(model_config);
        }
    }

    // Finally, let's get the hook libs!
    using namespace isc::hooks;
    HooksConfig& libraries = ctx->getHooksConfig();
    ElementPtr hooks = config->get("hooks-libraries");
    if (hooks) {
        HooksLibrariesParser hooks_parser;
        hooks_parser.parse(libraries, hooks);
        libraries.verifyLibraries(hooks->getPosition());
    }

    if (!check_only) {
        // This occurs last as if it succeeds, there is no easy way
        // revert it.  As a result, the failure to commit a subsequent
        // change causes problems when trying to roll back.
        HooksManager::prepareUnloadLibraries();
        static_cast<void>(HooksManager::unloadLibraries());
        libraries.loadLibraries();
    }
}

}  // namespace netconf
}  // namespace isc

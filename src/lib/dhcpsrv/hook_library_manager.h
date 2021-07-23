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

#ifndef DHCPSRV_HOOK_LIBRARY_MANAGER_H
#define DHCPSRV_HOOK_LIBRARY_MANAGER_H

#include <dhcpsrv/configuration.h>
#include <dhcpsrv/shard_config_mgr.h>

namespace isc {
namespace dhcp {

template <DhcpSpaceType D, bool has_IETF>
struct HookLibraryConstants {
    static std::regex const& regex() {
        static std::regex const _(
            "/masters\\[name='(.*?)'\\]/shards\\[name='(.*?)'\\]/shard-config/" +
            ConfigurationElements<D, has_IETF>::toplevel() +
            "/hooks-libraries\\[library='(.*?)'\\]/"
            "(lawful-interception-parameters|policy-engine-parameters|"
            "parameters)()");
        return _;
    }

    static std::regex const& subnetRegex() {
        static std::regex const _(
            "/masters\\[name='(.*?)'\\]/shards\\[name='(.*?)'\\]/shard-config/" +
            ConfigurationElements<D, has_IETF>::toplevel() +
            "/hooks-libraries\\[library='(.*?)'\\]/"
            "(lawful-interception-parameters|policy-engine-parameters|"
            "parameters)/config/network-topology/subnets\\[subnet='(.*?)'\\]");
        return _;
    }

    static std::string const xpath(std::string const& master,
                                   std::string const& shard,
                                   std::string const& hook_library,
                                   std::string const& parameters_key,
                                   std::string const& subnet = std::string()) {
        std::string result(ShardConstants::xpath(master, shard) + "/" +
                           ConfigurationElements<D, has_IETF>::toplevel() +
                           "/hooks-libraries[library='" + hook_library + "']/" + parameters_key +
                           "/config");
        if (!subnet.empty()) {
            result += "/network-topology/subnets[subnet='" + subnet + "']";
        }
        return result;
    }

    static std::string const
    xpath(std::string const& master, std::string const& shard, std::string const& hook_library) {
        for (std::string const& p : {"lawful-interception", "policy-engine"}) {
            if (hook_library.find(p) != std::string::npos) {
                return xpath(master, shard, hook_library, p + "-parameters");
            }
        }
        return xpath(master, shard, hook_library, "parameters");
    }

    static std::vector<std::string> const
    xpaths(std::string const& master, std::string const& shard, std::string const& hook_library) {
        std::vector<std::string> result;
        for (std::string const& p : {"lawful-interception-parameters", "policy-engine-parameters",
                                     "parameters"}) {
            result.push_back(xpath(master, shard, hook_library, p));
        }
        return result;
    }

    static auto fromMatch(std::smatch const& match) {
        return std::make_tuple(std::string(match[1]), std::string(match[2]), std::string(match[3]),
                               std::string(match[4]), std::string(match[5]), std::string(match[6]));
    }

private:
    /// @brief static methods only
    HookLibraryConstants() = delete;

    /// @brief non-copyable
    /// @{
    HookLibraryConstants(HookLibraryConstants const&) = delete;
    HookLibraryConstants& operator=(HookLibraryConstants const&) = delete;
    /// @}
};

template <DhcpSpaceType D, bool has_IETF>
struct HookLibraryManager {
    static void removeParameters(isc::data::ElementPtr const& config) {
        // Sanity checks
        isc::data::ElementPtr const& dhcp(ConfigurationManager<D, has_IETF>::getDhcp(config));
        isc::data::ElementPtr const& hooks(dhcp->get("hooks-libraries"));
        if (!hooks) {
            return;
        }

        // Set parameters for all hooks.
        for (isc::data::ElementPtr const& hook : hooks->listValue()) {

            // Determine which YANG-backed parameters key should be changed.
            isc::data::ElementPtr parameters;
            for (std::string const& p :
                 {"lawful-interception-parameters", "policy-engine-parameters",
                  "parameters"}) {
                parameters = hook->get(p);
                if (parameters && !parameters->empty()) {
                    break;
                }
            }
            if (!parameters) {
                continue;
            }

            // Effective parameter removal
            if (parameters->contains("arguments")) {
                parameters->remove("arguments");
            }
            if (parameters->contains("command")) {
                parameters->remove("command");
            }
        }
    }

    static void setParameters(isc::data::ElementPtr const& config,
                              isc::data::ElementPtr const& command,
                              isc::data::ElementPtr const& arguments) {
        // Sanity checks
        isc::data::ElementPtr const& dhcp(ConfigurationManager<D, has_IETF>::getDhcp(config));
        isc::data::ElementPtr const& hooks(dhcp->get("hooks-libraries"));
        if (!hooks) {
            return;
        }

        // Set parameters for all hooks.
        for (isc::data::ElementPtr const& hook : hooks->listValue()) {
            // Determine which YANG-backed parameters key should be changed.
            isc::data::ElementPtr parameters;
            for (std::string const& p :
                 {"lawful-interception-parameters", "policy-engine-parameters",
                  "parameters"}) {
                parameters = hook->get(p);
                if (parameters && !parameters->empty()) {
                    break;
                }
            }
            if (!parameters) {
                continue;
            }

            // Effective parameter setting
            parameters->set("arguments", arguments);
            parameters->set("command", command);
        }
    }

private:
    /// @brief static methods only
    HookLibraryManager() = delete;

    /// @brief non-copyable
    /// @{
    HookLibraryManager(HookLibraryManager const&) = delete;
    HookLibraryManager& operator=(HookLibraryManager const&) = delete;
    /// @}
};

}  // namespace dhcp
}  // namespace isc

#endif  // DHCPSRV_HOOK_LIBRARY_MANAGER_H

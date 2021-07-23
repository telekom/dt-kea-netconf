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

#ifndef GRANULAR_CHANGE_CONTROL_H
#define GRANULAR_CHANGE_CONTROL_H

#include <cc/command_interpreter.h>
#include <cc/data.h>
#include <dhcpsrv/master_config_mgr.h>
#include <dhcpsrv/subnet_mgr.h>
#include <kea_config_tool/controller.h>
#include <netconf/control_socket.h>
#include <netconf/kea_config_tool_control.h>
#include <netconf/netconf_log.h>
#include <util/command.h>
#include <util/dhcp.h>
#include <util/func.h>
#include <util/magic_enum.hpp>
#include <util/strutil.h>
#include <util/tuple_hash.h>

namespace isc {
namespace netconf {

/// @brief Class for control socket communication over UNIX socket.
///
/// This class is the derived class for control socket communication
/// over UNIX sockets.
/// This class implements config-get, config-test, config-set and leases-get
template <isc::dhcp::DhcpSpaceType D, bool has_IETF>
struct GranularChangeManager : ControlSocketBase<D> {
    using double_key_t = std::tuple<std::string const, std::string const>;
    using triple_key_t = std::tuple<std::string const, std::string const, std::string const>;
    using quadruple_key_t =
        std::tuple<std::string const, std::string const, std::string const, std::string const>;
    using quintuple_key_t = std::tuple<std::string const,
                                       std::string const,
                                       std::string const,
                                       std::string const,
                                       std::string const>;

    using boolean_triple_key_t =
        std::tuple<bool, std::string const, std::string const, std::string const>;
    using boolean_quadruple_key_t = std::
        tuple<bool, std::string const, std::string const, std::string const, std::string const>;
    using boolean_quintuple_key_t = std::tuple<bool,
                                               std::string const,
                                               std::string const,
                                               std::string const,
                                               std::string const,
                                               std::string const>;
    using boolean_sextuple_key_t = std::tuple<bool,
                                              std::string const,
                                              std::string const,
                                              std::string const,
                                              std::string const,
                                              std::string const,
                                              std::string const>;

    using hook_library_effects_t = std::unordered_map<quintuple_key_t, isc::data::effect_t>;
    using master_effects_t = std::unordered_map<quadruple_key_t, isc::data::effect_t>;
    using shard_effects_t = std::unordered_map<double_key_t, isc::data::effect_t>;
    using subnet_effects_t = std::unordered_map<triple_key_t, isc::data::effect_t>;

    using hook_library_set_t = std::set<quintuple_key_t>;
    using master_set_t = std::set<quadruple_key_t>;
    using shard_set_t = std::set<double_key_t>;
    using subnet_set_t = std::set<triple_key_t>;

    /// @brief Constructor.
    ///
    /// @param ctrl_sock The control socket configuration.
    GranularChangeManager(CfgControlSocketPtr ctrl_sock, std::string const& model)
        : ControlSocketBase<D>(ctrl_sock),
          kea_config_tool_control_(std::make_shared<KeaConfigToolControl<D, has_IETF>>(ctrl_sock)),
          model_(model) {
    }

    /// @brief .*toDistributedConfiguration() methods take configuration elements pulled from
    /// Cassandra and conveniently place them inside a distributed configuration.
    /// @{
    template <typename T>
    bool
    hookLibrariesToDistributedConfiguration(T& retrieved_hook_libraries,
                                            isc::data::ElementPtr const& distributed_configuration,
                                            isc::util::Dir const& workspace,
                                            isc::data::ElementPtr const& config,
                                            std::string const& xpath) {
        // Does xpath match or has it been processed already?
        auto const& [match, master, shard, hook_library, parameters, subnet,
                     the_rest](matchesHookLibraries(xpath));
        auto const& tuple(std::make_tuple(master, shard, hook_library, parameters, subnet));
        if (!match) {
            return false;
        }

        if (retrieved_hook_libraries.contains(tuple)) {
            return true;
        }

        // Mark entities as resolved for external visibility.
        retrieved_hook_libraries.emplace(tuple);

        // Determine shard config.
        isc::data::ElementPtr const& shard_config(
            config->xpath(isc::dhcp::ShardConstants::xpath(master, shard)));

        // Set config.timestamp, command & arguments.
        isc::data::ElementPtr arguments(isc::data::Element::createMap());
        isc::util::Dir const& shard_directory(workspace() + "/" + master + "/" + shard);
        std::string const& config_timestamp_path(shard_directory() + "/config.timestamp");
        arguments->set("config.timestamp", config_timestamp_path);
        isc::dhcp::HookLibraryManager<D, has_IETF>::setParameters(
            shard_config, isc::data::Element::create("get-config"), arguments);

        // Configure and wait for the hook library to load and retrieve configuration.
        isc::kea_config_tool::VersionedController<D, has_IETF>::configureDummyDhcpServer(
            isc::dhcp::ConfigurationManager<D, has_IETF>::getDhcp(shard_config));

        // Cleanup arguments.
        isc::dhcp::HookLibraryManager<D, has_IETF>::removeParameters(shard_config);
        arguments->remove("config.timestamp");

        // Retrieve the hook library configuration.
        isc::data::ElementPtr const& retrieved_hook_library_configuration(
            config->xpath(isc::dhcp::HookLibraryConstants<D, has_IETF>::xpath(
                master, shard, hook_library, parameters)));

        // Update the hook library configuration.
        distributed_configuration->xpath(isc::dhcp::HookLibraryConstants<D, has_IETF>::xpath(
                                             master, shard, hook_library, parameters),
                                         retrieved_hook_library_configuration);

        if (!subnet.empty()) {
            // Retrieve the subnet configuration.
            isc::data::ElementPtr const& retrieved_subnet_configuration(
                config->xpath(isc::dhcp::HookLibraryConstants<D, has_IETF>::xpath(
                    master, shard, hook_library, parameters, subnet)));

            // Update the subnet configuration.
            distributed_configuration->xpath(isc::dhcp::HookLibraryConstants<D, has_IETF>::xpath(
                                                 master, shard, hook_library, parameters, subnet),
                                             retrieved_subnet_configuration);
        }

        return true;
    }

    template <typename T>
    bool mastersToDistributedConfiguration(T& retrieved_masters,
                                           isc::data::ElementPtr const& distributed_configuration,
                                           isc::util::Dir const& workspace,
                                           isc::data::ElementPtr const& sysrepo_config,
                                           std::string const& xpath) {
        // Does xpath match or has it been processed already?
        auto const& [match, master, shard, modified_element, instance_ID,
                     the_rest](matchesMasters(xpath));
        auto const& tuple(std::make_tuple(master, shard, modified_element, instance_ID));
        if (!match) {
            return false;
        }

        if (retrieved_masters.contains(tuple)) {
            return true;
        }

        // Mark entities as resolved for external visibility.
        retrieved_masters.emplace(tuple);

        // Configure credentials.
        isc::data::ElementPtr credentials(
            sysrepo_config->xpath(isc::dhcp::MasterConstants::credentials_xpath(master)));
        if (!credentials) {
            isc_throw(Unexpected, "no credentials for master '" << master << "'");
        }
        isc::data::ElementPtr enclosed_credentials(
            isc::dhcp::ConfigurationManager<D, has_IETF>::credentialsToDhcpEnclosed(credentials));

        isc::dhcp::ConfigurationManager<D, has_IETF>::configureCredentialsToStaging(
            isc::dhcp::ConfigurationManager<D, has_IETF>::getDhcp(enclosed_credentials));
        std::optional<std::string> instance_ID_optional(std::nullopt);
        if (!instance_ID.empty()) {
            instance_ID_optional = instance_ID;
        }
        isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createMasterCfgMgrs<D>(
            instance_ID_optional);

        isc::util::Dir master_directory(workspace() + "/" + master);
        if (instance_ID.empty()) {
            if (modified_element == "admin-credentials") {
                std::string&& instance_ID_mutable("kea-netconf@" + shard);
                isc::dhcp::MasterConfigPtr master_configuration(
                    isc::dhcp::MasterConfigMgrFactory<D>::instance().select(
                        instance_ID_mutable, master_directory() + "/config.timestamp"));
                if (!master_configuration) {
                    return true;
                }
                distributed_configuration->xpath(
                    isc::dhcp::MasterConstants::xpath(master, shard, "admin-credentials"),
                    master_configuration->database_credentials());
            } else if (modified_element == "config-database") {
                isc::dhcp::MasterConfigCollection collection(
                    isc::dhcp::MasterConfigMgrFactory<D>::instance().selectByShard(
                        shard, master_directory() + "/config.timestamp"));
                for (isc::dhcp::MasterConfigPtr const& master_configuration : collection) {
                    if (master_configuration->instance_ID_.find("kea-netconf@") !=
                        std::string::npos) {
                        continue;
                    }
                    distributed_configuration->xpath(
                        isc::dhcp::MasterConstants::xpath(master, shard, "config-database"),
                        master_configuration->database_credentials());
                }
            } else {
                isc_throw(Unexpected,
                          "stumbled upon unhandled modified element '" << modified_element << "'");
            }
        } else {
            std::string instance_ID_mutable(instance_ID);
            isc::dhcp::MasterConfigPtr master_configuration(
                isc::dhcp::MasterConfigMgrFactory<D>::instance().select(
                    instance_ID_mutable, master_directory() + "/config.timestamp"));
            if (!master_configuration) {
                return true;
            }
            distributed_configuration->xpath(
                isc::dhcp::MasterConstants::xpath(master, shard, "config-database"),
                master_configuration->database_credentials());
            distributed_configuration->xpath(
                isc::dhcp::MasterConstants::xpath(master, shard, "master-config", instance_ID),
                master_configuration->server_specific_configuration());
        }
        return true;
    }

    template <typename T>
    bool shardsToDistributedConfiguration(T& retrieved_shards,
                                          isc::data::ElementPtr const& distributed_configuration,
                                          isc::util::Dir const& workspace,
                                          isc::data::ElementPtr const& sysrepo_config,
                                          std::string const& xpath) {
        // Does xpath match or has it been processed already?
        auto const& [match, master, shard, the_rest](matchesShards(xpath));
        auto const& tuple(std::make_tuple(master, shard));
        if (!match) {
            return false;
        }

        if (retrieved_shards.contains(tuple)) {
            return true;
        }

        // Mark entities as resolved for external visibility.
        retrieved_shards.emplace(tuple);

        // Configure credentials.
        isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createShardCfgMgrs<D>(
            {sysrepo_config->xpath(isc::dhcp::ShardConstants::credentials_xpath(master, shard))});

        isc::util::Dir shard_directory(workspace() + "/" + master + "/" + shard);
        isc::dhcp::ShardConfigPtr shard_configuration(
            isc::dhcp::ShardConfigMgrFactory<D>::instance().select(shard_directory() +
                                                                   "/config.timestamp"));

        if (!shard_configuration) {
            return true;
        }

        /// @{
        /// Subnets need to be restored as they are enclosed by the shard configuration
        /// being set here.
        std::string xpath_for_all_subnets(
            isc::dhcp::SubnetConstants<D>::xpath_for_all(master, shard));
        isc::data::ElementPtr all_subnets(distributed_configuration->xpath(xpath_for_all_subnets));
        /// @}

        distributed_configuration->xpath(isc::dhcp::ShardConstants::xpath(master, shard),
                                         shard_configuration->configuration());

        // Restore here.
        distributed_configuration->xpath(xpath_for_all_subnets, all_subnets);
        return true;
    }

    template <typename T>
    bool subnetsToDistributedConfiguration(T& retrieved_subnets,
                                           isc::data::ElementPtr const& distributed_configuration,
                                           isc::util::Dir const& workspace,
                                           isc::data::ElementPtr const& sysrepo_config,
                                           std::string const& xpath) {
        // Does xpath match or has it been processed already?
        auto const& [match, master, shard, subnet, the_rest](matchesSubnets(xpath));
        auto const& tuple(std::make_tuple(master, shard, subnet));
        if (!match) {
            return false;
        }

        if (retrieved_subnets.contains(tuple)) {
            return true;
        }

        // Mark entities as resolved for external visibility.
        retrieved_subnets.emplace(tuple);

        // Configure credentials.
        isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createShardCfgMgrs<D>(
            {sysrepo_config->xpath(isc::dhcp::ShardConstants::credentials_xpath(master, shard))});

        isc::util::Dir shard_directory(workspace() + "/" + master + "/" + shard);

        isc::data::ElementPtr subnet_element(sysrepo_config->xpath(
            isc::dhcp::SubnetConstantsT<D, has_IETF>::subnetElementXpath(master, shard, subnet)));
        if (!subnet_element) {
            return true;
        }

        std::string subnet_mutable(subnet_element->stringValue());
        isc::dhcp::SubnetInfoPtr<D> subnet_info(
            isc::dhcp::SubnetMgrFactory<D>::instance().selectBySubnet(
                subnet_mutable, shard_directory() + "/config.timestamp"));

        if (!subnet_info) {
            return true;
        }

        distributed_configuration->xpath(
            isc::dhcp::SubnetConstants<D>::xpath(master, shard, subnet), subnet_info->toElement());
        return true;
    }
    /// @}

    /// @brief Get configuration.
    ///
    /// Call config-get over the control socket.
    ///
    /// @return The JSON element answer of config-get.
    isc::data::ElementPtr configGet(ChangeCollection const& changes,
                                    isc::data::ElementPtr const& sysrepo_config) override final {
        ControlSocketBase<D>::recreateFileHierarchy(
            sysrepo_config, isc::dhcp::ConfigurationConstants::ONLY_MASTER_INI);

        if (changes.empty()) {
            // Delegate.
            return kea_config_tool_control_->configGet(changes, sysrepo_config);
        }

        if (!sysrepo_config) {
            isc_throw(Unexpected, "!sysrepo_config");
        }

        // Retrieve directory list.
        isc::util::Dir const workspace(ControlSocketBase<D>::workspace_);
        std::vector<isc::util::FileInfo> const& master_directories(
            isc::util::Filesystem::directoriesAtPath(workspace()));

        // Iterate through masters.
        isc::data::ElementPtr const& distributed_configuration(isc::data::Element::createMap());
        isc::data::ElementPtr const& config(isc::data::Element::createMap());
        for (isc::util::FileInfo const& master_info : master_directories) {
            // Create directories and files.
            isc::data::ElementPtr const& master(
                isc::dhcp::ConfigurationManager<D, has_IETF>::masterIniToAdminCredentials(
                    workspace(), master_info.name_));
            isc::data::ElementPtr const& admin_credentials(master->get("admin-credentials"));
            if (admin_credentials) {
                std::string const& xpath(
                    isc::dhcp::MasterConstants::credentials_xpath(master_info.name_));
                distributed_configuration->xpath(xpath, admin_credentials);
            }

            std::vector<isc::util::FileInfo> const& shard_directories(
                isc::util::Filesystem::directoriesAtPath(workspace() + "/" + master_info.name_));
            for (isc::util::FileInfo const& shard_info : shard_directories) {
                isc::data::ElementPtr credentials_json(
                    isc::data::Element::fromJSONFileExceptionSafe(
                        shard_info.full_path_ + "/credentials.json", true));

                if (!credentials_json) {
                    continue;
                }

                isc::data::ElementPtr const& shard_admin_credentials(
                    credentials_json->get("config-database"));
                if (!admin_credentials) {
                    continue;
                }

                std::string const& shard_xpath(isc::dhcp::ShardConstants::credentials_xpath(
                    master_info.name_, shard_info.name_));
                distributed_configuration->xpath(shard_xpath, shard_admin_credentials);
            }
        }

        hook_library_set_t retrieved_hook_libraries;
        master_set_t retrieved_masters;
        shard_set_t retrieved_shards;
        subnet_set_t retrieved_subnets;
        std::set<std::string> masters;
        for (Change const& change : changes) {
            std::string const& xpath(Changes::xpath(change));
            if (hookLibrariesToDistributedConfiguration(retrieved_hook_libraries,
                                                        distributed_configuration, workspace,
                                                        sysrepo_config, xpath)) {
                continue;
            }
            if (subnetsToDistributedConfiguration(retrieved_subnets, distributed_configuration,
                                                  workspace, sysrepo_config, xpath)) {
                continue;
            }
            if (shardsToDistributedConfiguration(retrieved_shards, distributed_configuration,
                                                 workspace, sysrepo_config, xpath)) {
                continue;
            }
            if (mastersToDistributedConfiguration(retrieved_masters, distributed_configuration,
                                                  workspace, sysrepo_config, xpath)) {
                continue;
            }
        }

        return isc::config::createAnswer(isc::config::CONTROL_RESULT_SUCCESS,
                                         distributed_configuration);
    }

    /// @brief Set configuration.
    ///
    /// Call config-set over the control socket.
    ///
    /// @param config The configuration to set.
    /// @return The JSON element answer of config-set.
    isc::data::ElementPtr
    configSet(isc::data::ElementPtr const& config,
              isc::data::ElementPtr const& db_config,
              isc::data::ElementPtr const& sysrepo_config,
              std::vector<change_result_t> const& change_results) override final {
        isc::util::Defer _([&] {
            ControlSocketBase<D>::recreateFileHierarchy(
                sysrepo_config, isc::dhcp::ConfigurationConstants::ONLY_MASTER_INI);
        });
        return configMutation(config, db_config, sysrepo_config, change_results);
    }

    /// @brief Test configuration.
    ///
    /// Call config-test over the control socket.
    ///
    /// @param config The configuration to test.
    /// @return The JSON element answer of config-test.
    isc::data::ElementPtr
    configTest([[maybe_unused]] isc::data::ElementPtr const& config,
               [[maybe_unused]] isc::data::ElementPtr const& db_config,
               [[maybe_unused]] std::vector<change_result_t> const& change_results) override final {
        return isc::config::createAnswer(isc::config::CONTROL_RESULT_SUCCESS, "testing successful");
    }

    /// @brief Get leases.
    ///
    /// Call leases-get over the control socket.
    ///
    /// @param subnet_id The subnet-id for which leases are returned (ignored).
    /// @return The JSON element answer of leases-get.
    isc::data::ElementPtr leasesGet(uint32_t /*subnet_id*/) override final {
        return isc::config::createAnswer(isc::config::CONTROL_RESULT_COMMAND_UNSUPPORTED,
                                         "Command not supported on granular change socket");
    }

    /// @brief Delete a lease.
    ///
    /// Call delete-lease over the control socket.
    ///
    /// @param type The lease type for the wanted lease (ignored).
    /// @param addr The lease address for the wanted lease (ignored).
    /// @return The JSON element answer of delete-lease.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr deleteLease(uint32_t /*type*/,
                                      std::string const& /*addr*/) override final {
        return isc::config::createAnswer(isc::config::CONTROL_RESULT_COMMAND_UNSUPPORTED,
                                         "Command not supported on granular change socket");
    }

    /// @brief Get statistic.
    ///
    /// Call statistic-get over the control socket.
    ///
    /// @param name Argument provided to the statistic-get command (ignored).
    /// @return The JSON element answer of statistic-get.
    isc::data::ElementPtr statisticGet(std::string /*name*/) override final {
        return isc::config::createAnswer(isc::config::CONTROL_RESULT_COMMAND_UNSUPPORTED,
                                         "Command not supported on granular change socket");
    }

    /// @brief Get resource.
    ///
    /// Call resource-get over the control socket.
    ///
    /// @param name Argument provided to the resource-get command (ignored).
    /// @return The JSON element answer of resource-get.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr resourceGet(std::string /*resource*/) override final {
        return isc::config::createAnswer(isc::config::CONTROL_RESULT_COMMAND_UNSUPPORTED,
                                         "Command not supported on granular change socket");
    }

private:
    /// @brief Updates the de facto effect of compartment with new effect.
    isc::data::effect_t combine(isc::data::effect_t const& effect,
                                isc::data::effect_t const& other,
                                bool const matches_container_xpath,
                                bool const matches_key_xpath) {
        isc::data::effect_t true_other(other);
        if (!matches_container_xpath) {
            true_other.reset(isc::data::KEY_IS_AFFECTED);
        }
        if (matches_key_xpath) {
            true_other.set(isc::data::KEY_IS_AFFECTED);
        }
        isc::data::effect_t combined_effect(effect | true_other);
        isc::data::effect_t keyless_effect(combined_effect);
        keyless_effect.reset(isc::data::KEY_IS_AFFECTED);
        bool const effect_conflict(keyless_effect.count() > 1);
        if (effect_conflict) {
            LOG_WARN(netconf_logger, NETCONF_WARNING)
                .arg("effect_conflict: " + isc::data::Effect::print(combined_effect));
            // bool const key_is_affected(combined_effect.test(isc::data::KEY_IS_AFFECTED));
            combined_effect.reset();
            combined_effect.set(isc::data::MODIFIED);
        }
        return combined_effect;
    }

    /// @brief Combine new effect into effect from bin.
    template <typename effects_bin_t, typename bin_key_t>
    void placeInEffectBin(effects_bin_t& effects_bin,
                          bin_key_t const& bin_key,
                          isc::data::effect_t const& other_effect,
                          bool const matches_container_xpath,
                          bool const matches_key_xpath) {
        if (!effects_bin.contains(bin_key)) {
            effects_bin.try_emplace(bin_key, isc::data::effect_t());
        }
        effects_bin.insert_or_assign(bin_key, combine(effects_bin.at(bin_key), other_effect,
                                                      matches_container_xpath, matches_key_xpath));

        LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_INFO)
            .arg(PRETTY_METHOD_NAME() + ": key '" + isc::util::str::fromTuple(bin_key) +
                 "', effect '" + isc::data::Effect::print(effects_bin.at(bin_key)) + "'");
    }

    /// @brief matches.*() methods determine if xpath matches a certain compartment.
    /// @{
    boolean_sextuple_key_t matchesHookLibraries(std::string const& xpath) {
        std::smatch sm;
        for (std::regex const& regex : {isc::dhcp::HookLibraryConstants<D, has_IETF>::subnetRegex(),
                                        isc::dhcp::HookLibraryConstants<D, has_IETF>::regex()}) {
            if (std::regex_search(xpath, sm, regex)) {
                auto const& [master, shard, hook_library, parameters, subnet,
                             the_rest](isc::dhcp::HookLibraryConstants<D, has_IETF>::fromMatch(sm));
                return std::make_tuple(true, master, shard, hook_library, parameters, subnet,
                                       the_rest);
            }
        }
        return std::make_tuple(false, std::string(), std::string(), std::string(), std::string(),
                               std::string(), std::string());
    }

    boolean_quintuple_key_t matchesMasters(std::string const& xpath) {
        for (std::regex const& regex : isc::dhcp::MasterConstants::regexes()) {
            std::smatch sm;
            if (std::regex_search(xpath, sm, regex)) {
                auto const& [master, shard, modified_element, instance_ID,
                             the_rest](isc::dhcp::MasterConstants::fromMatch(sm));
                return std::make_tuple(true, master, shard, modified_element, instance_ID,
                                       the_rest);
            }
        }
        return std::make_tuple(false, std::string(), std::string(), std::string(), std::string(),
                               std::string());
    }

    boolean_triple_key_t matchesShards(std::string const& xpath) {
        std::smatch sm;
        if (std::regex_search(xpath, sm, isc::dhcp::ShardConstants::regex())) {
            auto const& [master, shard, the_rest](isc::dhcp::ShardConstants::fromMatch(sm));
            return std::make_tuple(true, master, shard, the_rest);
        }
        return std::make_tuple(false, std::string(), std::string(), std::string());
    }

    boolean_quadruple_key_t matchesSubnets(std::string const& xpath) {
        std::smatch sm;
        if (std::regex_search(xpath, sm, isc::dhcp::SubnetConstantsT<D, has_IETF>::regex())) {
            auto const& [master, shard, subnet,
                         the_rest](isc::dhcp::SubnetConstantsT<D, has_IETF>::fromMatch(sm));
            return std::make_tuple(true, master, shard, subnet, the_rest);
        }
        return std::make_tuple(false, std::string(), std::string(), std::string(), std::string());
    }
    /// @}

    /// @brief compartmentalize.*() methods assigns xpaths to compartment managers
    /// @{
    bool compartmentalizeHookLibraries(isc::data::effect_t const& effect,
                                       std::string const& xpath) {
        auto const& [match, master, shard, hook_library, parameters, subnet,
                     the_rest](matchesHookLibraries(xpath));
        if (match) {
            auto const& key(std::make_tuple(master, shard, hook_library, parameters, subnet));
            bool const matches_key_xpath(
                std::regex_search(xpath, isc::dhcp::HookLibraryConstants<D, has_IETF>::regex()));
            placeInEffectBin(hook_library_effects_, key, effect, the_rest.empty(),
                             matches_key_xpath);
        }
        return match;
    }

    bool compartmentalizeMasters(isc::data::effect_t const& initial_effect,
                                 std::string const& xpath) {
        auto const& [match, master, shard, modified_element, instance_ID,
                     the_rest](matchesMasters(xpath));
        if (match) {
            isc::data::effect_t effect(initial_effect);
            bool matches_key_xpath(
                std::regex_search(xpath, isc::dhcp::MasterConstants::key_regex()));
            bool the_rest_empty(the_rest.empty());
            if (instance_ID.empty()) {
                if (modified_element == "admin-credentials") {
                    auto const& master_shard_instanceID(std::make_tuple(
                        master, shard, "netconf-credentials", "kea-netconf@" + shard));
                    placeInEffectBin(master_effects_, master_shard_instanceID, effect,
                                     the_rest.empty(), matches_key_xpath);
                } else if (modified_element == "config-database") {
                    has_config_database_xpath_been_handled_.try_emplace(
                        std::make_tuple(master, shard), true);

                    // Select all master configurations by shard.
                    if (!master_configurations_by_shard_.contains(shard)) {
                        master_configurations_by_shard_.try_emplace(
                            shard,
                            isc::dhcp::MasterConfigMgrFactory<D>::instance().selectByShard(shard));
                    }

                    // Entries filtered by shard that don't have "kea-netconf@" in their
                    // instance IDs
                    isc::dhcp::MasterConfigCollection master_configs(
                        master_configurations_by_shard_.at(shard));
                    for (isc::dhcp::MasterConfigPtr const& master_config : master_configs) {
                        if (master_config->instance_ID_.find("kea-netconf@") != std::string::npos) {
                            continue;
                        }

                        auto const& master_shard_instanceID(std::make_tuple(
                            master, shard, std::string(), master_config->instance_ID_));

                        if (!matches_key_xpath_map2_.empty()) {
                            matches_key_xpath &=
                                matches_key_xpath_map2_.at(master_shard_instanceID);
                        }

                        if (!the_rest_empty_map2_.empty()) {
                            the_rest_empty &= the_rest_empty_map2_.at(master_shard_instanceID);
                        }

                        placeInEffectBin(master_effects_, master_shard_instanceID, effect,
                                         the_rest_empty, matches_key_xpath);

                        if (matches_key_xpath_map1_.find(master_shard_instanceID) ==
                            matches_key_xpath_map1_.end()) {
                            matches_key_xpath_map1_.try_emplace(master_shard_instanceID, false);
                        }

                        if (the_rest_empty_map1_.find(master_shard_instanceID) ==
                            the_rest_empty_map1_.end()) {
                            the_rest_empty_map1_.try_emplace(master_shard_instanceID, false);
                        }

                        matches_key_xpath_map1_.insert_or_assign(
                            master_shard_instanceID,
                            matches_key_xpath_map1_.at(master_shard_instanceID) |
                                matches_key_xpath);

                        the_rest_empty_map1_.insert_or_assign(
                            master_shard_instanceID,
                            the_rest_empty_map1_.at(master_shard_instanceID) | the_rest_empty);
                    }
                } else {
                    isc_throw(Unexpected, PRETTY_METHOD_NAME() << ": unmatched modified element "
                                                               << modified_element);
                }
            } else {
                auto const& master_shard_instanceID(
                    std::make_tuple(master, shard, std::string(), instance_ID));

                auto const& master_shard(std::make_tuple(master, shard));
                bool const has_config_database_xpath_been_handled(
                    has_config_database_xpath_been_handled_.find(master_shard) !=
                        has_config_database_xpath_been_handled_.end() &&
                    has_config_database_xpath_been_handled_.at(master_shard));

                matches_key_xpath &= (has_config_database_xpath_been_handled &&
                                      (!matches_key_xpath_map1_.contains(master_shard_instanceID) ||
                                       matches_key_xpath_map1_.at(master_shard_instanceID)));
                the_rest_empty &= (has_config_database_xpath_been_handled &&
                                   (!the_rest_empty_map1_.contains(master_shard_instanceID) ||
                                    the_rest_empty_map1_.at(master_shard_instanceID)));

                placeInEffectBin(master_effects_, master_shard_instanceID, effect, the_rest_empty,
                                 matches_key_xpath);

                if (matches_key_xpath_map2_.find(master_shard_instanceID) ==
                    matches_key_xpath_map2_.end()) {
                    matches_key_xpath_map2_.try_emplace(master_shard_instanceID, false);
                }

                if (the_rest_empty_map2_.find(master_shard_instanceID) ==
                    the_rest_empty_map2_.end()) {
                    the_rest_empty_map2_.try_emplace(master_shard_instanceID, false);
                }

                matches_key_xpath_map2_.insert_or_assign(
                    master_shard_instanceID,
                    matches_key_xpath_map2_.at(master_shard_instanceID) | matches_key_xpath);

                the_rest_empty_map2_.insert_or_assign(
                    master_shard_instanceID,
                    the_rest_empty_map2_.at(master_shard_instanceID) | the_rest_empty);
            }
        }
        return match;
    }

    bool compartmentalizeShards(isc::data::effect_t const& effect, std::string const& xpath) {
        auto const& [match, master, shard, the_rest](matchesShards(xpath));
        if (match) {
            auto const& key(std::make_tuple(master, shard));
            bool const matches_key_xpath(
                std::regex_search(xpath, isc::dhcp::ShardConstants::key_regex()));
            placeInEffectBin(shard_effects_, key, effect, the_rest.empty(), matches_key_xpath);
        }
        return match;
    }

    bool compartmentalizeSubnets(isc::data::effect_t const& effect,
                                 std::string const& xpath,
                                 isc::data::ElementPtr const& db_config,
                                 isc::data::ElementPtr const& config) {
        auto const& [match, master, shard, subnet, the_rest](matchesSubnets(xpath));
        if (match) {
            isc::data::ElementPtr config_pointer;
            if (effect.test(isc::data::DELETED)) {
                config_pointer = db_config;
            } else {
                config_pointer = config;
            }
            std::string const& xpath(isc::dhcp::SubnetConstantsT<D, has_IETF>::subnetElementXpath(
                master, shard, subnet));
            isc::data::ElementPtr subnet_element(config_pointer->xpath(xpath));
            if (!subnet_element) {
                isc_throw(Unexpected, PRETTY_METHOD_NAME() << ": " << xpath);
            }

            std::string subnet_mutable(subnet_element->stringValue());
            auto const& key(std::make_tuple(master, shard, subnet_mutable));
            bool const matches_key_xpath(
                std::regex_search(xpath, isc::dhcp::SubnetConstants<D>::key_regex()));
            placeInEffectBin(subnet_effects_, key, effect, the_rest.empty(), matches_key_xpath);
        }
        return match;
    }
    /// @}

    /// @brief actOn.*() methods insert or update compartments into Cassandra
    /// @{
    void actOnHookLibraries([[maybe_unused]] isc::data::ElementPtr const& db_config,
                            isc::data::ElementPtr const& config,
                            [[maybe_unused]] isc::data::ElementPtr const& sysrepo_config,
                            isc::util::Dir const& workspace,
                            bool const test) {
        for (auto const& [key, effect] : hook_library_effects_) {
            if (effect.none()) {
                continue;
            }

            auto const& [master, shard, hook_library, parameters, subnet] = key;
            isc::util::Dir const& shard_directory(workspace() + "/" + master + "/" + shard);
            if (!test) {
                LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_INFO)
                    .arg(PRETTY_METHOD_NAME() +
                         ": isc::kea_config_tool::VersionedController::action(" +
                         isc::data::Effect::print(effect) + ")");
                isc::kea_config_tool::VersionedController<D, has_IETF>::action(
                    master, shard, hook_library, parameters, subnet, effect, config,
                    shard_directory() + "/config.timestamp");
                required_to_be_affected_shard_configurations_.emplace(
                    std::make_tuple(master, shard));
            }
        }
    }

    void actOnMasters(isc::data::ElementPtr const& db_config,
                      isc::data::ElementPtr const& config,
                      [[maybe_unused]] isc::data::ElementPtr const& sysrepo_config,
                      isc::util::Dir const& workspace,
                      bool const test) {
        for (auto const& [master_shard_instanceID, effect] : master_effects_) {
            if (effect.none() || (effect.test(isc::data::KEY_IS_AFFECTED) && effect.count() == 1)) {
                continue;
            }

            isc::data::ElementPtr config_pointer;
            if (effect.test(isc::data::DELETED)) {
                config_pointer = db_config;
            } else {
                config_pointer = config;
            }

            auto const& [master, shard, netconf_flag, instance_ID] = master_shard_instanceID;
            isc::dhcp::MasterConfigPtr master_config(std::make_shared<isc::dhcp::MasterConfig>());
            master_config->fromDistributedConfiguration(config_pointer, master, shard, netconf_flag,
                                                        instance_ID);

            // Configure credentials.
            isc::data::ElementPtr credentials(
                config->xpath(isc::dhcp::MasterConstants::credentials_xpath(master)));
            isc::data::ElementPtr enclosed_credentials(
                isc::dhcp::ConfigurationManager<D, has_IETF>::credentialsToDhcpEnclosed(
                    credentials));
            isc::dhcp::ConfigurationManager<D, has_IETF>::configureCredentialsToStaging(
                isc::dhcp::ConfigurationManager<D, has_IETF>::getDhcp(enclosed_credentials));
            std::optional<std::string> instance_ID_optional(std::nullopt);
            if (!instance_ID.empty()) {
                instance_ID_optional = instance_ID;
            }
            isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createMasterCfgMgrs<D>(
                instance_ID_optional);
            isc::util::Dir master_directory(workspace() + "/" + master);
            if (!test) {
                LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_INFO)
                    .arg(PRETTY_METHOD_NAME() + ": MasterConfigMgr::action(" +
                         isc::data::Effect::print(effect) + ")");
                isc::dhcp::MasterConfigMgrFactory<D>::instance().action(
                    effect, master_config, master_directory() + "/config.timestamp");
            }
        }
    }

    void actOnShards(isc::data::ElementPtr const& db_config,
                     isc::data::ElementPtr const& config,
                     [[maybe_unused]] isc::data::ElementPtr const& sysrepo_config,
                     isc::util::Dir const& workspace,
                     bool const test) {
        for (auto const& [master_shard, effect] : shard_effects_) {
            if (effect.none() || (effect.test(isc::data::KEY_IS_AFFECTED) && effect.count() == 1)) {
                continue;
            }

            isc::data::ElementPtr config_pointer;
            if (effect.test(isc::data::DELETED)) {
                config_pointer = db_config;
            } else {
                config_pointer = config;
            }

            auto const& [master, shard] = master_shard;
            isc::dhcp::ShardConfigPtr shard_config(std::make_shared<isc::dhcp::ShardConfig>());
            shard_config->fromDistributedConfiguration(config_pointer, master, shard);

            // Configure credentials.
            std::string const& credentials_xpath(
                isc::dhcp::ShardConstants::credentials_xpath(master, shard));
            isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createShardCfgMgrs<D>(
                {config->xpath(credentials_xpath)});

            isc::util::Dir shard_directory(workspace() + "/" + master + "/" + shard);
            if (!test) {
                LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_INFO)
                    .arg(PRETTY_METHOD_NAME() + ": ShardConfigMgr::action(" +
                         isc::data::Effect::print(effect) + ")");
                isc::dhcp::ShardConfigMgrFactory<D>::instance().action(
                    effect, shard_config, shard_directory() + "/config.timestamp");
                affected_shard_configurations_.emplace(master_shard);
            }
        }
    }

    void actOnSubnets(isc::data::ElementPtr const& db_config,
                      isc::data::ElementPtr const& config,
                      [[maybe_unused]] isc::data::ElementPtr const& sysrepo_config,
                      isc::util::Dir const& workspace,
                      bool const test) {
        for (auto const& [master_shard_subnet, effect] : subnet_effects_) {
            if (effect.none() || (effect.test(isc::data::KEY_IS_AFFECTED) && effect.count() == 1)) {
                continue;
            }

            isc::data::ElementPtr config_pointer;
            if (effect.test(isc::data::DELETED)) {
                config_pointer = db_config;
            } else {
                config_pointer = config;
            }

            auto const& [master, shard, subnet] = master_shard_subnet;
            isc::dhcp::SubnetInfoPtr<D> config_subnet(std::make_shared<isc::dhcp::SubnetInfo<D>>());
            config_subnet->fromDistributedConfiguration(config_pointer, master, shard, subnet);

            // Configure credentials.
            std::string const& credentials_xpath(
                isc::dhcp::ShardConstants::credentials_xpath(master, shard));
            isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createShardCfgMgrs<D>(
                {config->xpath(credentials_xpath)});

            isc::util::Dir const& shard_directory(workspace() + "/" + master + "/" + shard);
            if (!test) {
                LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_INFO)
                    .arg(PRETTY_METHOD_NAME() + ": SubnetMgr::action(" +
                         isc::data::Effect::print(effect) + ")");
                isc::dhcp::SubnetMgrFactory<D>::instance().action(
                    effect, config_subnet, shard_directory() + "/config.timestamp");
                required_to_be_affected_shard_configurations_.emplace(
                    std::make_tuple(master, shard));
            }
        }
    }
    /// @}

    /// @brief Update timestamp in shard configuration if the subnets have been modified so that a
    /// server is able to trigger reconfiguration.
    void maybeUpdateTimestamp(isc::data::ElementPtr const& config,
                              isc::util::Dir const& workspace) {
        for (auto const& [master, shard] : required_to_be_affected_shard_configurations_) {
            bool found(false);
            for (auto const& [affected_master, affected_shard] : affected_shard_configurations_) {
                if (shard == affected_shard) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                // Configure credentials.
                std::string const& credentials_xpath(
                    isc::dhcp::ShardConstants::credentials_xpath(master, shard));
                isc::dhcp::CfgMgr::instance()
                    .getStagingCfg()
                    ->getCfgDbAccess()
                    ->createShardCfgMgrs<D>({config->xpath(credentials_xpath)});

                isc::dhcp::ShardConfigMgrFactory<D>::instance().updateTimestamp(
                    workspace() + "/" + master + "/" + shard + "/config.timestamp");
            }
        }
    }

    void prepareForConfigSet(isc::data::ElementPtr const& config,
                             isc::data::ElementPtr const& db_config,
                             [[maybe_unused]] isc::data::ElementPtr const& sysrepo_config,
                             std::vector<change_result_t> const& change_results) override {
        auto const& [return_code, output](ControlSocketBase<D>::recreateFileHierarchy(config));
        if (return_code != 0) {
            LOG_ERROR(netconf_logger, NETCONF_INFO)
                .arg(PRETTY_METHOD_NAME() + ": recreateFileHierarchy(): " + output);
        }

        master_configurations_by_shard_.clear();
        affected_shard_configurations_.clear();
        required_to_be_affected_shard_configurations_.clear();
        matches_key_xpath_map1_.clear();
        matches_key_xpath_map2_.clear();
        the_rest_empty_map1_.clear();
        the_rest_empty_map2_.clear();
        has_config_database_xpath_been_handled_.clear();

        // Compartmentalize i.e. decide on action.
        for (auto const& [xpath, initial_effect] : change_results) {
            LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_INFO)
                .arg(PRETTY_METHOD_NAME() + ": xpath " + xpath + "...");

            if (initial_effect.none()) {
                continue;
            }

            // From specific to generic...
            if (compartmentalizeHookLibraries(initial_effect, xpath)) {
                continue;
            }
            if (compartmentalizeSubnets(initial_effect, xpath, db_config, config)) {
                continue;
            }
            if (compartmentalizeShards(initial_effect, xpath)) {
                continue;
            }
            if (compartmentalizeMasters(initial_effect, xpath)) {
                continue;
            }

            LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_INFO)
                .arg(PRETTY_METHOD_NAME() + ": unassigned xpath " + xpath);
        }
    }

    isc::data::ElementPtr
    configMutation(isc::data::ElementPtr const& config,
                   isc::data::ElementPtr const& db_config,
                   isc::data::ElementPtr const& sysrepo_config,
                   [[maybe_unused]] std::vector<change_result_t> const& change_results,
                   bool const test = false) {
        isc::util::Dir const workspace(ControlSocketBase<D>::workspace_);

        try {
            actOnHookLibraries(db_config, config, sysrepo_config, workspace, test);
            actOnSubnets(db_config, config, sysrepo_config, workspace, test);
            actOnShards(db_config, config, sysrepo_config, workspace, test);
            actOnMasters(db_config, config, sysrepo_config, workspace, test);
            maybeUpdateTimestamp(config, workspace);
        } catch (isc::db::TimestampHasChanged const& exception) {
            return isc::config::createAnswer(
                isc::kea_config_tool::Controller::EXIT_TIMESTAMP_HAS_CHANGED, exception.what());
        } catch (isc::db::StatementNotApplied const& exception) {
            return isc::config::createAnswer(
                isc::kea_config_tool::Controller::EXIT_TIMESTAMP_HAS_CHANGED, exception.what());
        }

        return isc::config::createAnswer(isc::config::CONTROL_RESULT_SUCCESS,
                                         "configuration successful");
    }

    ControlSocketBasePtr<D> kea_config_tool_control_;
    std::string model_;

    std::unordered_map<quadruple_key_t, bool> matches_key_xpath_map1_, the_rest_empty_map1_,
        matches_key_xpath_map2_, the_rest_empty_map2_;
    std::unordered_map<double_key_t, bool> has_config_database_xpath_been_handled_;
    std::unordered_map<std::string, isc::dhcp::MasterConfigCollection>
        master_configurations_by_shard_;
    std::unordered_set<double_key_t> affected_shard_configurations_;
    std::unordered_set<double_key_t> required_to_be_affected_shard_configurations_;

    master_effects_t master_effects_;
    shard_effects_t shard_effects_;
    subnet_effects_t subnet_effects_;
    hook_library_effects_t hook_library_effects_;
};

}  // namespace netconf
}  // namespace isc

#endif  // GRANULAR_CHANGE_CONTROL_H

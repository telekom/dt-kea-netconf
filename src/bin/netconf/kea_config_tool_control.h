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

#ifndef KEA_CONFIG_TOOL_CONTROL_H
#define KEA_CONFIG_TOOL_CONTROL_H

#include <cc/command_interpreter.h>
#include <cc/data.h>
#include <dhcpsrv/subnet_mgr.h>
#include <kea_config_tool/controller.h>
#include <netconf/control_socket.h>
#include <netconf/netconf_log.h>
#include <util/command.h>
#include <util/dhcp.h>
#include <util/magic_enum.hpp>
#include <util/tuple_hash.h>

namespace isc {
namespace netconf {

/// @brief Class for control socket communication over UNIX socket.
///
/// This class is the derived class for control socket communication
/// over UNIX sockets.
/// This class implements config-get, config-test and config-set.
template <isc::dhcp::DhcpSpaceType D, bool has_IETF>
struct KeaConfigToolControl : ControlSocketBase<D> {
    /// @brief Constructor.
    ///
    /// @param ctrl_sock The control socket configuration.
    KeaConfigToolControl(CfgControlSocketPtr ctrl_sock)
        : ControlSocketBase<D>(ctrl_sock), kea_config_tool_(std::string("kea-config-tool")) {
    }

    /// @brief Get configuration.
    ///
    /// Call config-get over the control socket.
    ///
    /// @return The JSON element answer of config-get.
    isc::data::ElementPtr
    configGet(ChangeCollection const& /* changes */,
              isc::data::ElementPtr const& /* sysrepo_config */) override final {
        // Retrieve directory list.
        isc::util::Dir workspace(ControlSocketBase<D>::workspace_);
        std::vector<isc::util::FileInfo> master_directories(
            isc::util::Filesystem::directoriesAtPath(workspace()));

        // Iterate through masters.
        isc::data::ElementPtr config(isc::data::Element::createMap());
        isc::data::ElementPtr masters(isc::data::Element::createList());
        for (isc::util::FileInfo master_info : master_directories) {
            // Create directories and files.
            isc::data::ElementPtr const& master(
                isc::dhcp::ConfigurationManager<D, has_IETF>::masterIniToAdminCredentials(
                    workspace(), master_info.name_));

            // get-servers
            std::string const& master_directory(workspace() + "/" + master_info.name_);
            std::string const& master_ini(master_directory + "/master.ini");
            try {
                kea_config_tool_.run("--master --get-servers -" +
                                     isc::dhcp::dhcpSpaceToString<D>() + " " + master_ini + " " +
                                     master_directory);
            } catch (std::exception const& exception) {
                LOG_ERROR(netconf_logger, NETCONF_INFO)
                    .arg(PRETTY_METHOD_NAME() + ": " + exception.what());
                throw;
            }

            // Retrieve directory list from the current master.
            std::vector<isc::util::FileInfo> shard_directories(
                isc::util::Filesystem::directoriesAtPath(master_directory));

            // Iterate through shards.
            isc::data::ElementPtr shards(isc::data::Element::createList());
            for (isc::util::FileInfo shard_info : shard_directories) {
                // Create directories and files.
                std::string shard_directory(master_directory + "/" + shard_info.name_);
                std::string credentials_json(shard_directory + "/credentials.json");
                std::string config_json(shard_directory + "/config.json");
                std::string servers_json(shard_directory + "/servers.json");

                // Build isc::data::Elements.
                isc::data::ElementPtr shard(isc::data::Element::createMap());
                shard->set("name", shard_info.name_);
                isc::data::ElementPtr credentials_config(
                    isc::data::Element::fromJSONFileExceptionSafe(credentials_json, true));
                isc::data::ElementPtr const admin_credentials(
                    credentials_config->get("config-database"));
                if (admin_credentials) {
                    shard->set("admin-credentials", admin_credentials);
                }
                isc::data::ElementPtr servers_config(
                    isc::data::Element::fromJSONFileExceptionSafe(servers_json, true));
                isc::data::ElementPtr config_database(servers_config->get("config-database"));
                if (config_database) {
                    shard->set("config-database", config_database);
                }

                // get-config
                try {
                    kea_config_tool_.run("--shard --get-config -" +
                                         isc::dhcp::dhcpSpaceToString<D>() + " " + master_ini +
                                         " " + master_directory + " " + shard_info.name_);
                } catch (std::exception const& exception) {
                    LOG_ERROR(netconf_logger, NETCONF_INFO)
                        .arg(PRETTY_METHOD_NAME() + ": " + exception.what());
                    throw;
                }

                shard->set("shard-config",
                           isc::data::Element::fromJSONFileExceptionSafe(config_json, true));
                isc::data::ElementPtr const master_config_for_this_shard(
                    servers_config->get("master-config"));
                if (master_config_for_this_shard) {
                    shard->set("master-config", master_config_for_this_shard);
                }
                shards->add(shard);
            }
            master->set("shards", shards);
            masters->add(master);
        }

        config->set("masters", masters);

        return isc::config::createAnswer(isc::config::CONTROL_RESULT_SUCCESS, config);
    }

    /// @brief internal function
    ///
    /// @param config The configuration to set.
    /// @return The JSON element answer of config-set.
    isc::data::ElementPtr
    configMutation(isc::data::ElementPtr const& config,
                   [[maybe_unused]] isc::data::ElementPtr const& db_config,
                   [[maybe_unused]] std::vector<change_result_t> const& change_results,
                   bool const test = false) {
        std::string command;
        if (test) {
            command += "-t ";
        }

        try {
            auto const& [exit_code, output](ControlSocketBase<D>::recreateFileHierarchy(
                config,
                [&](std::string const& master_ini, std::string const& master_directory,
                    std::string const& shard_name) -> std::tuple<int, std::string> {
                    // set-config
                    return kea_config_tool_.runExceptionSafe(
                        command + "--shard --set-config -" + isc::dhcp::dhcpSpaceToString<D>() +
                        " " + master_ini + " " + master_directory + " " + shard_name);
                },
                [&](std::string const& master_ini,
                    std::string const& master_directory) -> std::tuple<int, std::string> {
                    // set-servers
                    return kea_config_tool_.runExceptionSafe(
                        command + "--master --set-servers -" + isc::dhcp::dhcpSpaceToString<D>() +
                        " " + master_ini + " " + master_directory);
                },
                false));

            if (exit_code != 0) {
                return isc::config::createAnswer(exit_code, output);
            }
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

    /// @brief Set configuration.
    ///
    /// Call config-set over the control socket.
    ///
    /// @param config The configuration to set.
    /// @return The JSON element answer of config-set.
    isc::data::ElementPtr
    configSet(isc::data::ElementPtr const& config,
              isc::data::ElementPtr const& db_config,
              [[maybe_unused]] isc::data::ElementPtr const& sysrepo_config,
              [[maybe_unused]] std::vector<change_result_t> const& change_results) override final {
        return configMutation(config, db_config, change_results);
    }

    /// @brief Test configuration.
    ///
    /// Call config-test over the control socket.
    ///
    /// @param config The configuration to test.
    /// @return The JSON element answer of config-test.
    isc::data::ElementPtr
    configTest(isc::data::ElementPtr const& config,
               isc::data::ElementPtr const& db_config,
               std::vector<change_result_t> const& change_results) override final {
        return configMutation(config, db_config, change_results, true);
    }

    /// @brief Get leases.
    ///
    /// Call leases-get over the control socket.
    ///
    /// @param subnet_id The subnet-id for which leases are returned (ignored)
    /// @return The JSON element answer of leases-get.
    isc::data::ElementPtr leasesGet(uint32_t /* subnet_id */) override final {
        isc_throw(NotImplemented, "No leases-get for kea-config-tool control socket");
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
        isc_throw(NotImplemented, "No delete-lease for kea-config-tool control socket");
    }

    /// @brief Get statistic.
    ///
    /// Call statistic-get over the control socket.
    ///
    /// @param name Argument provided to the statistic-get command(ignored)
    /// @return The JSON element answer of statistic-get.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr statisticGet(std::string /* name */) override final {
        isc_throw(NotImplemented, "No statistic-get for kea-config-tool control socket");
    }

    /// @brief Get resource.
    ///
    /// Call resource-get over the control socket.
    ///
    /// @param name Argument provided to the resource-get command
    /// @return The JSON element answer of resource-get.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr resourceGet(std::string /* resource */) override final {
        isc_throw(NotImplemented, "No resource-get for kea-config-tool control socket");
    }

private:
    isc::kea_config_tool::Controller kea_config_tool_;
};

}  // namespace netconf
}  // namespace isc

#endif  // KEA_CONFIG_TOOL_CONTROL_H

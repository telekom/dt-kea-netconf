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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <database/dbaccess_parser.h>
#include <dhcpsrv/cfgmgr.h>
#include <util/dhcp.h>

namespace isc {
namespace dhcp {

struct ConfigurationConstants {
    static bool constexpr HAS_IETF = true;
    static bool constexpr NOT_IETF = false;
    static bool constexpr ONLY_MASTER_INI = true;

private:
    /// @brief static-only
    ConfigurationConstants() = delete;

    /// @brief non-copyable
    /// @{
    ConfigurationConstants(ConfigurationConstants const&) = delete;
    ConfigurationConstants& operator=(ConfigurationConstants const&) = delete;
    /// @}
};

template <DhcpSpaceType D, bool has_IETF>
struct ConfigurationElements {
    static std::string const hooks_libraries();
    static std::string const subnet();
    static std::string const subnetElement();
    static std::string const subnetIndex();
    static std::string const toplevel();

private:
    /// @brief static-only
    ConfigurationElements() = delete;

    /// @brief non-copyable
    /// @{
    ConfigurationElements(ConfigurationElements const&) = delete;
    ConfigurationElements& operator=(ConfigurationElements const&) = delete;
    /// @}
};

template <DhcpSpaceType D>
struct ConfigurationElements<D, ConfigurationConstants::NOT_IETF> {
    static std::string const hooks_libraries() {
        return "hooks-libraries";
    }
    static std::string const subnet() {
        return "subnet" + dhcpSpaceToString<D>();
    }
    static std::string const subnetElement() {
        return "subnet";
    }
    static std::string const subnetIndex() {
        return "subnet";
    }
    static std::string const toplevel() {
        return "Dhcp" + dhcpSpaceToString<D>();
    }
};

template <DhcpSpaceType D>
struct ConfigurationElements<D, ConfigurationConstants::HAS_IETF> {
    static std::string const hooks_libraries() {
        return "vendor-config/hooks-libraries";
    }
    static std::string const subnet() {
        return "network-ranges/network-range";
    }
    static std::string const subnetElement() {
        return "network-prefix";
    }
    static std::string const subnetIndex() {
        return "id";
    }
    static std::string const toplevel() {
        return "dhcpv" + dhcpSpaceToString<D>() + "-server";
    }
};

template <DhcpSpaceType D, bool has_IETF>
struct ConfigurationManager {
    using master_action_t =
        std::function<std::tuple<int, std::string>(std::string const&, std::string const&)>;
    using shard_action_t = std::function<std::tuple<int, std::string>(
        std::string const&, std::string const&, std::string const&)>;

    /// @brief Configures database access for the database managers through CfgMgr.
    ///
    /// @param configuration "Dhcp[46]" contents
    static void configureCredentialsToStaging(isc::data::ElementPtr const& configuration) {
        // Check if a 'configuration-type' parameter is provided.
        isc::data::ElementPtr const& configuration_type(configuration->get("configuration-type"));
        std::string const& type(configuration_type ? configuration_type->stringValue() : "file");

        // Valid values for "configuration-type" are "database" & "file".
        if (type == "database") {
            // The server configuration should be read from the database.
            // Update the configuration manager with the server configuration type.
            isc::data::ElementPtr const& master_database(configuration->get("master-database"));
            isc::data::ElementPtr const& config_database(configuration->get("config-database"));
            if (master_database) {
                isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() =
                    isc::dhcp::SrvConfigType::MASTER_DATABASE;
                isc::dhcp::CfgMgr::instance()
                    .getStagingCfg()
                    ->getCfgDbAccess()
                    ->setMasterCfgDbAccessString(isc::db::DbAccessParser::parse(master_database));
            } else if (config_database) {
                isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() =
                    isc::dhcp::SrvConfigType::CONFIG_DATABASE;
                isc::dhcp::CfgMgr::instance()
                    .getStagingCfg()
                    ->getCfgDbAccess()
                    ->setShardCfgDbAccessString(isc::db::DbAccessParser::parse(config_database));
            }
        } else if (type == "file") {
            isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() =
                isc::dhcp::SrvConfigType::FILE;
        } else {
            isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() =
                isc::dhcp::SrvConfigType::UNKNOWN;

            isc_throw(BadValue,
                      "Invalid value '"
                          << type << "' for 'configuration-type' parameter, can't process config.");
        }
    }

    static void configureServerIDToStaging(isc::data::ElementPtr const& configuration) {
        // Check if an 'instance-id' parameter is provided.
        isc::data::ElementPtr const instance_ID(configuration->get("instance-id"));
        if (instance_ID) {
            CfgMgr::instance().getStagingCfg()->getInstanceID() = instance_ID->stringValue();
        }
    }

    static std::tuple<int, std::string> createFileHierarchy(std::string const& workspace,
                                                            isc::data::ElementPtr const& config,
                                                            bool const only_master_ini = false) {
        auto const& nulled_lambda([](std::string const& /* master_ini */,
                                     std::string const& /* master_directory */, std::string const &
                                     /* shard_name */) -> std::tuple<int, std::string> {
            return {0, ""};
        });

        return createFileHierarchy(config, workspace, nulled_lambda, nulled_lambda,
                                   only_master_ini);
    }

    static std::tuple<int, std::string> createFileHierarchy(std::string const& workspace,
                                                            isc::data::ElementPtr const& config,
                                                            shard_action_t const& shard_action,
                                                            master_action_t const& master_action,
                                                            bool const only_master_ini = false) {
        if (!config) {
            return {0, ""};
        }

        isc::data::ElementPtr const masters(config->get("masters"));

        // Sanity check
        if (!masters) {
            return {1337, "no <masters> element found, wrong data or model"};
        }

        // Iterate through masters.
        for (isc::data::ElementPtr const& master : masters->listValue()) {
            // Sanity check
            isc::data::ElementPtr admin_credentials(master->get("admin-credentials"));
            if (!admin_credentials) {
                continue;
            }
            std::string database_name_key("name");
            isc::data::ElementPtr database_name_node(admin_credentials->get(database_name_key));
            if (!database_name_node) {
                database_name_key = "keyspace";
                database_name_node = admin_credentials->get(database_name_key);
            }
            if (!database_name_node) {
                isc_throw(Unexpected, "neither 'name' nor 'keyspace' entry in 'admin-credentials'");
            }

            std::string const& master_name(master->get("name")->stringValue());
            std::string const& keyspace(database_name_node->stringValue());
            if (master_name != keyspace) {
                isc_throw(Unexpected, "the master's directory name '"
                                          << master_name
                                          << "' should be the same as the master's database name '"
                                          << keyspace << "' specified in `master-database`.");
            }

            // Write to master.ini
            isc::data::ElementPtr const enclosed_credentials(
                credentialsToDhcpEnclosed(admin_credentials));
            isc::util::Dir const master_directory(workspace + "/" + master_name);
            isc::util::File const master_ini(master_directory() + "/master.ini",
                                             prettyPrint(enclosed_credentials));

            if (only_master_ini) {
                continue;
            }

            // Sanity check
            isc::data::ElementPtr const shards(master->get("shards"));

            // Iterate through shards.
            if (shards) {
                for (isc::data::ElementPtr const& shard : shards->listValue()) {
                    // Get isc::data::Elements.
                    isc::data::ElementPtr const config_database(shard->get("config-database"));
                    isc::data::ElementPtr const shard_admin_credentials(
                        shard->get("admin-credentials"));
                    isc::data::ElementPtr const master_config(shard->get("master-config"));
                    isc::data::ElementPtr const shard_config(shard->get("shard-config"));

                    // Set isc::data::Elements.
                    isc::data::ElementPtr const servers_json_content(
                        isc::data::Element::createMap());
                    if (config_database) {
                        servers_json_content->set("config-database", config_database);
                    }
                    if (master_config) {
                        servers_json_content->set("master-config", master_config);
                    }
                    isc::data::ElementPtr const credentials_json_content(
                        isc::data::Element::createMap());
                    if (shard_admin_credentials) {
                        credentials_json_content->set("config-database", shard_admin_credentials);
                    }

                    // Create directories and files, everything except config.timestamp, that should
                    // be kept from a previous operation.
                    std::string const& shard_name = shard->get("name")->stringValue();
                    isc::util::Dir const shard_directory(master_directory() + "/" + shard_name);
                    if (!credentials_json_content->empty()) {
                        // Only write credentials.json if it has content because the old one might
                        // be needed for updating shard configuration.
                        isc::util::File const credentials_json(
                            shard_directory() + "/credentials.json",
                            prettyPrint(credentials_json_content));
                    }
                    isc::util::File const servers_json(shard_directory() + "/servers.json",
                                                       prettyPrint(servers_json_content));
                    std::string config_json_content;
                    if (shard_config) {
                        config_json_content = prettyPrint(shard_config);
                    }
                    isc::util::File const config_json(shard_directory() + "/config.json",
                                                      config_json_content);

                    auto const& [exit_code, output](
                        shard_action(master_ini(), master_directory(), shard_name));
                    if (exit_code != 0) {
                        return {exit_code, output};
                    }
                }
            }

            auto const& [exit_code, output](master_action(master_ini(), master_directory()));
            if (exit_code != 0) {
                return {exit_code, output};
            }
        }

        return {0, ""};
    }

    static isc::data::ElementPtr credentialsToDhcpEnclosed(isc::data::ElementPtr& credentials) {
        isc::data::ElementPtr dhcp(isc::data::Element::createMap());
        dhcp->set("master-database", credentials);
        dhcp->set("configuration-type", "database");
        dhcp->set("instance-id", "kea-netconf");
        isc::data::ElementPtr enclosed_credentials(isc::data::Element::createMap());
        enclosed_credentials->set(ConfigurationElements<D, has_IETF>::toplevel(), dhcp);
        return enclosed_credentials;
    }

    static void excludeHookLibraries(isc::data::ElementPtr const& dhcp) {
        isc::data::ElementPtr const& hook_libraries(dhcp->get("hooks-libraries"));
        if (hook_libraries) {
            isc::data::ElementPtr new_hook_libraries(isc::data::Element::createList());
            for (isc::data::ElementPtr const& hook_library : hook_libraries->listValue()) {
                isc::data::ElementPtr const& library(hook_library->get("library"));
                if (library && !library->stringValue().empty()) {
                    for (std::string const& p :
                         {"lawful-interception-parameters", "policy-engine-parameters",
                          "parameters"}) {
                        isc::data::ElementPtr const& config_database(
                            hook_library->xpath("/" + p + "/config/config-database"));
                        if (config_database) {
                            isc::data::ElementPtr new_hook_library(isc::data::Element::createMap());
                            new_hook_library->set("library", library);
                            new_hook_library->xpath("/" + p + "/config/config-database",
                                                    config_database);
                            new_hook_libraries->add(new_hook_library);
                        }
                    }
                }
            }
            dhcp->set("hooks-libraries", new_hook_libraries);
        }
    }

    static void excludeNetworkTopologies(isc::data::ElementPtr const& config) {
        config->xpath("/network-topology/subnets", isc::data::ElementPtr(nullptr));
    }

    static isc::data::ElementPtr getDhcp(isc::data::ElementPtr const& config) {
        isc::data::ElementPtr dhcp(getDhcpExceptionSafe(config));
        if (!dhcp) {
            std::string const toplevel(ConfigurationElements<D, has_IETF>::toplevel());
            isc_throw(isc::BadValue,
                      "no mandatory '" << toplevel << "' entry in the configuration");
        }
        return dhcp;
    }

    static isc::data::ElementPtr getDhcpExceptionSafe(isc::data::ElementPtr const& config) {
        return config->get(ConfigurationElements<D, has_IETF>::toplevel());
    }

    static std::string getInstanceID(isc::data::ElementPtr const& config) {
        // Check if an 'instance-id' parameter is provided.
        isc::data::ElementPtr instance_ID(getDhcp(config)->get("instance-id"));
        if (!instance_ID) {
            isc_throw(RunTimeFail, "mandatory 'instance-id' parameter is missing");
        }

        return instance_ID->stringValue();
    }

    static isc::data::ElementPtr masterIniToAdminCredentials(std::string const& workspace,
                                                             std::string const& master_name) {
        std::string const& master_directory(workspace + "/" + master_name);
        std::string const& master_ini(master_directory + "/master.ini");

        // Build isc::data::Elements.
        isc::data::ElementPtr master(isc::data::Element::createMap());
        master->set("name", master_name);
        isc::data::ElementPtr master_config(
            isc::data::Element::fromJSONFileExceptionSafe(master_ini, true));
        isc::data::ElementPtr dhcp_element(
            isc::dhcp::ConfigurationManager<D, has_IETF>::getDhcpExceptionSafe(master_config));
        if (dhcp_element) {
            master->set("admin-credentials", dhcp_element->get("master-database"));
        }
        return master;
    }

private:
    /// @brief static-only
    ConfigurationManager() = delete;

    /// @brief non-copyable
    /// @{
    ConfigurationManager(ConfigurationManager const&) = delete;
    ConfigurationManager& operator=(ConfigurationManager const&) = delete;
    /// @}
};

}  // namespace dhcp
}  // namespace isc

#endif  // CONFIGURATION_H

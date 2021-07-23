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

#ifndef KEA_CONFIG_TOOL_CONTROLLER_H
#define KEA_CONFIG_TOOL_CONTROLLER_H

#include <kea_version.h>

#include <stdlib.h>
#include <wordexp.h>

#include <admin/kea_admin.h>
#include <cc/command_interpreter.h>
#include <database/dbaccess_parser.h>
#include <database/timestamp_store.h>
#include <dhcp4/dhcp4_srv.h>
#include <dhcp4/json_config_parser.h>
#include <dhcp6/dhcp6_srv.h>
#include <dhcp6/json_config_parser.h>
#include <dhcpsrv/cfg_db_access.h>
#include <dhcpsrv/cfgmgr.h>
#include <dhcpsrv/configuration.h>
#include <dhcpsrv/hook_library_manager.h>
#include <dhcpsrv/host_data_source_factory.h>
#include <dhcpsrv/host_mgr.h>
#include <dhcpsrv/lease_mgr_factory.h>
#include <dhcpsrv/master_config_mgr.h>
#include <dhcpsrv/master_config_mgr_factory.h>
#include <dhcpsrv/shard_config_mgr.h>
#include <dhcpsrv/shard_config_mgr_factory.h>
#include <dhcpsrv/subnet_mgr_factory.h>
#include <exceptions/exceptions.h>
#include <kea_config_tool/controller.h>
#include <kea_config_tool/messages.h>
#include <kea_config_tool/options.h>
#include <log/macros.h>
#include <util/command.h>
#include <util/defer.h>
#include <util/dhcp.h>
#include <util/filesystem.h>
#include <util/func.h>
#include <util/hash.h>
#include <util/magic_enum.hpp>
#include <util/types.h>

#ifdef HAVE_CQL
#include <dhcpsrv/cql_master_config_mgr.h>
#endif

#include <algorithm>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <boost/exception/all.hpp>

namespace isc {
namespace kea_config_tool {

/// @brief Template bindings
/// @{

// DhcpvSrv
namespace {

template <isc::dhcp::DhcpSpaceType D>
struct adapter_DhcpvSrv {};

template <>
struct adapter_DhcpvSrv<isc::dhcp::DHCP_SPACE_V4> {
    using type = isc::dhcp::Dhcpv4Srv;
};

template <>
struct adapter_DhcpvSrv<isc::dhcp::DHCP_SPACE_V6> {
    using type = isc::dhcp::Dhcpv6Srv;
};

}  // namespace

template <isc::dhcp::DhcpSpaceType D>
using DhcpvSrv = typename adapter_DhcpvSrv<D>::type;
/// @}

/// @brief Exception thrown when the command line is invalid.
struct InvalidUsage : public isc::Exception {
    InvalidUsage(char const* file, size_t line, char const* what, bool show_app_help = false)
        : isc::Exception(file, line, what), show_app_help_(show_app_help) {
    }

    bool getShowAppHelp() const {
        return show_app_help_;
    }

private:
    bool show_app_help_;
};

struct Controller {
    enum exit_code_t {
        EXIT_SUCCESSFUL = 0,
        EXIT_TRANSACTION_EXCEPTION = 5,
        EXIT_TIMESTAMP_HAS_CHANGED,
        EXIT_HOOKS_LIBRARY_LOAD_FAIL,
        EXIT_INVALID_USAGE,
        EXIT_BOOST_EXCEPTION,
        EXIT_STD_EXCEPTION,
        EXIT_UNKNOWN_EXCEPTION
    };

    Controller();
    Controller(std::string const& bin_name, std::unordered_map<std::string, std::string> env = {});

    /// @brief Acts as the primary entry point to start execution
    /// of the process.
    ///
    /// Provides the control logic to upload and download server configuration
    /// file on database back-ends
    ///
    /// -# parse command line arguments
    /// -# verify that it is the only instance
    /// -# parse the database credentials file provided and connects to database
    /// -# upload/download server configuration file
    /// -# exit to the caller
    ///
    /// @param argc Number of std::strings in the @c argv array.
    /// @param argv Array of arguments passed in via the program's main
    /// function.
    ///
    /// @throw InvalidUsage if the command line parameters are invalid.
    /// @throw isc::BadValue if wrong values are provided in the database credentials
    /// file
    /// @throw RunTimeFail if an unexpected error occurs at runtime
    /// (e.g. cannot connect to the database)
    void run(std::string const& arguments);

    void run(int const argc, char* const argv[]);

    std::tuple<int, std::string> runExceptionSafe(std::string const& arguments);

    int runExceptionSafe(int const argc, char* const argv[]);

    template <isc::dhcp::DhcpSpaceType D>
    static isc::data::ElementPtr configureDhcpvServer(DhcpvSrv<D>& server,
                                                      isc::data::ElementPtr const& config_set,
                                                      bool const check_only = false);

    static std::string_view constexpr APP_NAME = "kea-config-tool";
    static std::string_view constexpr BIN_NAME = "kea-config-tool";

    std::stringstream stream_;
    std::ostream& out_;

    /// @brief Defines the application name, it may be used to locate
    /// configuration data and appear in log statements.
    std::string app_name_;

    /// @brief Defines the executable name, by convention this should match
    /// the executable name.
    std::string bin_name_;

    /// @brief Defines the logger used within kea-config-tool.
    isc::log::Logger logger_;

    CommandOptions options_;
    std::unordered_map<std::string, std::string> env_;

    static std::mutex mutex_;

private:
    /// @brief non-copyable
    /// @{
    Controller(Controller const&) = delete;
    Controller& operator=(Controller const&) = delete;
    /// @}
};

template <isc::dhcp::DhcpSpaceType D, bool has_IETF>
struct VersionedController {
    using CM = isc::dhcp::ConfigurationManager<D, has_IETF>;

    VersionedController(Controller& c)
        : stream_(c.stream_), out_(c.out_), app_name_(c.app_name_), bin_name_(c.bin_name_),
          logger_(c.logger_), options_(c.options_), env_(c.env_) {
    }

    void launch() {
        // Destroy managers so they close their connections.
        isc::dhcp::HostMgr::create();
        isc::dhcp::HostMgr::delAllBackends();
        isc::dhcp::LeaseMgrFactory::destroy();
        isc::dhcp::MasterConfigMgrFactory<D>::destroy();
        isc::dhcp::ShardConfigMgrFactory<D>::destroy();
        isc::dhcp::SubnetMgrFactory<D>::destroy();

#ifdef TERASTREAM
        if (options_.count("debug")) {
            isc::db::DatabaseConnection::enable_all_traces_ = true;
        }
#endif  // TERASTREAM

        // Parse master.ini from file.
        isc::data::ElementPtr master_ini(isc::data::Element::fromJSONFile(
            options_["credentials-file"].template as<std::string>()));
        if (!master_ini) {
            isc_throw(isc::BadValue, "no configuration found");
        }
        // Let's do sanity check before we call json->get() which
        // works only for map.
        if (master_ini->getType() != isc::data::Element::map) {
            isc_throw(isc::BadValue,
                      "configuration file is expected to be a map, i.e. start with { and end with }"
                          << isc::data::prettyPrint(master_ini)
                          << " is a valid JSON, but its top element is a "
                          << magic_enum::enum_name(master_ini->getType()) << " instead of a "
                          << magic_enum::enum_name(isc::data::Element::map)
                          << ". Did you forget to add { } around your configuration?");
        }

        // Set credentials into staging configuration so that we can connect to:
        // - master database in the case of "master-database"
        // - shard database in the case of "config-database"
        CM::configureCredentialsToStaging(CM::getDhcp(master_ini));

        std::string const& shards_directory_path(
            options_["shards-directory-path"].template as<std::string>());

        std::set<std::string> options;
        for (auto const& [key, value] : options_) {
            options.insert(key);
        }

        if (options_.count("add-subnets")) {
            addSubnetsConfig(master_ini, options_.subnet_list_, options_.item_list_, options,
                             shards_directory_path);
        } else if (options_.count("del-subnets")) {
            delSubnetsConfig(master_ini, options_.subnet_list_, options_.item_list_, options,
                             shards_directory_path);
        } else if (options_.count("get-config")) {
            getShardsConfig(master_ini, options_.subnet_list_, options_.item_list_, options,
                            shards_directory_path);
        } else if (options_.count("get-servers")) {
            masterGetServers(master_ini, shards_directory_path);
        } else if (options_.count("get-subnets")) {
            getSubnetsConfig(master_ini, options_.subnet_list_, options_.item_list_, options,
                             shards_directory_path);
        } else if (options_.count("set-config")) {
            setShardsConfig(master_ini, options_.subnet_list_, options_.item_list_, options,
                            shards_directory_path);
        } else if (options_.count("set-servers")) {
            masterSetServers(master_ini, options_.item_list_, options, shards_directory_path);
        } else if (options_.count("set-subnets")) {
            setSubnetsConfig(master_ini, options_.subnet_list_, options_.item_list_, options,
                             shards_directory_path);
        }
    }

    static auto configureDummyDhcpServer(isc::data::ElementPtr const& dhcp) {
        bool const previousServerMode(isc::dhcp::IfaceMgr::instance().isServerMode());
        isc::util::Defer(
            [&] { isc::dhcp::IfaceMgr::instance().setServerMode(previousServerMode); });
        isc::dhcp::IfaceMgr::instance().setServerMode(false);

        DhcpvSrv<D> server(0);

        // Clear configured options & subnets to avoid conflicts if kea-config-tool is run as
        // controller e.g. kea-netconf rather than as a separate process.
        isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgOptionDef()->clear();
        isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgSubnets<D>()->clear();

        isc::data::ElementPtr const& answer(configureDhcpServer(server, dhcp));

        // Check if hooks libraries have been configured succesfully.
        int return_code;
        try {
            isc::config::parseAnswer(return_code, answer);
        } catch (std::exception const& ex) {
            isc_throw(RunTimeFail, PRETTY_FUNCTION_NAME() << ": parsing answer after configuring "
                                                             "hooks library yielded "
                                                          << ex.what());
        }
        if (return_code != 0) {
            isc_throw(RunTimeFail, PRETTY_FUNCTION_NAME()
                                       << ": configuring hooks library returned " << return_code
                                       << ", " << isc::config::answerToText(answer));
        }

        return std::make_tuple(return_code, answer);
    }

    static void action(std::string const& master,
                       std::string const& shard,
                       std::string const& hook_library,
                       std::string const& parameters,
                       std::string const& subnet,
                       isc::data::effect_t const& effect,
                       isc::data::ElementPtr const& config,
                       std::string const& config_timestamp_path) {
        std::string const& toplevel_xpath(isc::dhcp::HookLibraryConstants<D, has_IETF>::xpath(
            master, shard, hook_library, parameters));

        // Filter in the hook library config.
        isc::data::ElementPtr hook_library_config(config->xpath(toplevel_xpath));
        if (!hook_library_config) {
            hook_library_config = isc::data::Element::createMap();
        }

        isc::data::ElementPtr const& filtered_config(isc::data::Element::createMap());
        filtered_config->xpath(toplevel_xpath, hook_library_config);

        isc::data::ElementPtr network_topology(
            filtered_config->xpath(isc::dhcp::HookLibraryConstants<D, has_IETF>::xpath(
                                       master, shard, hook_library, parameters, subnet) +
                                   "/network-topology"));
        if (!network_topology) {
            network_topology = isc::data::Element::createList();
        }

        isc::data::ElementPtr shard_config(
            filtered_config->xpath(isc::dhcp::ShardConstants::xpath(master, shard)));
        if (!shard_config) {
            shard_config = isc::data::Element::createMap();
        }

        isc::data::ElementPtr const& netconf(isc::data::Element::createMap());
        netconf->set("effect", isc::data::Effect::convert(effect));
        netconf->set("master", master);
        netconf->set("shard", shard);
        netconf->set("subnet", subnet);
        netconf->set("network-topology", network_topology);

        isc::data::ElementPtr const& arguments(isc::data::Element::createMap());
        arguments->set("config.timestamp", config_timestamp_path);
        arguments->set("netconf", netconf);

        isc::dhcp::HookLibraryManager<D, has_IETF>::setParameters(
            shard_config, isc::data::Element::create("granular-change"), arguments);
        configureDummyDhcpServer(CM::getDhcp(shard_config));
        isc::dhcp::HookLibraryManager<D, has_IETF>::removeParameters(shard_config);
    }

private:
    /// @brief non-copyable
    /// @{
    VersionedController() = delete;
    VersionedController(VersionedController const&) = delete;
    VersionedController& operator=(VersionedController const&) = delete;
    /// @}

    void addSubnetsConfig(isc::data::ElementPtr const& master_ini,
                          subnet_list_t& subnet_list,
                          shard_set_t& item_list,
                          std::set<std::string>& options,
                          std::string const& shards_directory_path) {
        LOG_INFO(logger_, CONFIG_TOOL_ADD_SUBNETS_CONFIG);
        setShardsConfigCommon(master_ini, subnet_list, item_list, options, shards_directory_path);
    }

    void addSubnet(isc::data::ElementPtr const& subnets,
                   isc::dhcp::SubnetInfoPtr<D> const& subnet_info) {
        bool found(false);
        for (isc::data::ElementPtr s : subnets->listValue()) {
            if (s->get("subnet")->stringValue() == subnet_info->strings_.subnet_) {  // Overwrite.
                s = subnet_info->toElement();
                found = true;
                break;
            }
        }
        if (!found) {
            subnets->add(subnet_info->toElement());
        }
    }

    void addSubnetsToConfig(isc::data::ElementPtr const& config,
                            std::vector<std::string>& subnet_list,
                            std::set<std::string>& options,
                            std::string const& config_timestamp_path) {
        isc::data::ElementPtr const& dhcp(CM::getDhcp(config));
        std::string const& subnet("subnet" + isc::dhcp::dhcpSpaceToString<D>());
        if (!dhcp->contains(subnet)) {
            dhcp->set(subnet, isc::data::Element::createList());
        }
        isc::data::ElementPtr subnets(dhcp->get(subnet));
        if (options.count("get-subnets")) {
            for (std::string& subnet : subnet_list) {
                isc::dhcp::SubnetInfoPtr<D> subnet_info(
                    isc::dhcp::SubnetMgrFactory<D>::instance().selectBySubnet(
                        subnet, config_timestamp_path));
                if (subnet_info) {
                    addSubnet(subnets, subnet_info);
                }
            }
        } else {
            isc::dhcp::SubnetInfoCollection<D> collection(
                isc::dhcp::SubnetMgrFactory<D>::instance().select(config_timestamp_path));
            for (isc::dhcp::SubnetInfoPtr<D> const& subnet_info : collection) {
                addSubnet(subnets, subnet_info);
            }
        }
    }

    void configureInstanceIDToStaging(std::string const& instance_ID) const {
        isc::dhcp::CfgMgr::instance().getStagingCfg()->getInstanceID() = instance_ID;
    }

    static isc::data::ElementPtr configureDhcpServer(DhcpvSrv<D>& server,
                                                     isc::data::ElementPtr const& config_set,
                                                     bool const check_only = false) {
        return Controller::configureDhcpvServer<D>(server, config_set, check_only);
    }

    void delSubnetsConfig(isc::data::ElementPtr const& master_ini,
                          subnet_list_t& subnet_list,
                          shard_set_t& item_list,
                          std::set<std::string>& options,
                          std::string const& shards_directory_path) {
        LOG_INFO(logger_, CONFIG_TOOL_DEL_SUBNETS_CONFIG);
        setShardsConfigCommon(master_ini, subnet_list, item_list, options, shards_directory_path);
    }

    isc::util::shard_list_t selectShardsName() {
        isc::util::shard_list_t shards_list;
        isc::dhcp::MasterConfigMgrFactory<D>::instance().selectShardNames(shards_list);
        return shards_list;
    }

    void getShardJSONConfig(std::string const& shard,
                            subnet_list_t& subnet_list,
                            isc::data::ElementPtr const& credentials_config,
                            std::set<std::string>& options,
                            std::string const& output_config_path) {
        LOG_INFO(logger_, CONFIG_TOOL_GET_JSON_CONFIG);
        out_ << "Starting to retrieve the JSON configuration for shard '" << shard << "'..."
             << std::endl;

        if (shard.empty() || output_config_path.empty()) {
            isc_throw(InvalidUsage,
                      "neither the shard name nor the output config directory have been specified");
        }

        isc::dhcp::ShardConfigPtr config_data(readShardJSONConfig(
            shard, credentials_config, output_config_path + "/config.timestamp"));
        if (!config_data) {
            return;
        }

        // Configure hooks libraries.
        isc::data::ElementPtr config(config_data->configuration());
        std::string config_timestamp_path(output_config_path + "/config.timestamp");
        isc::dhcp::HookLibraryManager<D, has_IETF>::setParameters(
            config, determineCommand(options),
            determineArguments(options, subnet_list[shard], config_timestamp_path));
        configureDummyDhcpServer(CM::getDhcp(config));
        isc::dhcp::HookLibraryManager<D, has_IETF>::removeParameters(config);

        // Add subnets.
        addSubnetsToConfig(config, subnet_list[shard], options, config_timestamp_path);

        isc::util::File config_json(output_config_path + "/config.json", true,
                                    isc::data::prettyPrint(config));
    }

    void getShardsConfig(isc::data::ElementPtr const& master_ini,
                         subnet_list_t& subnet_list,
                         shard_set_t& item_list,
                         std::set<std::string>& options,
                         std::string const& shards_directory_path) {
        LOG_INFO(logger_, CONFIG_TOOL_GET_SHARDS_CONFIG);
        getShardsConfigCommon(master_ini, subnet_list, item_list, options, shards_directory_path);
    }

    void getShardsConfigCommon(isc::data::ElementPtr const& master_ini,
                               subnet_list_t& subnet_list,
                               shard_set_t& item_list,
                               std::set<std::string>& options,
                               std::string const& shards_directory_path) {
        shard_set_t filtered_shards_list;
        isc::data::ElementPtr credentials_config;

        std::string instance_ID;
        if (isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() ==
            isc::dhcp::SrvConfigType::MASTER_DATABASE) {
            instance_ID = CM::getInstanceID(master_ini);
            isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createMasterCfgMgrs<D>(
                instance_ID);
            filtered_shards_list = item_list;
        } else if (isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() ==
                   isc::dhcp::SrvConfigType::CONFIG_DATABASE) {
            // Get 'config-database' component from the config.
            isc::data::ElementPtr config_database(CM::getDhcp(master_ini)->get("config-database"));
            if (!config_database) {
                isc_throw(isc::BadValue,
                          "no mandatory 'config-database' entry in the configuration");
            }
            credentials_config = config_database;
            std::string database_name_key("name");
            isc::data::ElementPtr database_name_node(config_database->get(database_name_key));
            if (!database_name_node) {
                database_name_key = "keyspace";
                database_name_node = config_database->get(database_name_key);
            }
            if (!database_name_node) {
                isc_throw(isc::BadValue,
                          "neither 'name' nor 'keyspace' entry in 'config-database'");
            }
            if (database_name_node->getType() != isc::data::Element::string) {
                isc_throw(isc::BadValue, "'" << database_name_key << "' is of type '"
                                             << magic_enum::enum_name(database_name_node->getType())
                                             << "', expected '"
                                             << magic_enum::enum_name(isc::data::Element::string)
                                             << "'");
            }
            std::string database_name(database_name_node->stringValue());
            if (item_list.find(database_name) != item_list.end()) {
                out_ << "Detected credentials for shard " << database_name << std::endl;
                filtered_shards_list.insert(database_name_node->stringValue());
            }
            for (std::string const& shard : item_list) {
                if (filtered_shards_list.find(shard) == filtered_shards_list.end()) {
                    out_ << "Unknown credentials for shard " << shard << std::endl;
                }
            }
        } else {
            isc_throw(RunTimeFail,
                      "unknown configuration type " << magic_enum::enum_name(
                          isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType()));
        }

        for (std::string const& shard : filtered_shards_list) {
            if (isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() ==
                isc::dhcp::SrvConfigType::MASTER_DATABASE) {
                // Read the contents from database.
                isc::dhcp::MasterConfigPtr tool_config;
                std::string shard_instance_ID(instance_ID + "@" + shard);
                try {
                    tool_config =
                        isc::dhcp::MasterConfigMgrFactory<D>::instance().select(shard_instance_ID);
                } catch (isc::db::MultipleRecords const& exception) {
                    out_ << "WARNING: two or more credentials found in master database with "
                            "instance ID"
                         << instance_ID << "' for shard '" << shard << "', skipping..."
                         << std::endl;
                    continue;
                }

                if (!tool_config) {
                    out_ << "WARNING: no credentials found in master database with "
                            "instance ID '"
                         << instance_ID << "' for shard '" << shard << "', skipping..."
                         << std::endl;
                    continue;
                }

                credentials_config = tool_config->database_credentials();
            }

            isc::util::Dir shard_directory(shards_directory_path + "/" + shard, false, true);
            getShardJSONConfig(shard, subnet_list, credentials_config, options, shard_directory());

            isc::data::ElementPtr json_data(isc::data::Element::createMap());
            if (credentials_config) {
                json_data->set("config-database", credentials_config);
            }

            isc::util::File credentials_json(shard_directory() + "/credentials.json", true,
                                             isc::data::prettyPrint(json_data));
        }
    }

    void getSubnetsConfig(isc::data::ElementPtr const& master_ini,
                          subnet_list_t& subnet_list,
                          shard_set_t& item_list,
                          std::set<std::string>& options,
                          std::string const& shards_directory_path) {
        LOG_INFO(logger_, CONFIG_TOOL_GET_SUBNETS_CONFIG);
        getShardsConfigCommon(master_ini, subnet_list, item_list, options, shards_directory_path);
    }

    std::string const& table() {
#ifdef HAVE_CQL
        return isc::dhcp::CqlMasterConfigExchange<D>::table();
#else
        isc_throw(InvalidUsage, "not built with Cassandra support");
#endif
    }

    std::string masterCredentialsToJSON(isc::dhcp::MasterConfigCollection& masters) {
        std::string staged_instance_ID(
            isc::dhcp::CfgMgr::instance().getStagingCfg()->getInstanceID());
        isc::data::ElementPtr config_database;
        for (isc::dhcp::MasterConfigPtr const& server : masters) {
            if (server->instance_ID_ == staged_instance_ID + "@" + server->shard_) {
                config_database = server->database_credentials();
                break;
            }
        }

        // Serializing root-level JSON.
        isc::data::ElementPtr json(isc::data::Element::createMap());
        if (config_database) {
            json->set("config-database", config_database);
        }

        return isc::data::prettyPrint(json);
    }

    void masterGetServers(isc::data::ElementPtr const& master_ini,
                          std::string const& shards_directory_path) {
        LOG_INFO(logger_, CONFIG_TOOL_MASTER_GET_SERVERS);
        out_ << "Starting to retrieve the configuration from the master database..." << std::endl;

        if (isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() !=
            isc::dhcp::SrvConfigType::MASTER_DATABASE) {
            isc_throw(isc::BadValue, "no mandatory 'master-database' entry in the configuration");
        }

        std::string const& instance_ID(CM::getInstanceID(master_ini));
        isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createMasterCfgMgrs<D>(
            instance_ID);

        isc::dhcp::MasterConfigCollection master_configurations(
            getMasterConfigurations(shards_directory_path + "/config.timestamp"));

        // Write to file hierarchy.
        std::string credentials_data;
        std::string json_data;
        if (!master_configurations.empty()) {
            credentials_data = masterCredentialsToJSON(master_configurations);
            json_data = masterConfigDataToJSON(master_configurations);
        }
        for (isc::dhcp::MasterConfigPtr const& master_configuration : master_configurations) {
            isc::util::Dir shard_directory(
                shards_directory_path + "/" + master_configuration->shard_, false, true);
            isc::util::File credentials_json(shard_directory() + "/credentials.json", true,
                                             credentials_data);
            isc::util::File servers_file(shard_directory() + "/servers.json", true, json_data);
        }
    }

    isc::dhcp::MasterConfigCollection
    getMasterConfigurations(std::string const& config_timestamp_path) {
        isc::dhcp::MasterConfigCollection vectorized_master_configurations;
        std::map<std::string, isc::dhcp::MasterConfigPtr> master_configurations;
        isc::dhcp::MasterConfigMgrFactory<D>::instance().startTransaction();
        try {
            isc::util::shard_list_t shards_list(selectShardsName());
            for (std::string const& shard : shards_list) {
                isc::dhcp::MasterConfigCollection configurations(
                    isc::dhcp::MasterConfigMgrFactory<D>::instance().selectByShard(
                        shard, config_timestamp_path));
                for (isc::dhcp::MasterConfigPtr& master_configuration : configurations) {
                    master_configurations.try_emplace(master_configuration->instance_ID_,
                                                      master_configuration);
                }
            }
        } catch (std::exception const& ex) {
            isc::dhcp::MasterConfigMgrFactory<D>::instance().rollback();
            throw;
        }
        isc::dhcp::MasterConfigMgrFactory<D>::instance().commit();

        for (auto const& [instance_ID, master_configuration] : master_configurations) {
            vectorized_master_configurations.push_back(master_configuration);
        }

        return vectorized_master_configurations;
    }

    isc::dhcp::MasterConfigCollection getMasterConfigurationsFromFileHierarchy(
        isc::data::ElementPtr const& master_ini,
        std::vector<isc::util::FileInfo> filtered_shard_directories) {
        isc::dhcp::MasterConfigCollection master_configurations;
        std::string const& instance_ID(CM::getInstanceID(master_ini));

        // Parse file hierarchy into Collection<MasterConfigT<D>>.
        for (isc::util::FileInfo const& shard : filtered_shard_directories) {
            isc::dhcp::MasterConfigCollection master_configurations_for_this_shard(
                parseMastersJSON(shard.name_, shard.full_path_ + "/servers.json"));
            master_configurations.insert(master_configurations.end(),
                                         master_configurations_for_this_shard.begin(),
                                         master_configurations_for_this_shard.end());
        }

        // Set kea-config-tool@shard entry.
        for (isc::util::FileInfo const& shard : filtered_shard_directories) {
            isc::data::ElementPtr credentials_database(
                parseMasterCredentialsJSON(shard.name_, shard.full_path_ + "/credentials.json"));
            isc::dhcp::MasterConfigPtr config_tool_config(
                std::make_shared<isc::dhcp::MasterConfig>());
            config_tool_config->instance_ID_ = instance_ID + "@" + shard.name_;
            config_tool_config->shard_ = shard.name_;
            config_tool_config->database_credentials(credentials_database);
            master_configurations.push_back(config_tool_config);
        }

        return master_configurations;
    }

    void masterSetServers(isc::data::ElementPtr const& master_ini,
                          shard_set_t& item_list,
                          std::set<std::string>& options,
                          std::string const& shards_directory_path) {
        LOG_INFO(logger_, CONFIG_TOOL_MASTER_UPDATE_SERVERS);
        out_ << "Starting to update the configuration in the master database..." << std::endl;

        // Read directories representing shards.
        std::vector<isc::util::FileInfo> shard_directories(
            isc::util::Filesystem::directoriesAtPath(shards_directory_path));
        if (shard_directories.size() == 0) {
            isc_throw(RunTimeFail, "there are no shards in the provided directory");
        }

        std::vector<isc::util::FileInfo> filtered_shard_directories(
            filterShardDirectories(shard_directories, item_list));

        if (isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() !=
            isc::dhcp::SrvConfigType::MASTER_DATABASE) {
            isc_throw(isc::BadValue, "no mandatory 'master-database' entry in the configuration");
        }

        isc::dhcp::MasterConfigCollection master_configurations(
            getMasterConfigurationsFromFileHierarchy(master_ini, filtered_shard_directories));

        std::string const& instance_ID(CM::getInstanceID(master_ini));
        isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createMasterCfgMgrs<D>(
            instance_ID);

        if (options.count("dry-run")) {
            return;
        }

        setMasterConfigurations(master_configurations, shards_directory_path + "/config.timestamp");
    }

    std::string masterConfigDataToJSON(isc::dhcp::MasterConfigCollection& masters) {
        // Serialize all 'master-config' elements.
        isc::data::ElementPtr config_database;
        std::string config_database_str;
        bool found(false);
        isc::data::ElementPtr master_configs(isc::data::Element::createList());
        for (isc::dhcp::MasterConfigPtr const& server : masters) {
            if (server->instance_ID_ ==
                isc::dhcp::CfgMgr::instance().getStagingCfg()->getInstanceID() + "@" +
                    server->shard_) {
                continue;
            }
            // Populate config-database while checking to be the same for all master servers.
            if (!found) {
                // if 'config-database' was indeed empty than it should have been an empty map "{}".
                if (!server->database_credentials()) {
                    isc_throw(isc::BadValue,
                              "'config-database' is empty, would result in invalid JSON");
                }
                found = true;
                config_database = server->database_credentials();
                config_database_str = server->database_credentials_string();
            } else if (config_database_str != server->database_credentials_string()) {
                isc_throw(isc::BadValue,
                          "expected the same 'config-database' for all master servers, but "
                          "they are different: "
                              << std::endl
                              << config_database_str << std::endl
                              << server->database_credentials_string() << std::endl);
            }

            isc::data::ElementPtr master(isc::data::Element::createMap());
            master->set("instance-id", server->instance_ID_);
            isc::data::ElementPtr const&& server_specific_configuration(
                server->server_specific_configuration());
            if (server_specific_configuration) {
                // It is empty for kea-config-tool@shard entries.
                master->set("server-config", server_specific_configuration);
            }
            // Add the element's children.
            master_configs->add(master);
        }

        // Root-level JSON.
        isc::data::ElementPtr json_data(isc::data::Element::createMap());
        if (config_database) {
            json_data->set("config-database", config_database);
        }
        if (master_configs) {
            json_data->set("master-config", master_configs);
        }

        return isc::data::prettyPrint(json_data);
    }

    /// @brief Receives credentials data for shard and turns it into ElementPtr
    isc::data::ElementPtr parseMasterCredentialsJSON(std::string const& shard,
                                                     std::string const& credentials_config_file) {
        // Read contents of the file and parse it as JSON.
        isc::data::ElementPtr json(isc::data::Element::fromJSONFile(credentials_config_file));
        if (!json) {
            isc_throw(isc::BadValue, "no configuration found");
        }

        // Let's do sanity check before we call json->get() which
        // works only for map.
        if (json->getType() != isc::data::Element::map) {
            isc_throw(isc::BadValue,
                      "configuration file is expected to be a map, i.e. start with { and end with }"
                          << credentials_config_file
                          << " is a valid JSON, but its top element is a "
                          << magic_enum::enum_name(json->getType()) << " instead of a "
                          << magic_enum::enum_name(isc::data::Element::map)
                          << ". Did you forget to add { } around your configuration?");
        }

        // Get 'config-database' component from the config.
        isc::data::ElementPtr result(json->get("config-database"));

        // Sanity checks
        if (!result) {
            isc_throw(isc::BadValue, "no mandatory 'config-database' entry in the configuration");
        }
        std::string database_name_key("name");
        isc::data::ElementPtr database_name_node(result->get(database_name_key));
        if (!database_name_node) {
            database_name_key = "keyspace";
            database_name_node = result->get(database_name_key);
        }
        if (!database_name_node) {
            isc_throw(isc::BadValue, "neither 'name' nor 'keyspace' entry in 'config-database'");
        }
        if (database_name_node->getType() != isc::data::Element::string) {
            isc_throw(isc::BadValue,
                      "'" << database_name_key << "' is of type '"
                          << magic_enum::enum_name(database_name_node->getType()) << "', expected '"
                          << magic_enum::enum_name(isc::data::Element::string) << "'");
        }

        if (database_name_node->stringValue() != shard) {
            isc_throw(isc::BadValue, "the shard's directory name '"
                                         << shard
                                         << "' should be the same as the shard's database name '"
                                         << database_name_node->stringValue()
                                         << "' specified in `config-database`.");
        }

        return result;
    }

    isc::dhcp::MasterConfigCollection parseMastersJSON(std::string const& shard,
                                                       std::string const& servers_config_file) {
        isc::dhcp::MasterConfigCollection master_configurations;

        // Read contents of the file and parse it as JSON.
        isc::data::ElementPtr json(isc::data::Element::fromJSONFile(servers_config_file));
        if (!json) {
            isc_throw(isc::BadValue, "no configuration found");
        }

        // Let's do sanity check before we call json->get() which
        // works only for map.
        if (json->getType() != isc::data::Element::map) {
            isc_throw(isc::BadValue,
                      "configuration file is expected to be a map, i.e. start with { and end with }"
                          << servers_config_file << " is a valid JSON, but its top element is a "
                          << magic_enum::enum_name(json->getType()) << " instead of a "
                          << magic_enum::enum_name(isc::data::Element::map)
                          << ". Did you forget to add { } around your configuration?");
        }

        // Get 'config-database' component from the config.
        isc::data::ElementPtr config_database(json->get("config-database"));
        if (!config_database) {
            isc_throw(isc::BadValue, "no mandatory 'config-database' entry in the configuration");
        }
        std::string database_name_key("name");
        isc::data::ElementPtr database_name_node(config_database->get(database_name_key));
        if (!database_name_node) {
            database_name_key = "keyspace";
            database_name_node = config_database->get(database_name_key);
        }
        if (!database_name_node) {
            isc_throw(isc::BadValue, "neither 'name' nor 'keyspace' entry in 'config-database'");
        }

        if (database_name_node->stringValue() != shard) {
            isc_throw(isc::BadValue, "the shard's directory name '"
                                         << shard
                                         << "' should be the same as the shard's database name '"
                                         << database_name_node->stringValue()
                                         << "' specified in `config-database`.");
        }

        // Get 'master-config' component from the config.
        isc::data::ElementPtr master_config_nodes(json->get("master-config"));
        if (!master_config_nodes) {
            isc_throw(isc::BadValue, "no mandatory 'master-config' entry in the configuration");
        }

        for (isc::data::ElementPtr master_config_node : master_config_nodes->listValue()) {
            isc::data::ElementPtr instance_ID(master_config_node->get("instance-id"));
            if (!instance_ID) {
                isc_throw(isc::BadValue, "no mandatory 'instance-id' entry in the 'master-config' "
                                         "part of the configuration");
            }

            isc::dhcp::MasterConfigPtr master_config(std::make_shared<isc::dhcp::MasterConfig>());
            master_config->server_specific_configuration(master_config_node->get("server-config"));
            master_config->instance_ID_ = instance_ID->stringValue();
            master_config->shard_ = shard;
            master_config->database_credentials(config_database);
            master_configurations.push_back(master_config);
        }
        return master_configurations;
    }

    isc::dhcp::ShardConfigPtr readShardJSONConfig(std::string const& config_database_name,
                                                  isc::data::ElementPtr const& credentials_config,
                                                  std::string const& config_timestamp_path) {
        try {
            isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createShardCfgMgrs<D>(
                {credentials_config});
        } catch (std::exception const& ex) {
            isc_throw(RunTimeFail, "unable to open the database: " << config_database_name
                                                                   << ". Reason: " << ex.what());
        }

        isc::dhcp::ShardConfigPtr config_data;

        isc::dhcp::ShardConfigMgrFactory<D>::instance().startTransaction();
        try {
            // Read the contents from database.
            config_data =
                isc::dhcp::ShardConfigMgrFactory<D>::instance().select(config_timestamp_path);
        } catch (isc::db::MultipleRecords const&) {
            isc::dhcp::ShardConfigMgrFactory<D>::instance().rollback();
            isc_throw(RunTimeFail, "two or more entries found in the database");
        } catch (std::exception const& ex) {
            isc::dhcp::ShardConfigMgrFactory<D>::instance().rollback();
            throw;
        }
        isc::dhcp::ShardConfigMgrFactory<D>::instance().commit();

        return config_data;
    }

    isc::data::ElementPtr
    determineArguments(std::set<std::string> const& options,
                       std::vector<std::string> const& input_subnets,
                       std::string const& config_timestamp_path = std::string()) {
        isc::data::ElementPtr subnets(isc::data::Element::createList());
        for (std::string const& subnet : input_subnets) {
            subnets->add(isc::data::Element::create(subnet));
        }
        isc::data::ElementPtr arguments(isc::data::Element::createMap());
        arguments->set("subnets", subnets);
        if (!config_timestamp_path.empty()) {
            arguments->set("config.timestamp", config_timestamp_path);
        }
        if (options.count("dry-run")) {
            arguments->set("dry-run", true);
        }

        return arguments;
    }

    isc::data::ElementPtr determineCommand(std::set<std::string> const& options) {
        isc::data::ElementPtr command(isc::data::Element::create(std::string()));
        for (std::string const& o : {"get-config", "get-subnets", "set-config", "add-subnets",
                                     "del-subnets", "set-subnets"}) {
            if (options.count(o)) {
                command->setValue(o);
                break;
            }
        }
        return command;
    }

    static constexpr bool DRY_RUN = true;

    void setMasterConfigurations(isc::dhcp::MasterConfigCollection& master_configs,
                                 std::string const& config_timestamp_path) {
        isc::dhcp::MasterConfigMgrFactory<D>::instance().startTransaction();
        try {
            // Get current master IDs.
            std::list<std::string> master_IDs(
                isc::dhcp::MasterConfigMgrFactory<D>::instance().selectInstanceIDs());

            // Delete extra.
            for (std::string& master_ID : master_IDs) {
                bool found(false);
                for (isc::dhcp::MasterConfigPtr const& master_config : master_configs) {
                    if (master_config->instance_ID_ == master_ID) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    isc::dhcp::MasterConfigMgrFactory<D>::instance().del(master_ID,
                                                                         config_timestamp_path);
                }
            }

            // Upsert existing or new.
            for (isc::dhcp::MasterConfigPtr& master_config : master_configs) {
                try {
                    isc::dhcp::MasterConfigMgrFactory<D>::instance().update(master_config,
                                                                            config_timestamp_path);
                } catch (isc::db::StatementNotApplied const& exception) {
                    isc::dhcp::MasterConfigMgrFactory<D>::instance().insert(master_config,
                                                                            config_timestamp_path);
                }
            }
        } catch (std::exception const& ex) {
            isc::dhcp::MasterConfigMgrFactory<D>::instance().rollback();
            throw;
        }
        isc::dhcp::MasterConfigMgrFactory<D>::instance().commit();
    }

    void setShardsConfig(isc::data::ElementPtr const& master_ini,
                         subnet_list_t& subnet_list,
                         shard_set_t& item_list,
                         std::set<std::string>& options,
                         std::string const& shards_directory_path) {
        LOG_INFO(logger_, CONFIG_TOOL_UPDATE_SHARDS_CONFIG);
        setShardsConfigCommon(master_ini, subnet_list, item_list, options, shards_directory_path);
    }

    void setShardsConfigCommon(isc::data::ElementPtr const& master_ini,
                               subnet_list_t& subnet_list,
                               shard_set_t& item_list,
                               std::set<std::string>& options,
                               std::string const& shards_directory_path) {
        LOG_INFO(logger_, CONFIG_TOOL_UPDATE_SHARDS_CONFIG);

        std::vector<isc::util::FileInfo> shard_directories(
            isc::util::Filesystem::directoriesAtPath(shards_directory_path));

        if (isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() ==
            isc::dhcp::SrvConfigType::MASTER_DATABASE) {
            std::string const& instance_ID(CM::getInstanceID(master_ini));
            isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createMasterCfgMgrs<D>(
                instance_ID);
        } else if (isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() ==
                   isc::dhcp::SrvConfigType::CONFIG_DATABASE) {
            // Get 'config-database' component from the config.
            isc::data::ElementPtr config_database(CM::getDhcp(master_ini)->get("config-database"));
            if (!config_database) {
                isc_throw(isc::BadValue,
                          "no mandatory 'config-database' entry in the configuration");
            }
            std::string database_name_key("name");
            isc::data::ElementPtr database_name_node(config_database->get(database_name_key));
            if (!database_name_node) {
                database_name_key = "keyspace";
                database_name_node = config_database->get(database_name_key);
            }
            if (!database_name_node) {
                isc_throw(isc::BadValue,
                          "neither 'name' nor 'keyspace' entry in 'config-database'");
            }
            if (database_name_node->getType() != isc::data::Element::string) {
                isc_throw(isc::BadValue, "'" << database_name_key << "' is of type '"
                                             << magic_enum::enum_name(database_name_node->getType())
                                             << "', expected '"
                                             << magic_enum::enum_name(isc::data::Element::string)
                                             << "'");
            }
            std::string database_name(database_name_node->stringValue());
            out_ << "Searching credentials for shard " << database_name << std::endl;
            std::vector<isc::util::FileInfo> filtered_shard_directories;
            for (isc::util::FileInfo const& shard : shard_directories) {
                if (database_name == shard.name_) {
                    out_ << "Detected credentials for shard " << shard.name_ << std::endl;
                    filtered_shard_directories.push_back(shard);
                } else {
                    out_ << "Unknown credentials for shard " << shard.name_ << std::endl;
                }
            }
            shard_directories = filtered_shard_directories;
        } else {
            isc_throw(RunTimeFail, "unknown configuration type");
        }

        if (shard_directories.size() == 0) {
            isc_throw(RunTimeFail, "there are no shards in the provided directory");
        }

        std::vector<isc::util::FileInfo> filtered_shard_directories(
            filterShardDirectories(shard_directories, item_list));

        for (isc::util::FileInfo const& shard : filtered_shard_directories) {
            std::string credentials_cfg_file(shard.full_path_ + "/credentials.json");
            std::string json_cfg_file(shard.full_path_ + "/config.json");
            std::string timestamp_file(shard.full_path_ + "/config.timestamp");

            isc::data::ElementPtr master_credentials;
            if (isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() ==
                isc::dhcp::SrvConfigType::MASTER_DATABASE) {
                master_credentials = parseMasterCredentialsJSON(shard.name_, credentials_cfg_file);
            }

            updateShardConfig(shard.name_, subnet_list, master_credentials, json_cfg_file, options,
                              timestamp_file);
        }
    }

    std::vector<isc::util::FileInfo>
    filterShardDirectories(std::vector<isc::util::FileInfo> const& shard_directories,
                           shard_set_t const& item_list) {
        std::vector<isc::util::FileInfo> filtered_shard_directories;
        if (item_list.empty()) {
            filtered_shard_directories = shard_directories;
        } else {
            for (isc::util::FileInfo const& shard : shard_directories) {
                if (item_list.find(shard.name_) != item_list.end()) {
                    filtered_shard_directories.push_back(shard);
                }
            }
        }
        return filtered_shard_directories;
    }

    void setSubnetsConfig(isc::data::ElementPtr const& master_ini,
                          subnet_list_t& subnet_list,
                          shard_set_t& item_list,
                          std::set<std::string>& options,
                          std::string const& shards_directory_path) {
        LOG_INFO(logger_, CONFIG_TOOL_UPDATE_SUBNETS_CONFIG);
        setShardsConfigCommon(master_ini, subnet_list, item_list, options, shards_directory_path);
    }

    void setUniverseInStaging() {
        isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->setAppendedParameters(
            "universe=" + isc::dhcp::dhcpSpaceToString<D>());
    }

    void updateShardConfig(std::string const& shard,
                           subnet_list_t subnet_list,
                           isc::data::ElementPtr const& credentials_config,
                           std::string const& input_json_config_file,
                           std::set<std::string>& options,
                           std::string const& config_timestamp_path) {
        LOG_INFO(logger_, CONFIG_TOOL_UPDATE_CONFIG).arg(shard);
        out_ << "Starting to update the configuration for shard '" << shard << "'..." << std::endl;

        if (shard.empty()) {
            isc_throw(InvalidUsage, "the shard name cannot be empty");
        }

        isc::util::File config_json(input_json_config_file, true, std::string());
        std::string json_config;
        config_json >> json_config;

        // Configure hooks libraries.
        isc::data::ElementPtr config(isc::data::Element::fromJSON(json_config));
        isc::dhcp::HookLibraryManager<D, has_IETF>::setParameters(
            config, determineCommand(options),
            determineArguments(options, subnet_list[shard], config_timestamp_path));
        configureDummyDhcpServer(CM::getDhcp(config));
        isc::dhcp::HookLibraryManager<D, has_IETF>::removeParameters(config);

        if (options.count("dry-run")) {
            return;
        }

        updateShardConfig(shard, config, credentials_config, config_timestamp_path);

        isc::dhcp::HostMgr::instance().syncReservations();

        if (isc::dhcp::SubnetMgrFactory<D>::haveInstance()) {
            if (options.count("set-config")) {
                isc::dhcp::SubnetMgrFactory<D>::instance().syncSubnets(config_timestamp_path);
            } else if (options.count("add-subnets")) {
                isc::dhcp::SubnetMgrFactory<D>::instance().addSubnets(config_timestamp_path);
            } else if (options.count("del-subnets")) {
                isc::dhcp::SubnetMgrFactory<D>::instance().delSubnets(config_timestamp_path);
            } else if (options.count("set-subnets")) {
                isc::dhcp::SubnetMgrFactory<D>::instance().setSubnets(config_timestamp_path);
            }
        }
    }

    void updateShardConfig(std::string const& config_database_name,
                           isc::data::ElementPtr const& config,
                           isc::data::ElementPtr const& credentials_config,
                           std::string const& config_timestamp_path) {
        try {
            isc::dhcp::CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createShardCfgMgrs<D>(
                {credentials_config});
        } catch (std::exception const& ex) {
            isc_throw(RunTimeFail, "unable to open the database: " << config_database_name
                                                                   << ". Reason: " << ex.what());
        }

        isc::dhcp::ShardConfigPtr shard_config(std::make_shared<isc::dhcp::ShardConfig>());
        shard_config->configuration(config);
        isc::dhcp::ShardConfigMgrFactory<D>::instance().startTransaction();
        try {
            isc::dhcp::ShardConfigMgrFactory<D>::instance().upsert(shard_config,
                                                                   config_timestamp_path);
        } catch (std::exception const& ex) {
            isc::dhcp::ShardConfigMgrFactory<D>::instance().rollback();
            throw;
        }
        isc::dhcp::ShardConfigMgrFactory<D>::instance().commit();
    }

    std::stringstream& stream_;
    std::ostream& out_;

    /// @brief Defines the application name, it may be used to locate
    /// configuration data and appear in log statements.
    std::string& app_name_;

    /// @brief Defines the executable name, by convention this should match
    /// the executable name.
    std::string& bin_name_;

    /// @brief Defines the logger used within kea-config-tool.
    isc::log::Logger& logger_;

    CommandOptions& options_;
    std::unordered_map<std::string, std::string>& env_;
};

/// @brief thread-unsafe
struct Env {
    Env(std::unordered_map<std::string, std::string> e) : env_(e) {
        // Set the requested environment variables.
        swap();
    }

    ~Env() {
        // Revert the environment variables.
        swap();
    }

private:
    void swap() {
        for (std::string const& i : unset_variables) {
            unsetenv(i.c_str());
        }
        unset_variables.clear();
        std::unordered_map<std::string, std::string> env(env_);
        for (auto const& [key, value] : env) {
            char* var = getenv(key.c_str());
            setenv(key.c_str(), value.c_str(), 1);
            if (var) {
                // Remember what to revert to.
                env_[key] = var;
            } else {
                unset_variables.push_back(key);
                env_.erase(key);
            }
        }
    }

    std::unordered_map<std::string, std::string> env_;
    std::vector<std::string> unset_variables;
};

}  // namespace kea_config_tool
}  // namespace isc

#endif  // KEA_CONFIG_TOOL_CONTROLLER_H

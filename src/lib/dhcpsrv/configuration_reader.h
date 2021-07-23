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

#ifndef CONFIGURATION_READER_H
#define CONFIGURATION_READER_H

#include <cc/command_interpreter.h>
#include <cc/data.h>
#include <database/dbaccess_parser.h>
#include <dhcpsrv/configuration.h>
#include <dhcpsrv/master_config_mgr.h>
#include <dhcpsrv/master_config_mgr_factory.h>
#include <dhcpsrv/shard_config_mgr_factory.h>
#include <dhcpsrv/subnet_mgr_factory.h>
#include <process/daemon.h>
#include <util/dhcp.h>
#include <util/func.h>
#include <util/magic_enum.hpp>

namespace isc {
namespace dhcp {

template <DhcpSpaceType D, bool has_IETF>
struct ConfigurationReader {
    static std::tuple<isc::data::ElementPtr, MasterConfigPtr> readDhcpConfigFromMasterDatabase() {
        // Re-open configuration database with new parameters.
        std::string instance_ID(CfgMgr::instance().getStagingCfg()->getInstanceID());
        CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createMasterCfgMgrs<D>({instance_ID});

        // Read the contents from database.
        MasterConfigPtr const& config_data(
            MasterConfigMgrFactory<D>::instance().select(instance_ID));
        if (!config_data) {
            isc_throw(isc::BadValue, PRETTY_FUNCTION_NAME() << ": !config_data");
        }

        // Update timestamp.
        CfgMgr::instance().getStagingCfg()->setMasterCfgTimestamp(config_data->timestamp_);

        // Get the configuration.
        isc::data::ElementPtr const& json(config_data->server_specific_configuration());
        if (!json) {
            isc_throw(isc::BadValue, PRETTY_FUNCTION_NAME() << ": !server_specific_configuration");
        }
        isc::data::ElementPtr const local_dhcp_config(
            ConfigurationManager<D, has_IETF>::getDhcp(json));

        return {local_dhcp_config, config_data};
    }

    static isc::data::ElementPtr readDhcpConfigFromShardDatabase() {
        // Re-open configuration database with new parameters.
        CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->createShardCfgMgrs<D>();

        // Read the contents from database.
        ShardConfigPtr const& config_data(ShardConfigMgrFactory<D>::instance().select());
        if (!config_data) {
            isc_throw(isc::BadValue, PRETTY_FUNCTION_NAME() << ": !config_data");
        }

        // Update timestamp.
        CfgMgr::instance().getStagingCfg()->setServerCfgTimestamp(config_data->timestamp_);

        // Get the configuration.
        isc::data::ElementPtr json(config_data->configuration());
        if (!json) {
            isc_throw(isc::BadValue, "no configuration found");
        }
        isc::data::ElementPtr const dhcp_config(ConfigurationManager<D, has_IETF>::getDhcp(json));

        // Add any currently configured subnets to the configuration.
        if (SubnetMgrFactory<D>::haveInstance()) {
            if (dhcp_config->find("subnet" + dhcpSpaceToString<D>())) {
                isc::data::ElementPtr subnets(dhcp_config->find("subnet" + dhcpSpaceToString<D>()));
                SubnetInfoCollection<D> const& collection(SubnetMgrFactory<D>::instance().select());
                for (SubnetInfoPtr<D> const& subnet : collection) {
                    subnets->add(subnet->toElement());
                }
            }
        }

        return dhcp_config;
    }

    static isc::data::ElementPtr configureFromDatabase(isc::data::ElementPtr const& config) {
        // Read master/shard configuration into dhcp_config.
        isc::data::ElementPtr interfaces_config;
        if (CfgMgr::instance().getStagingCfg()->getConfigurationType() ==
            SrvConfigType::MASTER_DATABASE) {
            auto const& [dhcp_config, config_data](readDhcpConfigFromMasterDatabase());

            // If there's no logging element, we'll just pass NULL pointer,
            // which will be handled by configureLogger().
            isc::process::Daemon::configureLogger(dhcp_config, CfgMgr::instance().getStagingCfg());

            // Apply the new logger configuration to log4cplus.
            CfgMgr::instance().getStagingCfg()->applyLoggingCfg();

            isc::data::ElementPtr config_database(config_data->database_credentials());
            if (config_database) {
                // Update the config manager with the server configuration type
                CfgMgr::instance().getStagingCfg()->getConfigurationType() =
                    SrvConfigType::CONFIG_DATABASE;
                CfgMgr::instance().getStagingCfg()->getCfgDbAccess()->setShardCfgDbAccessString(
                    isc::db::DbAccessParser::parse(config_database));
            }

            interfaces_config = dhcp_config->get("interfaces-config");
        } else if (CfgMgr::instance().getStagingCfg()->getConfigurationType() ==
                   SrvConfigType::CONFIG_DATABASE) {
            interfaces_config = config->get("interfaces-config");
        }

        // Read database configuration and apply it to the current configuration.
        if (CfgMgr::instance().getStagingCfg()->getConfigurationType() ==
                SrvConfigType::MASTER_DATABASE ||
            CfgMgr::instance().getStagingCfg()->getConfigurationType() ==
                SrvConfigType::CONFIG_DATABASE) {
            if (!interfaces_config) {
                isc_throw(BadValue,
                          "no mandatory 'interfaces-config' entry in the local configuration");
            }

            isc::data::ElementPtr const& shard_config(readDhcpConfigFromShardDatabase());

            // Keep the 'interfaces-config' settings from the config file received at startup
            shard_config->set("interfaces-config", interfaces_config);
            return shard_config;
        }

        return nullptr;
    }

private:
    /// @brief static-only
    ConfigurationReader() = delete;

    /// @brief non-copyable
    /// @{
    ConfigurationReader(ConfigurationReader const&) = delete;
    ConfigurationReader& operator=(ConfigurationReader const&) = delete;
    /// @}
};

}  // namespace dhcp
}  // namespace isc

#endif  // CONFIGURATION_READER_H

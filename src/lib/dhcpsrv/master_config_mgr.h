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

#ifndef MASTER_CONFIG_MGR_H
#define MASTER_CONFIG_MGR_H

#include <database/common.h>
#include <database/db_exceptions.h>
#include <database/timestamp_store.h>
#include <dhcpsrv/cfgmgr.h>
#include <dhcpsrv/srv_config.h>
#include <util/dhcp.h>
#include <util/func.h>
#include <util/types.h>

#include <stdint.h>

#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace isc {
namespace dhcp {

struct MasterConstants {
    using array_of_regexes_t = std::array<std::regex, 2>;

    static std::string credentials_xpath(std::string const& master) {
        return "/masters[name='" + master + "']/admin-credentials";
    }

    static std::regex const& key_regex() {
        static std::regex const _("/masters\\[name='(.*?)'\\]/name|"
                                  "/masters\\[name='(.*?)'\\]/shards\\[name='(.*?)'\\]/name|"
                                  "/masters\\[name='(.*?)'\\]/shards\\[name='(.*?)'\\]/"
                                  "master-config\\[instance-id='(.*?)'\\]/instance-id");
        return _;
    }

    static std::string xpath(std::string const& master,
                             std::string const& shard,
                             std::string const& element,
                             std::string const& instance_ID = std::string()) {
        if (element == "master-config") {
            return "/masters[name='" + master + "']/shards[name='" + shard +
                   "']/master-config[instance-id='" + instance_ID + "']";
        } else {
            return "/masters[name='" + master + "']/shards[name='" + shard + "']/" + element;
        }
    }

    static array_of_regexes_t const& regexes() {
        static array_of_regexes_t const _{
            std::regex("/masters\\[name='(.*?)'\\]/shards\\[name='(.*?)'\\]/"
                       "(master-config)\\[instance-id='(.*?)'\\](.*)$"),
            std::regex("/masters\\[name='(.*?)'\\]/shards\\[name='(.*?)'\\]/"
                       "(admin-credentials|config-database)()(.*)$"),
        };
        return _;
    };

    static auto fromMatch(std::smatch const& match) {
        return std::make_tuple(std::string(match[1]), std::string(match[2]), std::string(match[3]),
                               std::string(match[4]), std::string(match[5]));
    }

private:
    /// @brief static-only & non-copyable
    /// @{
    MasterConstants() = delete;
    MasterConstants(MasterConstants const&) = delete;
    MasterConstants& operator=(MasterConstants const&) = delete;
    /// @}
};

/// @brief Holds server configuration data from the master database.
///
/// Holds server configuration data from the master database.
///
struct MasterConfig {
    std::string instance_ID_;
    std::string shard_;
    isc::db::timestamp_t timestamp_;

    isc::data::ElementPtr server_specific_configuration() {
        return server_specific_configuration_;
    }

    std::string server_specific_configuration_string() {
        return server_specific_configuration_string_;
    }

    std::string* server_specific_configuration_string_pointer() {
        return &server_specific_configuration_string_;
    }

    void server_specific_configuration(isc::data::ElementPtr configuration) {
        if (!configuration) {
            server_specific_configuration_ = nullptr;
            server_specific_configuration_string_ = std::string();
            return;
        }
        server_specific_configuration_ = configuration;
        server_specific_configuration_string_ =
            isc::data::prettyPrint(server_specific_configuration_);
    }

    void server_specific_configuration(std::string const& configuration) {
        if (configuration.empty()) {
            server_specific_configuration_ = nullptr;
            server_specific_configuration_string_ = std::string();
            return;
        }
        server_specific_configuration_ = isc::data::Element::fromJSON(configuration);
        server_specific_configuration_string_ =
            isc::data::prettyPrint(server_specific_configuration_);
    }

    void fromDistributedConfiguration(isc::data::ElementPtr distributed_configuration,
                                      std::string const& master,
                                      std::string const& shard,
                                      std::string const& netconf_flag,
                                      std::string const& instance_ID) {
        instance_ID_ = instance_ID;
        std::string xpath_prefix("/masters[name='" + master + "']/shards[name='" + shard + "']");
        if (netconf_flag == "netconf-credentials") {
            database_credentials(
                distributed_configuration->xpath(xpath_prefix + "/admin-credentials"));
            server_specific_configuration(std::string());
        } else if (netconf_flag.empty()) {
            database_credentials(
                distributed_configuration->xpath(xpath_prefix + "/config-database"));
            server_specific_configuration(distributed_configuration->xpath(
                xpath_prefix + "/master-config[instance-id='" + instance_ID + "']/server-config"));
        } else {
            isc_throw(Unexpected,
                      PRETTY_METHOD_NAME() + ": unmatched netconf flag '" + netconf_flag + "'");
        }
        shard_ = shard;
    }

    isc::data::ElementPtr database_credentials() {
        return database_credentials_;
    }

    std::string database_credentials_string() {
        return database_credentials_string_;
    }

    std::string* database_credentials_string_pointer() {
        return &database_credentials_string_;
    }

    void database_credentials(isc::data::ElementPtr configuration) {
        if (!configuration) {
            database_credentials_ = nullptr;
            database_credentials_string_ = std::string();
            return;
        }
        database_credentials_ = configuration;
        database_credentials_string_ = isc::data::prettyPrint(configuration);
    }

    void database_credentials(std::string const& configuration) {
        if (configuration.empty()) {
            database_credentials_ = nullptr;
            database_credentials_string_ = std::string();
            return;
        }
        database_credentials_ = isc::data::Element::fromJSON(configuration);
        database_credentials_string_ = isc::data::prettyPrint(database_credentials_);
    }

    void sanitize() {
        server_specific_configuration(server_specific_configuration_string_);
        database_credentials(database_credentials_string_);
    }

private:
    isc::data::ElementPtr database_credentials_;
    std::string database_credentials_string_;
    isc::data::ElementPtr server_specific_configuration_;
    std::string server_specific_configuration_string_;
};

template <isc::dhcp::DhcpSpaceType D>
struct MasterConfigT : MasterConfig {};

using MasterConfig4 = MasterConfigT<DHCP_SPACE_V4>;
using MasterConfig6 = MasterConfigT<DHCP_SPACE_V6>;

template <DhcpSpaceType D>
using MasterConfigTPtr = std::shared_ptr<MasterConfigT<D>>;

template <DhcpSpaceType D>
using MasterConfigTCollection = std::vector<MasterConfigTPtr<D>>;

using MasterConfigPtr = std::shared_ptr<MasterConfig>;
using MasterConfigCollection = std::vector<MasterConfigPtr>;

/// @brief abstract master configuration manager
///
/// This is an abstract API for server configuration database backends.
/// It provides unified interface to all backends. As this is an abstract class,
/// it should not be used directly, but rather specialized derived class should
/// be used instead.
///
/// This class throws no exceptions. However, methods in concrete
/// implementations of this class may throw exceptions: see the documentation
/// of those classes for details.
struct MasterConfigMgr {
    MasterConfigMgr() = default;
    virtual ~MasterConfigMgr() = default;

    void action(isc::data::effect_t const& effect,
                MasterConfigPtr& master_config,
                std::string const& config_timestamp_path) {

        isc::data::effect_t keyless_effect(effect);
        keyless_effect.set(isc::data::KEY_IS_AFFECTED, false);

        switch (keyless_effect.to_ulong()) {
        case (1 << isc::data::DELETED):
            upsert(master_config, config_timestamp_path);
            break;
        case (1 << isc::data::INSERTED):
            upsert(master_config, config_timestamp_path);
            break;
        case (1 << isc::data::MODIFIED):
            // It means it was INSERTED over sysrepo, MODIFIED over database, so update().
            upsert(master_config, config_timestamp_path);
            break;
        case 0:
            break;
        default:
            isc_throw(Unexpected, PRETTY_METHOD_NAME() << ": stumbled upon unhandled effect "
                                                       << isc::data::Effect::print(effect));
        }
    }

    /// @brief Deletes a single shard configuration.
    ///
    /// @param instance_ID identifier of server whose configuration is being
    ///     deleted
    virtual void del(std::string& instance_ID, std::string const& config_timestamp_path) = 0;

    void del(MasterConfigPtr& master_config, std::string const& config_timestamp_path) {
        // Delegate.
        del(master_config->instance_ID_, config_timestamp_path);
    }

    /// @brief Clears all DHCP shard configurations.
    virtual void deleteAll(std::string const& config_timestamp_path) = 0;

    /// @brief Inserts DHCP server configuration into the master database.
    ///
    /// @param master_config the configuration being added
    /// @param timestamp read from file, operation is valid only if it is equal to database-side
    ///     timestamp
    virtual void insert(MasterConfigPtr& master_config,
                        std::string const& config_timestamp_path) = 0;

    /// @brief Returns DHCP instance configuration information from the database.
    ///
    /// One database contains server configuration data only for one Kea server.
    ///
    /// @param instance_ID the instance identifier of the server being retrieved
    ///
    /// @return smart pointer to the configuration (or NULL if a configuration is not found)
    virtual MasterConfigPtr select(std::string& instance_ID,
                                   std::string const& config_timestamp_path = std::string()) = 0;

    MasterConfigPtr select(MasterConfigPtr& master_config,
                           std::string const& config_timestamp_path = std::string()) {
        // Delegate.
        return select(master_config->instance_ID_, config_timestamp_path);
    }

    /// @brief Retrieves configurations of DHCP servers belonging to a given
    ///     shard.
    ///
    /// @param shard name of the shard whose configuration is being retrieved
    /// @param[out] server_configurations a list of server configurations being retrieved
    virtual MasterConfigCollection
    selectByShard(std::string const& shard,
                  std::string const& config_timestamp_path = std::string()) = 0;

    MasterConfigPtr selectByShard(MasterConfigPtr& master_config,
                                  std::string const& config_timestamp_path = std::string()) {
        // Delegate.
        return select(master_config->shard_, config_timestamp_path);
    }

    virtual std::list<std::string>
    selectInstanceIDs(std::string const& config_timestamp_path = std::string()) = 0;

    /// @brief Retrieves all shard names containing DHCP servers from the master database.
    virtual void selectShardNames(isc::util::shard_list_t& shards_list,
                                  std::string const& config_timestamp_path = std::string()) = 0;

    /// @brief Retrieves timestamp for a shard server configuration.
    ///
    /// There is one timestamp for each shard in the master database. They are
    /// updated on every write (including updates) to the master database.
    ///
    /// @param instance_ID the instance identifier of the server for which the timestamp is being
    ///     retrieved
    ///
    /// @return pointer to the generic structure containing the timestamp
    virtual MasterConfigPtr
    selectTimestamp(std::string& instance_ID,
                    std::string const& config_timestamp_path = std::string()) = 0;

    MasterConfigPtr selectTimestamp(MasterConfigPtr& master_config,
                                    std::string const& config_timestamp_path = std::string()) {
        // Delegate.
        return selectTimestamp(master_config->instance_ID_, config_timestamp_path);
    }

    virtual void update(MasterConfigPtr& master_config,
                        std::string const& config_timestamp_path) = 0;

    void upsert(MasterConfigPtr& master_config, std::string const& config_timestamp_path) {
        try {
            update(master_config, config_timestamp_path);
        } catch (isc::db::StatementNotApplied const& exception) {
            insert(master_config, config_timestamp_path);
        }
    }

    /// @brief Return backend type
    ///
    /// Returns the type of the backend (e.g. "mysql", "memfile" etc.)
    ///
    /// @return Type of the backend.
    virtual std::string getType() const = 0;

    /// @brief Start Transaction
    ///
    /// Start transaction for database operations. On databases that don't
    /// support transactions, this is a no-op.
    virtual void startTransaction() = 0;

    /// @brief Commit Transactions
    ///
    /// Commits all pending database operations.  On databases that don't
    /// support transactions, this is a no-op.
    virtual void commit() = 0;

    /// @brief Rollback Transactions
    ///
    /// Rolls back all pending database operations.  On databases that don't
    /// support transactions, this is a no-op.
    virtual void rollback() = 0;

private:
    /// @brief non-copyable
    /// @{
    MasterConfigMgr(MasterConfigMgr const&) = delete;
    MasterConfigMgr& operator=(MasterConfigMgr const&) = delete;
    /// @}
};

}  // namespace dhcp
}  // namespace isc

#endif  // MASTER_CONFIG_MGR_H

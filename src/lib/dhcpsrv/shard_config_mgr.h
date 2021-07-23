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

#ifndef SHARD_CONFIG_MGR_H
#define SHARD_CONFIG_MGR_H

#include <database/common.h>
#include <database/db_exceptions.h>
#include <database/timestamp_store.h>
#include <dhcpsrv/cfgmgr.h>
#include <dhcpsrv/srv_config.h>
#include <util/func.h>

#include <stdint.h>

#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace isc {
namespace dhcp {

struct ShardConstants {
    static std::string credentials_xpath(std::string const& master, std::string const& shard) {
        return "/masters[name='" + master + "']/shards[name='" + shard + "']/admin-credentials";
    }

    static std::regex const& key_regex() {
        static std::regex const _("/masters\\[name='(.*?)'\\]/shards\\[name='(.*?)'\\]/name");
        return _;
    }

    static std::string xpath(std::string const& master, std::string const& shard) {
        return "/masters[name='" + master + "']/shards[name='" + shard + "']/shard-config";
    }

    static std::regex const& regex() {
        static std::regex const _(
            "/masters\\[name='(.*?)'\\]/shards\\[name='(.*?)'\\]/shard-config(.*)$");
        return _;
    }

    static auto fromMatch(std::smatch const& match) {
        return std::make_tuple(std::string(match[1]), std::string(match[2]), std::string(match[3]));
    }

private:
    /// @brief static-only & non-copyable
    /// @{
    ShardConstants() = delete;
    ShardConstants(ShardConstants const&) = delete;
    ShardConstants& operator=(ShardConstants const&) = delete;
    /// @}
};

/// @brief Holds server configuration data
///
/// Holds server configuration data in order to read/write the configuration
/// from/to the database.
struct ShardConfig {
    isc::db::timestamp_t timestamp_;

    isc::data::ElementPtr configuration() {
        return configuration_;
    }

    std::string configuration_string() {
        return configuration_string_;
    }

    std::string* configuration_string_pointer() {
        return &configuration_string_;
    }

    void configuration(isc::data::ElementPtr configuration) {
        if (!configuration) {
            configuration_ = nullptr;
            configuration_string_ = std::string();
            return;
        }
        configuration_ = configuration;
        configuration_string_ = isc::data::prettyPrint(configuration_);
    }

    void configuration(std::string const& configuration) {
        if (configuration.empty()) {
            configuration_ = nullptr;
            configuration_string_ = std::string();
            return;
        }
        configuration_ = isc::data::Element::fromJSON(configuration);
        configuration_string_ = isc::data::prettyPrint(configuration_);
    }

    void fromDistributedConfiguration(isc::data::ElementPtr distributed_configuration,
                                      std::string const& master,
                                      std::string const& shard) {
        configuration(distributed_configuration->xpath(ShardConstants::xpath(master, shard)));
    }

    void sanitize() {
        configuration(configuration_string_);
    }

private:
    isc::data::ElementPtr configuration_;
    std::string configuration_string_;
};

template <isc::dhcp::DhcpSpaceType D>
struct ShardConfigT : ShardConfig {
    static isc::dhcp::DhcpSpaceType constexpr DHCP_SPACE = D;
};

using ShardConfig4 = ShardConfigT<DHCP_SPACE_V4>;
using ShardConfig6 = ShardConfigT<DHCP_SPACE_V6>;

/// @brief Shared pointer to server configuration string
using ShardConfigPtr = std::shared_ptr<ShardConfig>;
using ShardConfigCollection = std::vector<ShardConfigPtr>;

/// @brief abstract shard configuration manager
///
/// This is an abstract API for server configuration database backends.
/// It provides unified interface to all backends. As this is an abstract class,
/// it should not be used directly, but rather specialized derived class should
/// be used instead.
///
/// This class throws no exceptions.  However, methods in concrete
/// implementations of this class may throw exceptions: see the documentation
/// of those classes for details.
struct ShardConfigMgr {
    ShardConfigMgr() = default;
    virtual ~ShardConfigMgr() = default;

    void action(isc::data::effect_t const& effect,
                ShardConfigPtr& shard_config,
                std::string const& config_timestamp_path) {

        isc::data::effect_t keyless_effect(effect);
        keyless_effect.set(isc::data::KEY_IS_AFFECTED, false);

        switch (keyless_effect.to_ulong()) {
        case (1 << isc::data::DELETED):
            del(config_timestamp_path);
            break;
        case (1 << isc::data::INSERTED):
            upsert(shard_config, config_timestamp_path);
            break;
        case (1 << isc::data::MODIFIED):
            // It means it was INSERTED over sysrepo, MODIFIED over database, so update().
            upsert(shard_config, config_timestamp_path);
            break;
        case 0:
            break;
        default:
            isc_throw(Unexpected, PRETTY_METHOD_NAME() << ": stumbled upon unhandled effect "
                                                       << isc::data::Effect::print(effect));
        }
    }

    virtual void del(std::string const& config_timestamp_path) = 0;

    virtual void insert(ShardConfigPtr const& shard_config,
                        std::string const& config_timestamp_path) = 0;

    /// @brief Returns server configuration information
    ///
    /// This method returns the server configuration.
    /// One database contains server configuration data only for one kea server.
    ///
    /// @return smart pointer to the configuration (or NULL if a configuration
    /// is not found)
    virtual ShardConfigPtr select(std::string const& config_timestamp_path = std::string()) = 0;

    virtual ShardConfigPtr
    selectTimestamp(std::string const& config_timestamp_path = std::string()) = 0;

    virtual void update(ShardConfigPtr const& shard_config,
                        std::string const& config_timestamp_path) = 0;

    virtual void updateTimestamp(std::string const& config_timestamp_path) = 0;

    /// @brief Adds if not exists or updates and existing DHCP server
    /// configuration
    ///
    /// One database contains server configuration data only for one kea server.
    ///
    /// @param json_data The server configuration to be written in json format
    /// @param config_timestamp_path path to the config.timestamp file
    ///
    /// @result true if the configuration was added/updated, false if not.
    virtual void upsert(ShardConfigPtr const& shard_config,
                        std::string const& config_timestamp_path) = 0;

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
    ShardConfigMgr(ShardConfigMgr const&) = delete;
    ShardConfigMgr& operator=(ShardConfigMgr const&) = delete;
    /// @}
};

}  // namespace dhcp
}  // namespace isc

#endif  // SHARD_CONFIG_MGR_H

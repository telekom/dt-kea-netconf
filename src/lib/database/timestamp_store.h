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

#ifndef TIMESTAMP_STORE_H
#define TIMESTAMP_STORE_H

#include <util/filesystem.h>

#include <memory>
#include <regex>
#include <string>

namespace isc {
namespace db {

using timestamp_t = int64_t;

/// @brief Used to manage the timestamps in a single config.timestamp file
///     Tables that are served by this store:
///         - server_configuration[4|6] (for shard keyspaces) and derivatives:
///             - policy_engine_configuration
///             - sign_up_portal_configuration
///         - subnet[4|6]
///
/// Used by Cassandra, but could be used by other backends as well
///
/// The format of timestamps is on each line in the format:
///     - <table> <entry_id> <timestamp>
struct TimestampStore {
    TimestampStore(std::string const& file) {
        if (!file.empty()) {
            config_timestamp_.reset(new isc::util::File(file));
        }
    }

    void del(std::string const& table, std::string const& key);
    timestamp_t read(std::string const& table, std::string const& key);
    void write(std::string const& table, std::string const& key, timestamp_t const& timestamp);

    static timestamp_t generate();
    static timestamp_t now();

private:
    std::unique_ptr<isc::util::File> config_timestamp_;

    /// @brief non-copyable
    /// @{
    TimestampStore(TimestampStore const&) = delete;
    TimestampStore& operator=(TimestampStore const&) = delete;
    /// @}
};

}  // namespace db
}  // namespace isc

#endif  // TIMESTAMP_STORE_H

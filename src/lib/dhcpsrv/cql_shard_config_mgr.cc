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

#include <config.h>

#include <dhcpsrv/cql_shard_config_mgr.h>

using isc::db::VersionTuple;
using isc::dhcp::DHCP_SPACE_V4;
using isc::dhcp::DHCP_SPACE_V6;
using std::string;

namespace {

/// @brief Define CQL master configuration schema version: 1.0
VersionTuple constexpr CQL_SHARD_CONFIG_SCHEMA_VERSION = std::make_tuple(3, 0);

}  // namespace

namespace isc {
namespace db {

template <>
VersionTuple const& CqlVersionExchange<isc::dhcp::ShardConfigVersionTuple>::codeVersion() {
    return CQL_SHARD_CONFIG_SCHEMA_VERSION;
}

template <>
string const& CqlVersionExchange<isc::dhcp::ShardConfigVersionTuple>::table() {
    static std::string const _("config_schema_version");
    return _;
}

}  // namespace db
}  // namespace isc

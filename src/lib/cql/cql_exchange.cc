// Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
// Copyright (C) 2020 Deutsche Telekom AG.
//
// Authors: Razvan Becheriu <razvan.becheriu@qualitance.com>
//          Andrei Pavel <andrei.pavel@qualitance.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//                 https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <config.h>

#include <cql/cql_exchange.h>

namespace isc {
namespace db {

#ifdef ENABLE_DEBUG
isc::log::Logger cql_logger("cql");
#endif  // ENABLE_DEBUG

template <>
isc::db::VersionTuple const& CqlVersionExchange<OperationalVersionTuple>::codeVersion() {
    return CQL_SCHEMA_VERSION;
}

template <>
std::string const& CqlVersionExchange<OperationalVersionTuple>::table() {
    static std::string const _("schema_version");
    return _;
}

}  // namespace db
}  // namespace isc

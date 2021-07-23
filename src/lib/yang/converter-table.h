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

#ifndef YANG_CONVERTER_TABLE_H
#define YANG_CONVERTER_TABLE_H

#include <string>
#include <tuple>
#include <vector>

namespace isc {
namespace yang {

/// @brief Identifiers for all transformations enumerated
enum transform_t {
    copy,
    DUID,
    network_ranges,
    network_range_option_sets,
};

/// @brief One entry in the table containing IETF xpath, Kea xpath, transformation identifier
using tuple_t = std::tuple<std::string, std::string, std::vector<transform_t>>;

/// @brief Single application-level table which maps xpaths between the two modules. Mapping is done
///     at leaf level as much as possible, but when convenient it maps higher level nodes. Is
///     separate struct in order to have a single table for both specialized converters.
///
/// * no entry: will not get converted
/// * empty set of transformations: will not get converted
/// * one or more transformations: get applied in order
/// * `copy` transformation copies as-is
struct Table {
    /// @brief The entire table, is optimally a linear data structure since it is parsed and
    ///     iterated through by @ref Converter.
    using table_t = std::vector<tuple_t>;

    /// @brief Singleton table
    static table_t const& table() {
        // clang-format off
        static table_t const _{
            //        IETF xpath                                                                                |        Kea xpath                              |        Transformations        //
            {"/dhcpv6-server/enabled",                                                                          "/Dhcp6/enabled",                               {copy}},
            {"/dhcpv6-server/server-duid",                                                                      "/Dhcp6/server-id",                             {DUID, copy}},
            {"/dhcpv6-server/option-sets/option-set[option-set-id='0']/option-set-id",                          "/Dhcp6/option-set-id",                         {copy}},
            {"/dhcpv6-server/option-sets/option-set[option-set-id='0']/description",                            "/Dhcp6/option-set-description",                {copy}},
            {"/dhcpv6-server/network-ranges",                                                                   "/Dhcp6/subnet6",                               {network_ranges, copy}},
            {"/dhcpv6-server",                                                                                  "/Dhcp6/subnet6",                               {network_range_option_sets}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:client-classes",                  "/Dhcp6/client-classes",                        {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:config-control",                  "/Dhcp6/config-control",                        {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:dhcp-queue-control",              "/Dhcp6/dhcp-queue-control",                    {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:dhcp4o6-port",                    "/Dhcp6/dhcp4o6-port",                          {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:dhcp-ddns",                       "/Dhcp6/dhcp-ddns",                             {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:expired-leases-processing",       "/Dhcp6/expired-leases-processing",             {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:hooks-libraries",                 "/Dhcp6/hooks-libraries",                       {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:hosts-database",                  "/Dhcp6/hosts-database",                        {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:host-char-set",                   "/Dhcp6/host-char-set",                         {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:host-char-replacement",           "/Dhcp6/host-char-replacement",                 {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:host-reservation-identifiers",    "/Dhcp6/host-reservation-identifiers",          {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:interfaces-config",               "/Dhcp6/interfaces-config",                     {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:lease-database",                  "/Dhcp6/lease-database",                        {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:loggers",                         "/Dhcp6/loggers",                               {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:mac-sources",                     "/Dhcp6/mac-sources",                           {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:relay-supplied-options",          "/Dhcp6/relay-supplied-options",                {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:reservation-mode",                "/Dhcp6/reservation-mode",                      {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:sanity-checks",                   "/Dhcp6/sanity-checks",                         {copy}},
            {"/dhcpv6-server/vendor-config/kea-augments-to-ietf-dhcpv6-server:server-tag",                      "/Dhcp6/server-tag",                            {copy}}
        };
        // clang-format on
        return _;
    }
};

}  // namespace yang
}  // namespace isc

#endif  // YANG_CONVERTER_TABLE_H

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

#ifndef UTIL_DHCP_H
#define UTIL_DHCP_H

#include <stdint.h>
#include <sys/socket.h>

#include <exceptions/exceptions.h>
#include <util/func.h>
#include <util/magic_enum.hpp>

#include <string>

namespace isc {
namespace dhcp {

enum DhcpSpaceType { DHCP_SPACE_UNKNOWN = 0, DHCP_SPACE_V4 = 4, DHCP_SPACE_V6 = 6 };

template <DhcpSpaceType D>
inline uint16_t constexpr dhcpSpaceToInetFamily();

template <>
inline uint16_t constexpr dhcpSpaceToInetFamily<DHCP_SPACE_V4>() {
    return AF_INET;
}

template <>
inline uint16_t constexpr dhcpSpaceToInetFamily<DHCP_SPACE_V6>() {
    return AF_INET6;
}

template <uint16_t const inet_family>
inline DhcpSpaceType constexpr inetFamilyToDhcpSpace();

template <>
inline DhcpSpaceType constexpr inetFamilyToDhcpSpace<AF_INET>() {
    return DHCP_SPACE_V4;
}

template <>
inline DhcpSpaceType constexpr inetFamilyToDhcpSpace<AF_INET6>() {
    return DHCP_SPACE_V6;
}

inline DhcpSpaceType constexpr inetFamilyToDhcpSpace(uint16_t const inet_family) {
    switch (inet_family) {
    case AF_INET:
        return DHCP_SPACE_V4;
    case AF_INET6:
        return DHCP_SPACE_V6;
    default:
        return DHCP_SPACE_UNKNOWN;
    }
}

template <DhcpSpaceType D>
inline std::string const& dhcpSpaceToString();

template <>
inline std::string const& dhcpSpaceToString<DHCP_SPACE_V4>() {
    static std::string const _("4");
    return _;
}

template <>
inline std::string const& dhcpSpaceToString<DHCP_SPACE_V6>() {
    static std::string const _("6");
    return _;
}

inline std::string const& dhcpSpaceToString(DhcpSpaceType const& D) {
    switch (D) {
    case DHCP_SPACE_V4:
        return dhcpSpaceToString<DHCP_SPACE_V4>();
    case DHCP_SPACE_V6:
        return dhcpSpaceToString<DHCP_SPACE_V6>();
    default:
        isc_throw(BadValue, PRETTY_FUNCTION_NAME() << ": " << magic_enum::enum_name(D));
    }
}

}  // namespace dhcp
}  // namespace isc

#endif  // DHCP_H

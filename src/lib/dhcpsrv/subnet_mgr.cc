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

#include <dhcpsrv/subnet_mgr.h>

using isc::data::ElementPtr;

namespace isc {
namespace dhcp {

template <>
SubnetInfoRaw<DHCP_SPACE_V4, SubnetInfoStrings<DHCP_SPACE_V4>>::SubnetInfoRaw(Subnet4Ptr s) {
    ElementPtr data = s->toElement();
    id_ = s->getID();
    // -> timestamp is ignored as it's not functional part of a subnet.
    renew_timer_ = s->getT1();
    rebind_timer_ = s->getT2();
    valid_lifetime_ = s->getValid();
    allocation_type_ = s->getAllocationType();
    rapid_commit_ = 0;
    reservation_mode_ = s->getHostReservationMode();
    match_client_id_ = s->getMatchClientId();
    subnet_v4_psid_offset_ = s->get4o6().getPsidOffset();
    subnet_v4_psid_len_ = s->get4o6().getPsidLen();
    preferred_lifetime_ = 0;
    strings_.client_class_ = s->getClientClass();
    strings_.interface_ = s->getIface();
    if (data->get("option-data")) {
        strings_.option_data_list_ = data->get("option-data")->str();
    }
    if (data->get("pools")) {
        strings_.pools_list_ = data->get("pools")->str();
    }
    if (data->get("relay")) {
        strings_.relay_ = data->get("relay")->str();
    }
    if (data->get("require-client-classes")) {
        strings_.require_client_classes_ = data->get("require-client-classes")->str();
    }
    if (data->get("reservations")) {
        strings_.reservations_ = data->get("reservations")->str();
    }
    strings_.subnet_ = s->toText();
    if (s->getContext()) {
        strings_.user_context_ = s->getContext()->str();
    }
    strings_.boot_file_name_ = s->getFilename();
    strings_.next_server_ = s->getSiaddr().get().toText();
    strings_.server_hostname_ = s->getSname();
    strings_.subnet_4o6_interface_ = s->get4o6().getIface4o6();
    if (s->get4o6().getInterfaceId()) {
        strings_.subnet_4o6_interface_id_ = s->get4o6().getInterfaceId()->toString();
    }
    if (data->get("4o6-subnet")) {
        strings_.subnet_4o6_subnet_ = data->get("4o6-subnet")->stringValue();
    }
    if (data->get("v4-excluded-psids")) {
        strings_.subnet_v4_excluded_psids_ = data->get("v4-excluded-psids")->str();
    }
}

template <>
SubnetInfoRaw<DHCP_SPACE_V6, SubnetInfoStrings<DHCP_SPACE_V6>>::SubnetInfoRaw(Subnet6Ptr s) {
    ElementPtr data = s->toElement();
    Subnet6InfoPtr subnetInfo = std::make_shared<Subnet6Info>();
    id_ = s->getID();
    // -> timestamp is ignored as it's not functional part of a subnet.
    renew_timer_ = s->getT1();
    rebind_timer_ = s->getT2();
    valid_lifetime_ = s->getValid();
    allocation_type_ = s->getAllocationType();
    rapid_commit_ = s->getRapidCommit();
    reservation_mode_ = s->getHostReservationMode();
    match_client_id_ = 0;
    subnet_v4_psid_offset_ = 0;
    subnet_v4_psid_len_ = 0;
    preferred_lifetime_ = s->getPreferred();
    strings_.client_class_ = s->getClientClass();
    strings_.interface_ = s->getIface();
    if (data->get("interface-id")) {
        strings_.interface_id_ = data->get("interface-id")->str();
    }
    if (data->get("option-data")) {
        strings_.option_data_list_ = data->get("option-data")->str();
    }
    if (data->get("pools")) {
        strings_.pools_list_ = data->get("pools")->str();
    }
    if (data->get("relay")) {
        strings_.relay_ = data->get("relay")->str();
    }
    if (data->get("require-client-classes")) {
        strings_.require_client_classes_ = data->get("require-client-classes")->str();
    }
    if (data->get("reservations")) {
        strings_.reservations_ = data->get("reservations")->str();
    }
    strings_.subnet_ = s->toText();
    if (s->getContext()) {
        strings_.user_context_ = s->getContext()->str();
    }
    if (data->get("pd-pools")) {
        strings_.pd_pools_list_ = data->get("pd-pools")->str();
    }
}

template <>
std::vector<std::string>& Subnet4Info::columns() {
    static std::vector<std::string> _{
        // common
        "id", "timestamp", "renew_timer", "rebind_timer", "valid_lifetime", "allocation_type",
        "client_class", "interface", "interface_id", "option_data_list", "pools_list",
        "rapid_commit", "relay", "require_client_classes", "reservations", "reservation_mode",
        "subnet", "user_context",
        // v4
        "boot_file_name", "match_client_id", "next_server", "server_hostname",
        "subnet_4o6_interface", "subnet_4o6_interface_id", "subnet_4o6_subnet",
        "subnet_v4_psid_offset", "subnet_v4_psid_len", "subnet_v4_excluded_psids"  //
    };
    return _;
}

template <>
std::vector<std::string>& Subnet6Info::columns() {
    static std::vector<std::string> _{
        // common
        "id", "timestamp", "renew_timer", "rebind_timer", "valid_lifetime", "allocation_type",
        "client_class", "interface", "interface_id", "option_data_list", "pools_list",
        "rapid_commit", "relay", "require_client_classes", "reservations", "reservation_mode",
        "subnet", "user_context",
        // v6
        "pd_pools_list", "preferred_lifetime"  //
    };
    return _;
}

template <>
std::vector<std::tuple<std::string, std::string>>& Subnet4Info::networkTopologyColumnsAndValues() {
    static std::vector<std::tuple<std::string, std::string>> _;
    return _;
}

template <>
std::vector<std::tuple<std::string, std::string>>& Subnet6Info::networkTopologyColumnsAndValues() {
    static std::vector<std::tuple<std::string, std::string>> _{{"network_topology", "'[  ]'"},
                                                               {"network_topology_timestamp", "0"}};
    return _;
}

template <>
void Subnet4InfoCStrings::allocateDelta() {
    boot_file_name_ = new char[C_STRING_SIZE];
    next_server_ = new char[C_STRING_SIZE];
    server_hostname_ = new char[C_STRING_SIZE];
    subnet_4o6_interface_ = new char[C_STRING_SIZE];
    subnet_4o6_interface_id_ = new char[C_STRING_SIZE];
    subnet_4o6_subnet_ = new char[C_STRING_SIZE];
    subnet_v4_excluded_psids_ = new char[C_STRING_SIZE];
}

template <>
void Subnet6InfoCStrings::allocateDelta() {
    pd_pools_list_ = new char[C_STRING_SIZE];
}

template <>
void Subnet4InfoCStrings::deallocateDelta() {
    delete[] boot_file_name_;
    delete[] next_server_;
    delete[] server_hostname_;
    delete[] subnet_4o6_interface_;
    delete[] subnet_4o6_interface_id_;
    delete[] subnet_4o6_subnet_;
    delete[] subnet_v4_excluded_psids_;
}

template <>
void Subnet6InfoCStrings::deallocateDelta() {
    delete[] pd_pools_list_;
}

template <>
bool Subnet4Info::equalDelta(Subnet4Info const& other) const {
    return match_client_id_ == other.match_client_id_ &&
           subnet_v4_psid_offset_ == other.subnet_v4_psid_offset_ &&
           subnet_v4_psid_len_ == other.subnet_v4_psid_len_;
}

template <>
bool Subnet6Info::equalDelta(Subnet6Info const& other) const {
    return preferred_lifetime_ == other.preferred_lifetime_;
}

template <>
bool Subnet4InfoWithCStrings::equalDelta(Subnet4InfoWithCStrings const& other) const {
    return match_client_id_ == other.match_client_id_ &&
           subnet_v4_psid_offset_ == other.subnet_v4_psid_offset_ &&
           subnet_v4_psid_len_ == other.subnet_v4_psid_len_;
}

template <>
bool Subnet6InfoWithCStrings::equalDelta(Subnet6InfoWithCStrings const& other) const {
    return preferred_lifetime_ == other.preferred_lifetime_;
}

template <>
void Subnet4InfoStrings::convertDelta(Subnet4InfoCStrings const& other) {
    boot_file_name_.assign(other.boot_file_name_, other.boot_file_name_length_);
    next_server_.assign(other.next_server_, other.next_server_length_);
    server_hostname_.assign(other.server_hostname_, other.server_hostname_length_);
    subnet_4o6_interface_.assign(other.subnet_4o6_interface_, other.subnet_4o6_interface_length_);
    subnet_4o6_interface_id_.assign(other.subnet_4o6_interface_id_,
                                    other.subnet_4o6_interface_id_length_);
    subnet_4o6_subnet_.assign(other.subnet_4o6_subnet_, other.subnet_4o6_subnet_length_);
    subnet_v4_excluded_psids_.assign(other.subnet_v4_excluded_psids_,
                                     other.subnet_v4_excluded_psids_length_);
}

template <>
void Subnet6InfoStrings::convertDelta(Subnet6InfoCStrings const& other) {
    pd_pools_list_.assign(other.pd_pools_list_, other.pd_pools_list_length_);
}

template <>
void Subnet4InfoCStrings::convertDelta(Subnet4InfoStrings const& other) {
    boot_file_name_length_ = other.boot_file_name_.size();
    next_server_length_ = other.next_server_.size();
    server_hostname_length_ = other.server_hostname_.size();
    subnet_4o6_interface_length_ = other.subnet_4o6_interface_.size();
    subnet_4o6_interface_id_length_ = other.subnet_4o6_interface_id_.size();
    subnet_4o6_subnet_length_ = other.subnet_4o6_subnet_.size();
    subnet_v4_excluded_psids_length_ = other.subnet_v4_excluded_psids_.size();

    std::strcpy(boot_file_name_, other.boot_file_name_.c_str());
    std::strcpy(next_server_, other.next_server_.c_str());
    std::strcpy(server_hostname_, other.server_hostname_.c_str());
    std::strcpy(subnet_4o6_interface_, other.subnet_4o6_interface_.c_str());
    std::strcpy(subnet_4o6_interface_id_, other.subnet_4o6_interface_id_.c_str());
    std::strcpy(subnet_4o6_subnet_, other.subnet_4o6_subnet_.c_str());
    std::strcpy(subnet_v4_excluded_psids_, other.subnet_v4_excluded_psids_.c_str());
}

template <>
void Subnet6InfoCStrings::convertDelta(Subnet6InfoStrings const& other) {
    pd_pools_list_length_ = other.pd_pools_list_.size();

    std::strcpy(pd_pools_list_, other.pd_pools_list_.c_str());
}

template <>
void Subnet4Info::convertDelta(Subnet4InfoWithCStrings& other) {
    match_client_id_ = other.match_client_id_;
    subnet_v4_psid_offset_ = other.subnet_v4_psid_offset_;
    subnet_v4_psid_len_ = other.subnet_v4_psid_len_;
}

template <>
void Subnet6Info::convertDelta(Subnet6InfoWithCStrings& other) {
    preferred_lifetime_ = other.preferred_lifetime_;
}

template <>
void Subnet4InfoWithCStrings::convertDelta(Subnet4InfoWithCStrings& other) {
    match_client_id_ = other.match_client_id_;
    subnet_v4_psid_offset_ = other.subnet_v4_psid_offset_;
    subnet_v4_psid_len_ = other.subnet_v4_psid_len_;
}

template <>
void Subnet6InfoWithCStrings::convertDelta(Subnet6InfoWithCStrings& other) {
    preferred_lifetime_ = other.preferred_lifetime_;
}

template <>
bool Subnet4InfoStrings::equalDelta(Subnet4InfoStrings const& other) const {
    return boot_file_name_ == other.boot_file_name_ && next_server_ == other.next_server_ &&
           server_hostname_ == other.server_hostname_ &&
           subnet_4o6_interface_ == other.subnet_4o6_interface_ &&
           subnet_4o6_interface_id_ == other.subnet_4o6_interface_id_ &&
           subnet_4o6_subnet_ == other.subnet_4o6_subnet_ &&
           subnet_v4_excluded_psids_ == other.subnet_v4_excluded_psids_;
}

template <>
bool Subnet6InfoStrings::equalDelta(Subnet6InfoStrings const& other) const {
    return pd_pools_list_ == other.pd_pools_list_;
}

template <>
bool Subnet4InfoCStrings::equalDelta(Subnet4InfoCStrings const& other) const {
    return boot_file_name_length_ == other.boot_file_name_length_ &&
           next_server_length_ == other.next_server_length_ &&
           server_hostname_length_ == other.server_hostname_length_ &&
           subnet_4o6_interface_length_ == other.subnet_4o6_interface_length_ &&
           subnet_4o6_interface_id_length_ == other.subnet_4o6_interface_id_length_ &&
           subnet_4o6_subnet_length_ == other.subnet_4o6_subnet_length_ &&
           subnet_v4_excluded_psids_length_ == other.subnet_v4_excluded_psids_length_ &&
           !(std::strcmp(boot_file_name_, other.boot_file_name_) ||
             std::strcmp(next_server_, other.next_server_) ||
             std::strcmp(server_hostname_, other.server_hostname_) ||
             std::strcmp(subnet_4o6_interface_, other.subnet_4o6_interface_) ||
             std::strcmp(subnet_4o6_interface_id_, other.subnet_4o6_interface_id_) ||
             std::strcmp(subnet_4o6_subnet_, other.subnet_4o6_subnet_) ||
             std::strcmp(subnet_v4_excluded_psids_, other.subnet_v4_excluded_psids_));
}

template <>
bool Subnet6InfoCStrings::equalDelta(Subnet6InfoCStrings const& other) const {
    return pd_pools_list_length_ == other.pd_pools_list_length_ &&
           !(std::strcmp(pd_pools_list_, other.pd_pools_list_));
}

template <>
void Subnet4Info::fromElement(isc::data::ElementPtr& from) {
    fromElementCommon(from);

    isc::data::ElementPtr element;

    element = from->get("match-client-id");
    if (element) {
        match_client_id_ = element->boolValue();
    }

    for (auto& [member, key] : std::vector<std::tuple<uint8_t&, std::string>>{
             {subnet_v4_psid_offset_, "v4-psid-offset"},  //
             {subnet_v4_psid_offset_, "v4-psid-len"}}) {
        element = from->get(key);
        if (element) {
            member = element->intValue();
        }
    }

    for (auto& [member, key] : std::vector<std::tuple<std::string&, std::string>>{
             {strings_.boot_file_name_, "boot-file-name"},
             {strings_.next_server_, "next-server"},
             {strings_.server_hostname_, "server-hostname"},
             {strings_.subnet_4o6_interface_, "4o6-interface"},
             {strings_.subnet_4o6_interface_id_, "4o6-interface-id"},
             {strings_.subnet_4o6_subnet_, "4o6-subnet"},
             {strings_.subnet_v4_excluded_psids_, "v4-excluded-psids"}}) {
        element = from->get(key);
        if (element) {
            member = element->stringValue();
        }
    }
}

template <>
void Subnet6Info::fromElement(isc::data::ElementPtr& from) {
    fromElementCommon(from);

    isc::data::ElementPtr element;

    element = from->get("interface-id");
    if (element) {
        strings_.interface_id_ = element->stringValue();
    }

    element = from->get("pd-pools");
    if (element) {
        strings_.pd_pools_list_ = element->str();
    }

    element = from->get("preferred-lifetime");
    if (element) {
        preferred_lifetime_ = element->intValue();
    }

    element = from->get("rapid-commit");
    if (element) {
        rapid_commit_ = element->boolValue();
    }
}

template <>
isc::data::ElementPtr Subnet4Info::toElement() {
    isc::data::ElementPtr map = isc::data::Element::createMap();
    toElementCommon(map);

    map->set("match-client-id", static_cast<bool>(match_client_id_));
    if (subnet_v4_psid_len_) {
        for (auto& [member, key] : std::vector<std::tuple<uint8_t&, std::string>>{
                 {subnet_v4_psid_offset_, "v4-psid-offset"},
                 {subnet_v4_psid_len_, "v4-psid-len"}}) {
            map->set(key, member);
        }
    }

    for (auto& [member, key] : std::vector<std::tuple<std::string&, std::string>>{
             {strings_.boot_file_name_, "boot-file-name"},
             {strings_.next_server_, "next-server"},
             {strings_.server_hostname_, "server-hostname"},
             {strings_.subnet_4o6_interface_, "4o6-interface"},
             {strings_.subnet_4o6_interface_id_, "4o6-interface-id"},
             {strings_.subnet_4o6_subnet_, "4o6-subnet"},
             {strings_.subnet_v4_excluded_psids_, "v4-excluded-psids"}}) {
        if (!member.empty()) {
            map->set(key, member);
        }
    }

    if (!strings_.subnet_v4_excluded_psids_.empty()) {
        map->set("v4-excluded-psids",
                 isc::data::Element::fromJSON(strings_.subnet_v4_excluded_psids_));
    }

    return map;
}

template <>
isc::data::ElementPtr Subnet6Info::toElement() {
    isc::data::ElementPtr map = isc::data::Element::createMap();
    toElementCommon(map);

    if (!strings_.interface_id_.empty()) {
        map->set("interface-id", strings_.interface_id_);
    }
    if (!strings_.pd_pools_list_.empty()) {
        map->set("pd-pools", isc::data::Element::fromJSON(strings_.pd_pools_list_));
    }
    map->set("preferred-lifetime", preferred_lifetime_);
    map->set("rapid-commit", static_cast<bool>(rapid_commit_));

    return map;
}

}  // namespace dhcp
}  // namespace isc

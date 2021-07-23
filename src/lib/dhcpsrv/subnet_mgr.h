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

#ifndef SUBNET_MGR_H
#define SUBNET_MGR_H

#include <cc/data.h>
#include <database/common.h>
#include <database/db_exceptions.h>
#include <database/timestamp_store.h>
#include <dhcpsrv/cfgmgr.h>
#include <dhcpsrv/configuration.h>
#include <dhcpsrv/lease.h>
#include <dhcpsrv/parsers/dhcp_parsers.h>
#include <dhcpsrv/pool.h>
#include <dhcpsrv/subnet.h>
#include <dhcpsrv/subnet_id.h>
#include <util/dhcp.h>
#include <util/func.h>
#include <util/hash.h>

#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace isc {
namespace dhcp {

template <DhcpSpaceType D, bool has_IETF>
struct SubnetConstantsT {
    static std::regex const& key_regex() {
        static std::regex const _(
            "/masters\\[name='(.*?)'\\]/shards\\[name='(.*?)'\\]/shard-config/" +
            ConfigurationElements<D, has_IETF>::toplevel() + "/" +
            ConfigurationElements<D, has_IETF>::subnet() + "\\[" +
            ConfigurationElements<D, has_IETF>::subnetIndex() + "='(.*?)'\\]/" +
            ConfigurationElements<D, has_IETF>::subnetIndex());
        return _;
    }

    static std::regex const& regex() {
        static std::regex const _(
            "/masters\\[name='(.*?)'\\]/shards\\[name='(.*?)'\\]/shard-config/" +
            ConfigurationElements<D, has_IETF>::toplevel() + "/" +
            ConfigurationElements<D, has_IETF>::subnet() + "\\[" +
            ConfigurationElements<D, has_IETF>::subnetIndex() + "='(.*?)'\\](.*)$");
        return _;
    }

    static std::string
    xpath(std::string const& master, std::string const& shard, std::string const& subnet) {
        return xpath_for_all(master, shard) + "[" +
               ConfigurationElements<D, has_IETF>::subnetIndex() + "='" + subnet + "']";
    }

    static std::string subnetElementXpath(std::string const& master,
                                          std::string const& shard,
                                          std::string const& subnet) {
        return xpath(master, shard, subnet) + "/" +
               isc::dhcp::ConfigurationElements<D, has_IETF>::subnetElement();
    }

    static std::string xpath_for_all(std::string const& master, std::string const& shard) {
        return "/masters[name='" + master + "']/shards[name='" + shard + "']/shard-config/" +
               ConfigurationElements<D, has_IETF>::toplevel() + "/" +
               ConfigurationElements<D, has_IETF>::subnet();
    }

    static auto fromMatch(std::smatch const& match) {
        return std::make_tuple(std::string(match[1]), std::string(match[2]), std::string(match[3]),
                               std::string(match[4]));
    }

private:
    /// @brief static-only & non-copyable
    /// @{
    SubnetConstantsT() = delete;
    SubnetConstantsT(SubnetConstantsT const&) = delete;
    SubnetConstantsT& operator=(SubnetConstantsT const&) = delete;
    /// @}
};

template <DhcpSpaceType D>
using SubnetConstants = SubnetConstantsT<D, ConfigurationConstants::NOT_IETF>;

template <DhcpSpaceType D>
using NetworkRangeConstants = SubnetConstantsT<D, ConfigurationConstants::HAS_IETF>;

/// @brief forward declaration of struct needed in convert methods in @ref SubnetInfoRaw
template <DhcpSpaceType D>
struct SubnetInfoCStrings;

/// @brief structure holding essential subnet information
template <DhcpSpaceType D, typename SubnetInfoStringsType>
struct SubnetInfoRaw {
    explicit SubnetInfoRaw() = default;
    explicit SubnetInfoRaw(SubnetTPtr<D> s);

    /// @brief constructor from one specialization, necessarily marked explicit to avoid copying
    /// through two constructors
    explicit SubnetInfoRaw(SubnetInfoRaw<D, SubnetInfoCStrings<D>>& other) {
        convertCommon(other);
        convertDelta(other);
    }

    SubnetInfoRaw& operator=(SubnetInfoRaw<D, SubnetInfoCStrings<D>>& other) {
        convertCommon(other);
        convertDelta(other);
        return *this;
    }

    bool operator==(SubnetInfoRaw<D, SubnetInfoStringsType> const& other) const {
        return equalCommon(other) && equalDelta(other);
    }

    bool operator!=(SubnetInfoRaw<D, SubnetInfoStringsType> const& other) const {
        return !(*this == other);
    }

    void fromDistributedConfiguration(isc::data::ElementPtr distributed_configuration,
                                      std::string const& master,
                                      std::string const& shard,
                                      std::string const& subnet) {
        isc::data::ElementPtr subnet_element(
            distributed_configuration->xpath(SubnetConstants<D>::xpath(master, shard, subnet)));
        if (!subnet_element) {
            isc_throw(Unexpected, PRETTY_METHOD_NAME()
                                      << ": subnet not found at xpath "
                                      << SubnetConstants<D>::xpath(master, shard, subnet));
        }
        fromElement(subnet_element);
    }

    void fromElement(isc::data::ElementPtr& from);

    void fromElementCommon(isc::data::ElementPtr& from) {
        isc::data::ElementPtr element;

        element = from->get("allocation-type");
        if (element) {
            allocation_type_ = Subnet::stringToAllocationType(element->stringValue());
        }

        element = from->get("id");
        if (element) {
            id_ = element->intValue();
        }

        element = from->get("reservation-mode");
        if (element) {
            reservation_mode_ = Network::stringToHostReservationMode(element->stringValue());
        }

        for (auto& [member, key] :
             std::vector<std::tuple<uint64_t&, std::string>>{{renew_timer_, "renew-timer"},
                                                             {rebind_timer_, "rebind-timer"},
                                                             {valid_lifetime_, "valid-lifetime"}}) {
            isc::data::ElementPtr element(from->get(key));
            if (element) {
                member = element->intValue();
            }
        }

        for (auto& [member, key] : std::vector<std::tuple<std::string&, std::string>>{
                 {strings_.client_class_, "client-class"},  //
                 {strings_.interface_, "interface"},
                 {strings_.subnet_, "subnet"}}) {
            element = from->get(key);
            if (element) {
                member = element->stringValue();
            }
        }

        for (auto& [member, key] : std::vector<std::tuple<std::string&, std::string>>{
                 {strings_.option_data_list_, "option-data"},
                 {strings_.pools_list_, "pools"},
                 {strings_.relay_, "relay"},
                 {strings_.require_client_classes_, "require-client-classes"},
                 {strings_.reservations_, "reservations"},
                 {strings_.user_context_, "user-context"}}) {
            element = from->get(key);
            if (element) {
                member = element->str();
            }
        }
    }

    isc::data::ElementPtr toElement();

    void toElementCommon(isc::data::ElementPtr& map) {
        map->set("allocation-type", Subnet::allocationTypeToString(
                                        static_cast<Subnet::AllocationType>(allocation_type_)));
        map->set("id", id_);
        map->set("reservation-mode", Network::hostReservationModeToString(
                                         static_cast<Network::HRMode>(reservation_mode_)));

        for (auto& [member, key] :
             std::vector<std::tuple<uint64_t&, std::string>>{{renew_timer_, "renew-timer"},
                                                             {rebind_timer_, "rebind-timer"},
                                                             {valid_lifetime_, "valid-lifetime"}}) {
            map->set(key, member);
        }

        for (auto& [member, key] : std::vector<std::tuple<std::string&, std::string>>{
                 {strings_.client_class_, "client-class"},
                 {strings_.interface_, "interface"},
                 {strings_.subnet_, "subnet"}}) {
            if (!member.empty()) {
                map->set(key, member);
            }
        }

        for (auto& [member, key] : std::vector<std::tuple<std::string&, std::string>>{
                 {strings_.option_data_list_, "option-data"},
                 {strings_.pools_list_, "pools"},
                 {strings_.relay_, "relay"},
                 {strings_.require_client_classes_, "require-client-classes"},
                 {strings_.reservations_, "reservations"},
                 {strings_.user_context_, "user-context"}}) {
            if (!member.empty()) {
                map->set(key, isc::data::Element::fromJSON(member));
            }
        }
    }

    /// @brief common
    /// @{
    uint32_t id_ = 0;
    isc::db::timestamp_t timestamp_ = 0;
    uint64_t renew_timer_ = 0;
    uint64_t rebind_timer_ = 0;
    uint64_t valid_lifetime_ = 0;
    uint8_t allocation_type_ = 0;
    uint8_t rapid_commit_ = 0;
    uint8_t reservation_mode_ = 0;
    /// @}

    /// @brief v4
    /// @{
    uint8_t match_client_id_ = 0;
    uint8_t subnet_v4_psid_offset_ = 0;
    uint8_t subnet_v4_psid_len_ = 0;
    /// @}

    /// @brief v6
    /// @{
    uint64_t preferred_lifetime_ = 0;
    /// @}

    SubnetInfoStringsType strings_;

    /// @brief require to be updated by derived classes
    /// @{
    static std::vector<std::string>& columns();
    static std::vector<std::tuple<std::string, std::string>>& networkTopologyColumnsAndValues();
    /// @}

    /// @brief index at which delta columns start which needs to be updated whenever a common
    /// column
    ///     is added. This can be determined by counting the common columns in static methods.
    ///     It is expressed in starting-at-zero fashion.
    static int constexpr DELTA_COLUMNS_INDEX = 18;

private:
    inline bool equalCommon(SubnetInfoRaw<D, SubnetInfoStringsType> const& other) const {
        return id_ == other.id_ && timestamp_ == other.timestamp_ &&
               renew_timer_ == other.renew_timer_ && rebind_timer_ == other.rebind_timer_ &&
               valid_lifetime_ == other.valid_lifetime_ &&
               allocation_type_ == other.allocation_type_ && rapid_commit_ == other.rapid_commit_ &&
               reservation_mode_ == other.reservation_mode_ && strings_ == other.strings_;
    }

    bool equalDelta(SubnetInfoRaw<D, SubnetInfoStringsType> const& other) const;

    inline void convertCommon(SubnetInfoRaw<D, SubnetInfoCStrings<D>>& other) {
        id_ = other.id_;
        timestamp_ = other.timestamp_;
        renew_timer_ = other.renew_timer_;
        rebind_timer_ = other.rebind_timer_;
        valid_lifetime_ = other.valid_lifetime_;
        allocation_type_ = other.allocation_type_;
        rapid_commit_ = other.rapid_commit_;
        reservation_mode_ = other.reservation_mode_;
        strings_ = other.strings_;
    }

    void convertDelta(SubnetInfoRaw<D, SubnetInfoCStrings<D>>& other);
};

/// @brief wrapper over STL strings used in SubnetInfoRaw
template <DhcpSpaceType D>
struct SubnetInfoStrings {
    explicit SubnetInfoStrings() = default;

    /// @brief constructor from one specialization, necessarily marked explicit to avoid copying
    /// through two constructors
    explicit SubnetInfoStrings(SubnetInfoCStrings<D> const& other) {
        convertCommon(other);
        convertDelta(other);
    }

    SubnetInfoStrings& operator=(SubnetInfoCStrings<D> const& other) {
        convertCommon(other);
        convertDelta(other);
        return *this;
    }

    bool operator==(SubnetInfoStrings<D> const& other) const {
        return equalCommon(other) && equalDelta(other);
    }

    bool operator!=(SubnetInfoStrings<D> const& other) const {
        return !(*this == other);
    }

    /// @brief common
    /// @{
    std::string client_class_;
    std::string interface_;
    std::string interface_id_;
    std::string option_data_list_;
    std::string pools_list_;
    std::string relay_;
    std::string require_client_classes_;
    std::string reservations_;
    std::string subnet_;
    std::string user_context_;
    /// @}

    /// @brief v4
    /// @{
    std::string boot_file_name_;
    std::string next_server_;
    std::string server_hostname_;
    std::string subnet_4o6_interface_;
    std::string subnet_4o6_interface_id_;
    std::string subnet_4o6_subnet_;
    std::string subnet_v4_excluded_psids_;
    /// @}

    /// @brief v6
    /// @{
    std::string pd_pools_list_;
    /// @}

private:
    inline bool equalCommon(SubnetInfoStrings<D> const& other) const {
        return option_data_list_ == other.option_data_list_ && pools_list_ == other.pools_list_ &&
               subnet_ == other.subnet_ && interface_ == other.interface_ &&
               interface_id_ == other.interface_id_ && client_class_ == other.client_class_ &&
               require_client_classes_ == other.require_client_classes_ &&
               reservations_ == other.reservations_ && relay_ == other.relay_ &&
               user_context_ == other.user_context_;
    }

    bool equalDelta(SubnetInfoStrings<D> const& other) const;

    inline void convertCommon(SubnetInfoCStrings<D> const& other) {
        option_data_list_.assign(other.option_data_list_, other.option_data_list_length_);
        pools_list_.assign(other.pools_list_, other.pools_list_length_);
        subnet_.assign(other.subnet_, other.subnet_length_);
        interface_.assign(other.interface_, other.interface_length_);
        interface_id_.assign(other.interface_id_, other.interface_id_length_);
        client_class_.assign(other.client_class_, other.client_class_length_);
        require_client_classes_.assign(other.require_client_classes_,
                                       other.require_client_classes_length_);
        reservations_.assign(other.reservations_, other.reservations_length_);
        relay_.assign(other.relay_, other.relay_length_);
        user_context_.assign(other.user_context_, other.user_context_length_);
    }

    void convertDelta(SubnetInfoCStrings<D> const& other);
};
using Subnet4InfoStrings = SubnetInfoStrings<DHCP_SPACE_V4>;
using Subnet6InfoStrings = SubnetInfoStrings<DHCP_SPACE_V6>;

/// @brief wrapper over C strings used in SubnetInfoRaw
template <DhcpSpaceType D>
struct SubnetInfoCStrings {
    explicit SubnetInfoCStrings() {
        allocateCommon();
        allocateDelta();
    }

    /// @brief constructor from one specialization, necessarily marked explicit to avoid copying
    /// through two constructors
    explicit SubnetInfoCStrings(SubnetInfoStrings<D> const& other) {
        convertCommon(other);
        convertDelta(other);
    }

    SubnetInfoCStrings<D>& operator=(SubnetInfoStrings<D> const& other) {
        convertCommon(other);
        convertDelta(other);
        return *this;
    }

    /// @brief maybe consider smart-pointerizing the char*, the tradeoff is not having to worry
    /// about freeing memory vs small overhead of shared_ptr
    ~SubnetInfoCStrings() {
        deallocateCommon();
        deallocateDelta();
    }

    bool operator==(SubnetInfoCStrings<D> const& other) const {
        return equalCommon(other) && equalDelta(other);
    }

    bool operator!=(SubnetInfoCStrings<D> const& other) const {
        return !(*this == other);
    }

    /// @brief common
    /// @{
    char* client_class_;
    char* interface_;
    char* interface_id_;
    char* option_data_list_;
    char* pools_list_;
    char* relay_;
    char* require_client_classes_;
    char* reservations_;
    char* subnet_;
    char* user_context_;
    size_t client_class_length_;
    size_t interface_length_;
    size_t interface_id_length_;
    size_t option_data_list_length_;
    size_t pools_list_length_;
    size_t relay_length_;
    size_t require_client_classes_length_;
    size_t reservations_length_;
    size_t subnet_length_;
    size_t user_context_length_;
    /// @}

    /// @brief v4
    /// @{
    char* boot_file_name_;
    char* next_server_;
    char* server_hostname_;
    char* subnet_4o6_interface_;
    char* subnet_4o6_interface_id_;
    char* subnet_4o6_subnet_;
    char* subnet_v4_excluded_psids_;
    size_t boot_file_name_length_;
    size_t next_server_length_;
    size_t server_hostname_length_;
    size_t subnet_4o6_interface_length_;
    size_t subnet_4o6_interface_id_length_;
    size_t subnet_4o6_subnet_length_;
    size_t subnet_v4_excluded_psids_length_;
    /// @}

    /// @brief v6
    /// @{
    char* pd_pools_list_;
    size_t pd_pools_list_length_;
    /// @}

private:
    /// @brief marked private and deleted to make sure the above constructor is called instead,
    /// if a public implementation is needed here, make sure you allocate the char*, deep copy
    /// by strcpy.
    /// @{
    SubnetInfoCStrings<D>(SubnetInfoCStrings<D> const&) = delete;
    SubnetInfoCStrings<D>& operator=(SubnetInfoCStrings<D> const&) = delete;
    /// @}

    inline void allocateCommon() {
        option_data_list_ = new char[C_STRING_SIZE];
        pools_list_ = new char[C_STRING_SIZE];
        subnet_ = new char[C_STRING_SIZE];
        interface_ = new char[C_STRING_SIZE];
        interface_id_ = new char[C_STRING_SIZE];
        client_class_ = new char[C_STRING_SIZE];
        require_client_classes_ = new char[C_STRING_SIZE];
        reservations_ = new char[C_STRING_SIZE];
        relay_ = new char[C_STRING_SIZE];
        user_context_ = new char[C_STRING_SIZE];
    }

    void allocateDelta();

    inline void deallocateCommon() {
        delete[] option_data_list_;
        delete[] pools_list_;
        delete[] subnet_;
        delete[] interface_;
        delete[] interface_id_;
        delete[] client_class_;
        delete[] require_client_classes_;
        delete[] reservations_;
        delete[] relay_;
        delete[] user_context_;
    }

    void deallocateDelta();

    inline void convertCommon(SubnetInfoStrings<D> const& other) {
        option_data_list_length_ = other.option_data_list_.size();
        pools_list_length_ = other.pools_list_.size();
        subnet_length_ = other.subnet_.size();
        interface_length_ = other.interface_.size();
        interface_id_length_ = other.interface_id_.size();
        client_class_length_ = other.client_class_.size();
        require_client_classes_length_ = other.require_client_classes_.size();
        reservations_length_ = other.reservations_.size();
        relay_length_ = other.relay_.size();
        user_context_length_ = other.user_context_.size();

        std::strcpy(option_data_list_, other.option_data_list_.c_str());
        std::strcpy(pools_list_, other.pools_list_.c_str());
        std::strcpy(subnet_, other.subnet_.c_str());
        std::strcpy(interface_, other.interface_.c_str());
        std::strcpy(interface_id_, other.interface_id_.c_str());
        std::strcpy(client_class_, other.client_class_.c_str());
        std::strcpy(require_client_classes_, other.require_client_classes_.c_str());
        std::strcpy(reservations_, other.reservations_.c_str());
        std::strcpy(relay_, other.relay_.c_str());
        std::strcpy(user_context_, other.user_context_.c_str());
    }

    void convertDelta(SubnetInfoStrings<D> const& other);

    inline bool equalCommon(SubnetInfoCStrings<D> const& other) const {
        return option_data_list_length_ == other.option_data_list_length_ &&
               pools_list_length_ == other.pools_list_length_ &&
               subnet_length_ == other.subnet_length_ &&
               interface_length_ == other.interface_length_ &&
               interface_id_length_ == other.interface_id_length_ &&
               client_class_length_ == other.client_class_length_ &&
               require_client_classes_length_ == other.require_client_classes_length_ &&
               reservations_length_ == other.reservations_length_ &&
               relay_length_ == other.relay_length_ &&
               user_context_length_ == other.user_context_length_ &&
               !(std::strcmp(option_data_list_, other.option_data_list_) ||
                 std::strcmp(pools_list_, other.pools_list_) ||
                 std::strcmp(subnet_, other.subnet_) || std::strcmp(interface_, other.interface_) ||
                 std::strcmp(interface_id_, other.interface_id_) ||
                 std::strcmp(client_class_, other.client_class_) ||
                 std::strcmp(require_client_classes_, other.require_client_classes_) ||
                 std::strcmp(reservations_, other.reservations_) ||
                 std::strcmp(relay_, other.relay_) ||
                 std::strcmp(user_context_, other.user_context_));
    }

    bool equalDelta(SubnetInfoCStrings<D> const& other) const;

    static size_t constexpr C_STRING_SIZE = 1024;
};
using Subnet4InfoCStrings = SubnetInfoCStrings<DHCP_SPACE_V4>;
using Subnet6InfoCStrings = SubnetInfoCStrings<DHCP_SPACE_V6>;

/// @brief specialized aliases, use these where convenient
/// @{
template <DhcpSpaceType D>
using SubnetInfo = SubnetInfoRaw<D, SubnetInfoStrings<D>>;

template <DhcpSpaceType D>
using SubnetInfoPtr = db::Ptr<SubnetInfo<D>>;

using Subnet4Info = SubnetInfo<DHCP_SPACE_V4>;
using Subnet4InfoPtr = SubnetInfoPtr<DHCP_SPACE_V4>;

using Subnet6Info = SubnetInfo<DHCP_SPACE_V6>;
using Subnet6InfoPtr = SubnetInfoPtr<DHCP_SPACE_V6>;

template <DhcpSpaceType D>
using SubnetInfoCollection = db::Collection<SubnetInfo<D>>;

template <DhcpSpaceType D>
using SubnetInfoCollectionPtr = std::shared_ptr<SubnetInfoCollection<D>>;

template <DhcpSpaceType D>
using SubnetInfoWithCStrings = SubnetInfoRaw<D, SubnetInfoCStrings<D>>;

using Subnet4InfoWithCStrings = SubnetInfoWithCStrings<DHCP_SPACE_V4>;

using Subnet6InfoWithCStrings = SubnetInfoWithCStrings<DHCP_SPACE_V6>;
/// @}

/// @brief Abstract subnet manager
///
/// This is an abstract API for subnet database backends. It provides unified
/// interface to all backends. As this is an abstract class, it should not
/// be used directly, but rather specialized derived class should be used
/// instead.
///
/// This class throws no exceptions.  However, methods in concrete
/// implementations of this class may throw exceptions: see the documentation
/// of those classes for details.
template <DhcpSpaceType D>
struct SubnetMgr {
    virtual ~SubnetMgr() = default;

    void action(isc::data::effect_t const& effect,
                SubnetInfoPtr<D>& subnet,
                std::string const& config_timestamp_path) {
        bool const is_key_affected(effect.test(isc::data::KEY_IS_AFFECTED));
        isc::data::effect_t keyless_effect(effect);
        keyless_effect.set(isc::data::KEY_IS_AFFECTED, false);

        switch (keyless_effect.to_ulong()) {
        case (1 << isc::data::DELETED):
            if (is_key_affected) {
                del(subnet, config_timestamp_path);
            } else {
                upsert(subnet, config_timestamp_path);
            }
            break;
        case (1 << isc::data::INSERTED):
            upsert(subnet, config_timestamp_path);
            break;
        case (1 << isc::data::MODIFIED):
            // It means it was INSERTED over sysrepo, MODIFIED over database, so update().
            upsert(subnet, config_timestamp_path);
            break;
        case 0:
            break;
        default:
            isc_throw(Unexpected, PRETTY_METHOD_NAME() << ": stumbled upon unhandled effect "
                                                       << isc::data::Effect::print(effect));
        }
    }

    /// @brief the functional interface
    /// @{
    virtual void del(SubnetInfoPtr<D>& subnet, std::string const& config_timestamp_path) = 0;
    virtual void insert(SubnetInfoPtr<D>& subnet, std::string const& config_timestamp_path) = 0;
    virtual SubnetInfoCollection<D>
    select(std::string const& config_timestamp_path = std::string()) = 0;
    virtual SubnetInfoPtr<D>
    selectBySubnet(std::string& subnet,
                   std::string const& config_timestamp_path = std::string()) = 0;
    virtual SubnetInfoPtr<D> selectBySubnetID(SubnetID subnet_id) = 0;
    SubnetInfoPtr<D> selectBySubnet(SubnetInfoPtr<D>& subnet,
                                    std::string const& config_timestamp_path = std::string()) {
        return selectBySubnet(subnet->strings_.subnet_, config_timestamp_path);
    }
    virtual void update(SubnetInfoPtr<D>& subnet, std::string const& config_timestamp_path) = 0;
    /// @}

    virtual void commit() = 0;
    virtual void rollback() = 0;
    virtual std::string getName() const = 0;
    virtual std::string getType() const = 0;
    virtual SubnetID maxSubnetID() = 0;

    void upsert(SubnetInfoPtr<D>& subnet, std::string const& config_timestamp_path) {
        try {
            update(subnet, config_timestamp_path);
        } catch (isc::db::StatementNotApplied const& exception) {
            insert(subnet, config_timestamp_path);
        }
    }

    /// @brief Abstractions
    void syncSubnets(std::string const& config_timestamp_path) {
        // Retrieve subnets first.
        SubnetInfoCollection<D> subnets(select(config_timestamp_path));

        // Get the subnets from the configuration which will be updated in the database.
        SubnetCollection<D> const* new_subnets(
            CfgMgr::instance().getStagingCfg()->getCfgSubnets<D>()->getAll());

        // Delete extra subnets.
        for (SubnetInfoPtr<D>& database_subnet : subnets) {
            bool found(false);
            for (SubnetTPtr<D> const& subnet : *new_subnets) {
                if (database_subnet->strings_.subnet_ == subnet->toText()) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                del(database_subnet, config_timestamp_path);
            }
        }

        // Upsert actual subnets.
        for (SubnetTPtr<D> const& subnet : *new_subnets) {
            SubnetInfoPtr<D> info(std::make_shared<SubnetInfo<D>>(subnet));
            upsert(info, config_timestamp_path);
        }
    }

    void addSubnets(std::string const& config_timestamp_path) {
        // Get the subnets from the configuration which will be updated in the database.
        SubnetCollection<D> const* new_subnets(
            CfgMgr::instance().getStagingCfg()->getCfgSubnets<D>()->getAll());

        Subnet::setSubnetID(maxSubnetID());
        Subnet::generateNextID();
        for (SubnetTPtr<D> const& subnet : *new_subnets) {
            try {
                SubnetInfoPtr<D> info(std::make_shared<SubnetInfo<D>>(subnet));
                if (subnet->generated()) {
                    info->id_ = Subnet::generateNextID();
                }
                insert(info, config_timestamp_path);
            } catch (isc::db::DuplicateEntry const& exception) {
                // A duplicate was being inserted. Duplicates are expected.
                // Carry on.
            }
        }
    }

    void delSubnets(std::string const& config_timestamp_path) {
        // Get the subnets from the configuration which will be updated in the database.
        SubnetCollection<D> const* new_subnets(
            CfgMgr::instance().getStagingCfg()->getCfgSubnets<D>()->getAll());

        for (SubnetTPtr<D> const& subnet : *new_subnets) {
            SubnetInfoPtr<D> info(std::make_shared<SubnetInfo<D>>(subnet));
            del(info, config_timestamp_path);
        }
    }

    void setSubnets(std::string const& config_timestamp_path) {
        // Get the subnets from the configuration which will be updated in the database.
        SubnetCollection<D> const* new_subnets(
            CfgMgr::instance().getStagingCfg()->getCfgSubnets<D>()->getAll());

        // Update.
        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        for (SubnetTPtr<D> const& subnet : *new_subnets) {
            SubnetInfoPtr<D> info(std::make_shared<SubnetInfo<D>>(subnet));
            update(info, config_timestamp_path);
        }
    }
    /// @}

    static isc::data::ElementPtr
    convertIETFNetworkRangetoKeaSubnet(isc::data::ElementPtr const& network_range) {
        isc::data::ElementPtr const subnet(isc::data::Element::createMap());
        subnet->checkAndSet("id", network_range, "id");
        isc::data::ElementPtr const& network_desc(network_range->get("description"));
        if (network_desc) {
            subnet->set("network-description", network_desc->toUnquotedString());
        }
        subnet->checkAndSet("subnet", network_range, "network-prefix");
        subnet->checkAndSet("valid-lifetime", network_range, "valid-lifetime");
        subnet->checkAndSet("renew-timer", network_range, "renew-time");
        subnet->checkAndSet("rebind-timer", network_range, "rebind-time");
        subnet->checkAndSet("preferred-lifetime", network_range, "preferred-lifetime");
        subnet->checkAndSet("rapid-commit", network_range, "rapid-commit");
        isc::data::ElementPtr const& address_pools(network_range->get("address-pools"));
        if (address_pools) {
            isc::data::ElementPtr const& address_pool(address_pools->get("address-pool"));
            if (address_pool) {
                isc::data::ElementPtr const pools(isc::data::Element::createList());
                for (isc::data::ElementPtr const& i : address_pool->listValue()) {
                    isc::data::ElementPtr const ii(isc::data::Element::createMap());
                    ii->checkAndSet("id", i, "pool-id");
                    isc::asiolink::IOAddress start_address(i->get("start-address")->stringValue());
                    isc::asiolink::IOAddress end_address(i->get("end-address")->stringValue());
                    std::string pool_prefix(i->get("pool-prefix")->stringValue());
                    auto const& [address, prefix_length](Subnet6::parsePrefix(pool_prefix));
                    Pool6 p1(Lease::TYPE_NA, start_address, end_address);
                    Pool6 p2(Lease::TYPE_NA, address, prefix_length);
                    if (p1.getCapacity() <= p2.getCapacity()) {
                        ii->set("pool", start_address.toText() + " - " + end_address.toText());
                    } else {
                        ii->set("pool", pool_prefix);
                    }
                    ii->checkAndSet("max-address-utilization", i, "max-address-utilization");
                    /// @todo(missing in kea)
                    /// ii->checkAndSet("valid-lifetime", i, "valid-lifetime");
                    /// @todo(missing in kea)
                    /// ii->checkAndSet("renew-time", i, "renew-time");
                    /// @todo(missing in kea)
                    /// ii->checkAndSet("rebind-time", i, "rebind-time");
                    /// @todo(missing in kea)
                    /// ii->checkAndSet("preferred-lifetime", i, "preferred-lifetime");
                    /// @todo(missing in kea)
                    /// ii->checkAndSet("rapid-commit", i, "rapid-commit");
#ifdef false
                    isc::data::ElementPtr const& host_reservations(ii->get("host-reservations"));
                    if (host_reservations) {
                        isc::data::ElementPtr const& host_reservation(
                            host_reservations->get("address-pool"));
                        if (host_reservation) {
                            isc::data::ElementPtr const hosts(isc::data::Element::createList());
                            for (isc::data::ElementPtr const& j : host_reservation->listValue()) {
                                isc::data::ElementPtr const jj(isc::data::Element::createMap());
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("client-duid", j, "client-duid");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("reserved-addr", j, "reserved-addr");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("option-set-id", j, "option-set-id");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("valid-lifetime", j, "valid-lifetime");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("renew-time", j, "renew-time");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("rebind-time", j, "rebind-time");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("preferred-lifetime", j,
                                /// "preferred-lifetime");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("rapid-commit", j, "rapid-commit");
                                hosts->add(jj);
                            }
                            ii->set("hosts", hosts);
                        }
                    }
#endif
                    pools->add(ii);
                }
                subnet->set("pools", pools);
            }
        }
        isc::data::ElementPtr const& prefix_pools(network_range->get("prefix-pools"));
        if (prefix_pools) {
            isc::data::ElementPtr const& prefix_pool(prefix_pools->get("prefix-pool"));
            if (prefix_pool) {
                isc::data::ElementPtr const pd_pools(isc::data::Element::createList());
                for (isc::data::ElementPtr const& i : prefix_pool->listValue()) {
                    isc::data::ElementPtr const ii(isc::data::Element::createMap());
                    ii->checkAndSet("id", i, "pool-id");
                    std::string pool_prefix(i->get("pool-prefix")->stringValue());
                    auto const& [address, prefix_length](Subnet6::parsePrefix(pool_prefix));
                    ii->set("prefix", address.toText());
                    ii->set("prefix-len", prefix_length);
                    ii->checkAndSet("delegated-len", i, "client-prefix-length");
                    ii->checkAndSet("max-pd-space-utilization", i, "max-pd-space-utilization");
                    isc::data::ElementPtr const& pd_exclude(
                        i->get("ietf-dhcpv6-options-rfc6603:pd-exclude"));
                    if (pd_exclude && pd_exclude->contains("ipv6-subnet")) {
                        auto [prefix_addr, prefix_size] =
                            Subnet6::parsePrefix(pd_exclude->get("ipv6-subnet")->stringValue());
                        ii->set("excluded-prefix", prefix_addr.toText());
                        ii->set("excluded-prefix-len", prefix_size);
                    }
                    /// @todo(missing in kea)
                    /// ii->checkAndSet("valid-lifetime", i, "valid-lifetime");
                    /// @todo(missing in kea)
                    /// ii->checkAndSet("renew-time", i, "renew-time");
                    /// @todo(missing in kea)
                    /// ii->checkAndSet("rebind-time", i, "rebind-time");
                    /// @todo(missing in kea)
                    /// ii->checkAndSet("preferred-lifetime", i, "preferred-lifetime");
                    /// @todo(missing in kea)
                    /// ii->checkAndSet("rapid-commit", i, "rapid-commit");
#ifdef false
                    isc::data::ElementPtr const& host_reservations(ii->get("host-reservations"));
                    if (host_reservations) {
                        isc::data::ElementPtr const& host_reservation(
                            host_reservations->get("address-pool"));
                        if (host_reservation) {
                            isc::data::ElementPtr const hosts(isc::data::Element::createList());
                            for (isc::data::ElementPtr const& j : host_reservation->listValue()) {
                                isc::data::ElementPtr const jj(isc::data::Element::createMap());
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("client-duid", j, "client-duid");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("reserved-addr", j, "reserved-addr");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("option-set-id", j, "option-set-id");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("valid-lifetime", j, "valid-lifetime");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("renew-time", j, "renew-time");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("rebind-time", j, "rebind-time");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("preferred-lifetime", j, "preferred-lifetime");
                                /// @todo(missing in kea)
                                /// jj->checkAndSet("rapid-commit", j, "rapid-commit");
                                hosts->add(jj);
                            }
                            ii->set("hosts", hosts);
                        }
                    }
#endif
                    pd_pools->add(ii);
                }
                subnet->set("pd-pools", pd_pools);
            }
        }
        return subnet;
    }

    static isc::data::ElementPtr
    convertKeaSubnetToIETFNetworkRange(isc::data::ElementPtr const& subnet) {
        isc::data::ElementPtr const network_range(isc::data::Element::createMap());
        network_range->checkAndSet("id", subnet, "id");
        network_range->checkAndSet("description", subnet, "network-description");
        network_range->checkAndSet("network-prefix", subnet, "subnet");
        network_range->checkAndSet("valid-lifetime", subnet, "valid-lifetime");
        network_range->checkAndSet("renew-time", subnet, "renew-timer");
        network_range->checkAndSet("rebind-time", subnet, "rebind-timer");
        network_range->checkAndSet("preferred-lifetime", subnet, "preferred-lifetime");
        network_range->checkAndSet("rapid-commit", subnet, "rapid-commit");
        isc::data::ElementPtr const& pools(subnet->get("pools"));
        if (pools) {
            isc::data::ElementPtr const address_pool(isc::data::Element::createList());
            for (isc::data::ElementPtr const& i : pools->listValue()) {
                isc::data::ElementPtr const subnet(isc::data::Element::createMap());
                subnet->checkAndSet("pool-id", i, "id");
                subnet->checkAndSet("max-address-utilization", i, "max-address-utilization");

                // start-address, end-address
                Pool6Parser parser;
                PoolStoragePtr pools(std::make_shared<PoolStorage>());
                parser.parse(pools, i, AF_INET6);
                std::unique_ptr<isc::asiolink::IOAddress> start_address;
                std::unique_ptr<isc::asiolink::IOAddress> end_address;
                size_t max_capacity(0);
                for (PoolPtr const& pool : *pools) {
                    isc::asiolink::IOAddress const s(pool->getFirstAddress());
                    isc::asiolink::IOAddress const e(pool->getLastAddress());
                    if (start_address) {
                        if (s < *start_address) {
                            start_address = std::make_unique<isc::asiolink::IOAddress>(s);
                        }
                    } else {
                        start_address = std::make_unique<isc::asiolink::IOAddress>(s);
                    }
                    if (end_address) {
                        if (*end_address < e) {
                            end_address = std::make_unique<isc::asiolink::IOAddress>(e);
                        }
                    } else {
                        end_address = std::make_unique<isc::asiolink::IOAddress>(e);
                    }
                    max_capacity += pool->getCapacity();
                }
                subnet->set("start-address", start_address->toText());
                subnet->set("end-address", end_address->toText());
                subnet->set("pool-prefix",
                            start_address->toText() + "/" +
                                std::to_string(size_t(128 - ceil(log2(max_capacity)))));

                address_pool->add(subnet);
            }
            isc::data::ElementPtr const address_pools(isc::data::Element::createMap());
            address_pools->set("address-pool", address_pool);
            network_range->set("address-pools", address_pools);
        }
        isc::data::ElementPtr const& pd_pools(subnet->get("pd-pools"));
        if (pd_pools) {
            isc::data::ElementPtr const prefix_pool(isc::data::Element::createList());
            for (isc::data::ElementPtr const& i : pd_pools->listValue()) {
                isc::data::ElementPtr const subnet(isc::data::Element::createMap());
                subnet->checkAndSet("pool-id", i, "id");
                isc::data::ElementPtr prefix_element(i->get("prefix"));
                isc::data::ElementPtr prefix_length_element(i->get("prefix-len"));
                if (prefix_element && prefix_length_element) {
                    subnet->set("pool-prefix",
                                prefix_element->stringValue() + "/" +
                                    std::to_string(prefix_length_element->intValue()));
                }
                subnet->checkAndSet("client-prefix-length", i, "delegated-len");
                subnet->checkAndSet("max-pd-space-utilization", i, "max-pd-space-utilization");
                prefix_pool->add(subnet);
            }
            isc::data::ElementPtr const prefix_pools(isc::data::Element::createMap());
            prefix_pools->set("prefix-pool", prefix_pool);
            network_range->set("prefix-pools", prefix_pools);
        }
        return network_range;
    }
};
using Subnet4Mgr = SubnetMgr<DHCP_SPACE_V4>;
using Subnet6Mgr = SubnetMgr<DHCP_SPACE_V6>;

}  // namespace dhcp
}  // namespace isc

#endif  // SUBNET_MGR_H

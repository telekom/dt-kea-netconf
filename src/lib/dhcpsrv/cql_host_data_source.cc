// Copyright (C) 2018-2020 Internet Systems Consortium, Inc. ("ISC")
// Copyright (C) 2016-2018 Deutsche Telekom AG.
//
// Author: Andrei Pavel <andrei.pavel@qualitance.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//           http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <config.h>

#include <asiolink/io_address.h>
#include <cql/cql_exchange.h>
#include <cql/cql_transaction.h>
#include <database/db_exceptions.h>
#include <dhcp/duid.h>
#include <dhcp/libdhcp++.h>
#include <dhcp/option.h>
#include <dhcp/option_definition.h>
#include <dhcpsrv/cfg_option.h>
#include <dhcpsrv/cfgmgr.h>
#include <dhcpsrv/cql_host_data_source.h>
#include <dhcpsrv/dhcpsrv_log.h>
#include <util/buffer.h>
#include <util/hash.h>
#include <util/optional.h>

#include <stdint.h>  // for uint64_t

#include <boost/algorithm/string/classification.hpp>  // for boost::is_any_of
#include <boost/algorithm/string/split.hpp>  // for boost::split

#include <string>

using namespace isc::asiolink;
using namespace isc::db;
using namespace isc::dhcp;
using namespace isc::util;
using namespace isc::data;

namespace {

/// @brief Host identifier consisting of DUID or hardware address
using HostIdentifier = std::vector<uint8_t>;

/// @brief key for HostMap containing objects which uniquely identify a
///     host: host identifier, host identifier type, subnets for IPv4 and IPv6
///     and the IPv4 reservation
using HostKey = std::tuple<HostIdentifier, Host::IdentifierType, SubnetID, SubnetID, IOAddress>;

/// @brief Identifies components of the host key
enum HostKeyComponent {
    HOST_IDENTIFIER,
    HOST_IDENTIFIER_TYPE,
    IPv4_SUBNET_ID,
    IPv6_SUBNET_ID,
    IPv4_RESERVATION
};

/// @brief Map used to merge reservations and options into a single host on
///     retrieve from database
using HostMap = std::unordered_map<HostKey, HostPtr, boost::hash<HostKey>>;

/// @brief A pair of (host key and a pointer to a host)
using HostPair = std::pair<HostKey, HostPtr>;

/// @brief Wrapper used to specify option space alongside option descriptor
struct OptionWrapper {
    OptionWrapper(OptionDescriptorPtr option_descriptor, std::string option_space)
        : option_descriptor_(option_descriptor), option_space_(option_space) {
    }

    OptionDescriptorPtr option_descriptor_;
    std::string option_space_;
};

/// @brief Maximum length of classes stored in a host_ipv4/6_client_classes
///     column.
static constexpr size_t CLIENT_CLASSES_MAX_LENGTH = 255u;

/// @brief Maximum length of the hostname stored in DNS. This length is
///     restricted by the length of the domain-name carried in the Client FQDN
///     Option (see RFC4702 and RFC4704).
static constexpr size_t HOSTNAME_MAX_LENGTH = 255u;

/// @brief Maximum length of option space name
static constexpr size_t OPTION_SPACE_MAX_LENGTH = 128u;

/// @brief Numeric value representing the last supported identifier. This value
///     is used to validate whether the identifier type stored in a database is
///     within bounds of supported identifiers.
static constexpr cass_int32_t MAX_IDENTIFIER_TYPE = static_cast<cass_int32_t>(Host::IDENT_FLEX);

/// @{
/// @brief Invalid values in the Cassandra database
static constexpr char NULL_DHCP4_SERVER_HOSTNAME[] = "";
static constexpr char NULL_DHCP4_BOOT_FILE_NAME[] = "";
static constexpr char NULL_USER_CONTEXT[] = "";
static constexpr char NULL_RESERVED_IPV6_PREFIX_ADDRESS[] = "::";
static constexpr cass_int32_t NULL_RESERVED_IPV6_PREFIX_LENGTH = 0;
static constexpr cass_int32_t NULL_RESERVED_IPV6_PREFIX_ADDRESS_TYPE = -1;
static constexpr cass_int32_t NULL_IAID = -1;
static constexpr cass_int32_t NULL_OPTION_UNIVERSE = -1;
static constexpr cass_int32_t NULL_OPTION_CODE = -1;
static const CassBlob NULL_OPTION_VALUE = CassBlob();
static constexpr char NULL_OPTION_FORMATTED_VALUE[] = "";
static constexpr char NULL_OPTION_SPACE[] = "";
static constexpr cass_bool_t NULL_OPTION_IS_PERSISTENT = cass_false;
static constexpr char NULL_OPTION_CLIENT_CLASS[] = "";
static constexpr cass_int32_t NULL_OPTION_SUBNET_ID = -1;
static constexpr char NULL_OPTION_USER_CONTEXT[] = "";
static constexpr cass_int32_t NULL_OPTION_SCOPE_ID = -1;
/// @}

/// @brief Invalid reservation used to check for an invalid IPv6Resrv formed
///     from database values.
static const IPv6Resrv NULL_IPV6_RESERVATION = IPv6Resrv(IPv6Resrv::TYPE_NA, IOAddress("::"), 128);

}  // namespace

namespace isc {
namespace dhcp {

/// @brief Provides mechanisms for sending and retrieving data from the
///     hosts table.
struct CqlHostExchange : CqlExchange<Host> {
    /// @brief Constructor
    ///
    /// Specifies table columns.
    CqlHostExchange();

    /// @brief Binds member variables to data array to receive @ref Host data.
    ///
    /// Creates a bind array to receive @ref Host data from the Cassandra
    /// database. After data is successfully received, @ref retrieve() can be
    /// called to retrieve the @ref Host object. Called in @ref
    /// isc::db::CqlExchange::executeSelect().
    ///
    /// @param data array of objects representing data being retrieved
    /// @param statement_tag prepared statement being executed; defaults to an
    ///     invalid statement
    void createBindForSelect(AnyArray &data,
                             StatementTag const &statement_tag = StatementTag()) override final;

    /// @brief Sets the exchange members with data of @ref Host.
    ///
    /// Fills in the members of the exchange with data from @ref Host object.
    ///
    /// @param host @ref Host object being modified in the Cassandra database
    /// @param subnet_id identifier of the subnet to which the host belongs
    /// @param reservation IPv6 reservation belonging to the host
    /// @param option_space option space
    /// @param option_descriptor structure used to hold option information
    void prepareExchange(const HostPtr &host,
                         const Optional<SubnetID> &subnet_id,
                         const IPv6Resrv *const reservation,
                         const std::string &option_space,
                         const OptionDescriptor &option_descriptor);

    /// @brief Binds @ref Host to data array to send data to the Cassandra
    ///     database.
    ///
    /// Fills in the bind array for sending data stored in the @ref Host object
    /// to the database.
    ///
    /// @param host @ref Host object being added to the Cassandra database
    /// @param subnet_id identifier of the subnet to which the host belongs
    /// @param reservation IPv6 reservation belonging to the host
    /// @param option_space option space
    /// @param option_descriptor structure used to hold option information
    /// @param statement_tag tag of the statement being executed
    /// @param data array being filled with data from to the Host object
    void createBindForMutation(const HostPtr &host,
                               const Optional<SubnetID> &subnet_id,
                               const IPv6Resrv *const reservation,
                               const std::string &option_space,
                               const OptionDescriptor &option_descriptor,
                               StatementTag statement_tag,
                               AnyArray &data);

    /// @brief Binds @ref Host to data array to send data to the Cassandra
    ///     database.
    ///
    /// Fills in the bind array for sending data stored in the @ref Host object
    /// to the database.
    ///
    /// @param host @ref Host object being deleted from the Cassandra database
    /// @param subnet_id identifier of the subnet to which the host belongs
    /// @param reservation IPv6 reservation belonging to the host
    /// @param option_space option space
    /// @param option_descriptor structure used to hold option information
    /// @param statement_tag tag of the statement being executed
    /// @param data array being filled with data from to the Host object
    void createBindForDelete(const HostPtr &host,
                             const Optional<SubnetID> &subnet_id,
                             const IPv6Resrv *const reservation,
                             const std::string &option_space,
                             const OptionDescriptor &option_descriptor,
                             StatementTag statement_tag,
                             AnyArray &data);

    /// @brief Create unique hash for storage in table id.
    ///
    /// Hash function used for creating a pseudo-unique hash from member
    /// values which uniquely determine an entry in the table. Uses FNV-1a
    /// on 64 bits.
    ///
    /// The primary key clustering column aggregates: host_dentifier,
    /// host_identifier_type, host_ipv4_subnet_id, host_ipv6_subnet_id,
    /// host_ipv4_address, reserved_ipv6_prefix_address,
    /// reserved_ipv6_prefix_length, option_code, option_space.
    uint64_t hashIntoId() const;

    /// @brief Create unique key for storage in table key.
    ///
    /// The primary key partition key aggregates: host_dentifier,
    /// host_identifier_type, host_ipv4_subnet_id, host_ipv6_subnet_id,
    /// host_ipv4_address
    uint64_t hashIntoKey() const;

    /// @brief Create unique key string for a host.
    ///
    /// The primary key partition key aggregates: host_dentifier,
    /// host_identifier_type, host_ipv4_subnet_id, host_ipv6_subnet_id,
    /// host_ipv4_address
    std::string hostKey() const;

    /// @brief Copy received data into Host object
    ///
    /// Copies information about the host into a newly created @ref Host object
    /// Called in @ref executeSelect after @ref createBindForSelect().
    ///
    /// @return Host Pointer to a @ref HostPtr object holding a pointer to the
    /// @ref Host object returned.
    HostPtr retrieve() override final;

    /// @brief Creates IPv6 reservation from the data contained in the
    ///     currently processed row.
    ///
    /// Called after createBindForSelect().
    ///
    /// @return IPv6Resrv object (containing IPv6 address or prefix reservation)
    const IPv6Resrv retrieveReservation() const;

    /// @brief Retrieves option from members.
    ///
    /// Called after createBindForSelect().
    ///
    /// @return OptionDescriptorPtr object (containing the option from the
    /// database)
    const OptionWrapper retrieveOption() const;

    static StatementMap &tagged_statements() {
        static StatementMap _{
            {INSERT_HOST(),
             {INSERT_HOST(), "INSERT INTO hosts ( "
                             "key, "
                             "id, "
                             "host_identifier, "
                             "host_identifier_type, "
                             "host_ipv4_subnet_id, "
                             "host_ipv6_subnet_id, "
                             "host_ipv4_address, "
                             "host_ipv4_next_server, "
                             "host_ipv4_server_hostname, "
                             "host_ipv4_boot_file_name, "
                             "auth_key, "
                             "hostname, "
                             "lower_case_hostname, "
                             "user_context, "
                             "host_ipv4_client_classes, "
                             "host_ipv6_client_classes, "
                             "reserved_ipv6_prefix_address, "
                             "reserved_ipv6_prefix_length, "
                             "reserved_ipv6_prefix_address_type, "
                             "iaid, "
                             "option_universe, "
                             "option_code, "
                             "option_value, "
                             "option_formatted_value, "
                             "option_space, "
                             "option_is_persistent, "
                             "option_client_class, "
                             "option_subnet_id, "
                             "option_user_context, "
                             "option_scope_id "
                             ") VALUES ( "
                             // key
                             "?, "
                             // id
                             "?, "
                             // host
                             "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
                             // denormalized reservation, option
                             "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? "
                             ") "
#ifdef TERASTREAM_FULL_TRANSACTIONS
                             "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
                             "IF NOT EXISTS "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST(),
             {GET_HOST(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_HOST_ID(),
             {GET_HOST_BY_HOST_ID(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_identifier = ? "
              "AND host_identifier_type = ? "
              "ALLOW FILTERING "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV4_ADDRESS(),
             {GET_HOST_BY_IPV4_ADDRESS(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv4_address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV4_SUBNET_ID_AND_HOST_ID(),
             {GET_HOST_BY_IPV4_SUBNET_ID_AND_HOST_ID(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv4_subnet_id = ? "
              "AND host_identifier = ? "
              "AND host_identifier_type = ? "
              "ALLOW FILTERING "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV6_SUBNET_ID_AND_HOST_ID(),
             {GET_HOST_BY_IPV6_SUBNET_ID_AND_HOST_ID(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv6_subnet_id = ? "
              "AND host_identifier = ? "
              "AND host_identifier_type = ? "
              "ALLOW FILTERING "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV4_SUBNET_ID_AND_ADDRESS(),
             {GET_HOST_BY_IPV4_SUBNET_ID_AND_ADDRESS(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv4_subnet_id = ? "
              "AND host_ipv4_address = ? "
              "ALLOW FILTERING "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV6_PREFIX(),
             {GET_HOST_BY_IPV6_PREFIX(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE reserved_ipv6_prefix_address = ? "
              "AND reserved_ipv6_prefix_length = ? "
              "ALLOW FILTERING "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV6_SUBNET_ID_AND_ADDRESS(),
             {GET_HOST_BY_IPV6_SUBNET_ID_AND_ADDRESS(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv6_subnet_id = ? "
              "AND reserved_ipv6_prefix_address = ? "
              "ALLOW FILTERING "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV4_SUBNET_ID(),
             {GET_HOST_BY_IPV4_SUBNET_ID(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv4_subnet_id = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV6_SUBNET_ID(),
             {GET_HOST_BY_IPV6_SUBNET_ID(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv6_subnet_id = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_HOST_NAME(),
             {GET_HOST_BY_HOST_NAME(), "SELECT "
                                       "key, "
                                       "id, "
                                       "host_identifier, "
                                       "host_identifier_type, "
                                       "host_ipv4_subnet_id, "
                                       "host_ipv6_subnet_id, "
                                       "host_ipv4_address, "
                                       "host_ipv4_next_server, "
                                       "host_ipv4_server_hostname, "
                                       "host_ipv4_boot_file_name, "
                                       "auth_key, "
                                       "hostname, "
                                       "user_context, "
                                       "host_ipv4_client_classes, "
                                       "host_ipv6_client_classes, "
                                       "reserved_ipv6_prefix_address, "
                                       "reserved_ipv6_prefix_length, "
                                       "reserved_ipv6_prefix_address_type, "
                                       "iaid, "
                                       "option_universe, "
                                       "option_code, "
                                       "option_value, "
                                       "option_formatted_value, "
                                       "option_space, "
                                       "option_is_persistent, "
                                       "option_client_class, "
                                       "option_subnet_id, "
                                       "option_user_context, "
                                       "option_scope_id "
                                       "FROM hosts "
                                       "WHERE lower_case_hostname = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
                                       "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_HOST_NAME_AND_IPV4_SUBNET_ID(),
             {GET_HOST_BY_HOST_NAME_AND_IPV4_SUBNET_ID(), "SELECT "
                                                          "key, "
                                                          "id, "
                                                          "host_identifier, "
                                                          "host_identifier_type, "
                                                          "host_ipv4_subnet_id, "
                                                          "host_ipv6_subnet_id, "
                                                          "host_ipv4_address, "
                                                          "host_ipv4_next_server, "
                                                          "host_ipv4_server_hostname, "
                                                          "host_ipv4_boot_file_name, "
                                                          "auth_key, "
                                                          "hostname, "
                                                          "user_context, "
                                                          "host_ipv4_client_classes, "
                                                          "host_ipv6_client_classes, "
                                                          "reserved_ipv6_prefix_address, "
                                                          "reserved_ipv6_prefix_length, "
                                                          "reserved_ipv6_prefix_address_type, "
                                                          "iaid, "
                                                          "option_universe, "
                                                          "option_code, "
                                                          "option_value, "
                                                          "option_formatted_value, "
                                                          "option_space, "
                                                          "option_is_persistent, "
                                                          "option_client_class, "
                                                          "option_subnet_id, "
                                                          "option_user_context, "
                                                          "option_scope_id "
                                                          "FROM hosts "
                                                          "WHERE lower_case_hostname = ? "
                                                          "AND host_ipv4_subnet_id = ? "
                                                          "ALLOW FILTERING"
#ifdef TERASTREAM_FULL_TRANSACTIONS
                                                          "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_HOST_NAME_AND_IPV6_SUBNET_ID(),
             {GET_HOST_BY_HOST_NAME_AND_IPV6_SUBNET_ID(), "SELECT "
                                                          "key, "
                                                          "id, "
                                                          "host_identifier, "
                                                          "host_identifier_type, "
                                                          "host_ipv4_subnet_id, "
                                                          "host_ipv6_subnet_id, "
                                                          "host_ipv4_address, "
                                                          "host_ipv4_next_server, "
                                                          "host_ipv4_server_hostname, "
                                                          "host_ipv4_boot_file_name, "
                                                          "auth_key, "
                                                          "hostname, "
                                                          "user_context, "
                                                          "host_ipv4_client_classes, "
                                                          "host_ipv6_client_classes, "
                                                          "reserved_ipv6_prefix_address, "
                                                          "reserved_ipv6_prefix_length, "
                                                          "reserved_ipv6_prefix_address_type, "
                                                          "iaid, "
                                                          "option_universe, "
                                                          "option_code, "
                                                          "option_value, "
                                                          "option_formatted_value, "
                                                          "option_space, "
                                                          "option_is_persistent, "
                                                          "option_client_class, "
                                                          "option_subnet_id, "
                                                          "option_user_context, "
                                                          "option_scope_id "
                                                          "FROM hosts "
                                                          "WHERE lower_case_hostname = ? "
                                                          "AND host_ipv6_subnet_id = ? "
                                                          "ALLOW FILTERING"
#ifdef TERASTREAM_FULL_TRANSACTIONS
                                                          "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV4_SUBNET_ID_LIMIT(),
             {GET_HOST_BY_IPV4_SUBNET_ID_LIMIT(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv4_subnet_id = ? "
              "LIMIT 1 "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV4_SUBNET_ID_NEXT_KEY(),
             {GET_HOST_BY_IPV4_SUBNET_ID_NEXT_KEY(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv4_subnet_id = ? "
              "AND TOKEN(key) > TOKEN(?) "
              "LIMIT 1 "
#ifdef SCYLLA_SUPPORT_ENABLED
              "ALLOW FILTERING"
#endif  // SCYLLA_SUPPORT_ENABLED
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV4_SUBNET_ID_KEY(),
             {GET_HOST_BY_IPV4_SUBNET_ID_KEY(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE key = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV4_SUBNET_ID_PAGE(),
             {GET_HOST_BY_IPV4_SUBNET_ID_PAGE(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv4_subnet_id = ? "
              "AND id = ? "
              "LIMIT 1 "
              "ALLOW FILTERING "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV6_SUBNET_ID_LIMIT(),
             {GET_HOST_BY_IPV6_SUBNET_ID_LIMIT(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv6_subnet_id = ? "
              "LIMIT 1 "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV6_SUBNET_ID_NEXT_KEY(),
             {GET_HOST_BY_IPV6_SUBNET_ID_NEXT_KEY(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv6_subnet_id = ? "
              "AND TOKEN(key) > TOKEN(?) "
              "LIMIT 1 "
#ifdef SCYLLA_SUPPORT_ENABLED
              "ALLOW FILTERING"
#endif  // SCYLLA_SUPPORT_ENABLED
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV6_SUBNET_ID_KEY(),
             {GET_HOST_BY_IPV6_SUBNET_ID_KEY(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE key = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {GET_HOST_BY_IPV6_SUBNET_ID_PAGE(),
             {GET_HOST_BY_IPV6_SUBNET_ID_PAGE(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
              "key, "
              "id, "
              "host_identifier, "
              "host_identifier_type, "
              "host_ipv4_subnet_id, "
              "host_ipv6_subnet_id, "
              "host_ipv4_address, "
              "host_ipv4_next_server, "
              "host_ipv4_server_hostname, "
              "host_ipv4_boot_file_name, "
              "auth_key, "
              "hostname, "
              "user_context, "
              "host_ipv4_client_classes, "
              "host_ipv6_client_classes, "
              "reserved_ipv6_prefix_address, "
              "reserved_ipv6_prefix_length, "
              "reserved_ipv6_prefix_address_type, "
              "iaid, "
              "option_universe, "
              "option_code, "
              "option_value, "
              "option_formatted_value, "
              "option_space, "
              "option_is_persistent, "
              "option_client_class, "
              "option_subnet_id, "
              "option_user_context, "
              "option_scope_id "
              "FROM hosts "
              "WHERE host_ipv6_subnet_id = ? "
              "AND id = ? "
              "LIMIT 1 "
              "ALLOW FILTERING"
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {DELETE_HOST(),
             {DELETE_HOST(), "DELETE FROM hosts WHERE key = ? AND id = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
                             "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
                             "IF EXISTS "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }}};

        return _;
    }

    /// @brief Statement tags
    /// @{
    // Inserts all parameters belonging to any reservation from a single host.
    static StatementTag &INSERT_HOST() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves hosts informations, IPv6 reservations and both IPv4 and IPv6
    // options associated with the hosts.
    static StatementTag &GET_HOST() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information, IPv6 reservations and both IPv4 and IPv6
    // options associated with the host.
    static StatementTag &GET_HOST_BY_HOST_ID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information along with the IPv4 options associated
    // with it.
    static StatementTag &GET_HOST_BY_IPV4_ADDRESS() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information and IPv4 options using subnet identifier
    // and client's identifier (i.e. hardware address or DUID).
    static StatementTag &GET_HOST_BY_IPV4_SUBNET_ID_AND_HOST_ID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information; IPv6 reservations and IPv6 options
    // associated with it using subnet identifier and client's
    // identifier (i.e. hardware address or DUID).
    static StatementTag &GET_HOST_BY_IPV6_SUBNET_ID_AND_HOST_ID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information and IPv4 options for the host using subnet
    // identifier and IPv4 reservation.
    static StatementTag &GET_HOST_BY_IPV4_SUBNET_ID_AND_ADDRESS() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information, IPv6 reservations and IPv6 options
    // associated with a host using prefix and prefix length. This query
    // returns host information for a single host. However, multiple rows
    // are returned due to left joining IPv6 reservations and IPv6 options.
    // The number of rows returned is multiplication of number of existing
    // IPv6 reservations and IPv6 options.
    static StatementTag &GET_HOST_BY_IPV6_PREFIX() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information and IPv6 options for the host using subnet
    // identifier and IPv6 reservation.
    static StatementTag &GET_HOST_BY_IPV6_SUBNET_ID_AND_ADDRESS() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Deletes a host reservation.
    static StatementTag &DELETE_HOST() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information along with the IPv4 options associated
    // with it using a subnet identifier.
    static StatementTag &GET_HOST_BY_IPV4_SUBNET_ID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information; IPv6 reservations and IPv6 options
    // associated with a host using subnet identifier.
    static StatementTag &GET_HOST_BY_IPV6_SUBNET_ID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information, IPv6 reservations and both IPv4 and IPv6
    // options associated with it using hostname.
    static StatementTag &GET_HOST_BY_HOST_NAME() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information along with the IPv4 options associated
    // with it using hostname and subnet identifier.
    static StatementTag &GET_HOST_BY_HOST_NAME_AND_IPV4_SUBNET_ID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information; IPv6 reservations and IPv6 options
    // associated with it using hostname and subnet identifier.
    static StatementTag &GET_HOST_BY_HOST_NAME_AND_IPV6_SUBNET_ID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information along with the IPv4 options associated
    // with it using a subnet identifier from first host (paging).
    static StatementTag &GET_HOST_BY_IPV4_SUBNET_ID_LIMIT() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information along with the IPv4 options associated
    // with it using a subnet identifier from host (paging).
    static StatementTag &GET_HOST_BY_IPV4_SUBNET_ID_KEY() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information along with the IPv4 options associated
    // with it using a subnet identifier from next host (paging).
    static StatementTag &GET_HOST_BY_IPV4_SUBNET_ID_NEXT_KEY() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information along with the IPv4 options associated
    // with it using a subnet identifier from host with a limit (paging).
    static StatementTag &GET_HOST_BY_IPV4_SUBNET_ID_PAGE() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information; IPv6 reservations and IPv6 options
    // associated with it using subnet identifier from first host (paging).
    static StatementTag &GET_HOST_BY_IPV6_SUBNET_ID_LIMIT() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information; IPv6 reservations and IPv6 options
    // associated with it using subnet identifier from host (paging).
    static StatementTag &GET_HOST_BY_IPV6_SUBNET_ID_KEY() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information; IPv6 reservations and IPv6 options
    // associated with it using subnet identifier from next host (paging).
    static StatementTag &GET_HOST_BY_IPV6_SUBNET_ID_NEXT_KEY() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    // Retrieves host information; IPv6 reservations and IPv6 options
    // associated with it using subnet identifier from host with a limit
    // (paging).
    static StatementTag &GET_HOST_BY_IPV6_SUBNET_ID_PAGE() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    /// @}

private:
    /// Pointer to Host object holding information being inserted into database.
    HostPtr host_;

    /// @brief Primary key. Partition key. Aggregates: host_dentifier,
    /// host_identifier_type, host_ipv4_subnet_id host_ipv6_subnet_id,
    /// host_ipv4_address
    cass_int64_t key_;

    /// @brief Primary key. Clustering key. Aggregates: host_identifier,
    /// host_identifier_type, reserved_ipv6_prefix_address,
    /// reserved_ipv6_prefix_length, option_code, option_space.
    cass_int64_t id_;

    /// @brief Client's identifier (e.g. DUID, HW address) in binary format
    CassBlob host_identifier_;

    /// @brief Type of the identifier in the host_identifier_
    /// This value corresponds to the @ref Host::IdentifierType value.
    cass_int32_t host_identifier_type_;

    /// @brief IPv4 subnet identifier
    cass_int32_t host_ipv4_subnet_id_;

    /// @brief IPv6 subnet identifier
    cass_int32_t host_ipv6_subnet_id_;

    /// @brief Reserved IPv4 address
    cass_int32_t host_ipv4_address_;

    /// @brief Next server address (siaddr).
    cass_int32_t host_ipv4_next_server_;

    /// @brief Server hostname (sname).
    std::string host_ipv4_server_hostname_;

    /// @brief Boot file name (file).
    std::string host_ipv4_boot_file_name_;

    /// @brief Key for authentication
    std::string auth_key_;

    /// @brief Name reserved for the host
    std::string hostname_;

    /// @brief Lower case name reserved for the host
    std::string lower_case_hostname_;

    /// @brief User context
    std::string user_context_;

    /// @brief A string holding comma separated list of IPv4 client classes
    std::string host_ipv4_client_classes_;

    /// @brief A string holding comma separated list of IPv6 client classes
    std::string host_ipv6_client_classes_;

    /// @brief Address belonging to the reserved IPv6 prefix
    std::string reserved_ipv6_prefix_address_;

    /// @brief Length of the reserved IPv6 prefix
    cass_int32_t reserved_ipv6_prefix_length_;

    /// @brief Reserver IPv6 prefix type
    /// This value corresponds to the @ref Host::IdentifierType value.
    cass_int32_t reserved_ipv6_prefix_address_type_;

    /// @brief The reservation's IAID
    cass_int32_t iaid_;

    /// @brief Version of DHCP (i.e. 0 for DHCPv4 and 1 for DHCPv6) to which the
    ///     option belongs.
    cass_int32_t option_universe_;

    /// @brief Option code
    cass_int32_t option_code_;

    /// @brief Option value
    CassBlob option_value_;

    /// @brief The textual value of an option
    std::string option_formatted_value_;

    /// @brief Option space name
    std::string option_space_;

    /// @brief Flag indicating if option is always sent or only on request
    cass_bool_t option_is_persistent_;

    /// @brief Option client class
    std::string option_client_class_;

    /// @brief Subnet identifier
    cass_int32_t option_subnet_id_;

    /// @brief Buffer holding textual user context of an option.
    std::string option_user_context_;

    /// @brief Option scope id
    cass_int32_t option_scope_id_;
};  // CqlHostExchange

CqlHostExchange::CqlHostExchange()
    : host_(NULL), id_(0), host_identifier_type_(0), host_ipv4_subnet_id_(0),
      host_ipv6_subnet_id_(0), host_ipv4_address_(0), host_ipv4_next_server_(0),
      host_ipv4_server_hostname_(NULL_DHCP4_SERVER_HOSTNAME),
      host_ipv4_boot_file_name_(NULL_DHCP4_BOOT_FILE_NAME), auth_key_(""),
      user_context_(NULL_USER_CONTEXT),
      reserved_ipv6_prefix_length_(NULL_RESERVED_IPV6_PREFIX_LENGTH),
      reserved_ipv6_prefix_address_type_(NULL_RESERVED_IPV6_PREFIX_ADDRESS_TYPE), iaid_(NULL_IAID),
      option_universe_(NULL_OPTION_UNIVERSE), option_code_(NULL_OPTION_CODE),
      option_is_persistent_(NULL_OPTION_IS_PERSISTENT), option_subnet_id_(NULL_OPTION_SUBNET_ID),
      option_user_context_(NULL_OPTION_USER_CONTEXT), option_scope_id_(NULL_OPTION_SCOPE_ID) {
}

void CqlHostExchange::createBindForSelect(AnyArray &data, StatementTag const & /* not used */) {
    // Start with a fresh array.
    data.clear();

    // key: bigint
    data.add(&key_);
    // id: bigint
    data.add(&id_);
    // host_identifier: blob
    data.add(&host_identifier_);
    // host_identifier_type: int
    data.add(&host_identifier_type_);
    // host_ipv4_subnet_id: int
    data.add(&host_ipv4_subnet_id_);
    // host_ipv6_subnet_id: int
    data.add(&host_ipv6_subnet_id_);
    // host_ipv4_address: int
    data.add(&host_ipv4_address_);
    // host_ipv4_next_server: int
    data.add(&host_ipv4_next_server_);
    // host_ipv4_server_hostname: text
    data.add(&host_ipv4_server_hostname_);
    // host_ipv4_boot_file_name: text
    data.add(&host_ipv4_boot_file_name_);
    // auth_key: text
    data.add(&auth_key_);
    // hostname: text
    data.add(&hostname_);
    // user_context: text
    data.add(&user_context_);
    // host_ipv4_client_classes: text
    data.add(&host_ipv4_client_classes_);
    // host_ipv6_client_classes: text
    data.add(&host_ipv6_client_classes_);
    /// @brief Denormalized reservation columns
    /// @{
    // reserved_ipv6_prefix_address: text
    data.add(&reserved_ipv6_prefix_address_);
    // reserved_ipv6_prefix_length: int
    data.add(&reserved_ipv6_prefix_length_);
    // reserved_ipv6_prefix_address_type: int
    data.add(&reserved_ipv6_prefix_address_type_);
    // iaid: int
    data.add(&iaid_);
    /// @}
    /// @brief Denormalized option columns
    /// @{
    // option_universe: int
    data.add(&option_universe_);
    // option_code: int
    data.add(&option_code_);
    // option_value: blob
    data.add(&option_value_);
    // option_formatted_value: text
    data.add(&option_formatted_value_);
    // option_space: text
    data.add(&option_space_);
    // option_is_persistent: boolean
    data.add(&option_is_persistent_);
    // option_client_class: text
    data.add(&option_client_class_);
    // option_subnet_id: int
    data.add(&option_subnet_id_);
    // option_user_context: text
    data.add(&option_user_context_);
    // option_scope_id: int
    data.add(&option_scope_id_);
    /// @}
}

void CqlHostExchange::prepareExchange(const HostPtr &host,
                                      const Optional<SubnetID> &subnet_id,
                                      const IPv6Resrv *const reservation,
                                      const std::string &option_space,
                                      const OptionDescriptor &option_descriptor) {

    // Store host object to ensure it remains valid.
    host_ = host;

    // Set up the structures for the various components of the host
    // structure.
    try {
        // host_identifier: blob
        // Convert from std::vector<uint8_t> to
        // std::vector<cass_byte_t>.
        HostIdentifier host_identifier = host->getIdentifier();
        host_identifier_ = CassBlob(host_identifier.begin(), host_identifier.end());
        if (host_identifier_.size() > DUID::MAX_DUID_LEN) {
            isc_throw(BadValue, "CqlHostExchange::prepareExchange(): host identifier "
                                    << host_identifier_.data() << " of length "
                                    << host_identifier_.size() << " is greater than allowed of "
                                    << DUID::MAX_DUID_LEN);
        }

        // host_identifier_type: tinyint
        host_identifier_type_ = static_cast<cass_int32_t>(host->getIdentifierType());
        if (host_identifier_type_ > MAX_IDENTIFIER_TYPE) {
            isc_throw(BadValue, "CqlHostExchange::prepareExchange(): invalid "
                                "host identifier type returned: "
                                    << host_identifier_type_);
        }

        // host_ipv4_subnet_id: int
        host_ipv4_subnet_id_ = static_cast<cass_int32_t>(host->getIPv4SubnetID());

        // host_ipv6_subnet_id: int
        host_ipv6_subnet_id_ = static_cast<cass_int32_t>(host->getIPv6SubnetID());

        // host_ipv4_address: int
        host_ipv4_address_ = static_cast<cass_int32_t>(host->getIPv4Reservation().toUint32());

        // host_ipv4_next_server: int
        host_ipv4_next_server_ = static_cast<cass_int32_t>(host->getNextServer().toUint32());

        // host_ipv4_server_hostname: text
        host_ipv4_server_hostname_ = host->getServerHostname();

        // host_ipv4_boot_file_name: text
        host_ipv4_boot_file_name_ = host->getBootFileName();

        // auth_key: varchar
        auth_key_ = host->getKey().toText();

        // hostname: text
        hostname_ = host->getHostname();
        if (hostname_.size() > HOSTNAME_MAX_LEN) {
            isc_throw(BadValue, "CqlHostExchange::prepareExchange(): hostname "
                                    << hostname_ << " of length " << hostname_.size()
                                    << " is greater than allowed of " << HOSTNAME_MAX_LEN);
        }

        // lower_case_hostname: text
        lower_case_hostname_ = host->getLowerHostname();
        if (lower_case_hostname_.size() > HOSTNAME_MAX_LEN) {
            // Should never happen...
            isc_throw(BadValue, "CqlHostExchange::prepareExchange(): lower "
                                "case hostname "
                                    << lower_case_hostname_ << " of length "
                                    << lower_case_hostname_.size() << " is greater than allowed of "
                                    << HOSTNAME_MAX_LEN);
        }

        // user_context: text
        ElementPtr ctx = host->getContext();
        if (ctx) {
            user_context_ = ctx->str();
        } else {
            user_context_ = NULL_USER_CONTEXT;
        }

        // host_ipv4_client_classes: text
        host_ipv4_client_classes_ = host->getClientClasses4().toText(",");
        if (host_ipv4_client_classes_.size() > CLIENT_CLASSES_MAX_LEN) {
            isc_throw(BadValue, "CqlHostExchange::prepareExchange(): "
                                "IPv4 client classes "
                                    << host_ipv4_client_classes_ << " of length "
                                    << host_ipv4_client_classes_.size()
                                    << " is greater than allowed of " << CLIENT_CLASSES_MAX_LEN);
        }

        // host_ipv6_client_classes: text
        host_ipv6_client_classes_ = host->getClientClasses6().toText(",");
        if (host_ipv6_client_classes_.size() > CLIENT_CLASSES_MAX_LEN) {
            isc_throw(BadValue, "CqlHostExchange::prepareExchange(): "
                                "IPv6 client classes "
                                    << host_ipv6_client_classes_ << " of length "
                                    << host_ipv6_client_classes_.size()
                                    << " is greater than allowed of " << CLIENT_CLASSES_MAX_LEN);
        }

        if (reservation == NULL) {
            // reserved_ipv6_prefix_address: text
            reserved_ipv6_prefix_address_ = NULL_RESERVED_IPV6_PREFIX_ADDRESS;
            // reserved_ipv6_prefix_length: int
            reserved_ipv6_prefix_length_ = NULL_RESERVED_IPV6_PREFIX_LENGTH;
            // reserved_ipv6_prefix_address_type: int
            reserved_ipv6_prefix_address_type_ = NULL_RESERVED_IPV6_PREFIX_ADDRESS_TYPE;
            iaid_ = NULL_IAID;
        } else {
            // reserved_ipv6_prefix_address: text
            reserved_ipv6_prefix_address_ = reservation->getPrefix().toText();

            // reserved_ipv6_prefix_length: int
            reserved_ipv6_prefix_length_ = static_cast<cass_int32_t>(reservation->getPrefixLen());

            // reserved_ipv6_prefix_address_type: int
            reserved_ipv6_prefix_address_type_ =
                reservation->getType() == IPv6Resrv::TYPE_NA ? 0 : 2;

            // iaid: int
            /// @todo: We don't support iaid in the IPv6Resrv yet.
            iaid_ = 0;
        }

        if (option_descriptor.option_ == NULL) {
            option_universe_ = NULL_OPTION_UNIVERSE;
            option_code_ = NULL_OPTION_CODE;
            option_value_ = NULL_OPTION_VALUE;
            option_formatted_value_ = NULL_OPTION_FORMATTED_VALUE;
            option_space_ = NULL_OPTION_SPACE;
            option_is_persistent_ = NULL_OPTION_IS_PERSISTENT;
            option_client_class_ = NULL_OPTION_CLIENT_CLASS;
            option_subnet_id_ = NULL_OPTION_SUBNET_ID;
            option_user_context_ = NULL_OPTION_USER_CONTEXT;
            option_scope_id_ = NULL_OPTION_SCOPE_ID;
        } else {
            // option_universe: int
            option_universe_ = option_descriptor.option_->getUniverse();

            // option_code: int
            option_code_ = option_descriptor.option_->getType();

            // option_value: blob
            // option_formatted_value: text
            if (option_descriptor.formatted_value_.empty()) {
                if (option_descriptor.option_->len() > option_descriptor.option_->getHeaderLen()) {
                    // The formatted_value is empty and the option value
                    // is not empty so we need to prepare on-wire format
                    // for the option and store it in the database as a
                    // blob.
                    OutputBuffer buffer(option_descriptor.option_->len());
                    option_descriptor.option_->pack(buffer);
                    const char *buffer_ptr = static_cast<const char *>(buffer.getData());
                    option_value_.assign(buffer_ptr + option_descriptor.option_->getHeaderLen(),
                                         buffer_ptr + buffer.getLength());
                } else {
                    option_value_.clear();
                }
                option_formatted_value_.clear();
            } else {
                option_value_.clear();
                option_formatted_value_ = option_descriptor.formatted_value_;
            }

            // option_space: text
            option_space_ = option_space;

            // option_is_persistent: boolean
            option_is_persistent_ = option_descriptor.persistent_ ? cass_true : cass_false;

            // option_client_class: text
            /// @todo Assign actual value to client class string. See #5503.
            option_client_class_.clear();

            // option_subnet_id: int
            if (!subnet_id.unspecified()) {
                option_subnet_id_ = subnet_id;
            } else {
                option_subnet_id_ = 0;
            }

            // option_user_context: text
            ElementPtr ctx = option_descriptor.getContext();
            if (ctx) {
                option_user_context_ = ctx->str();
            } else {
                option_user_context_ = NULL_OPTION_USER_CONTEXT;
            }

            // option_scope_id: int
            // Using fixed scope_id = 3, which associates an option with host.
            option_scope_id_ = 3;
        }

        // id: bigint
        id_ = static_cast<cass_int64_t>(hashIntoId());

        // key: bigint
        key_ = static_cast<cass_int64_t>(hashIntoKey());
    } catch (const Exception &ex) {
        isc_throw(DbOperationError, "CqlHostExchange::prepareExchange(): "
                                    "could not copy data from host "
                                        << host->getHostname() << ", reason: " << ex.what());
    }
}

void CqlHostExchange::createBindForMutation(const HostPtr &host,
                                            const Optional<SubnetID> &subnet_id,
                                            const IPv6Resrv *const reservation,
                                            const std::string &option_space,
                                            const OptionDescriptor &option_descriptor,
                                            StatementTag statement_tag,
                                            AnyArray &data) {
    prepareExchange(host, subnet_id, reservation, option_space, option_descriptor);

    try {
        // Add all parameters to bind array.
        data.clear();

        if (statement_tag == CqlHostExchange::INSERT_HOST()) {
            data.add(&key_);
            data.add(&id_);
            data.add(&host_identifier_);
            data.add(&host_identifier_type_);
            data.add(&host_ipv4_subnet_id_);
            data.add(&host_ipv6_subnet_id_);
            data.add(&host_ipv4_address_);
            data.add(&host_ipv4_next_server_);
            data.add(&host_ipv4_server_hostname_);
            data.add(&host_ipv4_boot_file_name_);
            data.add(&auth_key_);
            data.add(&hostname_);
            data.add(&lower_case_hostname_);
            data.add(&user_context_);
            data.add(&host_ipv4_client_classes_);
            data.add(&host_ipv6_client_classes_);
        }

        // Reservation
        data.add(&reserved_ipv6_prefix_address_);
        data.add(&reserved_ipv6_prefix_length_);
        data.add(&reserved_ipv6_prefix_address_type_);
        data.add(&iaid_);

        // Option
        data.add(&option_universe_);
        data.add(&option_code_);
        data.add(&option_value_);
        data.add(&option_formatted_value_);
        data.add(&option_space_);
        data.add(&option_is_persistent_);
        data.add(&option_client_class_);
        data.add(&option_subnet_id_);
        data.add(&option_user_context_);
        data.add(&option_scope_id_);

    } catch (const Exception &ex) {
        isc_throw(DbOperationError, "CqlHostExchange::createBindForMutation(): "
                                    "could not create bind array from host "
                                        << host->getHostname() << ", reason: " << ex.what());
    }
}

void CqlHostExchange::createBindForDelete(const HostPtr &host,
                                          const Optional<SubnetID> &subnet_id,
                                          const IPv6Resrv *const reservation,
                                          const std::string &option_space,
                                          const OptionDescriptor &option_descriptor,
                                          StatementTag statement_tag,
                                          AnyArray &data) {
    prepareExchange(host, subnet_id, reservation, option_space, option_descriptor);

    try {
        // Add all parameters to bind array.
        data.clear();

        if (statement_tag == CqlHostExchange::DELETE_HOST()) {
            data.add(&key_);
            data.add(&id_);
        }

    } catch (const Exception &ex) {
        isc_throw(DbOperationError, "CqlHostExchange::createBindForDelete(): "
                                    "could not create bind array from host "
                                        << host->getHostname() << ", reason: " << ex.what());
    }
}

uint64_t CqlHostExchange::hashIntoId() const {
    // Add a separator between aggregated field to avoid collisions
    // between distinct entries.

    // Get key.
    std::stringstream key_stream;
    key_stream << hostKey();
    key_stream << std::setw(V6ADDRESS_TEXT_MAX_LEN) << std::setfill('-')
               << reserved_ipv6_prefix_address_;
    key_stream << std::setw(4) << std::setfill('-') << reserved_ipv6_prefix_length_;
    key_stream << std::setw(4) << std::setfill('-') << option_code_;
    key_stream << std::setw(OPTION_SPACE_MAX_LEN) << std::setfill('-') << option_space_;
    const std::string key = key_stream.str();

    return Hash64::hash(key);
}

uint64_t CqlHostExchange::hashIntoKey() const {
    const std::string key = hostKey();

    return Hash64::hash(key);
}

std::string CqlHostExchange::hostKey() const {
    // Add a separator between aggregated field to avoid collisions
    // between distinct entries.
    // Get key.
    std::stringstream key_stream;
    if (host_ipv4_address_) {
        key_stream << std::setw(3 * DUID::MAX_DUID_LEN - 1) << std::setfill('-') << "-";
        key_stream << std::setw(10) << std::setfill('-') << "-";
    } else {
        key_stream << std::setw(3 * DUID::MAX_DUID_LEN - 1) << std::setfill('-')
                   << DUID(host_identifier_).toText();
        key_stream << std::setw(10) << std::setfill('-') << host_identifier_type_;
    }
    key_stream << std::setw(10) << std::setfill('-') << host_ipv4_subnet_id_;
    key_stream << std::setw(10) << std::setfill('-') << host_ipv6_subnet_id_;
    key_stream << std::setw(V4ADDRESS_TEXT_MAX_LEN) << std::setfill('-') << host_ipv4_address_;
    return key_stream.str();
}

HostPtr CqlHostExchange::retrieve() {
    const uint64_t id = static_cast<uint64_t>(id_);

    HostIdentifier host_identifier =
        HostIdentifier(host_identifier_.begin(), host_identifier_.end());

    // Set the host identifier type in a variable of the appropriate
    // data type.
    Host::IdentifierType host_identifier_type =
        static_cast<Host::IdentifierType>(host_identifier_type_);

    // Set IPv4 subnet ID to the value returned.
    SubnetID ipv4_subnet_id = static_cast<SubnetID>(host_ipv4_subnet_id_);

    // Set IPv6 subnet ID to the value returned.
    SubnetID ipv6_subnet_id = static_cast<SubnetID>(host_ipv6_subnet_id_);

    // Set IPv4 address reservation.
    asiolink::IOAddress ipv4_reservation =
        asiolink::IOAddress(static_cast<uint32_t>(host_ipv4_address_));

    HostPtr host(std::make_shared<Host>(
        host_identifier.data(), host_identifier.size(), host_identifier_type, ipv4_subnet_id,
        ipv6_subnet_id, ipv4_reservation, hostname_, host_ipv4_client_classes_,
        host_ipv6_client_classes_, static_cast<uint32_t>(host_ipv4_next_server_),
        host_ipv4_server_hostname_, host_ipv4_boot_file_name_, AuthKey(auth_key_)));

    // Set the user context if there is one.
    if (!user_context_.empty()) {
        try {
            ElementPtr ctx = Element::fromJSON(user_context_);
            if (!ctx || (ctx->getType() != Element::map)) {
                isc_throw(BadValue, "user context '" << user_context_ << "' is not a JSON map");
            }
            host->setContext(ctx);
        } catch (const isc::data::JSONError &ex) {
            isc_throw(BadValue,
                      "user context '" << user_context_ << "' is invalid JSON: " << ex.what());
        }
    }

    host->setHostId(id);

    const IPv6Resrv reservation = retrieveReservation();
    if (reservation != NULL_IPV6_RESERVATION && !host->hasReservation(reservation)) {
        host->addReservation(reservation);
    }

    OptionWrapper option_wrapper = retrieveOption();
    if (option_wrapper.option_descriptor_) {
        if (option_wrapper.option_descriptor_->option_->getUniverse() == Option::V4) {
            host->getCfgOption4()->add(*option_wrapper.option_descriptor_,
                                       option_wrapper.option_space_);
        } else if (option_wrapper.option_descriptor_->option_->getUniverse() == Option::V6) {
            host->getCfgOption6()->add(*option_wrapper.option_descriptor_,
                                       option_wrapper.option_space_);
        }
    }

    return (host);
}

const IPv6Resrv CqlHostExchange::retrieveReservation() const {
    // Set the IPv6 Reservation type (0 = IA_NA, 2 = IA_PD).
    IPv6Resrv::Type type;
    switch (reserved_ipv6_prefix_address_type_) {
    case 0:
        type = IPv6Resrv::TYPE_NA;
        break;
    case 2:
        type = IPv6Resrv::TYPE_PD;
        break;
    case NULL_RESERVED_IPV6_PREFIX_ADDRESS_TYPE:
        return (NULL_IPV6_RESERVATION);
    default:
        isc_throw(BadValue, "CqlHostExchange::retrieveReservation(): invalid IPv6 "
                            "reservation type returned: "
                                << reserved_ipv6_prefix_address_type_
                                << ". Only 0 (IA_NA) or 2 (IA_PD) are allowed.");
    }

    return IPv6Resrv(type, IOAddress(reserved_ipv6_prefix_address_), reserved_ipv6_prefix_length_);
}

const OptionWrapper CqlHostExchange::retrieveOption() const {
    // Options are held in a binary or textual format in the database.
    // This is similar to having an option specified in a server
    // configuration file. Such option is converted to appropriate C++
    // class, using option definition. Thus, we need to find the
    // option definition for this option code and option space.

    // If the option space is a standard DHCPv4 or DHCPv6 option space,
    // this is most likely a standard option, for which we have a
    // definition created within libdhcp++.
    if (option_space_.empty() || option_universe_ == NULL_OPTION_UNIVERSE) {
        return (OptionWrapper(OptionDescriptorPtr(), ""));
    }

    OptionDefinitionPtr option_definition_ptr = LibDHCP::getOptionDef(option_space_, option_code_);

    // Otherwise, we may check if this an option encapsulated within the
    // vendor space.
    if (!option_definition_ptr && option_space_ != DHCP4_OPTION_SPACE &&
        option_space_ != DHCP6_OPTION_SPACE) {
        uint32_t vendor_id = LibDHCP::optionSpaceToVendorId(option_space_);
        if (vendor_id > 0) {
            option_definition_ptr = LibDHCP::getVendorOptionDef(
                static_cast<Option::Universe>(option_universe_), vendor_id, option_code_);
        }
    }

    // In all other cases, we use runtime option definitions, which
    // should be also registered within the libdhcp++.
    if (!option_definition_ptr) {
        option_definition_ptr = LibDHCP::getRuntimeOptionDef(option_space_, option_code_);
    }

    OptionPtr option;
    if (!option_definition_ptr) {
        // If no definition found, we use generic option type.
        OptionBuffer option_buffer(option_value_.begin(), option_value_.end());
        option = std::make_shared<Option>(static_cast<Option::Universe>(option_universe_),
                                          static_cast<uint16_t>(option_code_),
                                          option_buffer.begin(), option_buffer.end());
    } else {
        // The option value may be specified in textual or binary format
        // in the
        // database. If formatted_value is empty, the binary format is
        // used.
        // Depending on the format we use a different variant of @ref
        // optionFactory().
        if (option_formatted_value_.empty()) {
            OptionBuffer option_buffer(option_value_.begin(), option_value_.end());
            option = option_definition_ptr->optionFactory(
                static_cast<Option::Universe>(option_universe_),
                static_cast<uint16_t>(option_code_), option_buffer.begin(), option_buffer.end());
        } else {
            // Spit the value specified in comma separated values
            // format.
            std::vector<std::string> split_vector;
            boost::split(split_vector, option_formatted_value_, boost::is_any_of(","));
            option = option_definition_ptr->optionFactory(
                static_cast<Option::Universe>(option_universe_),
                static_cast<uint16_t>(option_code_), split_vector);
        }
    }

    OptionWrapper result(
        std::make_shared<OptionDescriptor>(option, option_is_persistent_, option_formatted_value_),
        option_space_);
    // Set the user context if there is one into the option descriptor.
    if (!option_user_context_.empty()) {
        try {
            ElementPtr ctx = Element::fromJSON(option_user_context_);
            if (!ctx || (ctx->getType() != Element::map)) {
                isc_throw(BadValue,
                          "option user context '" << option_user_context_ << "' is no a JSON map");
            }
            result.option_descriptor_->setContext(ctx);
        } catch (const isc::data::JSONError &ex) {
            isc_throw(BadValue, "option user context '" << option_user_context_
                                                        << "' is invalid JSON: " << ex.what());
        }
    }

    return result;
}

/// @brief Implementation of the @ref CqlHostDataSource.
///
/// This class is encapsulate all the Cassandra communication details.
struct CqlHostDataSourceImpl {
    /// @brief Constructor.
    ///
    /// This constructor opens database connection and initializes
    /// prepared statements used in the queries.
    /// @param parameters parameters passed to the CQL connection.
    explicit CqlHostDataSourceImpl(CqlConnection::ParameterMap const &parameters);

    /// @brief Destructor.
    ///
    /// There is no need to close the database in this destructor: it is
    /// closed in the destructor of the connection_ member variable.
    virtual ~CqlHostDataSourceImpl() = default;

    /// @brief Implementation of @ref CqlHostDataSource::add() and del()
    ///
    /// See @ref CqlHostDataSource::add() for parameter details.
    ///
    /// @param host host to be added or deleted
    /// @param insert insert (true) or delete (false) the host
    bool insertOrDelete(const HostPtr &host, bool insert);

    /// @brief Implementation of @ref CqlHostDataSource::get4()
    ///
    /// See @ref CqlHostDataSource::get4() for parameter details.
    ///
    /// @param subnet_id Id of the subnet to look into
    /// @param address IPv4 address to be retrieved
    ConstHostPtr get4(const SubnetID &subnet_id, const asiolink::IOAddress &address) const;

    /// @brief Implementation of @ref CqlHostDataSource::get4()
    ///
    /// See @ref CqlHostDataSource::get4() for parameter details.
    ///
    /// @param subnet_id Id of the subnet to look into
    /// @param identifier_type type of the identifier
    /// @param identifier_begin pointer to the first byte of the identifier
    /// @param identifier_len length of the identifier
    ConstHostPtr get4(const SubnetID &subnet_id,
                      const Host::IdentifierType &identifier_type,
                      const uint8_t *identifier_begin,
                      const size_t identifier_len) const;

    /// @brief Retrieves a host by its reserved IPv6 address or prefix
    ///
    /// See @ref CqlHostDataSource::get6() for parameter details.
    ///
    /// @param prefix IPv6 address or prefix
    /// @param prefix_len length of the prefix (or 128 for address)
    ConstHostPtr get6(const asiolink::IOAddress &prefix, const uint8_t prefix_len) const;

    /// @brief Implementation of @ref CqlHostDataSource::get6()
    ///
    /// See @ref CqlHostDataSource::get6() for parameter details.
    ///
    /// @param subnet_id Id of the subnet to look into
    /// @param identifier_type type of the identifier (duid, hwaddr, flex, etc.)
    /// @param identifier_begin pointer to the first byte of the identifier
    /// @param identifier_len length of the identifier
    ConstHostPtr get6(const SubnetID &subnet_id,
                      const Host::IdentifierType &identifier_type,
                      const uint8_t *identifier_begin,
                      const size_t identifier_len) const;

    /// @brief Implementation of @ref CqlHostDataSource::get6()
    ///
    /// See @ref CqlHostDataSource::get6() for parameter details.
    ///
    /// @param subnet_id Id of the subnet to look into
    /// @param address IPv6 address to be retrieved
    ConstHostPtr get6(const SubnetID &subnet_id, const asiolink::IOAddress &address) const;

    /// @brief Implementation of @ref CqlHostDataSource::getAll()
    ///
    /// See @ref CqlHostDataSource::getAll() for parameter details.
    ///
    /// @param identifier_type type of the identifier (duid, hwaddr, flex, etc.)
    /// @param identifier_begin pointer to the first byte of the identifier
    /// @param identifier_len length of the identifier
    ConstHostCollection getAll(const Host::IdentifierType &identifier_type,
                               const uint8_t *identifier_begin,
                               const size_t identifier_len) const;

    /// @brief Implementation of @ref CqlHostDataSource::getAll4()
    ///
    /// See @ref CqlHostDataSource::getAll4() for parameter details.
    ///
    /// @param subnet_id identifier of the subnet to which hosts belong
    virtual ConstHostCollection getAll4(const SubnetID &subnet_id) const;

    /// @brief Implementation of @ref CqlHostDataSource::getAll6()
    ///
    /// See @ref CqlHostDataSource::getAll6() for parameter details.
    ///
    /// @param subnet_id identifier of the subnet to which hosts belong
    virtual ConstHostCollection getAll6(const SubnetID &subnet_id) const;

    /// @brief Implementation of @ref CqlHostDataSource::getAllbyHostname()
    ///
    /// See @ref CqlHostDataSource::getAllbyHostname() for parameter details.
    ///
    /// @param hostname The lower case hostname.
    virtual ConstHostCollection getAllbyHostname(const std::string &hostname) const;

    /// @brief Implementation of @ref CqlHostDataSource::getAllbyHostname4()
    ///
    /// See @ref CqlHostDataSource::getAllbyHostname4() for parameter details.
    ///
    /// @param hostname The lower case hostname.
    /// @param subnet_id Subnet identifier.
    virtual ConstHostCollection getAllbyHostname4(const std::string &hostname,
                                                  const SubnetID &subnet_id) const;

    /// @brief Implementation of @ref CqlHostDataSource::getAllbyHostname6()
    ///
    /// See @ref CqlHostDataSource::getAllbyHostname6() for parameter details.
    ///
    /// @param hostname The lower case hostname.
    /// @param subnet_id Subnet identifier.
    virtual ConstHostCollection getAllbyHostname6(const std::string &hostname,
                                                  const SubnetID &subnet_id) const;

    /// @brief Implementation of @ref CqlHostDataSource::getPage4()
    ///
    /// See @ref CqlHostDataSource::getPage4() for parameter details.
    ///
    /// @param subnet_id identifier of the subnet to which hosts belong
    /// @param lower_host_id Host identifier used as lower bound for the
    /// returned range.
    /// @param page_size maximum size of the page returned.
    virtual ConstHostCollection getPage4(const SubnetID &subnet_id,
                                         uint64_t lower_host_id,
                                         const HostPageSize &page_size) const;

    /// @brief Implementation of @ref CqlHostDataSource::getPage6()
    ///
    /// See @ref CqlHostDataSource::getPage6() for parameter details.
    ///
    /// @param subnet_id identifier of the subnet to which hosts belong
    /// @param lower_host_id Host identifier used as lower bound for the
    /// returned range.
    /// @param page_size maximum size of the page returned.
    virtual ConstHostCollection getPage6(const SubnetID &subnet_id,
                                         uint64_t lower_host_id,
                                         const HostPageSize &page_size) const;

    /// @brief Implementation of @ref CqlHostDataSource::getAll4()
    ///
    /// See @ref CqlHostDataSource::getAll4() for parameter details.
    ///
    /// @param address IPv4 address of the reservation to be retrieved
    ConstHostCollection getAll4(const asiolink::IOAddress &address) const;

    /// @brief Implementation of @ref CqlHostDataSource::getAllHosts()
    ///
    /// See @ref CqlHostDataSource::getAllHosts() for parameter details.
    ConstHostCollection getAllHosts() const;

    /// @brief Implementation of @ref CqlHostDataSource::getName()
    std::string getName() const;

    /// @brief Implementation of @ref CqlHostDataSource::commit()
    void commit();

    /// @brief Implementation of @ref CqlHostDataSource::rollback()
    void rollback();

    /// @brief Implementation of @ref
    /// CqlHostDataSource::syncReservations()
    void syncReservations();

protected:
    /// @brief Adds/deletes any options found in the @ref Host object to/from a separate
    ///     table entry.
    ///
    /// @param insert insert or delete a host with options
    /// @param host @ref Host object from which options are retrieved and
    ///     inserted/deleted into/from the Cassandra database
    /// @param reservation reservation for the current denormalized table entry
    /// @param option_spaces list of option spaces to search for
    /// @param cfg_option option configuration used to match option spaces in
    ///     order to obtain actual options
    bool insertOrDeleteHostWithOptions(
        bool insert,
        const HostPtr &host,
        const IPv6Resrv *const reservation = NULL,
        const std::list<std::string> &option_spaces = std::list<std::string>(),
        const ConstCfgOptionPtr cfg_option = ConstCfgOptionPtr());

    /// @brief Adds/deletes any reservations found in the @ref Host object to/from a separate
    ///     table entry.
    ///
    /// @param insert insert or deletes a host with reservations
    /// @param host @ref Host object from which reservations are retrieved and
    ///     inserted/deleted into/from the Cassandra database
    /// @param reservation reservation for the current denormalized table entry
    /// @param option_spaces4 list of option spaces for universe Option::V4 to search in
    /// @param cfg_option4 option configuration for universe Option::V4 used to
    ///     match option spaces in order to obtain actual options
    /// @param option_spaces6 list of option spaces for universe Option::V6 to
    ///     search in
    /// @param cfg_option6 option configuration for universe Option::V6 used to
    ///     match option spaces in order to obtain actual options
    bool insertOrDeleteHostWithReservations(bool insert,
                                            const HostPtr &host,
                                            const IPv6Resrv *const reservation,
                                            const std::list<std::string> &option_spaces4,
                                            const ConstCfgOptionPtr cfg_option4,
                                            const std::list<std::string> &option_spaces6,
                                            const ConstCfgOptionPtr cfg_option6);

    /// @brief Retrieves a single host.
    ///
    /// Calls @ref getHostCollection() and checks if a single host was
    /// returned.
    ///
    /// @param where_values array of bound objects used to filter the results
    /// @param statement_tag prepared statement being executed
    ///
    /// @return one host or a null pointer to a host
    ///
    /// @throw MultipleRecords exception if two or more hosts are
    /// returned
    ConstHostPtr getHost(StatementTag statement_tag, AnyArray &where_values) const;

    /// @brief Retrieves a collection of hosts.
    ///
    /// Calls @ref isc::db::CqlExchange::executeSelect().
    ///
    /// @param where_values array of bound objects used to filter the results
    /// @param statement_tag prepared statement being executed
    ///
    /// @return a collection of hosts containing one or more hosts
    ConstHostCollection getHostCollection(StatementTag statement_tag, AnyArray &where_values) const;

    /// @brief Retrieves a page of hosts.
    ///
    /// @param subnet_id identifier of the subnet to which hosts belong
    /// @param lower_host_id Host identifier used as lower bound for the
    /// returned range.
    /// @param count the size of the page
    ///
    /// @return a collection of hosts containing one or more hosts
    virtual ConstHostCollection getHostCollectionPage4(const SubnetID &subnet_id,
                                                       uint64_t lower_host_id,
                                                       size_t count = 0) const;

    /// @brief Retrieves a page of hosts.
    ///
    /// @param subnet_id identifier of the subnet to which hosts belong
    /// @param lower_host_id Host identifier used as lower bound for the
    /// returned range.
    /// @param count the size of the page
    ///
    /// @return a collection of hosts containing one or more hosts
    virtual ConstHostCollection getHostCollectionPage6(const SubnetID &subnet_id,
                                                       uint64_t lower_host_id,
                                                       size_t count = 0) const;

    /// @brief Retrieves a host by key.
    ///
    /// @param key identifier of the host
    ///
    /// @return a host for the specific key
    virtual ConstHostPtr getHostByKey4(uint64_t key) const;

    /// @brief Retrieves a host by key.
    ///
    /// @param key identifier of the host
    ///
    /// @return a host for the specific key
    virtual ConstHostPtr getHostByKey6(uint64_t key) const;

    /// @brief Retrieves a valid host key.
    /// if lower_host_id is 0 the key parameter will be updated with the key of
    /// the first host
    /// if lower_host_id is not 0 the key parameter will be updated with the
    /// next valid host key
    ///
    /// @param subnet_id identifier of the subnet to which hosts belong
    /// @param lower_host_id Host identifier used as lower bound for the
    /// returned range.
    /// @param key identifier of the host which will be updated
    ///
    /// @return true if there is sunh a host
    virtual bool
    getHostKey4(const SubnetID &subnet_id, uint64_t lower_host_id, uint64_t &key) const;

    /// @brief Retrieves a valid host key.
    /// if lower_host_id is 0 the key parameter will be updated with the key of
    /// the first host
    /// if lower_host_id is not 0 the key parameter will be updated with the
    /// next valid host key
    ///
    /// @param subnet_id identifier of the subnet to which hosts belong
    /// @param lower_host_id Host identifier used as lower bound for the
    /// returned range.
    /// @param key identifier of the host which will be updated
    ///
    /// @return true if there is sunh a host
    virtual bool
    getHostKey6(const SubnetID &subnet_id, uint64_t lower_host_id, uint64_t &key) const;

    /// @brief Retrieves next valid host key.
    ///
    /// @param subnet_id identifier of the subnet to which hosts belong
    /// @param key identifier of the host which will be updated with the next
    /// valid host key
    ///
    /// @return true if there is such a host
    virtual bool getNextHostKey4(const SubnetID &subnet_id, uint64_t &key) const;

    /// @brief Retrieves next valid host key.
    ///
    /// @param subnet_id identifier of the subnet to which hosts belong
    /// @param key identifier of the host which will be updated with the next
    /// valid host key
    ///
    /// @return true if there is such a host
    virtual bool getNextHostKey6(const SubnetID &subnet_id, uint64_t &key) const;

    /// @brief Inserts or deletes a single host.
    ///
    /// All information is available here. Calls @ref
    /// isc::db::CqlExchange::executeMutation().
    ///
    /// @param insert insert or delete a host
    /// @param host @ref Host object from which options are retrieved and
    ///     inserted/deleted into/from the Cassandra database
    /// @param subnet_id identifier of the subnet to which the host belongs
    /// @param reservation reservation for the current denormalized table entry
    /// @param option_space option space for the current denormalized table
    ///     entry's option
    /// @param option_descriptor option descriptor containing
    ///     information for the current denormalized table entry's option
    bool insertOrDeleteHost(bool insert,
                            const HostPtr &host,
                            const Optional<SubnetID> &subnet_id = Optional<SubnetID>(),
                            const IPv6Resrv *const reservation = NULL,
                            const std::string &option_space = NULL_OPTION_SPACE,
                            const OptionDescriptor &option_descriptor = OptionDescriptor(false));

    /// @brief Merge denormalized table entries that belong to the same host
    /// into a single host, one by one.
    ///
    /// @param target_host host which can contain multiple reservations and
    ///     options to which other distinct reservations and options are
    ///     added.
    /// @param source_host host that is being search for new reservations and
    ///     options that will be merged into the old host.
    void mergeHosts(const ConstHostPtr &source_host, HostPtr &target_host) const;

private:
    /// @brief Parameters
    db::DatabaseConnection::ParameterMap parameters_;

    /// @brief CQL connection
    mutable CqlConnection connection_;
};  // struct CqlHostDataSourceImpl

/// @brief hash function for HostMap
///
/// Returns a 64-bits key value. The key is generated with FNV-1a 64 bit
/// algorithm.
///
/// @param key being hashed
///
/// @return hash value
std::size_t hash_value(const HostKey &key) {
    // Get key.
    std::stringstream key_stream;
    HostIdentifier host_identifier = std::get<HOST_IDENTIFIER>(key);
    key_stream << DUID(host_identifier).toText() << "-";
    key_stream << std::get<HOST_IDENTIFIER_TYPE>(key) << "-";
    key_stream << std::get<IPv4_SUBNET_ID>(key) << "-";
    key_stream << std::get<IPv6_SUBNET_ID>(key) << "-";
    key_stream << std::get<IPv4_RESERVATION>(key);
    const std::string key_string = key_stream.str();

    uint64_t const &hash(Hash64::hash(key_string));

    return static_cast<std::size_t>(hash);
}

/// @brief equals operator for HostKey
///
/// @param key1 left hand side operand
/// @param key2 right hand side operand
///
/// @return true if keys are equal. Deep comparison is made.
bool operator==(HostKey const &key1, HostKey const &key2) {
    return std::get<HOST_IDENTIFIER>(key1) == std::get<HOST_IDENTIFIER>(key2) &&
           std::get<HOST_IDENTIFIER_TYPE>(key1) == std::get<HOST_IDENTIFIER_TYPE>(key2) &&
           std::get<IPv4_SUBNET_ID>(key1) == std::get<IPv4_SUBNET_ID>(key2) &&
           std::get<IPv6_SUBNET_ID>(key1) == std::get<IPv6_SUBNET_ID>(key2) &&
           std::get<IPv4_RESERVATION>(key1) == std::get<IPv4_RESERVATION>(key2);
}

CqlHostDataSourceImpl::CqlHostDataSourceImpl(CqlConnection::ParameterMap const &parameters)
    : parameters_(parameters), connection_(parameters) {
    CqlVersionExchange<isc::db::OperationalVersionTuple> version_exchange;
    version_exchange.validateSchema(connection_);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    connection_.setTransactionOperations(CqlTransactionExchange::BEGIN_TXN,
                                         CqlTransactionExchange::COMMIT_TXN,
                                         CqlTransactionExchange::ROLLBACK_TXN);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Prepare all possible statements.
    connection_.prepareStatements(CqlHostExchange::tagged_statements());
#ifdef TERASTREAM_FULL_TRANSACTIONS
    connection_.prepareStatements(CqlTransactionExchange::tagged_statements());
#endif  // TERASTREAM_FULL_TRANSACTIONS
}

bool CqlHostDataSourceImpl::insertOrDelete(const HostPtr &host, bool insert) {
    // If there is no host, there is nothing to do.
    if (!host) {
        return (false);
    }

    // Start transaction.
    CqlTransaction transaction(connection_);

    // Get option space names and vendor space names and combine them within a
    // single list.

    // For IPv4:
    ConstCfgOptionPtr cfg_option4 = host->getCfgOption4();
    std::list<std::string> option_spaces4 = cfg_option4->getOptionSpaceNames();
    std::list<std::string> vendor_spaces4 = cfg_option4->getVendorIdsSpaceNames();
    option_spaces4.insert(option_spaces4.end(), vendor_spaces4.begin(), vendor_spaces4.end());

    // For IPv6:
    ConstCfgOptionPtr cfg_option6 = host->getCfgOption6();
    std::list<std::string> option_spaces6 = cfg_option6->getOptionSpaceNames();
    std::list<std::string> vendor_spaces6 = cfg_option6->getVendorIdsSpaceNames();
    option_spaces6.insert(option_spaces6.end(), vendor_spaces6.begin(), vendor_spaces6.end());

    bool result = true;

    // For every IPv6 reservation, add each of their options to the
    // database.
    IPv6ResrvRange reservations = host->getIPv6Reservations();
    if (std::distance(reservations.first, reservations.second) > 0) {
        for (IPv6ResrvIterator it = reservations.first; result && it != reservations.second; ++it) {
            result = insertOrDeleteHostWithReservations(insert, host, &it->second, option_spaces4,
                                                        cfg_option4, option_spaces6, cfg_option6);
        }
    } else {
        // If host has no reservation, add entries with null
        // reservation. Options could still be present.
        result = insertOrDeleteHostWithReservations(insert, host, NULL, option_spaces4, cfg_option4,
                                                    option_spaces6, cfg_option6);
    }

    transaction.commit();

    return (result);
}

void CqlHostDataSourceImpl::syncReservations() {
    ConstHostCollection old_hosts = getAllHosts();
    for (ConstHostPtr old_host : old_hosts) {
        HostPtr host = std::const_pointer_cast<Host>(old_host);
        insertOrDelete(host, false);
    }
    HostCollection new_hosts = CfgMgr::instance().getStagingCfg()->getCfgHosts()->getAll();
    for (HostPtr &host : new_hosts) {
        try {
            insertOrDelete(host, true);
        } catch (DuplicateEntry const &exception) {
            // A duplicate was being inserted. Duplicates are expected.
            // Carry on.
        }
    }
}

ConstHostPtr CqlHostDataSourceImpl::get4(const SubnetID &subnet_id,
                                         const asiolink::IOAddress &address) const {
    // Start transaction.
    CqlTransaction transaction(connection_);

    if (!address.isV4()) {
        isc_throw(BadValue, "CqlHostDataSource::get4(2): wrong address type, "
                            "address supplied is not an IPv4 address");
    }

    // Convert to CQL data types.
    cass_int32_t host_ipv4_subnet_id = static_cast<cass_int32_t>(subnet_id);
    cass_int32_t host_ipv4_address = static_cast<cass_int32_t>(address.toUint32());

    // Bind to array.
    AnyArray where_values{&host_ipv4_subnet_id, &host_ipv4_address};

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    where_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Run statement.
    ConstHostPtr result =
        getHost(CqlHostExchange::GET_HOST_BY_IPV4_SUBNET_ID_AND_ADDRESS(), where_values);

    transaction.commit();

    return (result);
}

ConstHostPtr CqlHostDataSourceImpl::get4(const SubnetID &subnet_id,
                                         const Host::IdentifierType &identifier_type,
                                         const uint8_t *identifier_begin,
                                         const size_t identifier_len) const {
    // Start transaction.
    CqlTransaction transaction(connection_);

    // Convert to CQL data types.
    CassBlob host_identifier(identifier_begin, identifier_begin + identifier_len);
    cass_int32_t host_identifier_type = static_cast<cass_int32_t>(identifier_type);
    cass_int32_t host_ipv4_subnet_id = static_cast<cass_int32_t>(subnet_id);

    // Bind to array.
    AnyArray where_values{&host_ipv4_subnet_id, &host_identifier, &host_identifier_type};

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    where_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Run statement.
    ConstHostPtr result =
        getHost(CqlHostExchange::GET_HOST_BY_IPV4_SUBNET_ID_AND_HOST_ID(), where_values);

    transaction.commit();

    return (result);
}

ConstHostPtr CqlHostDataSourceImpl::get6(const asiolink::IOAddress &prefix,
                                         const uint8_t prefix_len) const {
    // Start transaction.
    CqlTransaction transaction(connection_);

    // Convert to CQL data types.
    std::string reserved_ipv6_prefix_address = prefix.toText();
    cass_int32_t reserved_ipv6_prefix_length = prefix_len;

    ConstHostPtr host;
    // Bind to array.
    AnyArray where_values{&reserved_ipv6_prefix_address, &reserved_ipv6_prefix_length};

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    where_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get host id.
    host = getHost(CqlHostExchange::GET_HOST_BY_IPV6_PREFIX(), where_values);

    if (!host) {
        return ConstHostPtr();
    }

    // Get host.
    HostIdentifier host_identifier = host->getIdentifier();
    // Delegate to getAll(3).
    ConstHostCollection collection =
        getAll(host->getIdentifierType(), host_identifier.data(), host_identifier.size());

    if (collection.empty()) {
        return (ConstHostPtr());
    }

    if (collection.size() >= 2u) {
        isc_throw(MultipleRecords, "CqlHostDataSource::get6(2): multiple records were "
                                   "found in the "
                                   "database where only one was expected for statement "
                                       << CqlHostExchange::GET_HOST_BY_IPV6_PREFIX());
    }

    ConstHostPtr result = collection.front();

    transaction.commit();

    return (result);
}

ConstHostPtr CqlHostDataSourceImpl::get6(const SubnetID &subnet_id,
                                         const Host::IdentifierType &identifier_type,
                                         const uint8_t *identifier_begin,
                                         const size_t identifier_len) const {
    // Start transaction.
    CqlTransaction transaction(connection_);

    // Convert to CQL data types.
    cass_int32_t host_ipv6_subnet_id = static_cast<cass_int32_t>(subnet_id);
    CassBlob host_identifier(identifier_begin, identifier_begin + identifier_len);
    cass_int32_t host_identifier_type = static_cast<cass_int32_t>(identifier_type);

    // Bind to array.
    AnyArray where_values{&host_ipv6_subnet_id, &host_identifier, &host_identifier_type};

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    where_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Run statement.
    ConstHostPtr result =
        getHost(CqlHostExchange::GET_HOST_BY_IPV6_SUBNET_ID_AND_HOST_ID(), where_values);

    transaction.commit();

    return (result);
}

ConstHostPtr CqlHostDataSourceImpl::get6(const SubnetID &subnet_id,
                                         const IOAddress &address) const {
    // Start transaction.
    CqlTransaction transaction(connection_);

    // Convert to CQL data types.
    cass_int32_t host_ipv6_subnet_id = static_cast<cass_int32_t>(subnet_id);
    std::string reserved_ipv6_prefix_address = address.toText();

    // Bind to array.
    AnyArray where_values{&host_ipv6_subnet_id, &reserved_ipv6_prefix_address};

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    where_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Run statement.
    ConstHostPtr result =
        getHost(CqlHostExchange::GET_HOST_BY_IPV6_SUBNET_ID_AND_ADDRESS(), where_values);

    transaction.commit();

    return (result);
}

ConstHostCollection CqlHostDataSourceImpl::getAll(const Host::IdentifierType &identifier_type,
                                                  const uint8_t *identifier_begin,
                                                  const size_t identifier_len) const {
    // Start transaction.
    CqlTransaction transaction(connection_);

    // Convert to CQL data types.
    CassBlob host_identifier(identifier_begin, identifier_begin + identifier_len);
    cass_int32_t host_identifier_type = static_cast<cass_int32_t>(identifier_type);

    // Bind to array.
    AnyArray where_values{&host_identifier, &host_identifier_type};

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    where_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Run statement.
    ConstHostCollection result =
        getHostCollection(CqlHostExchange::GET_HOST_BY_HOST_ID(), where_values);

    transaction.commit();

    return (result);
}

ConstHostCollection CqlHostDataSourceImpl::getAll4(const SubnetID &subnet_id) const {
    // Convert to CQL data types.
    cass_int32_t host_ipv4_subnet_id = static_cast<cass_int32_t>(subnet_id);

    // Bind to array.
    AnyArray where_values;
    where_values.add(&host_ipv4_subnet_id);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    where_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Run statement.
    ConstHostCollection result =
        getHostCollection(CqlHostExchange::GET_HOST_BY_IPV4_SUBNET_ID(), where_values);

    return (result);
}

ConstHostCollection CqlHostDataSourceImpl::getAll6(const SubnetID &subnet_id) const {
    // Convert to CQL data types.
    cass_int32_t host_ipv6_subnet_id = static_cast<cass_int32_t>(subnet_id);

    // Bind to array.
    AnyArray where_values;
    where_values.add(&host_ipv6_subnet_id);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    where_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Run statement.
    ConstHostCollection result =
        getHostCollection(CqlHostExchange::GET_HOST_BY_IPV6_SUBNET_ID(), where_values);

    return (result);
}

ConstHostCollection CqlHostDataSourceImpl::getAllbyHostname(const std::string &hostname) const {
    // Convert to CQL data types.
    std::string hostname_ = hostname;

    // Bind to array.
    AnyArray where_values;
    where_values.add(&hostname_);

    // Run statement.
    ConstHostCollection result =
        getHostCollection(CqlHostExchange::GET_HOST_BY_HOST_NAME(), where_values);

    return (result);
}

ConstHostCollection CqlHostDataSourceImpl::getAllbyHostname4(const std::string &hostname,
                                                             const SubnetID &subnet_id) const {
    // Convert to CQL data types.
    std::string hostname_ = hostname;
    cass_int32_t host_ipv4_subnet_id = static_cast<cass_int32_t>(subnet_id);

    // Bind to array.
    AnyArray where_values;
    where_values.add(&hostname_);
    where_values.add(&host_ipv4_subnet_id);

    // Run statement.
    ConstHostCollection result = getHostCollection(
        CqlHostExchange::GET_HOST_BY_HOST_NAME_AND_IPV4_SUBNET_ID(), where_values);

    return (result);
}

ConstHostCollection CqlHostDataSourceImpl::getAllbyHostname6(const std::string &hostname,
                                                             const SubnetID &subnet_id) const {
    // Convert to CQL data types.
    std::string hostname_ = hostname;
    cass_int32_t host_ipv6_subnet_id = static_cast<cass_int32_t>(subnet_id);

    // Bind to array.
    AnyArray where_values;
    where_values.add(&hostname_);
    where_values.add(&host_ipv6_subnet_id);

    // Run statement.
    ConstHostCollection result = getHostCollection(
        CqlHostExchange::GET_HOST_BY_HOST_NAME_AND_IPV6_SUBNET_ID(), where_values);

    return (result);
}

// There are some problems implementing this for Cassandra.
// Attempts show the per page ordering does not work and
// it is not possible to order by TOKEN(host_id).
// If the issue solved by paging is the Kea API overhead then
// a solution is to get and cache all reservations and to handle
// paging at the API level.

ConstHostCollection CqlHostDataSourceImpl::getPage4(const SubnetID &subnet_id,
                                                    uint64_t lower_host_id,
                                                    const HostPageSize &page_size) const {
    // Run statement.
    ConstHostCollection result =
        getHostCollectionPage4(subnet_id, lower_host_id, page_size.page_size_);

    return (result);
}

ConstHostCollection CqlHostDataSourceImpl::getPage6(const SubnetID &subnet_id,
                                                    uint64_t lower_host_id,
                                                    const HostPageSize &page_size) const {
    // Run statement.
    ConstHostCollection result =
        getHostCollectionPage6(subnet_id, lower_host_id, page_size.page_size_);

    return (result);
}

ConstHostCollection CqlHostDataSourceImpl::getAll4(const asiolink::IOAddress &address) const {
    // Start transaction.
    CqlTransaction transaction(connection_);

    // Convert to CQL data types.
    cass_int32_t host_ipv4_address = static_cast<cass_int32_t>(address.toUint32());

    // Bind to array.
    AnyArray where_values{&host_ipv4_address};

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    where_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Run statement.
    ConstHostCollection result =
        getHostCollection(CqlHostExchange::GET_HOST_BY_IPV4_ADDRESS(), where_values);

    transaction.commit();

    return (result);
}

ConstHostCollection CqlHostDataSourceImpl::getAllHosts() const {
    // Start transaction.
    CqlTransaction transaction(connection_);

    // Bind to array.
    AnyArray where_values;

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    where_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Run statement.
    ConstHostCollection result = getHostCollection(CqlHostExchange::GET_HOST(), where_values);

    transaction.commit();

    return (result);
}

std::string CqlHostDataSourceImpl::getName() const {
    std::string name;
    try {
        name = connection_.getParameter("name");
    } catch (...) {
        // Return an empty name.
    }
    return (name);
}

void CqlHostDataSourceImpl::commit() {
    connection_.commit();
}

void CqlHostDataSourceImpl::rollback() {
    connection_.rollback();
}

bool CqlHostDataSourceImpl::insertOrDeleteHostWithOptions(
    bool insert,
    const HostPtr &host,
    const IPv6Resrv *const reservation,
    const std::list<std::string> &option_spaces,
    const ConstCfgOptionPtr cfg_option) {
    // If there is no host, there is nothing to do.
    if (!host) {
        return (false);
    }

    bool result = true;

    // For each option space retrieve all options and insert them into
    // the database.
    bool option_found = false;
    for (const std::string &space : option_spaces) {
        if (!result) {
            break;
        }
        OptionContainerPtr options = cfg_option->getAll(space);
        if (options && !options->empty()) {
            for (const OptionDescriptor &option : *options) {
                if (!result) {
                    break;
                }
                option_found = true;
                /// @todo: Assign actual value to subnet id.
                result = insertOrDeleteHost(insert, host, Optional<SubnetID>(), reservation, space,
                                            option);
            }
        }
    }
    if (result && !option_found) {
        // @todo: Assign actual value to subnet id.
        result = insertOrDeleteHost(insert, host, Optional<SubnetID>(), reservation);
    }

    return (result);
}

bool CqlHostDataSourceImpl::insertOrDeleteHostWithReservations(
    bool insert,
    const HostPtr &host,
    const IPv6Resrv *const reservation,
    const std::list<std::string> &option_spaces4,
    const ConstCfgOptionPtr cfg_option4,
    const std::list<std::string> &option_spaces6,
    const ConstCfgOptionPtr cfg_option6) {
    // If there is no host, there is nothing to do.
    if (!host) {
        return (false);
    }

    bool result = true;

    // If host has no reservation, add entries with null reservation.
    // Options could still be present.
    if (result && cfg_option4 && !cfg_option4->empty()) {
        result =
            insertOrDeleteHostWithOptions(insert, host, reservation, option_spaces4, cfg_option4);
    }
    if (result && cfg_option6 && !cfg_option6->empty()) {
        result =
            insertOrDeleteHostWithOptions(insert, host, reservation, option_spaces6, cfg_option6);
    }
    if (result && (!cfg_option4 || cfg_option4->empty()) &&
        (!cfg_option6 || cfg_option6->empty())) {
        result = insertOrDeleteHostWithOptions(insert, host, reservation);
    }

    return (result);
}

ConstHostPtr CqlHostDataSourceImpl::getHost(StatementTag statement_tag,
                                            AnyArray &where_values) const {
    ConstHostCollection collection = getHostCollection(statement_tag, where_values);

    if (collection.empty()) {
        return (ConstHostPtr());
    }

    if (collection.size() >= 2u) {
        isc_throw(MultipleRecords,
                  "CqlHostDataSourceImpl::getHost(): multiple records were "
                  "found in the database where only one was expected for statement "
                      << statement_tag);
    }

    return collection.front();
}

ConstHostCollection CqlHostDataSourceImpl::getHostCollection(StatementTag statement_tag,
                                                             AnyArray &where_values) const {
    // Run statement.
    CqlHostExchange exchange;
    HostCollection collection =
        exchange.executeSelect(connection_, where_values, statement_tag, false);

    // Merge the denormalized table entries that belong to the same host into a single host.
    HostMap map;
    for (HostPtr &host : collection) {
        HostKey key =
            HostKey(host->getIdentifier(), host->getIdentifierType(), host->getIPv4SubnetID(),
                    host->getIPv6SubnetID(), host->getIPv4Reservation());
        if (map.find(key) == map.end()) {
            map[key] = host;
        } else {
            mergeHosts(host, map[key]);
        }
    }

    ConstHostCollection result_collection;

    for (HostPtr &host : collection) {
        HostKey key =
            HostKey(host->getIdentifier(), host->getIdentifierType(), host->getIPv4SubnetID(),
                    host->getIPv6SubnetID(), host->getIPv4Reservation());
        if (map.find(key) != map.end()) {
            result_collection.push_back(map[key]);
            map.erase(key);
        }
    }
    return (result_collection);
}

ConstHostPtr CqlHostDataSourceImpl::getHostByKey4(uint64_t key) const {
    // Bind to array.
    AnyArray where_values;
    cass_int64_t key_data = static_cast<cass_int64_t>(key);
    where_values.add(&key_data);

    // Run statement.
    ConstHostCollection collection =
        getHostCollection(CqlHostExchange::GET_HOST_BY_IPV4_SUBNET_ID_KEY(), where_values);

    if (collection.empty()) {
        return (ConstHostPtr());
    }

    if (collection.size() >= 2u) {
        isc_throw(MultipleRecords,
                  "CqlHostDataSourceImpl::getHost(): multiple records were "
                  "found in the database where only one was expected for statement "
                      << CqlHostExchange::GET_HOST_BY_IPV4_SUBNET_ID_KEY());
    }

    return (*collection.begin());
}

ConstHostPtr CqlHostDataSourceImpl::getHostByKey6(uint64_t key) const {
    // Bind to array.
    AnyArray where_values;
    cass_int64_t key_data = static_cast<cass_int64_t>(key);
    where_values.add(&key_data);

    // Run statement.
    ConstHostCollection collection =
        getHostCollection(CqlHostExchange::GET_HOST_BY_IPV6_SUBNET_ID_KEY(), where_values);

    if (collection.empty()) {
        return (ConstHostPtr());
    }

    if (collection.size() >= 2u) {
        isc_throw(MultipleRecords,
                  "CqlHostDataSourceImpl::getHost(): multiple records were "
                  "found in the database where only one was expected for statement "
                      << CqlHostExchange::GET_HOST_BY_IPV6_SUBNET_ID_KEY());
    }

    return (*collection.begin());
}

ConstHostCollection CqlHostDataSourceImpl::getHostCollectionPage4(const SubnetID &subnet_id,
                                                                  uint64_t lower_host_id,
                                                                  size_t count) const {
    ConstHostCollection result_collection;
    for (; count; count--) {
        uint64_t key;
        bool valid_key = getHostKey4(subnet_id, lower_host_id, key);

        if (!valid_key) {
            break;
        }

        ConstHostPtr host = getHostByKey4(key);
        result_collection.push_back(host);
        lower_host_id = host->getHostId();
    }

    return (result_collection);
}

ConstHostCollection CqlHostDataSourceImpl::getHostCollectionPage6(const SubnetID &subnet_id,
                                                                  uint64_t lower_host_id,
                                                                  size_t count) const {
    ConstHostCollection result_collection;
    for (; count; count--) {
        uint64_t key;
        bool valid_key = getHostKey6(subnet_id, lower_host_id, key);

        if (!valid_key) {
            break;
        }

        ConstHostPtr host = getHostByKey6(key);
        result_collection.push_back(host);
        lower_host_id = host->getHostId();
    }

    return (result_collection);
}

bool CqlHostDataSourceImpl::getHostKey4(const SubnetID &subnet_id,
                                        uint64_t lower_host_id,
                                        uint64_t &key) const {
    // Convert to CQL data types.
    cass_int32_t host_subnet_id = static_cast<cass_int32_t>(subnet_id);

    // Bind to array.
    AnyArray where_values;
    where_values.add(&host_subnet_id);

    cass_int64_t host_data = 0;
    if (lower_host_id) {
        host_data = static_cast<cass_int64_t>(lower_host_id);
        where_values.add(&host_data);
    }

    // Run statement.
    // This will retrieve first row of the first host (lower_host_id == 0)
    // or the first row of the host (lower_host_id != 0)
    CqlHostExchange exchange;
    HostCollection collection;
    if (lower_host_id) {
        collection = exchange.executeSelect(
            connection_, where_values, CqlHostExchange::GET_HOST_BY_IPV4_SUBNET_ID_PAGE(), false);
    } else {
        collection = exchange.executeSelect(
            connection_, where_values, CqlHostExchange::GET_HOST_BY_IPV4_SUBNET_ID_LIMIT(), false);
    }

    // If there is no host, just exit
    if (collection.empty()) {
        return false;
    }

    key = exchange.hashIntoKey();

    if (lower_host_id) {
        return getNextHostKey4(subnet_id, key);
    }

    return true;
}

bool CqlHostDataSourceImpl::getHostKey6(const SubnetID &subnet_id,
                                        uint64_t lower_host_id,
                                        uint64_t &key) const {
    // Convert to CQL data types.
    cass_int32_t host_subnet_id = static_cast<cass_int32_t>(subnet_id);

    // Bind to array.
    AnyArray where_values;
    where_values.add(&host_subnet_id);

    cass_int64_t host_data = 0;
    if (lower_host_id) {
        host_data = static_cast<cass_int64_t>(lower_host_id);
        where_values.add(&host_data);
    }

    // Run statement.
    // This will retrieve first row of the first host (lower_host_id == 0)
    // or the first row of the host (lower_host_id != 0)
    CqlHostExchange exchange;
    HostCollection collection;
    if (lower_host_id) {
        collection = exchange.executeSelect(
            connection_, where_values, CqlHostExchange::GET_HOST_BY_IPV6_SUBNET_ID_PAGE(), false);
    } else {
        collection = exchange.executeSelect(
            connection_, where_values, CqlHostExchange::GET_HOST_BY_IPV6_SUBNET_ID_LIMIT(), false);
    }

    // If there is no host, just exit
    if (collection.empty()) {
        return false;
    }

    key = exchange.hashIntoKey();

    if (lower_host_id) {
        return getNextHostKey6(subnet_id, key);
    }

    return true;
}

bool CqlHostDataSourceImpl::getNextHostKey4(const SubnetID &subnet_id, uint64_t &key) const {
    // Convert to CQL data types.
    cass_int32_t host_subnet_id = static_cast<cass_int32_t>(subnet_id);
    cass_int64_t key_data = static_cast<cass_int64_t>(key);

    // Bind to array.
    AnyArray where_values;
    where_values.add(&host_subnet_id);
    where_values.add(&key_data);

    // This will retrieve first row of the next host (lower_host_id != 0)
    CqlHostExchange exchange;
    HostCollection collection = exchange.executeSelect(
        connection_, where_values, CqlHostExchange::GET_HOST_BY_IPV4_SUBNET_ID_NEXT_KEY(), false);

    if (collection.empty()) {
        return false;
    }

    key = exchange.hashIntoKey();
    return true;
}

bool CqlHostDataSourceImpl::getNextHostKey6(const SubnetID &subnet_id, uint64_t &key) const {
    // Convert to CQL data types.
    cass_int32_t host_subnet_id = static_cast<cass_int32_t>(subnet_id);
    cass_int64_t key_data = static_cast<cass_int64_t>(key);

    // Bind to array.
    AnyArray where_values;
    where_values.add(&host_subnet_id);
    where_values.add(&key_data);

    // This will retrieve first row of the next host (lower_host_id != 0)
    CqlHostExchange exchange;
    HostCollection collection = exchange.executeSelect(
        connection_, where_values, CqlHostExchange::GET_HOST_BY_IPV6_SUBNET_ID_NEXT_KEY(), false);

    if (collection.empty()) {
        return false;
    }

    key = exchange.hashIntoKey();
    return true;
}

bool CqlHostDataSourceImpl::insertOrDeleteHost(bool insert,
                                               const HostPtr &host,
                                               const Optional<SubnetID> &subnet_id,
                                               const IPv6Resrv *const reservation,
                                               const std::string &option_space,
                                               const OptionDescriptor &option_descriptor) {
    // If there is no host, there is nothing to do.
    if (!host) {
        return (false);
    }

    AnyArray assigned_values;

    CqlHostExchange exchange;

    try {
        if (insert) {
            exchange.createBindForMutation(host, subnet_id, reservation, option_space,
                                           option_descriptor, CqlHostExchange::INSERT_HOST(),
                                           assigned_values);

#ifdef TERASTREAM_FULL_TRANSACTIONS
            CassUuid txid(connection_.getTransactionID());
            assigned_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

            exchange.executeMutation(connection_, assigned_values, CqlHostExchange::INSERT_HOST());
        } else {
            exchange.createBindForDelete(host, subnet_id, reservation, option_space,
                                         option_descriptor, CqlHostExchange::DELETE_HOST(),
                                         assigned_values);

#ifdef TERASTREAM_FULL_TRANSACTIONS
            CassUuid txid(connection_.getTransactionID());
            assigned_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

            exchange.executeMutation(connection_, assigned_values, CqlHostExchange::DELETE_HOST());
        }
    } catch (StatementNotApplied const &exception) {
        if (insert) {
            isc_throw(DuplicateEntry, exception.what());
        } else {
            return (false);
        }
    }

    return (true);
}

void CqlHostDataSourceImpl::mergeHosts(const ConstHostPtr &source_host,
                                       HostPtr &target_host) const {
    // Merge reservations.
    const IPv6ResrvRange reservations_range = source_host->getIPv6Reservations();
    if (std::distance(reservations_range.first, reservations_range.second) > 0) {
        for (IPv6ResrvIterator reservations_iterator = reservations_range.first;
             reservations_iterator != reservations_range.second; ++reservations_iterator) {
            if (!target_host->hasReservation(reservations_iterator->second)) {
                target_host->addReservation(reservations_iterator->second);
            }
        }
    }

    // Merge DHCPv4 options.
    source_host->getCfgOption4()->mergeTo(*target_host->getCfgOption4());

    // Merge DHCPv6 options.
    source_host->getCfgOption6()->mergeTo(*target_host->getCfgOption6());
}

CqlHostDataSource::CqlHostDataSource(CqlConnection::ParameterMap const &parameters)
    : impl_(new CqlHostDataSourceImpl(parameters)) {
}

void CqlHostDataSource::add(const HostPtr &host) {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_ADD);

    impl_->insertOrDelete(host, true);
}

bool CqlHostDataSource::del(const SubnetID &subnet_id, const asiolink::IOAddress &address) {
    HostPtr host = std::const_pointer_cast<Host>(impl_->get4(subnet_id, address));

    return (host ? impl_->insertOrDelete(host, false) : false);
}

bool CqlHostDataSource::del4(const SubnetID &subnet_id,
                             const Host::IdentifierType &identifier_type,
                             const uint8_t *identifier_begin,
                             const size_t identifier_len) {
    HostPtr host = std::const_pointer_cast<Host>(
        impl_->get4(subnet_id, identifier_type, identifier_begin, identifier_len));

    return (host ? impl_->insertOrDelete(host, false) : false);
}

bool CqlHostDataSource::del6(const SubnetID &subnet_id,
                             const Host::IdentifierType &identifier_type,
                             const uint8_t *identifier_begin,
                             const size_t identifier_len) {
    HostPtr host = std::const_pointer_cast<Host>(
        impl_->get6(subnet_id, identifier_type, identifier_begin, identifier_len));

    return (host ? impl_->insertOrDelete(host, false) : false);
}

ConstHostCollection CqlHostDataSource::getAll(const Host::IdentifierType &identifier_type,
                                              const uint8_t *identifier_begin,
                                              const size_t identifier_len) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET_ALL);

    return (impl_->getAll(identifier_type, identifier_begin, identifier_len));
}

ConstHostCollection CqlHostDataSource::getAll4(const SubnetID &subnet_id) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET_ALL);

    return (impl_->getAll4(subnet_id));
}

ConstHostCollection CqlHostDataSource::getAll6(const SubnetID &subnet_id) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET_ALL);

    return (impl_->getAll6(subnet_id));
}

ConstHostCollection CqlHostDataSource::getAllbyHostname(const std::string &hostname) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET_ALL);

    return (impl_->getAllbyHostname(hostname));
}

ConstHostCollection CqlHostDataSource::getAllbyHostname4(const std::string &hostname,
                                                         const SubnetID &subnet_id) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET_ALL);

    return (impl_->getAllbyHostname4(hostname, subnet_id));
}

ConstHostCollection CqlHostDataSource::getAllbyHostname6(const std::string &hostname,
                                                         const SubnetID &subnet_id) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET_ALL);

    return (impl_->getAllbyHostname6(hostname, subnet_id));
}

ConstHostCollection CqlHostDataSource::getPage4(const SubnetID &subnet_id,
                                                size_t & /*source_index*/,
                                                uint64_t lower_host_id,
                                                const HostPageSize &page_size) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET_ALL);

    return (impl_->getPage4(subnet_id, lower_host_id, page_size));
}

ConstHostCollection CqlHostDataSource::getPage6(const SubnetID &subnet_id,
                                                size_t & /*source_index*/,
                                                uint64_t lower_host_id,
                                                const HostPageSize &page_size) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET_ALL);

    return (impl_->getPage6(subnet_id, lower_host_id, page_size));
}

ConstHostCollection CqlHostDataSource::getAll4(const asiolink::IOAddress &address) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET_ALL);

    return (impl_->getAll4(address));
}

ConstHostPtr CqlHostDataSource::get4(const SubnetID &subnet_id,
                                     const Host::IdentifierType &identifier_type,
                                     const uint8_t *identifier_begin,
                                     const size_t identifier_len) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET4);

    return (impl_->get4(subnet_id, identifier_type, identifier_begin, identifier_len));
}

ConstHostPtr CqlHostDataSource::get4(const SubnetID &subnet_id,
                                     const asiolink::IOAddress &address) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET4);

    return (impl_->get4(subnet_id, address));
}

ConstHostPtr CqlHostDataSource::get6(const SubnetID &subnet_id,
                                     const Host::IdentifierType &identifier_type,
                                     const uint8_t *identifier_begin,
                                     const size_t identifier_len) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET6);

    return (impl_->get6(subnet_id, identifier_type, identifier_begin, identifier_len));
}

ConstHostPtr CqlHostDataSource::get6(const asiolink::IOAddress &prefix,
                                     const uint8_t prefix_len) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET6);

    return (impl_->get6(prefix, prefix_len));
}

ConstHostPtr CqlHostDataSource::get6(const SubnetID &subnet_id,
                                     const asiolink::IOAddress &address) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_GET6);

    return (impl_->get6(subnet_id, address));
}

ConstHostCollection CqlHostDataSource::getAllHosts() const {
    return (impl_->getAllHosts());
}

std::string CqlHostDataSource::getType() const {
    return "cql";
}

std::string CqlHostDataSource::getName() const {
    return (impl_->getName());
}

std::string CqlHostDataSource::getDescription() const {
    return std::string("Host data source that stores host information in the CQL database");
}

void CqlHostDataSource::syncReservations() {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_HOST_SYNC_RESERVATIONS);

    impl_->syncReservations();
}

void CqlHostDataSource::commit() {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_COMMIT);

    impl_->commit();
}

void CqlHostDataSource::rollback() {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_ROLLBACK);

    impl_->rollback();
}

}  // namespace dhcp
}  // namespace isc

// Copyright (C) 2016-2020 Internet Systems Consortium, Inc. ("ISC")
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
#include <cql/cql_transaction.h>
#include <dhcpsrv/cql_lease_mgr.h>
#include <dhcpsrv/utils.h>

#include <dhcp/duid.h>
#include <dhcp/hwaddr.h>

#include <asiolink/io_address.h>

#include <mutex>

using namespace isc::data;
using namespace isc::db;
using isc::asiolink::IOAddress;

namespace isc {
namespace dhcp {

static constexpr size_t HOSTNAME_MAX_LEN = 255u;
static constexpr size_t ADDRESS6_TEXT_MAX_LEN = 39u;
static constexpr char NULL_USER_CONTEXT[] = "";

#ifdef CASSANDRA_DENORMALIZED_TABLES
static const CassBlob NULL_HWADDR({0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
static const CassBlob NULL_CLIENTID({0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
static const CassBlob NULL_PRIVACY_HASH({0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
#endif  // CASSANDRA_DENORMALIZED_TABLES

/// @brief Common CQL and Lease Data Methods
///
/// The @ref CqlLease4Exchange and @ref CqlLease6Exchange classes provide the
/// functionality to set up binding information between variables in the
/// program and data extracted from the database. This class is the common
/// base to both of them, containing some common methods.
template <typename T>
struct CqlLeaseExchange : CqlExchange<T> {
    /// @brief Constructor
    ///
    /// @param connection already open Cassandra connection.
    CqlLeaseExchange(CqlConnection const &connection)
        : connection_(connection), valid_lifetime_(0), expire_(0), old_expire_(0), subnet_id_(0),
          fqdn_fwd_(cass_false), fqdn_rev_(cass_false), state_(0),
          user_context_(NULL_USER_CONTEXT) {
    }

    /// @brief Create BIND array to receive C++ data.
    ///
    /// Used in executeSelect() to retrieve from database
    ///
    /// @param data array of bound objects representing data to be retrieved
    /// @param statement_tag prepared statement being executed; defaults to an
    ///     invalid index
    virtual void
    createBindForSelect(AnyArray &data,
                        StatementTag const &statement_tag = StatementTag()) override = 0;

    /// @brief Copy received data into the derived class' object.
    ///
    /// Copies information about the entity to be retrieved into a holistic
    /// object. Called in @ref executeSelect(). Not implemented for base class
    /// CqlExchange. To be implemented in derived classes.
    ///
    /// @return a pointer to the object retrieved.
    virtual Ptr<T> retrieve() override = 0;

protected:
    /// @brief Database connection
    CqlConnection const &connection_;

#ifdef TERASTREAM
    /// @brief Lease privacy expire
    cass_int64_t privacy_expire_;

    /// @brief Lease privacy hash
    CassBlob privacy_hash_;

    /// @brief Lease privacy history
    std::string privacy_history_;
#endif  // TERASTREAM

#ifdef TERASTREAM_FULL_TRANSACTIONS
    /// @brief transaction uuid
    CassUuid txid_;
#endif  // TERASTREAM_FULL_TRANSACTIONS

    /// @brief Hardware address
    CassBlob hwaddr_;

    /// @brief Lease timer
    cass_int64_t valid_lifetime_;

    /// @brief Lease expiry time
    cass_int64_t expire_;

    /// @brief Expiration time of lease before update
    cass_int64_t old_expire_;

    /// @brief Subnet identifier
    cass_int32_t subnet_id_;

    /// @brief Has forward DNS update been performed?
    cass_bool_t fqdn_fwd_;

    /// @brief Has reverse DNS update been performed?
    cass_bool_t fqdn_rev_;

    /// @brief Client hostname
    std::string hostname_;

    /// @brief Lease state
    cass_int32_t state_;

    /// @brief User context
    std::string user_context_;
};

/// @brief Exchange Lease4 information between Kea and CQL
///
/// On any CQL operation, arrays of CQL BIND structures must be built to
/// describe the parameters in the prepared statements. Where information is
/// inserted or retrieved - INSERT, UPDATE, SELECT - a large amount of that
/// structure is identical. This class handles the creation of that array.
///
/// Owing to the CQL API, the process requires some intermediate variables
/// to hold things like data length etc. This object holds those variables.
///
/// @note There are no unit tests for this class. It is tested indirectly
/// in all CqlLeaseMgr::xxx4() calls where it is used.
struct CqlLease4Exchange : CqlLeaseExchange<Lease4> {
    /// @brief Constructor
    ///
    /// The initialization of the variables here is only to satisfy
    /// cppcheck - all variables are initialized/set in the methods before
    /// they are used.
    ///
    /// @param connection connection used for this query
    explicit CqlLease4Exchange(CqlConnection const &connection);

    /// @brief Create CQL_BIND objects for Lease4 Pointer
    ///
    /// Fills in the CQL_BIND array for sending data in the Lease4 object to
    /// the database. Used for INSERT statements.
    ///
    /// @param lease The lease information to be inserted
    /// @param data Lease info will be stored here in CQL format
    void createBindForInsert(const Lease4Ptr &lease, AnyArray &data);

    /// @brief Create CQL_BIND objects for Lease4 Pointer
    ///
    /// Fills in the CQL_BIND array for sending data in the Lease4 object to
    /// the database. Used for UPDATE statements.
    ///
    /// @param lease Updated lease information.
    /// @param data lease info in CQL format will be stored here
    /// @param statement_tag tag identifying the query (optional)
    void createBindForUpdate(const Lease4Ptr &lease,
                             AnyArray &data,
                             StatementTag statement_tag = StatementTag());

    /// @brief Create CQL_BIND objects for Lease4 Pointer
    ///
    /// Fills in the CQL_BIND array for sending data in the Lease4 object to
    /// the database. Used for DELETE statements.
    ///
    /// @param lease lease to be deleted
    /// @param data lease info in CQL format will be stored here
    /// @param statement_tag tag identifying the query (optional)
    void createBindForDelete(const Lease4Ptr &lease,
                             AnyArray &data,
                             StatementTag statement_tag = StatementTag());

    /// @brief Create BIND array to receive data
    ///
    /// Creates a CQL_BIND array to receive Lease4 data from the database.
    ///
    /// @param data info returned by CQL will be stored here
    /// @param statement_tag tag identifying the query (optional)
    void createBindForSelect(AnyArray &data,
                             StatementTag const &statement_tag = StatementTag()) override final;

    /// @brief Retrieves the Lease4 object in Kea format
    ///
    /// @return C++ representation of the object being returned
    Lease4Ptr retrieve() override final;

    /// @brief Retrieves zero or more IPv4 leases
    ///
    /// @param statement_tag query to be executed
    /// @param data parameters for the query
    /// @param result this lease collection will be updated
    void
    getLeaseCollection(StatementTag const &statement_tag, AnyArray &data, Lease4Collection &result);

    /// @brief Retrieves one IPv4 lease
    ///
    /// @param statement_tag query to be executed
    /// @param data parameters for the query
    /// @param result pointer to the lease being returned (or null)
    void getLease(StatementTag const &statement_tag, AnyArray &data, Lease4Ptr &result);

    /// @brief Returns expired leases.
    ///
    /// This method returns up to specified number (see max_leases) of
    /// expired leases.
    ///
    /// @param max_leases at most this number of leases will be returned
    /// @param expired_leases expired leases will be stored here
    void getExpiredLeases(const size_t &max_leases, Lease4Collection &expired_leases);

    static StatementMap &tagged_statements() {
        static StatementMap _{
#ifdef CASSANDRA_DENORMALIZED_TABLES
            {INSERT_LEASE4_ADDRESS(),  //
             { INSERT_LEASE4_ADDRESS(),  //
               "INSERT INTO lease4_address( "
#ifdef TERASTREAM
               "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
               "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
               "fqdn_fwd, fqdn_rev, hostname, state, user_context "
               ") VALUES ( "
#ifdef TERASTREAM
               "?, ?, ?, ?, "
#endif  // TERASTREAM
               "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? "
               ") "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF NOT EXISTS "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {INSERT_LEASE4_CLIENTID_SUBNETID(),  //
             { INSERT_LEASE4_CLIENTID_SUBNETID(),  //
               "INSERT INTO lease4_clientid_subnetid( "
#ifdef TERASTREAM
               "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
               "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
               "fqdn_fwd, fqdn_rev, hostname, state, user_context "
               ") VALUES ( "
#ifdef TERASTREAM
               "?, ?, ?, ?, "
#endif  // TERASTREAM
               "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? "
               ") "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF NOT EXISTS "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {INSERT_LEASE4_HWADDR_SUBNETID(),  //
             { INSERT_LEASE4_HWADDR_SUBNETID(),  //
               "INSERT INTO lease4_hwaddr_subnetid( "
#ifdef TERASTREAM
               "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
               "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
               "fqdn_fwd, fqdn_rev, hostname, state, user_context "
               ") VALUES ( "
#ifdef TERASTREAM
               "?, ?, ?, ?, "
#endif  // TERASTREAM
               "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? "
               ") "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF NOT EXISTS "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {INSERT_LEASE4_SUBNETID(),  //
             { INSERT_LEASE4_SUBNETID(),  //
               "INSERT INTO lease4_subnetid( "
#ifdef TERASTREAM
               "privacy_expire, privacy_hash, privacy_history, "
#endif  // TERASTREAM
               "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
               "fqdn_fwd, fqdn_rev, hostname, state, user_context "
               ") VALUES ( "
#ifdef TERASTREAM
               "?, ?, ?, "
#endif  // TERASTREAM
               "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? "
               ") "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF NOT EXISTS "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {UPDATE_LEASE4_ADDRESS(),  //
             { UPDATE_LEASE4_ADDRESS(),  //
               "UPDATE lease4_address SET "
#ifdef TERASTREAM
               "privacy_expire = ?, "
               "privacy_hash = ?, "
               "privacy_history = ?, "
               "sw_4o6_src_address = ?, "
#endif  // TERASTREAM
               "hwaddr = ?, "
               "client_id = ?, "
               "subnet_id = ?, "
               "valid_lifetime = ?, "
               "expire = ?, "
               "fqdn_fwd = ?, "
               "fqdn_rev = ?, "
               "hostname = ?, "
               "state = ?, "
               "user_context = ? "
               "WHERE address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {UPDATE_LEASE4_CLIENTID_SUBNETID(),  //
             { UPDATE_LEASE4_CLIENTID_SUBNETID(),  //
               "UPDATE lease4_clientid_subnetid SET "
#ifdef TERASTREAM
               "privacy_expire = ?, "
               "privacy_hash = ?, "
               "privacy_history = ?, "
               "sw_4o6_src_address = ?, "
#endif  // TERASTREAM
               "hwaddr = ?, "
               "valid_lifetime = ?, "
               "expire = ?, "
               "fqdn_fwd = ?, "
               "fqdn_rev = ?, "
               "hostname = ?, "
               "state = ?, "
               "user_context = ? "
               "WHERE client_id = ? "
               "AND subnet_id = ? "
               "AND address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {UPDATE_LEASE4_HWADDR_SUBNETID(),  //
             { UPDATE_LEASE4_HWADDR_SUBNETID(),  //
               "UPDATE lease4_hwaddr_subnetid SET "
#ifdef TERASTREAM
               "privacy_expire = ?, "
               "privacy_hash = ?, "
               "privacy_history = ?, "
               "sw_4o6_src_address = ?, "
#endif  // TERASTREAM
               "client_id = ?, "
               "valid_lifetime = ?, "
               "expire = ?, "
               "fqdn_fwd = ?, "
               "fqdn_rev = ?, "
               "hostname = ?, "
               "state = ?, "
               "user_context = ? "
               "WHERE hwaddr = ? "
               "AND subnet_id = ? "
               "AND address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {UPDATE_LEASE4_SUBNETID(),  //
             { UPDATE_LEASE4_SUBNETID(),  //
               "UPDATE lease4_subnetid SET "
#ifdef TERASTREAM
               "privacy_expire = ?, "
               "privacy_hash = ?, "
               "privacy_history = ?, "
#endif  // TERASTREAM
               "hwaddr = ?, "
               "client_id = ?, "
               "valid_lifetime = ?, "
               "expire = ?, "
               "fqdn_fwd = ?, "
               "fqdn_rev = ?, "
               "hostname = ?, "
               "state = ?, "
               "user_context = ? "
               "WHERE subnet_id = ? "
               "AND address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {DELETE_LEASE4_ADDRESS(),  //
             { DELETE_LEASE4_ADDRESS(),  //
               "DELETE FROM lease4_address "
               "WHERE address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {DELETE_LEASE4_CLIENTID_SUBNETID(),  //
             { DELETE_LEASE4_CLIENTID_SUBNETID(),  //
               "DELETE FROM lease4_clientid_subnetid "
               "WHERE client_id = ? "
               "AND subnet_id = ? "
               "AND address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {DELETE_LEASE4_HWADDR_SUBNETID(),  //
             { DELETE_LEASE4_HWADDR_SUBNETID(),  //
               "DELETE FROM lease4_hwaddr_subnetid "
               "WHERE hwaddr = ? "
               "AND subnet_id = ? "
               "AND address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {DELETE_LEASE4_SUBNETID(),  //
             { DELETE_LEASE4_SUBNETID(),  //
               "DELETE FROM lease4_subnetid "
               "WHERE subnet_id = ? "
               "AND address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

#else  // CASSANDRA_DENORMALIZED_TABLES

            // Inserts new IPv4 lease
            {INSERT_LEASE4(),
             {INSERT_LEASE4(), "INSERT INTO lease4( "
#ifdef TERASTREAM
                               "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
                               "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
                               "fqdn_fwd, fqdn_rev, hostname, state, user_context "
                               ") VALUES ( "
#ifdef TERASTREAM
                               "?, ?, ?, ?, "
#endif  // TERASTREAM
                               "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? "
                               ") "
#ifdef TERASTREAM_FULL_TRANSACTIONS
                               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
                               "IF NOT EXISTS "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Updates existing IPv4 lease
            {UPDATE_LEASE4(),
             {UPDATE_LEASE4(), "UPDATE lease4 SET "
#ifdef TERASTREAM
                               "privacy_expire = ?, "
                               "privacy_hash = ?, "
                               "privacy_history = ?, "
                               "sw_4o6_src_address = ?, "
#endif  // TERASTREAM
                               "hwaddr = ?, "
                               "client_id = ?, "
                               "subnet_id = ?, "
                               "valid_lifetime = ?, "
                               "expire = ?, "
                               "fqdn_fwd = ?, "
                               "fqdn_rev = ?, "
                               "hostname = ?, "
                               "state = ?, "
                               "user_context = ? "
                               "WHERE address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
                               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
                               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Deletes existing IPv4 lease
            {DELETE_LEASE4(),
             {DELETE_LEASE4(), "DELETE FROM lease4 "
                               "WHERE address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
                               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
                               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Gets up to a certain number of expired IPv4 leases
            {GET_LEASE4_EXPIRE(),
             {GET_LEASE4_EXPIRE(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
              "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
              "fqdn_fwd, fqdn_rev, hostname, state, user_context "
              "FROM lease4 "
              "WHERE state = ? "
              "AND expire < ? "
              "LIMIT ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
              "ALLOW FILTERING "
#endif  // CASSANDRA_DENORMALIZED_TABLES
             }},

#endif  // CASSANDRA_DENORMALIZED_TABLES

            // Gets an IPv4 lease(s)
            {GET_LEASE4(),
             {GET_LEASE4(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
              "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
              "fqdn_fwd, fqdn_rev, hostname, state, user_context "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4_address "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Gets an IPv4 lease with specified IPv4 address
            {GET_LEASE4_ADDR(),
             {GET_LEASE4_ADDR(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
              "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
              "fqdn_fwd, fqdn_rev, hostname, state, user_context "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4_address "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "WHERE address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Gets an IPv4 lease(s) with specified client-id
            {GET_LEASE4_CLIENTID(),
             {GET_LEASE4_CLIENTID(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
              "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
              "fqdn_fwd, fqdn_rev, hostname, state, user_context "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4_clientid_subnetid "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "WHERE client_id = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Gets an IPv4 lease with specified client-id and subnet-id
            {GET_LEASE4_CLIENTID_SUBID(),
             {GET_LEASE4_CLIENTID_SUBID(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
              "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
              "fqdn_fwd, fqdn_rev, hostname, state, user_context "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4_clientid_subnetid "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "WHERE client_id = ? "
              "AND subnet_id = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
              "ALLOW FILTERING "
#endif  // CASSANDRA_DENORMALIZED_TABLES
             }},

            // Gets all IPv4 leases with specified hardware address
            {GET_LEASE4_HWADDR(),
             {GET_LEASE4_HWADDR(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
              "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
              "fqdn_fwd, fqdn_rev, hostname, state, user_context "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4_hwaddr_subnetid "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "WHERE hwaddr = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Gets an IPv4 lease with specified hardware addr and subnet-id
            {GET_LEASE4_HWADDR_SUBID(),
             {GET_LEASE4_HWADDR_SUBID(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
              "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
              "fqdn_fwd, fqdn_rev, hostname, state, user_context "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4_hwaddr_subnetid "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "WHERE hwaddr = ? "
              "AND subnet_id = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
              "ALLOW FILTERING "
#endif  // CASSANDRA_DENORMALIZED_TABLES
             }},

            // Get range of lease4 from first lease with a limit (paging)
            {GET_LEASE4_LIMIT(),
             {GET_LEASE4_LIMIT(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
              "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
              "fqdn_fwd, fqdn_rev, hostname, state, user_context "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4_address "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "LIMIT ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Get range of lease4 from address with a limit (paging)
            {GET_LEASE4_PAGE(),
             {GET_LEASE4_PAGE(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
              "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
              "fqdn_fwd, fqdn_rev, hostname, state, user_context "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4_address "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "WHERE TOKEN(address) > TOKEN(?) "
              "LIMIT ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Gets an IPv4 lease(s) with specified subnet-id
            {GET_LEASE4_SUBID(),
             {GET_LEASE4_SUBID(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, sw_4o6_src_address, "
#endif  // TERASTREAM
              "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
              "fqdn_fwd, fqdn_rev, hostname, state, user_context "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4_subnetid "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease4 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "WHERE subnet_id = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Gets an IPv4 lease(s) with specified hostname
            {GET_LEASE4_HOSTNAME(),
             {GET_LEASE4_HOSTNAME(),
              "SELECT "
              "address, hwaddr, client_id, valid_lifetime, expire, subnet_id, "
              "fqdn_fwd, fqdn_rev, hostname, state, user_context "
              "FROM lease4 "
              "WHERE hostname = ? "
              "ALLOW FILTERING "}}};
        return _;
    }

    /// @brief Statement tags
    /// @{
#ifdef CASSANDRA_DENORMALIZED_TABLES
    static StatementTag &INSERT_LEASE4_ADDRESS() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &INSERT_LEASE4_CLIENTID_SUBNETID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &INSERT_LEASE4_HWADDR_SUBNETID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &INSERT_LEASE4_SUBNETID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &UPDATE_LEASE4_ADDRESS() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &UPDATE_LEASE4_CLIENTID_SUBNETID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &UPDATE_LEASE4_HWADDR_SUBNETID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &UPDATE_LEASE4_SUBNETID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &DELETE_LEASE4_ADDRESS() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &DELETE_LEASE4_CLIENTID_SUBNETID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &DELETE_LEASE4_HWADDR_SUBNETID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &DELETE_LEASE4_SUBNETID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
#else  // CASSANDRA_DENORMALIZED_TABLES
    // Add entry to lease4 table
    static StatementTag &INSERT_LEASE4() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Update a Lease4 entry
    static StatementTag &UPDATE_LEASE4() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Delete from lease4 by address
    static StatementTag &DELETE_LEASE4() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Delete expired lease4s in certain state
    static StatementTag &GET_LEASE4_EXPIRE() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
#endif  // CASSANDRA_DENORMALIZED_TABLES
    // Get lease4
    static StatementTag &GET_LEASE4() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Get lease4 by address
    static StatementTag &GET_LEASE4_ADDR() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Get lease4 by client ID
    static StatementTag &GET_LEASE4_CLIENTID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Get lease4 by client ID & subnet ID
    static StatementTag &GET_LEASE4_CLIENTID_SUBID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Get lease4 by HW address
    static StatementTag &GET_LEASE4_HWADDR() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Get lease4 by HW address & subnet ID
    static StatementTag &GET_LEASE4_HWADDR_SUBID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Get range of lease4 from first lease with a limit
    static StatementTag &GET_LEASE4_LIMIT() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Get range of lease4 from address with limit (paging)
    static StatementTag &GET_LEASE4_PAGE() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Get lease4 by subnet ID
    static StatementTag &GET_LEASE4_SUBID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Get lease4 by hostname
    static StatementTag &GET_LEASE4_HOSTNAME() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    /// @}

private:
    // Pointer to lease object
    Lease4Ptr lease_;
    // IPv4 address plus port
    cass_int64_t address_;
    // Client identification
    CassBlob client_id_;
#ifdef TERASTREAM
    /// @brief IPv6 address
    std::string sw_4o6_src_address_;
#endif  // TERASTREAM
};  // CqlLease4Exchange

CqlLease4Exchange::CqlLease4Exchange(CqlConnection const &connection)
    : CqlLeaseExchange(connection), address_(0) {
}

void CqlLease4Exchange::createBindForInsert(const Lease4Ptr &lease, AnyArray &data) {
    if (!lease) {
        isc_throw(BadValue, "CqlLease4Exchange::createBindForInsert(): "
                            "Lease4 object is NULL");
    }
    // Store lease object to ensure it remains valid.
    lease_ = lease;
    // Set up the structures for the various components of the lease4
    // structure.

    try {
        // address: bigint
        // The address in the Lease structure is an IOAddress object.
        // Convert this to an integer for storage.
        address_ = static_cast<cass_int64_t>(lease_->addr_.addressPlusPortToUint64());

        // hwaddr: blob
        if (lease_->hwaddr_ && lease_->hwaddr_->hwaddr_.size() > 0) {
            if (lease_->hwaddr_->hwaddr_.size() > HWAddr::MAX_HWADDR_LEN) {
                isc_throw(DbOperationError, "hardware address "
                                                << lease_->hwaddr_->toText() << " of length "
                                                << lease_->hwaddr_->hwaddr_.size()
                                                << " exceeds maximum allowed length of "
                                                << HWAddr::MAX_HWADDR_LEN);
            }
            hwaddr_ = lease_->hwaddr_->hwaddr_;
        } else {
#ifdef CASSANDRA_DENORMALIZED_TABLES
            hwaddr_ = NULL_HWADDR;
#else  // CASSANDRA_DENORMALIZED_TABLES
            hwaddr_.clear();
#endif  // CASSANDRA_DENORMALIZED_TABLES
        }

        // client_id: blob
        if (lease_->client_id_ && lease_->client_id_->getClientId().size() > 0) {
            client_id_ = lease_->client_id_->getClientId();
        } else {
#ifdef CASSANDRA_DENORMALIZED_TABLES
            client_id_ = NULL_CLIENTID;
#else  // CASSANDRA_DENORMALIZED_TABLES
            client_id_.clear();
#endif  // CASSANDRA_DENORMALIZED_TABLES
        }

        // valid lifetime: bigint
        valid_lifetime_ = static_cast<cass_int64_t>(lease_->valid_lft_);

        // expire: bigint
        // The lease structure holds the client last transmission time
        /// (cltt_)
        // For convenience for external tools, this is converted to lease
        // expiry time (expire). The relationship is given by:
        // expire = cltt_ + valid_lft_
        CqlExchange<Lease4>::convertToDatabaseTime(lease_->cltt_, lease_->valid_lft_, expire_);

        // subnet_id: int
        subnet_id_ = static_cast<cass_int32_t>(lease_->subnet_id_);

        // fqdn_fwd: boolean
        fqdn_fwd_ = lease_->fqdn_fwd_ ? cass_true : cass_false;

        // fqdn_rev: boolean
        fqdn_rev_ = lease_->fqdn_rev_ ? cass_true : cass_false;

        // hostname: varchar
        if (lease_->hostname_.size() > HOSTNAME_MAX_LEN) {
            isc_throw(BadValue,
                      "hostname " << lease_->hostname_ << " of length " << lease_->hostname_.size()
                                  << " exceeds maximum allowed length of " << HOSTNAME_MAX_LEN);
        }
        hostname_ = lease_->hostname_;

#ifdef TERASTREAM
        // privacy_expire: bigint
        privacy_expire_ = static_cast<cass_int64_t>(lease_->privacy_expire_);

        // privacy_hash: blob
        if (lease_->privacy_hash_) {
            privacy_hash_ = *lease_->privacy_hash_;
        } else {
#ifdef CASSANDRA_DENORMALIZED_TABLES
            privacy_hash_ = NULL_PRIVACY_HASH;
#else  // CASSANDRA_DENORMALIZED_TABLES
            privacy_hash_.clear();
#endif  // CASSANDRA_DENORMALIZED_TABLES
        }

        // privacy_history: varchar
        privacy_history_ = lease_->privacy_history_;

        // sw_4o6_src_address: varchar
        sw_4o6_src_address_ = lease_->sw_4o6_src_address_.toText();
        if (sw_4o6_src_address_.size() > ADDRESS6_TEXT_MAX_LEN) {
            isc_throw(BadValue, "address " << sw_4o6_src_address_ << " of length "
                                           << sw_4o6_src_address_.size()
                                           << " exceeds maximum allowed length of "
                                           << ADDRESS6_TEXT_MAX_LEN);
        }
#endif  // TERASTREAM

        // state: int
        state_ = static_cast<cass_int32_t>(lease_->state_);

        // user_context: text
        ElementPtr ctx = lease_->getContext();
        if (ctx) {
            user_context_ = ctx->str();
        } else {
            user_context_ = NULL_USER_CONTEXT;
        }

        // Start with a fresh array.
        data.clear();
#ifdef TERASTREAM
        data.add(&privacy_expire_);
        data.add(&privacy_hash_);
        data.add(&privacy_history_);
        data.add(&sw_4o6_src_address_);
#endif  // TERASTREAM
        data.add(&address_);
        data.add(&hwaddr_);
        data.add(&client_id_);
        data.add(&valid_lifetime_);
        data.add(&expire_);
        data.add(&subnet_id_);
        data.add(&fqdn_fwd_);
        data.add(&fqdn_rev_);
        data.add(&hostname_);
        data.add(&state_);
        data.add(&user_context_);

#ifdef TERASTREAM_FULL_TRANSACTIONS
        txid_ = connection_.getTransactionID();
        data.add(&txid_);
#endif  // TERASTREAM_FULL_TRANSACTIONS
    } catch (const Exception &ex) {
        isc_throw(DbOperationError, "CqlLease4Exchange::createBindForInsert(): "
                                    "could not create bind array from Lease4: "
                                        << lease_->addr_.toText() << ", reason: " << ex.what());
    }
}

void CqlLease4Exchange::createBindForUpdate(const Lease4Ptr &lease,
                                            AnyArray &data,
                                            StatementTag statement_tag) {
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
    // We only need statement_tag to know which table to update which is obvious
    //      when there is a single table.
    (void)statement_tag;  // [maybe_unused]
#endif  // CASSANDRA_DENORMALIZED_TABLES

    if (!lease) {
        isc_throw(BadValue, "CqlLease4Exchange::createBindForUpdate(): "
                            "Lease4 object is NULL");
    }
    // Store lease object to ensure it remains valid.
    lease_ = lease;
    // Set up the structures for the various components of the lease4
    // structure.

    try {
        // address: bigint
        // The address in the Lease structure is an IOAddress object.
        // Convert this to an integer for storage.
        address_ = static_cast<cass_int64_t>(lease_->addr_.addressPlusPortToUint64());

        // hwaddr: blob
        if (lease_->hwaddr_ && lease_->hwaddr_->hwaddr_.size() > 0) {
            if (lease_->hwaddr_->hwaddr_.size() > HWAddr::MAX_HWADDR_LEN) {
                isc_throw(DbOperationError, "hardware address "
                                                << lease_->hwaddr_->toText() << " of length "
                                                << lease_->hwaddr_->hwaddr_.size()
                                                << " exceeds maximum allowed length of "
                                                << HWAddr::MAX_HWADDR_LEN);
            }
            hwaddr_ = lease_->hwaddr_->hwaddr_;
        } else {
#ifdef CASSANDRA_DENORMALIZED_TABLES
            hwaddr_ = NULL_HWADDR;
#else  // CASSANDRA_DENORMALIZED_TABLES
            hwaddr_.clear();
#endif  // CASSANDRA_DENORMALIZED_TABLES
        }

        // client_id: blob
        if (lease_->client_id_ && lease_->client_id_->getClientId().size() > 0) {
            client_id_ = lease_->client_id_->getClientId();
        } else {
#ifdef CASSANDRA_DENORMALIZED_TABLES
            client_id_ = NULL_CLIENTID;
#else  // CASSANDRA_DENORMALIZED_TABLES
            client_id_.clear();
#endif  // CASSANDRA_DENORMALIZED_TABLES
        }

        // valid lifetime: bigint
        valid_lifetime_ = static_cast<cass_int64_t>(lease_->valid_lft_);

        // expire: bigint
        // The lease structure holds the client last transmission time
        /// (cltt_)
        // For convenience for external tools, this is converted to lease
        // expiry time (expire). The relationship is given by:
        // expire = cltt_ + valid_lft_
        CqlExchange<Lease4>::convertToDatabaseTime(lease_->cltt_, lease_->valid_lft_, expire_);

        // subnet_id: int
        subnet_id_ = static_cast<cass_int32_t>(lease_->subnet_id_);

        // fqdn_fwd: boolean
        fqdn_fwd_ = lease_->fqdn_fwd_ ? cass_true : cass_false;

        // fqdn_rev: boolean
        fqdn_rev_ = lease_->fqdn_rev_ ? cass_true : cass_false;

        // hostname: varchar
        if (lease_->hostname_.size() > HOSTNAME_MAX_LEN) {
            isc_throw(BadValue,
                      "hostname " << lease_->hostname_ << " of length " << lease_->hostname_.size()
                                  << " exceeds maximum allowed length of " << HOSTNAME_MAX_LEN);
        }
        hostname_ = lease_->hostname_;

#ifdef TERASTREAM
        // privacy_expire: bigint
        privacy_expire_ = static_cast<cass_int64_t>(lease_->privacy_expire_);

        // privacy_hash: blob
        if (lease_->privacy_hash_) {
            privacy_hash_ = *lease_->privacy_hash_;
        } else {
#ifdef CASSANDRA_DENORMALIZED_TABLES
            privacy_hash_ = NULL_PRIVACY_HASH;
#else  // CASSANDRA_DENORMALIZED_TABLES
            privacy_hash_.clear();
#endif  // CASSANDRA_DENORMALIZED_TABLES
        }

        // privacy_history: varchar
        privacy_history_ = lease_->privacy_history_;

        // sw_4o6_src_address: varchar
        sw_4o6_src_address_ = lease_->sw_4o6_src_address_.toText();
        if (sw_4o6_src_address_.size() > ADDRESS6_TEXT_MAX_LEN) {
            isc_throw(BadValue, "address " << sw_4o6_src_address_ << " of length "
                                           << sw_4o6_src_address_.size()
                                           << " exceeds maximum allowed length of "
                                           << ADDRESS6_TEXT_MAX_LEN);
        }
#endif  // TERASTREAM

        // state: int
        state_ = static_cast<cass_int32_t>(lease_->state_);

        // user_context: text
        ElementPtr ctx = lease_->getContext();
        if (ctx) {
            user_context_ = ctx->str();
        } else {
            user_context_ = NULL_USER_CONTEXT;
        }

        // Start with a fresh array.
        data.clear();
#ifdef TERASTREAM
        data.add(&privacy_expire_);
        data.add(&privacy_hash_);
        data.add(&privacy_history_);
        data.add(&sw_4o6_src_address_);
#endif  // TERASTREAM
#ifdef CASSANDRA_DENORMALIZED_TABLES
        if (std::strcmp(statement_tag, UPDATE_LEASE4_HWADDR_SUBNETID) != 0) {
            data.add(&hwaddr_);
        }
        if (std::strcmp(statement_tag, UPDATE_LEASE4_CLIENTID_SUBNETID) != 0) {
            data.add(&client_id_);
        }
        if (std::strcmp(statement_tag, UPDATE_LEASE4_HWADDR_SUBNETID) != 0 &&
            std::strcmp(statement_tag, UPDATE_LEASE4_CLIENTID_SUBNETID) != 0 &&
            std::strcmp(statement_tag, UPDATE_LEASE4_SUBNETID) != 0) {
            data.add(&subnet_id_);
        }
#else  // CASSANDRA_DENORMALIZED_TABLES
        data.add(&hwaddr_);
        data.add(&client_id_);
        data.add(&subnet_id_);
#endif  // CASSANDRA_DENORMALIZED_TABLES
        data.add(&valid_lifetime_);
        data.add(&expire_);
        data.add(&fqdn_fwd_);
        data.add(&fqdn_rev_);
        data.add(&hostname_);
        data.add(&state_);
        data.add(&user_context_);
#ifdef CASSANDRA_DENORMALIZED_TABLES
        if (std::strcmp(statement_tag, UPDATE_LEASE4_CLIENTID_SUBNETID) == 0) {
            data.add(&client_id_);
            data.add(&subnet_id_);
        } else if (std::strcmp(statement_tag, UPDATE_LEASE4_HWADDR_SUBNETID) == 0) {
            data.add(&hwaddr_);
            data.add(&subnet_id_);
        } else if (std::strcmp(statement_tag, UPDATE_LEASE4_SUBNETID) == 0) {
            data.add(&subnet_id_);
        }
#endif  // CASSANDRA_DENORMALIZED_TABLES
        data.add(&address_);

#ifdef TERASTREAM_FULL_TRANSACTIONS
        txid_ = connection_.getTransactionID();
        data.add(&txid_);
#else  // TERASTREAM_FULL_TRANSACTIONS
        CqlExchange::convertToDatabaseTime(lease_->old_cltt_, lease_->old_valid_lft_, old_expire_);
        data.add(&old_expire_);
#endif  // TERASTREAM_FULL_TRANSACTIONS
    } catch (const Exception &ex) {
        isc_throw(DbOperationError, "CqlLease4Exchange::createBindUpdate(): "
                                    "could not create bind array from Lease4: "
                                        << lease_->addr_.toText() << ", reason: " << ex.what());
    }
}

void CqlLease4Exchange::createBindForDelete(const Lease4Ptr &lease,
                                            AnyArray &data,
                                            StatementTag statement_tag /* = "" */) {
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
    // We only need statement_tag to know which table to update which is obvious
    //      when there is a single table.
    (void)statement_tag;  // [maybe_unused]
#endif  // CASSANDRA_DENORMALIZED_TABLES

    if (!lease) {
        isc_throw(BadValue, "CqlLease4Exchange::createBindForDelete(): "
                            "Lease4 object is NULL");
    }
    // Store lease object to ensure it remains valid.
    lease_ = lease;
    // Set up the structures for the various components of the lease4
    // structure.

    try {
        // address: bigint
        address_ = static_cast<cass_int64_t>(lease_->addr_.addressPlusPortToUint64());

        // hwaddr: blob
        if (lease_->hwaddr_ && lease_->hwaddr_->hwaddr_.size() > 0) {
            if (lease_->hwaddr_->hwaddr_.size() > HWAddr::MAX_HWADDR_LEN) {
                isc_throw(DbOperationError, "hardware address "
                                                << lease_->hwaddr_->toText() << " of length "
                                                << lease_->hwaddr_->hwaddr_.size()
                                                << " exceeds maximum allowed length of "
                                                << HWAddr::MAX_HWADDR_LEN);
            }
            hwaddr_ = lease_->hwaddr_->hwaddr_;
        } else {
#ifdef CASSANDRA_DENORMALIZED_TABLES
            hwaddr_ = NULL_HWADDR;
#else  // CASSANDRA_DENORMALIZED_TABLES
            hwaddr_.clear();
#endif  // CASSANDRA_DENORMALIZED_TABLES
        }

        // client_id: blob
        if (lease_->client_id_ && lease_->client_id_->getClientId().size() > 0) {
            client_id_ = lease_->client_id_->getClientId();
        } else {
#ifdef CASSANDRA_DENORMALIZED_TABLES
            client_id_ = NULL_CLIENTID;
#else  // CASSANDRA_DENORMALIZED_TABLES
            client_id_.clear();
#endif  // CASSANDRA_DENORMALIZED_TABLES
        }

        // subnet_id: int
        subnet_id_ = static_cast<cass_int32_t>(lease_->subnet_id_);

        // Start with a fresh array.
        data.clear();
#ifdef CASSANDRA_DENORMALIZED_TABLES
        if (std::strcmp(statement_tag, DELETE_LEASE4_CLIENTID_SUBNETID) == 0) {
            data.add(&client_id_);
            data.add(&subnet_id_);
        } else if (std::strcmp(statement_tag, DELETE_LEASE4_HWADDR_SUBNETID) == 0) {
            data.add(&hwaddr_);
            data.add(&subnet_id_);
        } else if (std::strcmp(statement_tag, DELETE_LEASE4_SUBNETID) == 0) {
            data.add(&subnet_id_);
        }
#endif  // CASSANDRA_DENORMALIZED_TABLES
        data.add(&address_);

#ifdef TERASTREAM_FULL_TRANSACTIONS
        txid_ = connection_.getTransactionID();
        data.add(&txid_);
#else  // TERASTREAM_FULL_TRANSACTIONS
        CqlExchange::convertToDatabaseTime(lease_->old_cltt_, lease_->old_valid_lft_, old_expire_);
        data.add(&old_expire_);
#endif  // TERASTREAM_FULL_TRANSACTIONS
    } catch (const Exception &ex) {
        isc_throw(DbOperationError, "CqlLease4Exchange::createBindForDelete(): "
                                    "could not create bind array from Lease4: "
                                        << lease_->addr_.toText() << ", reason: " << ex.what());
    }
}

void CqlLease4Exchange::createBindForSelect(AnyArray &data, StatementTag const & /* unused */) {
    // Start with a fresh array.
    data.clear();

#ifdef TERASTREAM
    // privacy_expire: bigint
    data.add(&privacy_expire_);

    // privacy_hash: blob
    data.add(&privacy_hash_);

    // privacy_history: varchar
    data.add(&privacy_history_);

    // sw_4o6_src_address: varchar
    data.add(&sw_4o6_src_address_);
#endif  // TERASTREAM

    // address: bigint
    data.add(&address_);

    // hwaddr: blob
    data.add(&hwaddr_);

    // client_id: blob
    data.add(&client_id_);

    // valid_lifetime: bigint
    data.add(&valid_lifetime_);

    // expire: bigint
    data.add(&expire_);

    // subnet_id: int
    data.add(&subnet_id_);

    // fqdn_fwd: boolean
    data.add(&fqdn_fwd_);

    // fqdn_rev: boolean
    data.add(&fqdn_rev_);

    // hostname: varchar
    data.add(&hostname_);

    // state: int
    data.add(&state_);

    // user_context: text
    data.add(&user_context_);
}

Lease4Ptr CqlLease4Exchange::retrieve() {
    try {
        // Sanity checks
        if (hwaddr_.size() > HWAddr::MAX_HWADDR_LEN) {
            isc_throw(BadValue, "hardware address " << HWAddr(hwaddr_, HTYPE_ETHER).toText()
                                                    << " of length " << hwaddr_.size()
                                                    << " exceeds maximum allowed length of "
                                                    << HWAddr::MAX_HWADDR_LEN);
        }
        if (client_id_.size() > ClientId::MAX_CLIENT_ID_LEN) {
            isc_throw(BadValue, "client ID " << ClientId(client_id_).toText() << " of length "
                                             << client_id_.size()
                                             << " exceeds maximum allowed length of "
                                             << ClientId::MAX_CLIENT_ID_LEN);
        }
        if (hostname_.size() > HOSTNAME_MAX_LEN) {
            isc_throw(BadValue, "hostname" << hostname_ << " of length " << hostname_.size()
                                           << " exceeds maximum allowed length of "
                                           << HOSTNAME_MAX_LEN);
        }
#ifdef TERASTREAM
        if (privacy_hash_.size() > Lease::MAX_PRIVACY_HASH_LEN) {
            isc_throw(BadValue, "privacy_hash" << DUID(privacy_hash_).toText() << " of length "
                                               << privacy_hash_.size()
                                               << " exceeds maximum allowed length of "
                                               << Lease::MAX_PRIVACY_HASH_LEN);
        }
#endif  // TERASTREAM

        time_t cltt = 0;
        CqlExchange<Lease4>::convertFromDatabaseTime(expire_, valid_lifetime_, cltt);

        HWAddrPtr hwaddr = std::make_shared<HWAddr>(hwaddr_, HTYPE_ETHER);

        uint64_t addr4 = static_cast<uint64_t>(address_);

#ifdef CASSANDRA_DENORMALIZED_TABLES
        if (hwaddr->hwaddr_ == NULL_HWADDR) {
            hwaddr->hwaddr_.clear();
        }

        if (client_id_ == NULL_CLIENTID) {
            client_id_.clear();
        }
#endif  // CASSANDRA_DENORMALIZED_TABLES

        ElementPtr ctx;
        if (!user_context_.empty()) {
            ctx = Element::fromJSON(user_context_);
            if (!ctx || (ctx->getType() != Element::map)) {
                isc_throw(BadValue, "user context '" << user_context_ << "' is not a JSON map");
            }
        }

        Lease4Ptr result(std::make_shared<Lease4>(addr4, hwaddr, client_id_.data(),
                                                  client_id_.size(), valid_lifetime_, cltt,
                                                  subnet_id_, fqdn_fwd_, fqdn_rev_, hostname_));

#ifdef TERASTREAM
        result->privacy_expire_ = privacy_expire_;
#ifdef CASSANDRA_DENORMALIZED_TABLES
        if (privacy_hash_ == NULL_PRIVACY_HASH) {
            privacy_hash_.clear();
        }
#endif  // CASSANDRA_DENORMALIZED_TABLES
        result->privacy_hash_ = std::make_shared<PrivacyHash>(privacy_hash_);

        result->privacy_history_ = privacy_history_;

        if (sw_4o6_src_address_.size() > ADDRESS6_TEXT_MAX_LEN) {
            isc_throw(BadValue, "address " << sw_4o6_src_address_ << " of length "
                                           << sw_4o6_src_address_.size()
                                           << " exceeds maximum allowed length of "
                                           << ADDRESS6_TEXT_MAX_LEN);
        }

        result->sw_4o6_src_address_ = IOAddress(sw_4o6_src_address_);
#endif  // TERASTREAM

        result->state_ = state_;

        if (ctx) {
            result->setContext(ctx);
        }

        return (result);
    } catch (const Exception &ex) {
        isc_throw(DbOperationError, "CqlLease4Exchange::retrieve(): "
                                    "could not convert data to Lease4, reason: "
                                        << ex.what());
    }
}

void CqlLease4Exchange::getLeaseCollection(StatementTag const &statement_tag,
                                           AnyArray &data,
                                           Lease4Collection &result) {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_ADDR4).arg(statement_tag);

    result = executeSelect(connection_, data, statement_tag);
}

void CqlLease4Exchange::getLease(StatementTag const &statement_tag,
                                 AnyArray &data,
                                 Lease4Ptr &result) {
    // This particular method is called when only one or zero matches is
    // expected.
    Lease4Collection collection;
    getLeaseCollection(statement_tag, data, collection);

    // Return single record if present, else clear the lease.
    const size_t collection_size = collection.size();
    if (collection_size >= 2u) {
        isc_throw(MultipleRecords, "CqlLease4Exchange::getLease(): multiple records were found in "
                                   "the database where only one was expected for statement "
                                       << statement_tag);
    } else if (collection_size == 0u) {
        result.reset();
    } else {
        result = collection.front();
    }
}

void CqlLease4Exchange::getExpiredLeases(const size_t &max_leases,
                                         Lease4Collection &expired_leases) {
#ifdef CASSANDRA_DENORMALIZED_TABLES
    // This functions is disabled.
    (void)max_leases;  // [maybe_unused]
    (void)expired_leases;  // [maybe_unused]
#else  // CASSANDRA_DENORMALIZED_TABLES
    // Set up the WHERE clause value
    cass_int32_t keep_state = Lease::STATE_EXPIRED_RECLAIMED;
    cass_int64_t timestamp = static_cast<cass_int64_t>(time(NULL));

    // If the number of leases is 0, we will return all leases. This is
    // achieved by setting the limit to a very high value.
    cass_int32_t limit = max_leases > 0u ? static_cast<cass_int32_t>(max_leases) :
                                           std::numeric_limits<cass_int32_t>::max();

    for (cass_int32_t state = Lease::STATE_DEFAULT; state <= Lease::STATE_EXPIRED_RECLAIMED;
         state++) {
        if (state == keep_state) {
            continue;
        }

        AnyArray data;
        data.add(&state);
        data.add(&timestamp);
        data.add(&limit);

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection_.getTransactionID());
        data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        // Retrieve leases from the database.
        Lease4Collection temp_collection;
        getLeaseCollection(CqlLease4Exchange::GET_LEASE4_EXPIRE(), data, temp_collection);

        for (Lease4Ptr &lease : temp_collection) {
            expired_leases.push_back(lease);
        }
    }
#endif  // CASSANDRA_DENORMALIZED_TABLES
}

/// @brief Exchange Lease6 information between Kea and CQL
///
/// On any CQL operation, arrays of CQL BIND structures must be built to
/// describe the parameters in the prepared statements. Where information is
/// inserted or retrieved - INSERT, UPDATE, SELECT - a large amount of that
/// structure is identical. This class handles the creation of that array.
///
/// Owing to the CQL API, the process requires some intermediate variables
/// to hold things like data length etc. This object holds those variables.
///
/// @note There are no unit tests for this class. It is tested indirectly
/// in all CqlLeaseMgr::xxx6() calls where it is used.
struct CqlLease6Exchange : CqlLeaseExchange<Lease6> {
    /// @brief Constructor
    ///
    /// The initialization of the variables here is only to satisfy
    /// cppcheck - all variables are initialized/set in the methods before
    /// they are used.
    ///
    /// @param connection connection used for this query
    explicit CqlLease6Exchange(CqlConnection const &connection);

    /// @brief Create CQL_BIND objects for Lease6 Pointer
    ///
    /// Fills in the CQL_BIND array for sending data in the Lease6 object to
    /// the database. Used for INSERT statements.
    ///
    /// @param lease The lease information to be inserted
    /// @param data Lease info will be stored here in CQL format
    void createBindForInsert(const Lease6Ptr &lease, AnyArray &data);

    /// @brief Create CQL_BIND objects for Lease6 Pointer
    ///
    /// Fills in the CQL_BIND array for sending data in the Lease6 object to
    /// the database. Used for UPDATE statements.
    ///
    /// @param lease Updated lease information.
    /// @param data lease info in CQL format will be stored here
    /// @param statement_tag tag identifying the query (optional)
    void createBindForUpdate(const Lease6Ptr &lease,
                             AnyArray &data,
                             StatementTag statement_tag = StatementTag());

    /// @brief Create CQL_BIND objects for Lease4 Pointer
    ///
    /// Fills in the CQL_BIND array for sending data in the Lease6 object to
    /// the database. Used for DELETE statements.
    ///
    /// @param lease lease to be deleted
    /// @param data lease info in CQL format will be stored here
    /// @param statement_tag tag identifying the query (optional)
    void createBindForDelete(const Lease6Ptr &lease,
                             AnyArray &data,
                             StatementTag statement_tag = StatementTag());

    /// @brief Create BIND array to receive data
    ///
    /// Creates a CQL_BIND array to receive Lease6 data from the database.
    ///
    /// @param data info returned by CQL will be stored here
    /// @param statement_tag tag identifying the query (optional)
    void createBindForSelect(AnyArray &data,
                             StatementTag const &statement_tag = StatementTag()) override final;

    /// @brief Retrieves the Lease6 object in Kea format
    ///
    /// @return C++ representation of the object being returned
    Lease6Ptr retrieve() override final;

    /// @brief Retrieves zero or more IPv6 leases
    ///
    /// @param statement_tag query to be executed
    /// @param data parameters for the query
    /// @param result this lease collection will be updated
    void
    getLeaseCollection(StatementTag const &statement_tag, AnyArray &data, Lease6Collection &result);

    /// @brief Retrieves one IPv6 lease
    ///
    /// @param statement_tag query to be executed
    /// @param data parameters for the query
    /// @param result pointer to the lease being returned (or null)
    void getLease(StatementTag const &statement_tag, AnyArray &data, Lease6Ptr &result);

    /// @brief Returns expired leases.
    ///
    /// This method returns up to specified number (see max_leases) of
    /// expired leases.
    ///
    /// @param max_leases at most this number of leases will be returned
    /// @param expired_leases expired leases will be stored here
    void getExpiredLeases(const size_t &max_leases, Lease6Collection &expired_leases);

    static StatementMap &tagged_statements() {
        static StatementMap _{
#ifdef CASSANDRA_DENORMALIZED_TABLES
            {INSERT_LEASE6_DUID_IAID_LEASETYPE_SUBNETID(),  //
             { INSERT_LEASE6_DUID_IAID_LEASETYPE_SUBNETID(),  //
               "INSERT INTO lease6_duid_iaid_leasetype_subnetid( "
#ifdef TERASTREAM
               "privacy_expire, privacy_hash, privacy_history, "
#endif  // TERASTREAM
               "address, valid_lifetime, expire, subnet_id, pref_lifetime, duid, iaid, "
               "lease_type, prefix_len, fqdn_fwd, fqdn_rev, hostname, hwaddr, hwtype, "
               "hwaddr_source, state, user_context, pool_id "
               ") VALUES ( "
#ifdef TERASTREAM
               "?, ?, ?, "
#endif  // TERASTREAM
               "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? "
               ") "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF NOT EXISTS "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {INSERT_LEASE6_ADDRESS_LEASETYPE(),  //
             { INSERT_LEASE6_ADDRESS_LEASETYPE(),  //
               "INSERT INTO lease6_address_leasetype( "
#ifdef TERASTREAM
               "privacy_expire, privacy_hash, privacy_history, "
#endif  // TERASTREAM
               "address, valid_lifetime, expire, subnet_id, pref_lifetime, duid, iaid, "
               "lease_type, prefix_len, fqdn_fwd, fqdn_rev, hostname, hwaddr, hwtype, "
               "hwaddr_source, state, user_context, pool_id "
               ") VALUES ( "
#ifdef TERASTREAM
               "?, ?, ?, "
#endif  // TERASTREAM
               "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? "
               ") "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF NOT EXISTS "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {UPDATE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID(),  //
             { UPDATE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID(),  //
               "UPDATE lease6_duid_iaid_leasetype_subnetid SET "
#ifdef TERASTREAM
               "privacy_expire = ?, "
               "privacy_hash = ?, "
               "privacy_history = ?, "
#endif  // TERASTREAM
               "valid_lifetime = ?, "
               "expire = ?, "
               "pref_lifetime = ?, "
               "prefix_len = ?, "
               "fqdn_fwd = ?, "
               "fqdn_rev = ?, "
               "hostname = ?, "
               "hwaddr = ?, "
               "hwtype = ?, "
               "hwaddr_source = ?, "
               "state = ?, "
               "user_context = ?, "
               "pool_id = ? "
               "WHERE address = ? "
               "AND lease_type = ? "
               "AND duid = ? "
               "AND iaid = ? "
               "AND subnet_id = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {UPDATE_LEASE6_ADDRESS_LEASETYPE(),  //
             { UPDATE_LEASE6_ADDRESS_LEASETYPE(),  //
               "UPDATE lease6_address_leasetype SET "
#ifdef TERASTREAM
               "privacy_expire = ?, "
               "privacy_hash = ?, "
               "privacy_history = ?, "
#endif  // TERASTREAM
               "valid_lifetime = ?, "
               "expire = ?, "
               "pref_lifetime = ?, "
               "duid = ?, "
               "iaid = ?, "
               "subnet_id = ?, "
               "prefix_len = ?, "
               "fqdn_fwd = ?, "
               "fqdn_rev = ?, "
               "hostname = ?, "
               "hwaddr = ?, "
               "hwtype = ?, "
               "hwaddr_source = ?, "
               "state = ?, "
               "user_context = ?, "
               "pool_id = ? "
               "WHERE address = ? "
               "AND lease_type = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {DELETE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID(),  //
             { DELETE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID(),  //
               "DELETE FROM lease6_duid_iaid_leasetype_subnetid "
               "WHERE address = ? "
               "AND lease_type = ? "
               "AND duid = ? "
               "AND iaid = ? "
               "AND subnet_id = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            {DELETE_LEASE6_ADDRESS_LEASETYPE(),  //
             { DELETE_LEASE6_ADDRESS_LEASETYPE(),  //
               "DELETE FROM lease6_address_leasetype "
               "WHERE address = ? "
               "AND lease_type = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

#else  // CASSANDRA_DENORMALIZED_TABLES

            // Inserts new IPv6 lease
            {INSERT_LEASE6(),
             {INSERT_LEASE6(),
              "INSERT INTO lease6("
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, "
#endif  // TERASTREAM
              "address, valid_lifetime, expire, subnet_id, pref_lifetime, duid, iaid, "
              "lease_type, prefix_len, fqdn_fwd, fqdn_rev, hostname, hwaddr, hwtype, "
              "hwaddr_source, state, user_context, allocation_time, pool_id "
              ") VALUES ("
#ifdef TERASTREAM
              "?, ?, ?, "
#endif  // TERASTREAM
              "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? "
              ") "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
              "IF NOT EXISTS "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Updates existing IPv6 lease
            {UPDATE_LEASE6(),
             {UPDATE_LEASE6(), "UPDATE lease6 SET "
#ifdef TERASTREAM
                               "privacy_expire = ?, "
                               "privacy_hash = ?, "
                               "privacy_history = ?, "
#endif  // TERASTREAM
                               "valid_lifetime = ?, "
                               "expire = ?, "
                               "pref_lifetime = ?, "
                               "duid = ?, "
                               "iaid = ?, "
                               "subnet_id = ?, "
                               "lease_type = ?, "
                               "prefix_len = ?, "
                               "fqdn_fwd = ?, "
                               "fqdn_rev = ?, "
                               "hostname = ?, "
                               "hwaddr = ?, "
                               "hwtype = ?, "
                               "hwaddr_source = ?, "
                               "state = ?, "
                               "user_context = ?, "
                               "pool_id = ? "
                               "WHERE address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
                               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
                               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Deletes existing IPv6 lease
            {DELETE_LEASE6(),
             {DELETE_LEASE6(), "DELETE FROM lease6 "
                               "WHERE address = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
                               "IN TXN ? "
#else  // TERASTREAM_FULL_TRANSACTIONS
                               "IF expire = ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Gets up to a certain number of expired IPv6 leases
            {GET_LEASE6_EXPIRE(),
             {GET_LEASE6_EXPIRE(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, "
#endif  // TERASTREAM
              "address, valid_lifetime, expire, subnet_id, pref_lifetime, duid, iaid, "
              "lease_type, prefix_len, fqdn_fwd, fqdn_rev, hostname, hwaddr, hwtype, "
              "hwaddr_source, state, user_context, allocation_time, pool_id "
              "FROM lease6 "
              "WHERE state = ? "
              "AND expire < ? "
              "LIMIT ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
              "ALLOW FILTERING "
#endif  // CASSANDRA_DENORMALIZED_TABLES
             }},

#endif  // CASSANDRA_DENORMALIZED_TABLES

            // Gets an IPv6 lease with specified IPv6 address
            {GET_LEASE6_ADDR(),
             {GET_LEASE6_ADDR(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, "
#endif  // TERASTREAM
              "address, valid_lifetime, expire, subnet_id, pref_lifetime, duid, iaid, "
              "lease_type, prefix_len, fqdn_fwd, fqdn_rev, hostname, hwaddr, hwtype, "
              "hwaddr_source, state, user_context, allocation_time, pool_id "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease6_address_leasetype "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease6 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "WHERE address = ? "
              "AND lease_type = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Gets an IPv6 lease with specified duid
            {GET_LEASE6_DUID(),
             {GET_LEASE6_DUID(),
              "SELECT "
              "address, valid_lifetime, expire, subnet_id, pref_lifetime, duid, iaid, "
              "lease_type, prefix_len, fqdn_fwd, fqdn_rev, hostname, hwaddr, hwtype, "
              "hwaddr_source, state, user_context, allocation_time, pool_id "
              "FROM lease6 "
              "WHERE duid = ? "}},

            // Gets an IPv6 lease(s) with specified duid and iaid
            {GET_LEASE6_DUID_IAID(),
             {GET_LEASE6_DUID_IAID(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, "
#endif  // TERASTREAM
              "address, valid_lifetime, expire, subnet_id, pref_lifetime, duid, iaid, "
              "lease_type, prefix_len, fqdn_fwd, fqdn_rev, hostname, hwaddr, hwtype, "
              "hwaddr_source, state, user_context, allocation_time, pool_id "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease6_duid_iaid_leasetype_subnetid "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease6 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "WHERE duid = ? AND iaid = ? "
              "AND lease_type = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
              "ALLOW FILTERING "
#endif  // CASSANDRA_DENORMALIZED_TABLES
             }},

            // Gets an IPv6 lease with specified duid, iaid and subnet-id
            {GET_LEASE6_DUID_IAID_SUBID(),
             {GET_LEASE6_DUID_IAID_SUBID(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, "
#endif  // TERASTREAM
              "address, valid_lifetime, expire, subnet_id, pref_lifetime, duid, iaid, "
              "lease_type, prefix_len, fqdn_fwd, fqdn_rev, hostname, hwaddr, hwtype, "
              "hwaddr_source, state, user_context, allocation_time, pool_id "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease6_duid_iaid_leasetype_subnetid "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease6 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "WHERE duid = ? AND iaid = ? "
              "AND lease_type = ? "
              "AND subnet_id = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
              "ALLOW FILTERING "
#endif  // CASSANDRA_DENORMALIZED_TABLES
             }},

            // Get range of IPv6 leases from first lease with a limit (paging)
            {GET_LEASE6_LIMIT(),
             {GET_LEASE6_LIMIT(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, "
#endif  // TERASTREAM
              "address, valid_lifetime, expire, subnet_id, pref_lifetime, duid, iaid, "
              "lease_type, prefix_len, fqdn_fwd, fqdn_rev, hostname, hwaddr, hwtype, "
              "hwaddr_source, state, user_context, allocation_time, pool_id "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease6_address_leasetype "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease6 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "LIMIT ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Get range of IPv6 leases from address with a limit (paging)
            {GET_LEASE6_PAGE(),
             {GET_LEASE6_PAGE(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, "
#endif  // TERASTREAM
              "address, valid_lifetime, expire, subnet_id, pref_lifetime, duid, iaid, "
              "lease_type, prefix_len, fqdn_fwd, fqdn_rev, hostname, hwaddr, hwtype, "
              "hwaddr_source, state, user_context, allocation_time, pool_id "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease6_address_leasetype "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease6 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "WHERE TOKEN(address) > TOKEN(?) "
              "LIMIT ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Gets an IPv6 lease(s) with specified subnet-id
            {GET_LEASE6_SUBID(),
             {GET_LEASE6_SUBID(),
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "@free = "
#endif  // TERASTREAM_FULL_TRANSACTIONS
              "SELECT "
#ifdef TERASTREAM
              "privacy_expire, privacy_hash, privacy_history, "
#endif  // TERASTREAM
              "address, valid_lifetime, expire, subnet_id, pref_lifetime, duid, iaid, "
              "lease_type, prefix_len, fqdn_fwd, fqdn_rev, hostname, hwaddr, hwtype, "
              "hwaddr_source, state, user_context, allocation_time, pool_id "
#ifdef CASSANDRA_DENORMALIZED_TABLES
              "FROM lease6_subnetid "
#else  // CASSANDRA_DENORMALIZED_TABLES
              "FROM lease6 "
#endif  // CASSANDRA_DENORMALIZED_TABLES
              "WHERE subnet_id = ? "
#ifdef TERASTREAM_FULL_TRANSACTIONS
              "IN TXN ? "
#endif  // TERASTREAM_FULL_TRANSACTIONS
             }},

            // Gets an IPv6 lease(s) with specified hostname
            {GET_LEASE6_HOSTNAME(),
             {GET_LEASE6_HOSTNAME(),
              "SELECT "
              "address, valid_lifetime, expire, subnet_id, pref_lifetime, duid, iaid, "
              "lease_type, prefix_len, fqdn_fwd, fqdn_rev, hostname, hwaddr, hwtype, "
              "hwaddr_source, state, user_context, allocation_time, pool_id "
              "FROM lease6 "
              "WHERE hostname = ? "
              "ALLOW FILTERING "}}

        };

        return _;
    }

    /// @brief Statement tags
#ifdef CASSANDRA_DENORMALIZED_TABLES
    static StatementTag &INSERT_LEASE6_DUID_IAID_LEASETYPE_SUBNETID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &INSERT_LEASE6_ADDRESS_LEASETYPE() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &UPDATE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &UPDATE_LEASE6_ADDRESS_LEASETYPE() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &DELETE_LEASE6_ADDRESS_LEASETYPE() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &DELETE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
#else  // CASSANDRA_DENORMALIZED_TABLES
    static StatementTag &INSERT_LEASE6() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &UPDATE_LEASE6() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &DELETE_LEASE6() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &GET_LEASE6_EXPIRE() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
#endif  // CASSANDRA_DENORMALIZED_TABLES
    static StatementTag &GET_LEASE6_ADDR() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &GET_LEASE6_DUID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &GET_LEASE6_DUID_IAID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &GET_LEASE6_DUID_IAID_SUBID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &GET_LEASE6_LIMIT() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &GET_LEASE6_PAGE() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &GET_LEASE6_SUBID() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag &GET_LEASE6_HOSTNAME() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // @}

private:
    /// @brief Lease
    Lease6Ptr lease_;

    /// @brief IPv6 address
    std::string address_;

    /// @brief Preferred lifetime
    cass_int64_t pref_lifetime_;

    /// @brief Client identifier
    CassBlob duid_;

    /// @brief Identity association identifier
    cass_int32_t iaid_;

    /// @brief Lease type (NA, TA or PD)
    cass_int32_t lease_type_;

    /// @brief Prefix length
    cass_int32_t prefix_len_;

    /// @brief Hardware type
    cass_int32_t hwtype_;

    /// @brief Source of the hardware address
    cass_int32_t hwaddr_source_;

    /// @brief Time when the lease was allocated
    cass_int64_t alloc_time_;

    /// @brief Pool identifier
    cass_int64_t pool_id_;
};  // CqlLease6Exchange

CqlLease6Exchange::CqlLease6Exchange(CqlConnection const &connection)
    : CqlLeaseExchange(connection), pref_lifetime_(0), iaid_(0), lease_type_(0), prefix_len_(0),
      hwtype_(0), hwaddr_source_(0), alloc_time_(0), pool_id_(-1) {
}

void CqlLease6Exchange::createBindForInsert(const Lease6Ptr &lease, AnyArray &data) {
    if (!lease) {
        isc_throw(BadValue, "CqlLease6Exchange::createBindForInsert(): "
                            "Lease6 object is NULL");
    }
    // Store lease object to ensure it remains valid.
    lease_ = lease;

    // Set up the structures for the various components of the lease4
    // structure.
    try {
        // address: varchar
        address_ = lease_->addr_.toText();
        if (address_.size() > ADDRESS6_TEXT_MAX_LEN) {
            isc_throw(BadValue, "address " << address_ << " of length " << address_.size()
                                           << " exceeds maximum allowed length of "
                                           << ADDRESS6_TEXT_MAX_LEN);
        }

        // valid lifetime: bigint
        valid_lifetime_ = static_cast<cass_int64_t>(lease_->valid_lft_);

        // expire: bigint
        // The lease structure holds the client last transmission time
        // (cltt_)
        // For convenience for external tools, this is converted to lease
        // expiry time (expire). The relationship is given by:
        // expire = cltt_ + valid_lft_
        CqlExchange<Lease6>::convertToDatabaseTime(lease_->cltt_, lease_->valid_lft_, expire_);

        // subnet_id: int
        subnet_id_ = static_cast<cass_int32_t>(lease_->subnet_id_);

        // pref_lifetime: bigint
        pref_lifetime_ = static_cast<cass_int64_t>(lease_->preferred_lft_);

        // allocation_time: bigint
        alloc_time_ = static_cast<cass_int64_t>(lease_->alloc_time_);

        // pool_id: bigint
        pool_id_ = lease->pool_id_;

        // duid: blob
        if (!lease_->duid_) {
            isc_throw(DbOperationError,
                      "lease6 with address " << address_ << " is missing mandatory duid");
        }
        duid_ = lease_->duid_->getDuid();

        // iaid: int
        iaid_ = static_cast<cass_int32_t>(lease_->iaid_);

        // lease_type: int
        lease_type_ = static_cast<cass_int32_t>(lease_->type_);

        // prefix_len: int
        prefix_len_ = static_cast<cass_int32_t>(lease_->prefixlen_);

        // fqdn_fwd: boolean
        fqdn_fwd_ = lease_->fqdn_fwd_ ? cass_true : cass_false;

        // fqdn_rev: boolean
        fqdn_rev_ = lease_->fqdn_rev_ ? cass_true : cass_false;

        // hostname: varchar
        if (lease_->hostname_.size() > HOSTNAME_MAX_LEN) {
            isc_throw(BadValue,
                      "hostname" << lease_->hostname_ << " of length " << lease_->hostname_.size()
                                 << " exceeds maximum allowed length of " << HOSTNAME_MAX_LEN);
        }
        hostname_ = lease_->hostname_;

        // hwaddr: blob
        if (lease_->hwaddr_ && lease_->hwaddr_->hwaddr_.size() > 0) {
            if (lease_->hwaddr_->hwaddr_.size() > HWAddr::MAX_HWADDR_LEN) {
                isc_throw(DbOperationError, "hardware address "
                                                << lease_->hwaddr_->toText() << " of length "
                                                << lease_->hwaddr_->hwaddr_.size()
                                                << " exceeds maximum allowed length of "
                                                << HWAddr::MAX_HWADDR_LEN);
            }
            hwaddr_ = lease_->hwaddr_->hwaddr_;
        } else {
#ifdef CASSANDRA_DENORMALIZED_TABLES
            hwaddr_ = NULL_HWADDR;
#else  // CASSANDRA_DENORMALIZED_TABLES
            hwaddr_.clear();
#endif  // CASSANDRA_DENORMALIZED_TABLES
        }

        // hwtype: int
        if (lease_->hwaddr_) {
            hwtype_ = static_cast<cass_int32_t>(lease_->hwaddr_->htype_);
        } else {
            hwtype_ = 0;
        }

        // hwaddr_source: int
        if (lease_->hwaddr_) {
            hwaddr_source_ = static_cast<cass_int32_t>(lease_->hwaddr_->source_);
        } else {
            hwaddr_source_ = 0;
        }

#ifdef TERASTREAM
        // privacy_expire: bigint
        privacy_expire_ = static_cast<cass_int64_t>(lease_->privacy_expire_);

        // privacy_hash: blob
        if (lease_->privacy_hash_) {
            privacy_hash_ = *lease_->privacy_hash_;
        } else {
#ifdef CASSANDRA_DENORMALIZED_TABLES
            privacy_hash_ = NULL_PRIVACY_HASH;
#else  // CASSANDRA_DENORMALIZED_TABLES
            privacy_hash_.clear();
#endif  // CASSANDRA_DENORMALIZED_TABLES
        }

        // privacy_history: varchar
        privacy_history_ = lease_->privacy_history_;
#endif  // TERASTREAM

        // state: int
        state_ = static_cast<cass_int32_t>(lease_->state_);

        // user_context: text
        ElementPtr ctx = lease_->getContext();
        if (ctx) {
            user_context_ = ctx->str();
        } else {
            user_context_ = NULL_USER_CONTEXT;
        }

        // Start with a fresh array.
        data.clear();

        // Add them all to data.
#ifdef TERASTREAM
        data.add(&privacy_expire_);
        data.add(&privacy_hash_);
        data.add(&privacy_history_);
#endif  // TERASTREAM
        data.add(&address_);
        data.add(&valid_lifetime_);
        data.add(&expire_);
        data.add(&subnet_id_);
        data.add(&pref_lifetime_);
        data.add(&duid_);
        data.add(&iaid_);
        data.add(&lease_type_);
        data.add(&prefix_len_);
        data.add(&fqdn_fwd_);
        data.add(&fqdn_rev_);
        data.add(&hostname_);
        data.add(&hwaddr_);
        data.add(&hwtype_);
        data.add(&hwaddr_source_);
        data.add(&state_);
        data.add(&user_context_);
        data.add(&alloc_time_);
        data.add(&pool_id_);

#ifdef TERASTREAM_FULL_TRANSACTIONS
        txid_ = connection_.getTransactionID();
        data.add(&txid_);
#endif  // TERASTREAM_FULL_TRANSACTIONS
    } catch (const Exception &ex) {
        isc_throw(DbOperationError, "CqlLease6Exchange::createBindForInsert(): "
                                    "could not create bind array from Lease6: "
                                        << lease_->addr_.toText() << ", reason: " << ex.what());
    }
}

void CqlLease6Exchange::createBindForUpdate(const Lease6Ptr &lease,
                                            AnyArray &data,
                                            StatementTag statement_tag) {
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
    // We only need statement_tag to know which table to update which is obvious
    //      when there is a single table.
    (void)statement_tag;  // [maybe_unused]
#endif  // CASSANDRA_DENORMALIZED_TABLES

    if (!lease) {
        isc_throw(BadValue, "CqlLease6Exchange::createBindForUpdate(): "
                            "Lease6 object is NULL");
    }
    // Store lease object to ensure it remains valid.
    lease_ = lease;

    // Set up the structures for the various components of the lease4
    // structure.
    try {
        // address: varchar
        address_ = lease_->addr_.toText();
        if (address_.size() > ADDRESS6_TEXT_MAX_LEN) {
            isc_throw(BadValue, "address " << address_ << " of length " << address_.size()
                                           << " exceeds maximum allowed length of "
                                           << ADDRESS6_TEXT_MAX_LEN);
        }

        // valid lifetime: bigint
        valid_lifetime_ = static_cast<cass_int64_t>(lease_->valid_lft_);

        // expire: bigint
        // The lease structure holds the client last transmission time
        // (cltt_)
        // For convenience for external tools, this is converted to lease
        // expiry time (expire). The relationship is given by:
        // expire = cltt_ + valid_lft_
        CqlExchange<Lease6>::convertToDatabaseTime(lease_->cltt_, lease_->valid_lft_, expire_);

        // subnet_id: int
        subnet_id_ = static_cast<cass_int32_t>(lease_->subnet_id_);

        // pref_lifetime: bigint
        pref_lifetime_ = static_cast<cass_int64_t>(lease_->preferred_lft_);

        // duid: blob
        if (!lease_->duid_) {
            isc_throw(DbOperationError,
                      "lease6 with address " << address_ << " is missing mandatory duid");
        }
        duid_ = lease_->duid_->getDuid();

        // iaid: int
        iaid_ = static_cast<cass_int32_t>(lease_->iaid_);

        // lease_type: int
        lease_type_ = static_cast<cass_int32_t>(lease_->type_);

        // prefix_len: int
        prefix_len_ = static_cast<cass_int32_t>(lease_->prefixlen_);

        // fqdn_fwd: boolean
        fqdn_fwd_ = lease_->fqdn_fwd_ ? cass_true : cass_false;

        // fqdn_rev: boolean
        fqdn_rev_ = lease_->fqdn_rev_ ? cass_true : cass_false;

        // hostname: varchar
        if (lease_->hostname_.size() > HOSTNAME_MAX_LEN) {
            isc_throw(BadValue,
                      "hostname" << lease_->hostname_ << " of length " << lease_->hostname_.size()
                                 << " exceeds maximum allowed length of " << HOSTNAME_MAX_LEN);
        }
        hostname_ = lease_->hostname_;

        // hwaddr: blob
        if (lease_->hwaddr_ && lease_->hwaddr_->hwaddr_.size() > 0) {
            if (lease_->hwaddr_->hwaddr_.size() > HWAddr::MAX_HWADDR_LEN) {
                isc_throw(DbOperationError, "hardware address "
                                                << lease_->hwaddr_->toText() << " of length "
                                                << lease_->hwaddr_->hwaddr_.size()
                                                << " exceeds maximum allowed length of "
                                                << HWAddr::MAX_HWADDR_LEN);
            }
            hwaddr_ = lease_->hwaddr_->hwaddr_;
        } else {
#ifdef CASSANDRA_DENORMALIZED_TABLES
            hwaddr_ = NULL_HWADDR;
#else  // CASSANDRA_DENORMALIZED_TABLES
            hwaddr_.clear();
#endif  // CASSANDRA_DENORMALIZED_TABLES
        }

        // hwtype: int
        if (lease_->hwaddr_) {
            hwtype_ = static_cast<cass_int32_t>(lease_->hwaddr_->htype_);
        } else {
            hwtype_ = 0;
        }

        // hwaddr_source: int
        if (lease_->hwaddr_) {
            hwaddr_source_ = static_cast<cass_int32_t>(lease_->hwaddr_->source_);
        } else {
            hwaddr_source_ = 0;
        }

#ifdef TERASTREAM
        // privacy_expire: bigint
        privacy_expire_ = static_cast<cass_int64_t>(lease_->privacy_expire_);

        // privacy_hash: blob
        if (lease_->privacy_hash_) {
            privacy_hash_ = *lease_->privacy_hash_;
        } else {
#ifdef CASSANDRA_DENORMALIZED_TABLES
            privacy_hash_ = NULL_PRIVACY_HASH;
#else  // CASSANDRA_DENORMALIZED_TABLES
            privacy_hash_.clear();
#endif  // CASSANDRA_DENORMALIZED_TABLES
        }

        // privacy_history: varchar
        privacy_history_ = lease_->privacy_history_;
#endif  // TERASTREAM

        // state: int
        state_ = static_cast<cass_int32_t>(lease_->state_);

        // user_context: text
        ElementPtr ctx = lease_->getContext();
        if (ctx) {
            user_context_ = ctx->str();
        } else {
            user_context_ = NULL_USER_CONTEXT;
        }

        pool_id_ = lease_->pool_id_;
        // Start with a fresh array.
        data.clear();

        // Add them all to data.
#ifdef TERASTREAM
        data.add(&privacy_expire_);
        data.add(&privacy_hash_);
        data.add(&privacy_history_);
#endif  // TERASTREAM
        data.add(&valid_lifetime_);
        data.add(&expire_);
        data.add(&pref_lifetime_);
#ifdef CASSANDRA_DENORMALIZED_TABLES
        if (std::strcmp(statement_tag, UPDATE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID) != 0) {
            data.add(&duid_);
            data.add(&iaid_);
            data.add(&subnet_id_);
        }
#else  // CASSANDRA_DENORMALIZED_TABLES
        data.add(&duid_);
        data.add(&iaid_);
        data.add(&subnet_id_);
        data.add(&lease_type_);
#endif  // CASSANDRA_DENORMALIZED_TABLES
        data.add(&prefix_len_);
        data.add(&fqdn_fwd_);
        data.add(&fqdn_rev_);
        data.add(&hostname_);
        data.add(&hwaddr_);
        data.add(&hwtype_);
        data.add(&hwaddr_source_);
        data.add(&state_);
        data.add(&user_context_);
        data.add(&pool_id_);
#ifdef CASSANDRA_DENORMALIZED_TABLES
        if (std::strcmp(statement_tag, UPDATE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID) == 0) {
            data.add(&address_);
            data.add(&lease_type_);
            data.add(&duid_);
            data.add(&iaid_);
            data.add(&subnet_id_);
        } else if (std::strcmp(statement_tag, UPDATE_LEASE6_ADDRESS_LEASETYPE) == 0) {
            data.add(&address_);
            data.add(&lease_type_);
        }
#else  // CASSANDRA_DENORMALIZED_TABLES
        data.add(&address_);
#endif  // CASSANDRA_DENORMALIZED_TABLES

#ifdef TERASTREAM_FULL_TRANSACTIONS
        txid_ = connection_.getTransactionID();
        data.add(&txid_);
#else  // TERASTREAM_FULL_TRANSACTIONS
        CqlExchange<Lease6>::convertToDatabaseTime(lease_->old_cltt_, lease_->old_valid_lft_,
                                                   old_expire_);
        data.add(&old_expire_);
#endif  // TERASTREAM_FULL_TRANSACTIONS
    } catch (const Exception &ex) {
        isc_throw(DbOperationError, "CqlLease6Exchange::createBindForUpdate(): "
                                    "could not create bind array from Lease6: "
                                        << lease_->addr_.toText() << ", reason: " << ex.what());
    }
}

void CqlLease6Exchange::createBindForDelete(const Lease6Ptr &lease,
                                            AnyArray &data,
                                            StatementTag statement_tag /* = "" */) {
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
    // We only need statement_tag to know which table to delete form which is
    //      obvious when there is a single table.
    (void)statement_tag;  // [maybe_unused]
#endif  // CASSANDRA_DENORMALIZED_TABLES

    if (!lease) {
        isc_throw(BadValue, "Lease6 object is NULL");
    }
    // Store lease object to ensure it remains valid.
    lease_ = lease;

    // Set up the structures for the various components of the lease4
    // structure.
    try {
        // address: varchar
        address_ = lease_->addr_.toText();
        if (address_.size() > ADDRESS6_TEXT_MAX_LEN) {
            isc_throw(BadValue, "address " << address_ << " of length " << address_.size()
                                           << " exceeds maximum allowed length of "
                                           << ADDRESS6_TEXT_MAX_LEN);
        }

        // subnet_id: int
        subnet_id_ = static_cast<cass_int32_t>(lease_->subnet_id_);

        // duid: blob
        if (!lease_->duid_) {
            isc_throw(DbOperationError,
                      "lease6 with address " << address_ << " is missing mandatory duid");
        }
        duid_ = lease_->duid_->getDuid();

        // iaid: int
        iaid_ = static_cast<cass_int32_t>(lease_->iaid_);

        // lease_type: int
        lease_type_ = static_cast<cass_int32_t>(lease_->type_);

        // Start with a fresh array.
        data.clear();
#ifdef CASSANDRA_DENORMALIZED_TABLES
        if (std::strcmp(statement_tag, DELETE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID) == 0) {
            data.add(&address_);
            data.add(&lease_type_);
            data.add(&duid_);
            data.add(&iaid_);
            data.add(&subnet_id_);
        } else if (std::strcmp(statement_tag, DELETE_LEASE6_ADDRESS_LEASETYPE) == 0) {
            data.add(&address_);
            data.add(&lease_type_);
        }
#else  // CASSANDRA_DENORMALIZED_TABLES
        data.add(&address_);
#endif  // CASSANDRA_DENORMALIZED_TABLES

#ifdef TERASTREAM_FULL_TRANSACTIONS
        txid_ = connection_.getTransactionID();
        data.add(&txid_);
#else  // TERASTREAM_FULL_TRANSACTIONS
        CqlExchange::convertToDatabaseTime(lease_->old_cltt_, lease_->old_valid_lft_, old_expire_);
        data.add(&old_expire_);
#endif  // TERASTREAM_FULL_TRANSACTIONS
    } catch (const Exception &ex) {
        isc_throw(DbOperationError, "CqlLease6Exchange::createBindForDelete(): "
                                    "could not create bind array from Lease6: "
                                        << lease_->addr_.toText() << ", reason: " << ex.what());
    }
}

void CqlLease6Exchange::createBindForSelect(AnyArray &data, StatementTag const & /* unused */) {
    // Start with a fresh array.
    data.clear();

#ifdef TERASTREAM
    // privacy_expire: bigint
    data.add(&privacy_expire_);

    // privacy_hash: blob
    data.add(&privacy_hash_);

    // privacy_history: varchar
    data.add(&privacy_history_);
#endif  // TERASTREAM

    // address: varchar
    data.add(&address_);

    // valid_lifetime_: bigint
    data.add(&valid_lifetime_);

    // expire: bigint
    data.add(&expire_);

    // subnet_id: int
    data.add(&subnet_id_);

    // pref_lifetime: bigint
    data.add(&pref_lifetime_);

    // duid: blob
    data.add(&duid_);

    // iaid: int
    data.add(&iaid_);

    // lease_type: int
    data.add(&lease_type_);

    // prefix_len: int
    data.add(&prefix_len_);

    // fqdn_fwd: boolean
    data.add(&fqdn_fwd_);

    // fqdn_rev: boolean
    data.add(&fqdn_rev_);

    // hostname: varchar
    data.add(&hostname_);

    // hwaddr: blob
    data.add(&hwaddr_);

    // hwtype: int
    data.add(&hwtype_);

    // hwaddr_source: int
    data.add(&hwaddr_source_);

    // state: int
    data.add(&state_);

    // user_context: text
    data.add(&user_context_);

    // allocation_time: bigint
    data.add(&alloc_time_);

    // pool_id: bigint
    data.add(&pool_id_);
}

Lease6Ptr CqlLease6Exchange::retrieve() {
    try {
        // Sanity checks
        if (address_.size() > ADDRESS6_TEXT_MAX_LEN) {
            isc_throw(BadValue, "address " << address_ << " of length " << address_.size()
                                           << " exceeds maximum allowed length of "
                                           << ADDRESS6_TEXT_MAX_LEN);
        }
        if (duid_.size() > DUID::MAX_DUID_LEN) {
            isc_throw(BadValue, "duid " << DUID(duid_).toText() << " of length " << duid_.size()
                                        << " exceeds maximum allowed length of "
                                        << DUID::MAX_DUID_LEN);
        }
        if (lease_type_ != Lease::TYPE_NA && lease_type_ != Lease::TYPE_TA &&
            lease_type_ != Lease::TYPE_PD) {
            isc_throw(BadValue, "invalid lease type " << lease_type_ << " for lease with address "
                                                      << address_ << ". Expected 0, 1 or 2.");
        }
        if (hostname_.size() > HOSTNAME_MAX_LEN) {
            isc_throw(BadValue, "hostname " << hostname_ << " of length " << hostname_.size()
                                            << " exceeds maximum allowed length of "
                                            << HOSTNAME_MAX_LEN);
        }
        if (hwaddr_.size() > HWAddr::MAX_HWADDR_LEN) {
            isc_throw(BadValue, "hwaddr " << HWAddr(hwaddr_, hwtype_).toText(false) << " of length "
                                          << hwaddr_.size() << " exceeds maximum allowed length of "
                                          << HWAddr::MAX_HWADDR_LEN);
        }
#ifdef TERASTREAM
        if (privacy_hash_.size() > Lease::MAX_PRIVACY_HASH_LEN) {
            isc_throw(BadValue, "privacy_hash" << DUID(privacy_hash_).toText() << " of length "
                                               << privacy_hash_.size()
                                               << " exceeds maximum allowed length of "
                                               << Lease::MAX_PRIVACY_HASH_LEN);
        }
#endif  // TERASTREAM

        IOAddress addr(address_);

        DuidPtr duid(std::make_shared<DUID>(duid_));

        HWAddrPtr hwaddr;
        if (hwaddr_.size()) {
            hwaddr = std::make_shared<HWAddr>(hwaddr_, hwtype_);
            hwaddr->source_ = hwaddr_source_;
        }

#ifdef CASSANDRA_DENORMALIZED_TABLES
        if (hwaddr && hwaddr->hwaddr_ == NULL_HWADDR) {
            hwaddr.reset();
        }
#endif  // CASSANDRA_DENORMALIZED_TABLES

        ElementPtr ctx;
        if (!user_context_.empty()) {
            ctx = Element::fromJSON(user_context_);
            if (!ctx || (ctx->getType() != Element::map)) {
                isc_throw(BadValue, "user context '" << user_context_ << "' is not a JSON map");
            }
        }

        // Create the lease and set the cltt (after converting from the
        // expire time retrieved from the database).
        Lease6Ptr result(std::make_shared<Lease6>(static_cast<Lease::Type>(lease_type_), addr, duid,
                                                  iaid_, pref_lifetime_, valid_lifetime_,
                                                  subnet_id_, fqdn_fwd_, fqdn_rev_, hostname_,
                                                  hwaddr, prefix_len_, alloc_time_));

        time_t cltt = 0;
        CqlExchange<Lease6>::convertFromDatabaseTime(expire_, valid_lifetime_, cltt);
        result->cltt_ = cltt;
        result->old_cltt_ = cltt;
        result->pool_id_ = pool_id_;

#ifdef TERASTREAM
        result->privacy_expire_ = privacy_expire_;
#ifdef CASSANDRA_DENORMALIZED_TABLES
        if (privacy_hash_ == NULL_PRIVACY_HASH) {
            privacy_hash_.clear();
        }
#endif  // CASSANDRA_DENORMALIZED_TABLES
        result->privacy_hash_ = std::make_shared<PrivacyHash>(privacy_hash_);

        result->privacy_history_ = privacy_history_;
#endif  // TERASTREAM

        result->state_ = state_;

        if (ctx) {
            result->setContext(ctx);
        }

        return (result);
    } catch (const Exception &ex) {
        isc_throw(DbOperationError, "CqlLease6Exchange::retrieve(): "
                                    "could not convert data to Lease6, reason: "
                                        << ex.what());
    }
    return Lease6Ptr();
}

void CqlLease6Exchange::getLeaseCollection(StatementTag const &statement_tag,
                                           AnyArray &data,
                                           Lease6Collection &result) {
    result = executeSelect(connection_, data, statement_tag);
}

void CqlLease6Exchange::getLease(StatementTag const &statement_tag,
                                 AnyArray &data,
                                 Lease6Ptr &result) {
    // This particular method is called when only one or zero matches is
    // expected.
    Lease6Collection collection;
    getLeaseCollection(statement_tag, data, collection);

    // Return single record if present, else clear the lease.
    const size_t collection_size = collection.size();
    if (collection_size >= 2u) {
        isc_throw(MultipleRecords, "CqlLease6Exchange::getLease(): multiple records were found in "
                                   "the database where only one was expected for statement "
                                       << statement_tag);
    } else if (collection_size == 0u) {
        result.reset();
    } else {
        result = collection.front();
    }
}

void CqlLease6Exchange::getExpiredLeases(const size_t &max_leases,
                                         Lease6Collection &expired_leases) {
#ifdef CASSANDRA_DENORMALIZED_TABLES
    // This functions is disabled.
    (void)max_leases;  // [maybe_unused]
    (void)expired_leases;  // [maybe_unused]
#else  // CASSANDRA_DENORMALIZED_TABLES
    // Set up the WHERE clause value
    cass_int32_t keep_state = Lease::STATE_EXPIRED_RECLAIMED;
    cass_int64_t timestamp = static_cast<cass_int64_t>(time(NULL));

    // If the number of leases is 0, we will return all leases. This is
    // achieved by setting the limit to a very high value.
    cass_int32_t limit = max_leases > 0u ? static_cast<cass_int32_t>(max_leases) :
                                           std::numeric_limits<cass_int32_t>::max();

    for (cass_int32_t state = Lease::STATE_DEFAULT; state <= Lease::STATE_EXPIRED_RECLAIMED;
         state++) {
        if (state == keep_state) {
            continue;
        }

        AnyArray data;
        data.add(&state);
        data.add(&timestamp);
        data.add(&limit);

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection_.getTransactionID());
        data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        // Retrieve leases from the database.
        Lease6Collection temp_collection;
        getLeaseCollection(CqlLease6Exchange::GET_LEASE6_EXPIRE(), data, temp_collection);

        for (Lease6Ptr &lease : temp_collection) {
            expired_leases.push_back(lease);
        }
    }
#endif  // CASSANDRA_DENORMALIZED_TABLES
}

/// @brief Base CQL derivation of the statistical lease data query
///
/// This class provides the functionality such as results storage and row
/// fetching common to fulfilling the statistical lease data query.
///
struct CqlLeaseStatsQuery : LeaseStatsQuery, CqlExchange<LeaseStatsRow> {
    /// @brief Constructor to query for all subnets' stats
    ///
    ///  The query created will return statistics for all subnets
    ///
    /// @param connection An open connection to the database housing the lease data
    /// @param statement The lease data SQL prepared statement tag to execute
    /// @param fetch_type Indicates whether or not lease_type should be
    /// fetched from the result set (should be true for v6)
    CqlLeaseStatsQuery(CqlConnection &connection,
                       StatementTag const &statement,
                       const bool fetch_type)
        : connection_(connection), statement_(statement), fetch_type_(fetch_type),
          cummulative_rows_(), next_row_(cummulative_rows_.begin()), subnet_id_(0), lease_type_(0),
          state_(0) {
    }

    /// @brief Constructor to query for a single subnet's stats
    ///
    /// The query created will return statistics for a single subnet
    ///
    /// @param connection An open connection to the database housing the lease data
    /// @param statement The lease data SQL prepared statement tag to execute
    /// @param fetch_type Indicates whether or not lease_type should be
    /// fetched from the result set (should be true for v6)
    /// @param subnet_id id of the subnet for which stats are desired
    CqlLeaseStatsQuery(CqlConnection &connection,
                       StatementTag const &statement,
                       const bool fetch_type,
                       const SubnetID &subnet_id)
        : LeaseStatsQuery(subnet_id), connection_(connection), statement_(statement),
          fetch_type_(fetch_type), cummulative_rows_(), next_row_(cummulative_rows_.begin()),
          subnet_id_(0), lease_type_(0), state_(0) {
    }

    /// @brief Constructor to query for the stats for a range of subnets
    ///
    /// The query created will return statistics for the inclusive range of
    /// subnets described by first and last sunbet IDs.
    ///
    /// @param connection An open connection to the database housing the lease data
    /// @param statement The lease data SQL prepared statement tag to execute
    /// @param fetch_type Indicates whether or not lease_type should be
    /// fetched from the result set (should be true for v6)
    /// @param first_subnet_id first subnet in the range of subnets
    /// @param last_subnet_id last subnet in the range of subnets
    CqlLeaseStatsQuery(CqlConnection &connection,
                       StatementTag const &statement,
                       const bool fetch_type,
                       const SubnetID &first_subnet_id,
                       const SubnetID &last_subnet_id)
        : LeaseStatsQuery(first_subnet_id, last_subnet_id), connection_(connection),
          statement_(statement), fetch_type_(fetch_type), cummulative_rows_(),
          next_row_(cummulative_rows_.begin()), subnet_id_(0), lease_type_(0), state_(0) {
    }

    /// @brief Destructor
    virtual ~CqlLeaseStatsQuery() = default;

    /// @brief Creates the lease statistical data result set
    ///
    /// The result set is populated by executing a  prepared SQL query
    /// against the database which sums the leases per lease state per
    /// subnet id.  Positions internal row tracking to point to the
    /// first row of the aggregate results.
    void start() override final;

    /// @brief Fetches the next row in the result set
    ///
    /// Once the internal result set has been populated by invoking the
    /// the start() method, this method is used to iterate over the
    /// result set rows. Once the last row has been fetched, subsequent
    /// calls will return false.
    ///
    /// @param row Storage for the fetched row
    ///
    /// @return True if the fetch succeeded, false if there are no more
    /// rows to fetch.
    bool getNextRow(LeaseStatsRow &row) override final;

    /// @brief Create BIND array to receive C++ data.
    ///
    /// Used in executeSelect() to retrieve from database
    ///
    /// @param data array of bound objects representing data to be retrieved
    /// @param statement_tag prepared statement being executed; defaults to an
    ///     invalid index
    void createBindForSelect(AnyArray &data,
                             StatementTag const &statement_tag = StatementTag()) override final;

    /// @brief Copy received data into the derived class' object.
    ///
    /// Copies information about the entity to be retrieved into a holistic
    /// object. Called in @ref executeSelect(). Not implemented for base class
    /// CqlExchange. To be implemented in derived classes.
    ///
    /// @return a pointer to the object retrieved.
    LeaseStatsRowPtr retrieve() override final;

    static StatementMap &tagged_statements() {
        static StatementMap _{
            // Return subnet_id and state of each v4 lease
            {ALL_LEASE4_STATS(),
             {ALL_LEASE4_STATS(), "SELECT "
                                  "subnet_id, state "
                                  "FROM lease4 "}},

            // Return state of each v4 lease for a single subnet
            {SUBNET_LEASE4_STATS(),
             {SUBNET_LEASE4_STATS(), "SELECT "
                                     "subnet_id, state "
                                     "FROM lease4 "
                                     "WHERE subnet_id = ? "}},

            // Return state of each v4 lease for a subnet range
            {SUBNET_RANGE_LEASE4_STATS(),
             {SUBNET_RANGE_LEASE4_STATS(), "SELECT "
                                           "subnet_id, state "
                                           "FROM lease4 "
                                           "WHERE subnet_id >= ? and subnet_id <= ? "
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
                                           "ALLOW FILTERING "
#endif  // CASSANDRA_DENORMALIZED_TABLES
             }},

            // Return subnet_id, lease_type, and state of each v6 lease
            {ALL_LEASE6_STATS(),
             {ALL_LEASE6_STATS(), "SELECT "
                                  "subnet_id, lease_type, state "
                                  "FROM lease6 "}},

            // Return type and state of each v6 lease for a single subnet
            {SUBNET_LEASE6_STATS(),
             {SUBNET_LEASE6_STATS(), "SELECT "
                                     "subnet_id, lease_type, state "
                                     "FROM lease6 "
                                     "WHERE subnet_id = ? "}},

            // Return type and state of each v6 lease for single range
            {SUBNET_RANGE_LEASE6_STATS(),
             {SUBNET_RANGE_LEASE6_STATS(), "SELECT "
                                           "subnet_id, lease_type, state "
                                           "FROM lease6 "
                                           "WHERE subnet_id >= ? and subnet_id <= ? "
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
                                           "ALLOW FILTERING "
#endif  // CASSANDRA_DENORMALIZED_TABLES
             }},
        };

        return _;
    }

    /// @brief Statement tags
    /// @{
    // Return lease4 lease statistics for all subnets
    static StatementTag &ALL_LEASE4_STATS() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    /// Return lease4 lease statistics for a single subnet
    static StatementTag &SUBNET_LEASE4_STATS() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    /// Return lease4 lease statistics for a range of subnets
    static StatementTag &SUBNET_RANGE_LEASE4_STATS() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    // Return lease6 lease statistics for all subnets
    static StatementTag &ALL_LEASE6_STATS() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    /// Return lease6 lease statistics for a single subnet
    static StatementTag &SUBNET_LEASE6_STATS() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    /// Return lease6 lease statistics for a range of subnets
    static StatementTag &SUBNET_RANGE_LEASE6_STATS() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    /// @}

private:
    /// @brief Database connection
    CqlConnection const &connection_;

    /// @brief The query's prepared statement tag
    StatementTag statement_;

    /// @brief fetch from the result set? (should be true for v6)
    bool fetch_type_;

    /// @brief map containing the aggregated lease counts
    std::map<LeaseStatsRow, int> cummulative_rows_;

    /// @brief cursor pointing to the next row to read in aggregate map
    std::map<LeaseStatsRow, int>::iterator next_row_;

    /// @brief Subnet identifier
    cass_int32_t subnet_id_;

    /// @brief Lease type (NA, TA or PD)
    cass_int32_t lease_type_;

    /// @brief Lease state
    cass_int32_t state_;
};

void CqlLeaseStatsQuery::start() {
    // Set up where clause parameters as needed
    AnyArray data;
    cass_int32_t first_subnet_id_data;
    cass_int32_t last_subnet_id_data;
    if (getSelectMode() != ALL_SUBNETS) {
        first_subnet_id_data = static_cast<cass_int32_t>(first_subnet_id_);
        data.add(&first_subnet_id_data);

        if (getSelectMode() == SUBNET_RANGE) {
            last_subnet_id_data = static_cast<cass_int32_t>(last_subnet_id_);
            data.add(&last_subnet_id_data);
        }
    }
    Collection<LeaseStatsRow> collection = executeSelect(connection_, data, statement_);

    // Form LeaseStatsRowPtr objects.
    for (LeaseStatsRowPtr &stats : collection) {
        if (stats->lease_state_ != Lease::STATE_DEFAULT &&
            stats->lease_state_ != Lease::STATE_DECLINED) {
            continue;
        }
        auto cum_row = cummulative_rows_.find(*stats);
        if (cum_row != cummulative_rows_.end()) {
            cummulative_rows_[*stats] = cum_row->second + 1;
        } else {
            cummulative_rows_.insert(std::make_pair(*stats, 1));
        }
    }

    // Set our row iterator to the beginning
    next_row_ = cummulative_rows_.begin();
}

bool CqlLeaseStatsQuery::getNextRow(LeaseStatsRow &row) {
    // If we're past the end, punt.
    if (next_row_ == cummulative_rows_.end()) {
        return (false);
    }

    // Start by copying from the map row key
    row.subnet_id_ = next_row_->first.subnet_id_;
    row.lease_type_ = next_row_->first.lease_type_;
    row.lease_state_ = next_row_->first.lease_state_;

    // Grab the count from the map value
    row.state_count_ = next_row_->second;

    // Point to the next row.
    ++next_row_;
    return (true);
}

void CqlLeaseStatsQuery::createBindForSelect(AnyArray &data,
                                             StatementTag const & /* statement_tag */) {
    // Start with a fresh array.
    data.clear();

    // subnet_id: int
    data.add(&subnet_id_);

    // lease_type: int
    if (fetch_type_) {
        data.add(&lease_type_);
    } else {
        lease_type_ = Lease::TYPE_NA;  // lease type is always NA for v4
    }

    // state: int
    data.add(&state_);
}

LeaseStatsRowPtr CqlLeaseStatsQuery::retrieve() {
    return std::make_shared<LeaseStatsRow>(subnet_id_, static_cast<Lease::Type>(lease_type_),
                                           state_, 1);
}

CqlLeaseMgr::CqlLeaseMgr(const DatabaseConnection::ParameterMap &parameters)
    : parameters_(parameters), connection_(parameters)
#ifdef TERASTREAM_LOCK
      ,
      lock_mgr4_(connection_), lock_mgr6_(connection_)
#endif  // TERASTREAM_LOCK
{
    CqlVersionExchange<isc::db::OperationalVersionTuple> version_exchange;
    version_exchange.validateSchema(connection_);

    // Now prepare the rest of the exchanges.
#ifdef TERASTREAM_FULL_TRANSACTIONS
    connection_.setTransactionOperations(CqlTransactionExchange::BEGIN_TXN,
                                         CqlTransactionExchange::COMMIT_TXN,
                                         CqlTransactionExchange::ROLLBACK_TXN);
#endif  // TERASTREAM_FULL_TRANSACTIONS
    connection_.prepareStatements(CqlLease4Exchange::tagged_statements());
    connection_.prepareStatements(CqlLease6Exchange::tagged_statements());
#ifdef TERASTREAM_LOCK
#endif  // TERASTREAM_LOCK
#ifdef TERASTREAM_FULL_TRANSACTIONS
    connection_.prepareStatements(CqlTransactionExchange::tagged_statements());
#endif  // TERASTREAM_FULL_TRANSACTIONS
    connection_.prepareStatements(CqlLeaseStatsQuery::tagged_statements());
}

std::string CqlLeaseMgr::getDBVersion() {
    std::stringstream tmp;
    tmp << "CQL backend " << std::get<MAJOR>(CQL_SCHEMA_VERSION);
    tmp << "." << std::get<MINOR>(CQL_SCHEMA_VERSION);
    tmp << ", library cassandra_static " << CASS_VERSION_MAJOR;
    tmp << "." << CASS_VERSION_MINOR;
    tmp << "." << CASS_VERSION_PATCH;
    return tmp.str();
}

bool CqlLeaseMgr::addLease(const Lease4Ptr &lease) {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_ADD_ADDR4)
        .arg(lease->addr_.toText());

    AnyArray data;

    CqlLease4Exchange exchange(connection_);
    exchange.createBindForInsert(lease, data);
    try {
#ifdef CASSANDRA_DENORMALIZED_TABLES
        exchange.executeMutation(connection_, data, CqlLease4Exchange::INSERT_LEASE4_ADDRESS());
        exchange.executeMutation(connection_, data,
                                 CqlLease4Exchange::INSERT_LEASE4_CLIENTID_SUBNETID());
        exchange.executeMutation(connection_, data,
                                 CqlLease4Exchange::INSERT_LEASE4_HWADDR_SUBNETID());
        exchange.executeMutation(connection_, data, CqlLease4Exchange::INSERT_LEASE4_SUBNETID());
#else  // CASSANDRA_DENORMALIZED_TABLES
        exchange.executeMutation(connection_, data, CqlLease4Exchange::INSERT_LEASE4());
#endif  // CASSANDRA_DENORMALIZED_TABLES
    } catch (const Exception &exception) {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_LEASE_EXCEPTION_THROWN)
            .arg(exception.what());
        return false;
    }

    lease->old_cltt_ = lease->cltt_;
    lease->old_valid_lft_ = lease->valid_lft_;

    return true;
}

bool CqlLeaseMgr::addLease(const Lease6Ptr &lease) {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_ADD_ADDR6)
        .arg(lease->addr_.toText());

    AnyArray data;

    CqlLease6Exchange exchange(connection_);
    exchange.createBindForInsert(lease, data);
    try {
#ifdef CASSANDRA_DENORMALIZED_TABLES
        exchange.executeMutation(connection_, data,
                                 CqlLease6Exchange::INSERT_LEASE6_ADDRESS_LEASETYPE());
        exchange.executeMutation(connection_, data,
                                 CqlLease6Exchange::INSERT_LEASE6_DUID_IAID_LEASETYPE_SUBNETID());
#else  // CASSANDRA_DENORMALIZED_TABLES
        exchange.executeMutation(connection_, data, CqlLease6Exchange::INSERT_LEASE6());
#endif  // CASSANDRA_DENORMALIZED_TABLES
    } catch (const Exception &exception) {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_LEASE_EXCEPTION_THROWN)
            .arg(exception.what());
        return false;
    }

    lease->old_cltt_ = lease->cltt_;
    lease->old_valid_lft_ = lease->valid_lft_;

    return true;
}

Lease4Ptr CqlLeaseMgr::getLease4(const IOAddress &addr) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_ADDR4).arg(addr.toText());

    // Set up the WHERE clause value
    cass_int64_t addr4 = static_cast<cass_int64_t>(addr.addressPlusPortToUint64());
    AnyArray data{&addr4};

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease4Ptr result;

    CqlLease4Exchange exchange(connection_);
    exchange.getLease(CqlLease4Exchange::GET_LEASE4_ADDR(), data, result);

    return result;
}

Lease4Collection CqlLeaseMgr::getLease4(const HWAddr &hwaddr) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_HWADDR)
        .arg(hwaddr.toText());

    // Set up the WHERE clause value
    AnyArray data;

    CassBlob hwaddr_data(hwaddr.hwaddr_);
#ifdef CASSANDRA_DENORMALIZED_TABLES
    if (hwaddr.hwaddr_.size() == 0) {
        hwaddr_data = NULL_HWADDR;
    }
#endif  // CASSANDRA_DENORMALIZED_TABLES
    data.add(&hwaddr_data);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease4Collection result;
    CqlLease4Exchange exchange(connection_);
    exchange.getLeaseCollection(CqlLease4Exchange::GET_LEASE4_HWADDR(), data, result);

    return (result);
}

Lease4Ptr CqlLeaseMgr::getLease4(const HWAddr &hwaddr, SubnetID subnet_id) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_SUBID_HWADDR)
        .arg(subnet_id)
        .arg(hwaddr.toText());

    // Set up the WHERE clause value
    AnyArray data;

    CassBlob hwaddr_data(hwaddr.hwaddr_);
#ifdef CASSANDRA_DENORMALIZED_TABLES
    if (hwaddr.hwaddr_.size() == 0) {
        hwaddr_data = NULL_HWADDR;
    }
#endif  // CASSANDRA_DENORMALIZED_TABLES
    data.add(&hwaddr_data);

    cass_int32_t subnet_id_data = static_cast<cass_int32_t>(subnet_id);
    data.add(&subnet_id_data);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease4Ptr result;
    CqlLease4Exchange exchange(connection_);
    exchange.getLease(CqlLease4Exchange::GET_LEASE4_HWADDR_SUBID(), data, result);

    return (result);
}

Lease4Collection CqlLeaseMgr::getLease4(const ClientId &clientid) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_CLIENTID)
        .arg(clientid.toText());

    // Set up the WHERE clause value
    AnyArray data;

    CassBlob client_id_data(clientid.getClientId());
#ifdef CASSANDRA_DENORMALIZED_TABLES
    if (client_id_data.size() == 0) {
        client_id_data = NULL_CLIENTID;
    }
#endif  // CASSANDRA_DENORMALIZED_TABLES
    data.add(&client_id_data);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease4Collection result;
    CqlLease4Exchange exchange(connection_);
    exchange.getLeaseCollection(CqlLease4Exchange::GET_LEASE4_CLIENTID(), data, result);

    return (result);
}

Lease4Ptr
CqlLeaseMgr::getLease4(const ClientId &clientid, const HWAddr &hwaddr, SubnetID subnet_id) const {
    /// @todo: Remove this method in this and all other implementations.
    /// This method is currently not implemented because allocation engine
    /// searches for the lease using HW address or client identifier.
    /// It never uses both parameters in the same time. We need to
    /// consider if this method is needed at all.
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_CLIENTID_HWADDR_SUBID)
        .arg(clientid.toText())
        .arg(hwaddr.toText())
        .arg(subnet_id);

    isc_throw(NotImplemented, "CqlLeaseMgr::getLease4() is obsolete");
}

Lease4Ptr CqlLeaseMgr::getLease4(const ClientId &clientid, SubnetID subnet_id) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_SUBID_CLIENTID)
        .arg(subnet_id)
        .arg(clientid.toText());

    // Set up the WHERE clause value
    AnyArray data;

    CassBlob client_id_data(clientid.getClientId());
#ifdef CASSANDRA_DENORMALIZED_TABLES
    if (client_id_data.size() == 0) {
        client_id_data = NULL_CLIENTID;
    }
#endif  // CASSANDRA_DENORMALIZED_TABLES
    data.add(&client_id_data);

    cass_int32_t subnet_id_data = static_cast<cass_int32_t>(subnet_id);
    data.add(&subnet_id_data);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease4Ptr result;
    CqlLease4Exchange exchange(connection_);
    exchange.getLease(CqlLease4Exchange::GET_LEASE4_CLIENTID_SUBID(), data, result);

    return (result);
}

Lease4Collection CqlLeaseMgr::getLeases4(SubnetID subnet_id) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_SUBID4).arg(subnet_id);

    // Set up the WHERE clause value
    AnyArray data;

    cass_int32_t subnet_id_data = static_cast<cass_int32_t>(subnet_id);
    data.add(&subnet_id_data);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease4Collection result;
    CqlLease4Exchange exchange(connection_);
    exchange.getLeaseCollection(CqlLease4Exchange::GET_LEASE4_SUBID(), data, result);

    return (result);
}

Lease4Collection CqlLeaseMgr::getLeases4(const std::string &hostname) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_HOSTNAME4).arg(hostname);

    // Set up the WHERE clause value
    AnyArray data;

    std::string hostname_data(hostname);
    data.add(&hostname_data);

    // Get the data.
    Lease4Collection result;
    CqlLease4Exchange exchange(connection_);
    exchange.getLeaseCollection(CqlLease4Exchange::GET_LEASE4_HOSTNAME(), data, result);

    return (result);
}

Lease4Collection CqlLeaseMgr::getLeases4() const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET4);

    // Set up the WHERE clause value
    AnyArray data;

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease4Collection result;
    CqlLease4Exchange exchange(connection_);
    exchange.getLeaseCollection(CqlLease4Exchange::GET_LEASE4(), data, result);

    return (result);
}

Lease4Collection CqlLeaseMgr::getLeases4(const asiolink::IOAddress &lower_bound_address,
                                         const LeasePageSize &page_size) const {
    // Expecting IPv4 address.
    if (!lower_bound_address.isV4()) {
        isc_throw(InvalidAddressFamily, "expected IPv4 address while "
                                        "retrieving leases from the lease database, got "
                                            << lower_bound_address);
    }

    if (page_size.page_size_ == 0) {
        isc_throw(OutOfRange, "page size of retrieved leases must not be 0");
    }

    if (page_size.page_size_ > std::numeric_limits<uint32_t>::max()) {
        isc_throw(OutOfRange, "page size of retrieved leases must not be greater than "
                                  << std::numeric_limits<uint32_t>::max());
    }

    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_PAGE4)
        .arg(page_size.page_size_)
        .arg(lower_bound_address.toText());

    AnyArray data;

    cass_int64_t address_data = 0;
    if (!lower_bound_address.isV4Zero()) {
        address_data = static_cast<cass_int64_t>(lower_bound_address.toUint32());
        data.add(&address_data);
    }

    cass_int32_t page_size_data = static_cast<cass_int32_t>(page_size.page_size_);
    data.add(&page_size_data);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease4Collection result;
    CqlLease4Exchange exchange(connection_);
    exchange.getLeaseCollection(lower_bound_address.isV4Zero() ?
                                    CqlLease4Exchange::GET_LEASE4_LIMIT() :
                                    CqlLease4Exchange::GET_LEASE4_PAGE(),
                                data, result);

    return (result);
}

Lease6Ptr CqlLeaseMgr::getLease6(Lease::Type lease_type, const IOAddress &addr) const {
    std::string addr_data = addr.toText();
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_ADDR6)
        .arg(addr_data)
        .arg(lease_type);

    // Set up the WHERE clause value
    AnyArray data;

    if (addr_data.size() > ADDRESS6_TEXT_MAX_LEN) {
        isc_throw(BadValue, "CqlLeaseMgr::getLease6(): "
                            "address "
                                << addr_data << " of length " << addr_data.size()
                                << " exceeds maximum allowed length of " << ADDRESS6_TEXT_MAX_LEN);
    }
    data.add(&addr_data);

    cass_int32_t lease_type_data = static_cast<cass_int32_t>(lease_type);
    data.add(&lease_type_data);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    Lease6Ptr result;
    CqlLease6Exchange exchange(connection_);
    exchange.getLease(CqlLease6Exchange::GET_LEASE6_ADDR(), data, result);

    return (result);
}

Lease6Collection CqlLeaseMgr::getLeases6(const DUID &duid) const {

    // Set up the WHERE clause value
    AnyArray data;

    CassBlob duid_data(duid.getDuid());

    data.add(&duid_data);

    // Get the data.
    Lease6Collection result;
    CqlLease6Exchange exchange(connection_);
    exchange.getLeaseCollection(CqlLease6Exchange::GET_LEASE6_DUID(), data, result);

    return (result);
}

Lease6Collection
CqlLeaseMgr::getLeases6(Lease::Type lease_type, const DUID &duid, uint32_t iaid) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_IAID_DUID)
        .arg(iaid)
        .arg(duid.toText())
        .arg(lease_type);

    // Set up the WHERE clause value

    CassBlob duid_data(duid.getDuid());
    cass_int32_t iaid_data = static_cast<cass_int32_t>(iaid);
    cass_int32_t lease_type_data = static_cast<cass_int32_t>(lease_type);
    AnyArray data{&duid_data, &iaid_data, &lease_type_data};

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease6Collection result;
    CqlLease6Exchange exchange(connection_);
    exchange.getLeaseCollection(CqlLease6Exchange::GET_LEASE6_DUID_IAID(), data, result);

    return (result);
}

Lease6Collection CqlLeaseMgr::getLeases6(Lease::Type lease_type,
                                         const DUID &duid,
                                         uint32_t iaid,
                                         SubnetID subnet_id) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_IAID_SUBID_DUID)
        .arg(iaid)
        .arg(subnet_id)
        .arg(duid.toText())
        .arg(lease_type);

    // Set up the WHERE clause value
    AnyArray data;

    CassBlob duid_data(duid.getDuid());
    cass_int32_t iaid_data = static_cast<cass_int32_t>(iaid);

    data.add(&duid_data);
    data.add(&iaid_data);

    cass_int32_t lease_type_data = static_cast<cass_int32_t>(lease_type);
    data.add(&lease_type_data);

    cass_int32_t subnet_id_data = static_cast<cass_int32_t>(subnet_id);
    data.add(&subnet_id_data);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease6Collection result;
    CqlLease6Exchange exchange(connection_);
    exchange.getLeaseCollection(CqlLease6Exchange::GET_LEASE6_DUID_IAID_SUBID(), data, result);

    return (result);
}

Lease6Collection CqlLeaseMgr::getLeases6(SubnetID subnet_id) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_SUBID6).arg(subnet_id);

    // Set up the WHERE clause value
    AnyArray data;

    cass_int32_t subnet_id_data = static_cast<cass_int32_t>(subnet_id);
    data.add(&subnet_id_data);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease6Collection result;
    CqlLease6Exchange exchange(connection_);
    exchange.getLeaseCollection(CqlLease6Exchange::GET_LEASE6_SUBID(), data, result);

    return (result);
}

Lease6Collection CqlLeaseMgr::getLeases6(const std::string &hostname) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_HOSTNAME6).arg(hostname);

    // Set up the WHERE clause value
    AnyArray data;

    std::string hostname_data(hostname);
    data.add(&hostname_data);

    // Get the data.
    Lease6Collection result;
    CqlLease6Exchange exchange(connection_);
    exchange.getLeaseCollection(CqlLease6Exchange::GET_LEASE6_HOSTNAME(), data, result);

    return (result);
}

Lease6Collection CqlLeaseMgr::getLeases6() const {
    isc_throw(NotImplemented, "getLeases6() is not implemented");
}

Lease6Collection CqlLeaseMgr::getLeases6(const asiolink::IOAddress &lower_bound_address,
                                         const LeasePageSize &page_size) const {
    // Expecting IPv6 address.
    if (!lower_bound_address.isV6()) {
        isc_throw(InvalidAddressFamily, "expected IPv6 address while "
                                        "retrieving leases from the lease database, got "
                                            << lower_bound_address);
    }

    if (page_size.page_size_ == 0) {
        isc_throw(OutOfRange, "page size of retrieved leases must not be 0");
    }

    if (page_size.page_size_ > std::numeric_limits<uint32_t>::max()) {
        isc_throw(OutOfRange, "page size of retrieved leases must not be greater than "
                                  << std::numeric_limits<uint32_t>::max());
    }

    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_PAGE6)
        .arg(page_size.page_size_)
        .arg(lower_bound_address.toText());

    AnyArray data;

    std::string lb_address_data;
    if (!lower_bound_address.isV6Zero()) {
        lb_address_data = lower_bound_address.toText();
        if (lb_address_data.size() > ADDRESS6_TEXT_MAX_LEN) {
            isc_throw(BadValue, "CqlLeaseMgr::getLeases6(lower_bound_address, page_size): "
                                "address "
                                    << lb_address_data << " of length " << lb_address_data.size()
                                    << " exceeds maximum allowed length of "
                                    << ADDRESS6_TEXT_MAX_LEN);
        }
        data.add(&lb_address_data);
    }

    cass_int32_t page_size_data = static_cast<cass_int32_t>(page_size.page_size_);
    data.add(&page_size_data);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the leases.
    Lease6Collection result;
    CqlLease6Exchange exchange(connection_);
    exchange.getLeaseCollection(lower_bound_address.isV6Zero() ?
                                    CqlLease6Exchange::GET_LEASE6_LIMIT() :
                                    CqlLease6Exchange::GET_LEASE6_PAGE(),
                                data, result);

    return (result);
}

void CqlLeaseMgr::getExpiredLeases4(Lease4Collection &expired_leases,
                                    const size_t max_leases) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_EXPIRED4).arg(max_leases);

    CqlLease4Exchange exchange(connection_);
    exchange.getExpiredLeases(max_leases, expired_leases);
}

void CqlLeaseMgr::getExpiredLeases6(Lease6Collection &expired_leases,
                                    const size_t max_leases) const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_EXPIRED6).arg(max_leases);

    CqlLease6Exchange exchange(connection_);
    exchange.getExpiredLeases(max_leases, expired_leases);
}

void CqlLeaseMgr::updateLease4(const Lease4Ptr &lease) {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_UPDATE_ADDR4)
        .arg(lease->addr_.toText());

    CqlLease4Exchange exchange(connection_);

    try {
        AnyArray data;
#if CASSANDRA_DENORMALIZED_TABLES
        exchange.createBindForUpdate(lease, data, CqlLease4Exchange::UPDATE_LEASE4_ADDRESS());
        exchange.executeMutation(connection_, data, CqlLease4Exchange::UPDATE_LEASE4_ADDRESS());
        exchange.createBindForUpdate(lease, data,
                                     CqlLease4Exchange::UPDATE_LEASE4_CLIENTID_SUBNETID());
        exchange.executeMutation(connection_, data,
                                 CqlLease4Exchange::UPDATE_LEASE4_CLIENTID_SUBNETID());
        exchange.createBindForUpdate(lease, data,
                                     CqlLease4Exchange::UPDATE_LEASE4_HWADDR_SUBNETID());
        exchange.executeMutation(connection_, data,
                                 CqlLease4Exchange::UPDATE_LEASE4_HWADDR_SUBNETID());
        exchange.createBindForUpdate(lease, data, CqlLease4Exchange::UPDATE_LEASE4_SUBNETID());
        exchange.executeMutation(connection_, data, CqlLease4Exchange::UPDATE_LEASE4_SUBNETID());
#else  // CASSANDRA_DENORMALIZED_TABLES
        exchange.createBindForUpdate(lease, data, CqlLease4Exchange::UPDATE_LEASE4());
        exchange.executeMutation(connection_, data, CqlLease4Exchange::UPDATE_LEASE4());
#endif  // CASSANDRA_DENORMALIZED_TABLES
    } catch (StatementNotApplied const &exception) {
        isc_throw(NoSuchLease, exception.what());
    }

    lease->old_cltt_ = lease->cltt_;
    lease->old_valid_lft_ = lease->valid_lft_;
}

void CqlLeaseMgr::updateLease6(const Lease6Ptr &lease) {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_UPDATE_ADDR6)
        .arg(lease->addr_.toText());

    CqlLease6Exchange exchange(connection_);

    try {
        AnyArray data;
#if CASSANDRA_DENORMALIZED_TABLES
        exchange.createBindForUpdate(lease, data,
                                     CqlLease6Exchange::UPDATE_LEASE6_ADDRESS_LEASETYPE());
        exchange.executeMutation(connection_, data,
                                 CqlLease6Exchange::UPDATE_LEASE6_ADDRESS_LEASETYPE());
        exchange.createBindForUpdate(
            lease, data, CqlLease6Exchange::UPDATE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID());
        exchange.executeMutation(connection_, data,
                                 CqlLease6Exchange::UPDATE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID());
#else  // CASSANDRA_DENORMALIZED_TABLES
        exchange.createBindForUpdate(lease, data, CqlLease6Exchange::UPDATE_LEASE6());
        exchange.executeMutation(connection_, data, CqlLease6Exchange::UPDATE_LEASE6());
#endif  // CASSANDRA_DENORMALIZED_TABLES
    } catch (StatementNotApplied const &exception) {
        isc_throw(NoSuchLease, exception.what());
    }

    lease->old_cltt_ = lease->cltt_;
    lease->old_valid_lft_ = lease->valid_lft_;
}

bool CqlLeaseMgr::deleteLease(const Lease4Ptr &lease) {
    return deleteLeaseInternal(lease);
}

bool CqlLeaseMgr::deleteLeaseInternal(const Lease4Ptr &lease) {
    const IOAddress &addr = lease->addr_;
    std::string addr_data = addr.toText();
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_DELETE_ADDR).arg(addr_data);

    // Set up the WHERE clause value
    AnyArray data;

    CqlLease4Exchange exchange(connection_);

    try {
#if CASSANDRA_DENORMALIZED_TABLES
        exchange.createBindForDelete(lease, data, CqlLease4Exchange::DELETE_LEASE4_ADDRESS());
        exchange.executeMutation(connection_, data, CqlLease4Exchange::DELETE_LEASE4_ADDRESS());
        exchange.createBindForDelete(lease, data,
                                     CqlLease4Exchange::DELETE_LEASE4_CLIENTID_SUBNETID());
        exchange.executeMutation(connection_, data,
                                 CqlLease4Exchange::DELETE_LEASE4_CLIENTID_SUBNETID());
        exchange.createBindForDelete(lease, data,
                                     CqlLease4Exchange::DELETE_LEASE4_HWADDR_SUBNETID());
        exchange.executeMutation(connection_, data,
                                 CqlLease4Exchange::DELETE_LEASE4_HWADDR_SUBNETID());
        exchange.createBindForDelete(lease, data, CqlLease4Exchange::DELETE_LEASE4_SUBNETID());
        exchange.executeMutation(connection_, data, CqlLease4Exchange::DELETE_LEASE4_SUBNETID());
#else  // CASSANDRA_DENORMALIZED_TABLES
        exchange.createBindForDelete(lease, data, CqlLease4Exchange::DELETE_LEASE4());
        exchange.executeMutation(connection_, data, CqlLease4Exchange::DELETE_LEASE4());
#endif  // CASSANDRA_DENORMALIZED_TABLES
    } catch (const Exception &exception) {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_LEASE_EXCEPTION_THROWN)
            .arg(exception.what());
        return false;
    }
    return true;
}

bool CqlLeaseMgr::deleteLease(const Lease6Ptr &lease) {
    return deleteLeaseInternal(lease);
}

bool CqlLeaseMgr::deleteLeaseInternal(const Lease6Ptr &lease) {
    const IOAddress &addr = lease->addr_;
    std::string addr_data = addr.toText();
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_DELETE_ADDR).arg(addr_data);

    // Set up the WHERE clause value
    AnyArray data;

    CqlLease6Exchange exchange(connection_);

    try {
#if CASSANDRA_DENORMALIZED_TABLES
        exchange.createBindForDelete(lease, data,
                                     CqlLease6Exchange::DELETE_LEASE6_ADDRESS_LEASETYPE());
        exchange.executeMutation(connection_, data,
                                 CqlLease6Exchange::DELETE_LEASE6_ADDRESS_LEASETYPE());
        exchange.createBindForDelete(
            lease, data, CqlLease6Exchange::DELETE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID());
        exchange.executeMutation(connection_, data,
                                 CqlLease6Exchange::DELETE_LEASE6_DUID_IAID_LEASETYPE_SUBNETID());
#else  // CASSANDRA_DENORMALIZED_TABLES
        exchange.createBindForDelete(lease, data, CqlLease6Exchange::DELETE_LEASE6());
        exchange.executeMutation(connection_, data, CqlLease6Exchange::DELETE_LEASE6());
#endif  // CASSANDRA_DENORMALIZED_TABLES
    } catch (const Exception &exception) {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_LEASE_EXCEPTION_THROWN)
            .arg(exception.what());
        return false;
    }
    return true;
}

uint64_t CqlLeaseMgr::deleteExpiredReclaimedLeases4(const uint32_t secs) {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_DELETE_EXPIRED_RECLAIMED4)
        .arg(secs);
#ifdef CASSANDRA_DENORMALIZED_TABLES
    return 0u;
#else  // CASSANDRA_DENORMALIZED_TABLES
    AnyArray data;
    uint64_t deleted = 0u;
    cass_int32_t limit = 1024;

    // State is reclaimed.
    cass_int32_t state = static_cast<cass_int32_t>(Lease::STATE_EXPIRED_RECLAIMED);
    data.add(&state);

    // Expiration timestamp.
    cass_int64_t expiration = static_cast<cass_int64_t>(time(NULL) - static_cast<time_t>(secs));
    data.add(&expiration);

    data.add(&limit);

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease4Collection leases;
    CqlLease4Exchange exchange(connection_);
    exchange.getLeaseCollection(CqlLease4Exchange::GET_LEASE4_EXPIRE(), data, leases);
    for (Lease4Ptr &lease : leases) {
        if (deleteLeaseInternal(lease)) {
            ++deleted;
        }
    }
    return (deleted);
#endif  // CASSANDRA_DENORMALIZED_TABLES
}

uint64_t CqlLeaseMgr::deleteExpiredReclaimedLeases6(const uint32_t secs) {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_DELETE_EXPIRED_RECLAIMED6)
        .arg(secs);
#ifdef CASSANDRA_DENORMALIZED_TABLES
    return 0u;
#else  // CASSANDRA_DENORMALIZED_TABLES
    uint64_t deleted = 0u;
    cass_int32_t limit = 1024;

    // State is reclaimed.
    cass_int32_t state = static_cast<cass_int32_t>(Lease::STATE_EXPIRED_RECLAIMED);

    // Expiration timestamp.
    cass_int64_t expiration = static_cast<cass_int64_t>(time(NULL) - static_cast<time_t>(secs));

    AnyArray data{&state, &expiration, &limit};

#ifdef TERASTREAM_FULL_TRANSACTIONS
    CassUuid txid(connection_.getTransactionID());
    data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

    // Get the data.
    Lease6Collection leases;
    CqlLease6Exchange exchange(connection_);
    exchange.getLeaseCollection(CqlLease6Exchange::GET_LEASE6_EXPIRE(), data, leases);
    for (Lease6Ptr &lease : leases) {
        if (deleteLeaseInternal(lease)) {
            ++deleted;
        }
    }
    return (deleted);
#endif  // CASSANDRA_DENORMALIZED_TABLES
}

LeaseStatsQueryPtr CqlLeaseMgr::startLeaseStatsQuery4() {
    LeaseStatsQueryPtr query(
        new CqlLeaseStatsQuery(connection_, CqlLeaseStatsQuery::ALL_LEASE4_STATS(), false));
    query->start();
    return query;
}

LeaseStatsQueryPtr CqlLeaseMgr::startSubnetLeaseStatsQuery4(const SubnetID &subnet_id) {
    LeaseStatsQueryPtr query(new CqlLeaseStatsQuery(
        connection_, CqlLeaseStatsQuery::SUBNET_LEASE4_STATS(), false, subnet_id));
    query->start();
    return query;
}

LeaseStatsQueryPtr CqlLeaseMgr::startSubnetRangeLeaseStatsQuery4(const SubnetID &first_subnet_id,
                                                                 const SubnetID &last_subnet_id) {
    LeaseStatsQueryPtr query(new CqlLeaseStatsQuery(connection_,
                                                    CqlLeaseStatsQuery::SUBNET_RANGE_LEASE4_STATS(),
                                                    false, first_subnet_id, last_subnet_id));
    query->start();
    return query;
}

LeaseStatsQueryPtr CqlLeaseMgr::startLeaseStatsQuery6() {
    LeaseStatsQueryPtr query(
        new CqlLeaseStatsQuery(connection_, CqlLeaseStatsQuery::ALL_LEASE6_STATS(), true));
    query->start();
    return query;
}

LeaseStatsQueryPtr CqlLeaseMgr::startSubnetLeaseStatsQuery6(const SubnetID &subnet_id) {
    LeaseStatsQueryPtr query(new CqlLeaseStatsQuery(
        connection_, CqlLeaseStatsQuery::SUBNET_LEASE6_STATS(), true, subnet_id));
    query->start();
    return query;
}

LeaseStatsQueryPtr CqlLeaseMgr::startSubnetRangeLeaseStatsQuery6(const SubnetID &first_subnet_id,
                                                                 const SubnetID &last_subnet_id) {
    LeaseStatsQueryPtr query(new CqlLeaseStatsQuery(connection_,
                                                    CqlLeaseStatsQuery::SUBNET_RANGE_LEASE6_STATS(),
                                                    true, first_subnet_id, last_subnet_id));
    query->start();
    return query;
}

size_t CqlLeaseMgr::wipeLeases4(const SubnetID & /*subnet_id*/) {
    /// @todo: Need to implement this, so wipe leases would work.
    isc_throw(NotImplemented, "wipeLeases4 is not implemented for Cassandra backend");
}

size_t CqlLeaseMgr::wipeLeases6(const SubnetID & /*subnet_id*/) {
    /// @todo: Need to implement this, so wipe leases would work.
    isc_throw(NotImplemented, "wipeLeases6 is not implemented for Cassandra backend");
}

std::string CqlLeaseMgr::getName() const {
    std::string name;
    try {
        name = connection_.getParameter("name");
    } catch (...) {
        // Return an empty name
    }
    return name;
}

std::string CqlLeaseMgr::getDescription() const {
    return std::string("Cassandra Database");
}

VersionTuple CqlLeaseMgr::getVersion() const {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_VERSION);

    CqlVersionExchange<VersionTuple> exchange;
    return *exchange.retrieveVersion(connection_);
}

void CqlLeaseMgr::commit() {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_COMMIT);
    connection_.commit();
}

void CqlLeaseMgr::rollback() {
    LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_ROLLBACK);
    connection_.rollback();
}

}  // namespace dhcp
}  // namespace isc

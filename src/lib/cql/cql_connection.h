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

#ifndef CQL_CONNECTION_H
#define CQL_CONNECTION_H

#include <cql/cql_memory_mgr.h>
#include <database/common.h>
#include <database/database_connection.h>

#include <cassandra.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace isc {
namespace db {

using StatementTag = std::string;
using StatementText = std::string;

/// @brief Defines a single statement or query
struct CqlTaggedStatement {
    /// @brief Constructor
    /// @param name brief name of the query
    /// @param text text (CQL) representation of the query
    /// @param is_raw should the statement be executed raw?
    CqlTaggedStatement(StatementTag const& name,
                       StatementText const& text,
                       bool const is_raw = false)
        : name_(name), text_(text), is_raw_(is_raw) {
    }

    /// Short description of the query
    StatementTag const name_;

    /// Text representation of the actual query
    StatementText const text_;

    /// Internal Cassandra object representing the prepared statement
    CassPreparedPtr prepared_statement_;

    /// Should the statement be executed raw or with binds?
    bool const is_raw_;
};

using StatementMap = std::unordered_map<StatementTag, CqlTaggedStatement>;

/// @brief Common CQL connector pool
///
/// Provides common operations for the Cassandra database connection used by
/// CqlLeaseMgr, CqlHostDataSource and CqlShardConfigMgr. Manages the connection
/// to the Cassandra database and preparing of compiled statements. Its fields
/// are public because they are used (both set and retrieved) in classes that
/// use instances of CqlConnection.
struct CqlConnection : isc::db::DatabaseConnection {
    /// @brief Constructor
    ///
    /// Initialize CqlConnection object with parameters needed for connection.
    /// @param parameters specify the connection details (username, ip addresses etc.)
    explicit CqlConnection(ParameterMap const& parameters);

    /// @brief Destructor
    virtual ~CqlConnection() = default;

    /// @brief Get the schema version.
    ///
    /// @param parameters A data structure relating keywords and values
    ///        concerned with the database.
    ///
    /// @return Version number as a pair of unsigned integers.  "first" is the
    ///         major version number, "second" the minor number.
    ///
    /// @throw isc::db::DbOperationError An operation on the open database has
    ///        failed.
    static std::pair<uint32_t, uint32_t> getVersion(const ParameterMap& parameters);

    /// @brief Prepare statements
    ///
    /// Creates the prepared statements for all of the CQL statements used
    /// by the CQL backend.
    /// @param statements statements to be prepared
    ///
    /// @throw isc::db::DbOperationError if an operation on the open database
    ///     has failed
    /// @throw isc::InvalidParameter if there is an invalid access in the
    ///     vector. This represents an internal error within the code.
    void prepareStatements(StatementMap& statements);

    /// @brief Open database
    ///
    /// Opens the database using the information supplied in the parameters
    /// passed to the constructor. If no parameters are supplied, the default
    /// values will be used. The parameters supported as as follows (default
    /// values specified in parentheses):
    /// - keyspace: name of the database to which to connect (keatest)
    /// - contact-points: IP addresses of the nodes to connect to (127.0.0.1)
    /// - consistency: consistency level (quorum)
    /// - serial-consistency: serial consistency level (serial)
    /// - port: The TCP port to use (9042)
    /// - user - credentials to use when connecting (no username)
    /// - password - credentials to use when connecting (no password)
    /// - reconnect-wait-time 2000
    /// - connect-timeout 5000
    /// - request-timeout 12000
    /// - tcp-keepalive no
    /// - tcp-nodelay no
    /// - max-statement-tries 3
    ///
    /// @throw DbOpenError error opening the database
    void openDatabase();

#ifdef TERASTREAM_FULL_TRANSACTIONS
    /// @brief Set transaction operations
    void setTransactionOperations(StatementTag const& begin_index,
                                  StatementTag const& commit_index,
                                  StatementTag const& rollback_index);

    /// @brief Get transaction identifier
    CassUuid getTransactionID() const;
#endif  // TERASTREAM_FULL_TRANSACTIONS

    /// @brief Set consistency
    void
    setConsistency(bool force, CassConsistency consistency, CassConsistency serial_consistency);

    /// @brief Start transaction
    void startTransaction();

    /// @brief Commit Transactions
    virtual void commit();

    /// @brief Rollback Transactions
    virtual void rollback();

    /// @brief parse Consistency value
    static CassConsistency parseConsistency(std::string const& value);

    /// @brief Check for errors
    ///
    /// Check for errors on the current database operation and returns text
    /// description of what happened. In case of success, also returns
    /// some logging friendly text.
    ///
    /// @param what text description of the operation
    /// @param future the structure that holds the status of operation
    /// @param statement_tag statement that was used (optional)
    /// @return text description of the error
    static std::string const checkFutureError(std::string const& what,
                                              CassFuturePtr& future,
                                              StatementTag const& statement_tag = StatementTag());

#ifdef TERASTREAM_FULL_TRANSACTIONS
    /// @brief Begin index
    std::string begin_index_;

    /// @brief Commit index
    std::string commit_index_;

    /// @brief Rollback index
    std::string rollback_index_;
#endif  // TERASTREAM_FULL_TRANSACTIONS

    /// @brief Pointer to external array of tagged statements containing
    ///     statement name, array of names of bind parameters and text query
    StatementMap statements_;

    /// @brief CQL connection handle
    CassClusterPtr cluster_;

    /// @brief CQL session handle
    CassSessionPtr session_;

    /// @brief CQL consistency
    CassConsistency consistency_;

    /// @brief CQL serial consistency
    CassConsistency serial_consistency_;

    /// @brief Maximum tries for any executeMutation().
    uint32_t max_statement_tries_;

    /// @brief CQL consistency enabled
    bool force_consistency_;
};

using CqlConnectionPtr = std::shared_ptr<CqlConnection>;

struct CqlUuid {
    static CassUuidGen*& generator() {
        static CassUuidGen* generator(cass_uuid_gen_new());
        return generator;
    }

private:
    /// @brief Singleton
    CqlUuid() = default;

    /// @brief non-copyable
    /// @{
    CqlUuid(CqlUuid const&) = delete;
    CqlUuid& operator=(CqlUuid const&) = delete;
    /// @}
};

}  // namespace db
}  // namespace isc

#endif  // CQL_CONNECTION_H

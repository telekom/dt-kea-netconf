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

#ifndef CQL_TRANSACTION_H
#define CQL_TRANSACTION_H

#include <cql/cql_connection.h>
#include <cql/cql_exchange.h>

#include <mutex>

namespace isc {
namespace db {

/// @brief Structure to store the transaction for each tread
/// @{
struct TransactionData {
    /// @brief Transaction UUID
    CassUuid uuid_;
    /// @brief List of connections using this transaction
    std::vector<CqlConnection*> connections_;
};
using TransactionDataPtr = std::shared_ptr<TransactionData>;
using TransactionContainer = std::map<pthread_t, TransactionDataPtr>;
/// @}

/// @brief Forward declaration
struct CqlTransactionMgr;

/// @brief Defines a pointer to a CqlTransactionMgr.
using CqlTransactionMgrPtr = std::shared_ptr<CqlTransactionMgr>;

/// @brief Exchange used to enclose multiple operations into a transaction
struct CqlTransactionExchange : CqlExchange<CassUuid> {
    /// @brief Create BIND array to receive C++ data.
    ///
    /// Used in executeSelect() to retrieve from database
    ///
    /// @param data array of bound objects representing data to be retrieved.
    /// @param statement_tag prepared statement being executed; defaults to an
    ///     invalid index
    void createBindForSelect(AnyArray& data,
                             StatementTag const& statement_tag = StatementTag()) override final;

    /// @brief Copy received data into a holistic object.
    ///
    /// Returns a reference to the transaction ID.
    ///
    /// @return a pointer to the object retrieved.
    Ptr<CassUuid> retrieve() override final;

    /// @brief start a transaction
    void startTransaction(CqlConnection& connection, CassUuid& uuid);

    /// @brief commit a transaction
    void commit(CqlConnection& connection, CassUuid& uuid);

    /// @brief rollback a transaction
    void rollback(CqlConnection& connection, CassUuid& uuid);

    /// @brief check if commit/rollback action has succeeded
    static bool hasTransactionActionBeenApplied(CassFuturePtr& future, uint32_t& status);

    static StatementMap& tagged_statements() {
        static StatementMap _{{BEGIN_TXN(), {BEGIN_TXN(), "BEGIN TXN ? "}},
                              {COMMIT_TXN(), {COMMIT_TXN(), "COMMIT TXN ? "}},
                              {ROLLBACK_TXN(), {ROLLBACK_TXN(), "ABORT TXN ? "}}};

        return _;
    }

    /// @brief Statement tags
    /// @{
    static StatementTag& BEGIN_TXN() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag& COMMIT_TXN() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }

    static StatementTag& ROLLBACK_TXN() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    /// @}

private:
    /// @brief transaction UUID
    CassUuid txn_;
};

/// @brief Transaction Manager responsible for Cassandra transactions
struct CqlTransactionMgr {
    /// @brief Singleton instance retriever
    static CqlTransactionMgr& instance();

    /// @brief start a new transaction on connection
    bool startTransaction(CqlConnection* connection);

    /// @brief register commit on transaction using connection
    bool commit(CqlConnection* connection);

    /// @brief register rollback on transaction using connection
    bool rollback(CqlConnection* connection);

    /// @brief get UUID of the transaction opened on connection
    CassUuid getTransactionID(CqlConnection const* connection);

    /// @brief add connection for transaction on current thread
    bool pushTransaction(CqlConnection* connection, CassUuid uuid);

    /// @brief remove connection for transaction on current thread
    bool popTransaction(CqlConnection* connection, CassUuid uuid);

private:
    /// @brief Singleton
    CqlTransactionMgr() = default;

    /// @brief non-copyable
    /// @{
    CqlTransactionMgr(CqlTransactionMgr const&) = delete;
    CqlTransactionMgr& operator=(CqlTransactionMgr const&) = delete;
    /// @}

    /// @brief CQL map of opened transactions for each thread
    TransactionContainer transactions_;

    /// @brief interface for the singleton
    static CqlTransactionMgrPtr const& instancePtr();
};

/// @brief RAII object representing CQL transaction.
///
/// An instance of this class should be created in a scope where multiple
/// INSERT statements should be executed within a single transaction. The
/// transaction is started when the constructor of this class is invoked.
/// The transaction is ended when the @ref CqlTransaction::commit is
/// explicitly called or when the instance of this class is destroyed.
/// The @ref CqlTransaction::commit commits changes to the database
/// and the changes remain in the database when the instance of the
/// class is destroyed. If the class instance is destroyed before the
/// @ref CqlTransaction::commit is called, the transaction is rolled
/// back. The rollback on destruction guarantees that partial data is
/// not stored in the database when there is an error during any
/// of the operations belonging to a transaction.
struct CqlTransaction {
    /// @brief Constructor
    ///
    /// Starts transaction by making a "START TRANSACTION" query.
    ///
    /// @param conn CQL connection to use for the transaction. This connection
    ///     will be later used to commit or rollback changes.
    ///
    /// @throw DbOperationError if "START TRANSACTION" query fails.
    explicit CqlTransaction(CqlConnection& conn);

    /// @brief Destructor
    ///
    /// Rolls back the transaction if changes haven't been committed.
    ~CqlTransaction();

    /// @brief Commits transaction
    ///
    /// Calls @ref CqlConnection::commit()..
    void commit();

private:
    /// @brief non-copyable
    /// @{
    CqlTransaction(CqlTransaction const&) = delete;
    CqlTransaction& operator=(CqlTransaction const&) = delete;
    /// @}

    /// @brief Holds reference to the CQL database connection.
    CqlConnection& connection_;

    /// @brief Boolean flag indicating if the transaction has been committed.
    ///
    /// This flag is used in the class destructor to assess if the
    /// transaction should be rolled back.
    bool committed_;
};

}  // namespace db
}  // namespace isc

#endif  // CQL_TRANSACTION_H

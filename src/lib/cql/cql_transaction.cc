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

#include <cql/cql_connection.h>
#include <cql/cql_transaction.h>

#include <database/db_exceptions.h>
#include <database/db_log.h>

using namespace isc::log;

using std::mutex;

namespace isc {
namespace db {

CqlTransactionMgr& CqlTransactionMgr::instance() {
    return *instancePtr();
}

CqlTransactionMgrPtr const& CqlTransactionMgr::instancePtr() {
    static CqlTransactionMgrPtr transactionMgr(new CqlTransactionMgr());
    return transactionMgr;
}

bool CqlTransactionMgr::pushTransaction(CqlConnection* connection, CassUuid uuid) {
#ifdef NDEBUG
    // Parameters are only used in assert() macro which is disabled when
    //      NDEBUG is enabled.
    (void)connection;  // [maybe_unused]
    (void)uuid;  // [maybe_unused]
#endif  // NDEBUG

    bool result = true;
    pthread_t thread_id = pthread_self();
    TransactionContainer::const_iterator it = transactions_.find(thread_id);
    if (it == transactions_.end()) {
        TransactionDataPtr transaction_data(std::make_shared<TransactionData>());
        transaction_data->uuid_ = uuid;
        transaction_data->connections_.push_back(connection);
        transactions_[thread_id] = transaction_data;
    } else {
        TransactionDataPtr transaction_data = it->second;
        assert(memcmp(static_cast<void*>(&transaction_data->uuid_), static_cast<void*>(&uuid),
                      sizeof(uuid)) == 0);
        transaction_data->connections_.push_back(connection);
        result = false;
    }
    return result;
}

bool CqlTransactionMgr::popTransaction(CqlConnection* connection, CassUuid uuid) {
#ifdef NDEBUG
    // Parameters are only used in assert() macro which is disabled when
    //      NDEBUG is enabled.
    (void)connection;  // [maybe_unused]
    (void)uuid;  // [maybe_unused]
#endif  // NDEBUG

    pthread_t thread_id = pthread_self();
    TransactionContainer::const_iterator it = transactions_.find(thread_id);
    if (it == transactions_.end()) {
        DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_TRANSACTION_MGR_TRANSACTION_NOT_FOUND);
        isc_throw(TransactionException, "CqlTransactionMgr::popTransaction(): "
                                        "no transaction started on thread "
                                            << thread_id);
    }
    TransactionDataPtr transaction_data = it->second;
    assert(memcmp(static_cast<void*>(&transaction_data->uuid_), static_cast<void*>(&uuid),
                  sizeof(uuid)) == 0);
    size_t count = transaction_data->connections_.size();
    assert(count > 0);
    assert(transaction_data->connections_[count - 1] == connection);
    transaction_data->connections_.pop_back();
    if (count == 1) {
        transactions_.erase(thread_id);
    } else {
        return false;
    }
    return true;
}

bool CqlTransactionMgr::startTransaction(CqlConnection* connection) {
    DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_TRANSACTION_MGR_TRANSACTION_START);
#ifdef NDEBUG
    // Parameters are only used in assert macro which is disabled when
    //      NDEBUG is enabled.
    (void)connection;  // [maybe_unused]
    (void)uuid;  // [maybe_unused]
#endif  // NDEBUG

#ifdef TERASTREAM_FULL_TRANSACTIONS
    char uuid_str[CASS_UUID_STRING_LENGTH];
    bool transaction_error = false;
    bool transaction = true;
    CassUuid uuid;
    pthread_t thread_id = pthread_self();
    TransactionContainer::const_iterator it = transactions_.find(thread_id);
    if (it != transactions_.end()) {
        TransactionDataPtr transaction_data = it->second;
        uuid = transaction_data->uuid_;
        transaction = false;
        cass_uuid_string(uuid, uuid_str);
        DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_TRANSACTION_MGR_REGISTER).arg(uuid_str);
    }
    if (transaction) {
        try {
            CqlTransactionExchange exchange;
            transaction_exchange.startTransaction(*connection, uuid);
            cass_uuid_string(uuid, uuid_str);
            DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_TRANSACTION_MGR_TRANSACTION_START).arg(uuid_str);
        } catch (std::exception const&) {
            transaction_error = true;
        }
    }
    if (!transaction_error) {
        pushTransaction(connection, uuid);
    }
    if (transaction_error) {
        isc_throw(TransactionException,
                  "CqlTransactionMgr::startTransaction() failed on thread " << thread_id);
    }
#else  // TERASTREAM_FULL_TRANSACTIONS
    (void)connection;  // [maybe_unused]
#endif  // TERASTREAM_FULL_TRANSACTIONS
    return true;
}

bool CqlTransactionMgr::commit(CqlConnection* connection) {
    DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_TRANSACTION_MGR_TRANSACTION_COMMIT);

#ifdef NDEBUG
    // Parameters are only used in assert macro which is disabled when
    //      NDEBUG is enabled.
    (void)connection;  // [maybe_unused]
#endif  // NDEBUG

#ifdef TERASTREAM_FULL_TRANSACTIONS
    char uuid_str[CASS_UUID_STRING_LENGTH];
    bool transaction_error = false;
    bool transaction = true;
    CassUuid uuid;
    pthread_t thread_id = pthread_self();
    TransactionContainer::const_iterator it = transactions_.find(thread_id);
    if (it == transactions_.end()) {
        transaction_error = true;
        transaction = false;
    } else {
        TransactionDataPtr transaction_data = it->second;
        size_t count = transaction_data->connections_.size();
        assert(count > 0);
        assert(transaction_data->connections_[count - 1] == connection);
        uuid = transaction_data->uuid_;
        if (count != 1) {
            transaction = false;
            cass_uuid_string(uuid, uuid_str);
            DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_TRANSACTION_MGR_REGISTER_TRANSACTION_COMMIT)
                .arg(uuid_str);
        }
        popTransaction(connection, uuid);
    }
    if (transaction) {
        try {
            CqlTransactionExchange exchange;
            exchange.commit(*connection, uuid);
            cass_uuid_string(uuid, uuid_str);
            DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_TRANSACTION_MGR_TRANSACTION_COMMIT).arg(uuid_str);
        } catch (std::exception const&) {
            transaction_error = true;
        }
    }
    if (transaction_error) {
        isc_throw(TransactionException,
                  "CqlTransactionMgr::commit() failed on thread " << thread_id);
    }
#else  // TERASTREAM_FULL_TRANSACTIONS
    (void)connection;  // [maybe_unused]
#endif  // TERASTREAM_FULL_TRANSACTIONS
    return true;
}

bool CqlTransactionMgr::rollback(CqlConnection* connection) {
    DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_TRANSACTION_MGR_TRANSACTION_ROLLBACK);

#ifdef TERASTREAM_FULL_TRANSACTIONS
#ifdef NDEBUG
    // Parameters are only used in assert macro which is disabled when
    //      NDEBUG is enabled.
    (void)connection;  // [maybe_unused]
#endif  // NDEBUG
    char uuid_str[CASS_UUID_STRING_LENGTH];
    bool transaction_error = false;
    bool transaction = true;
    CassUuid uuid;
    pthread_t thread_id = pthread_self();
    TransactionContainer::const_iterator it = transactions_.find(thread_id);
    if (it == transactions_.end()) {
        transaction_error = true;
        transaction = false;
    } else {
        TransactionDataPtr transaction_data = it->second;
        size_t count = transaction_data->connections_.size();
        assert(count > 0);
        assert(transaction_data->connections_[count - 1] == connection);
        uuid = transaction_data->uuid_;
        if (count != 1) {
            transaction = false;
            cass_uuid_string(uuid, uuid_str);
            DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_TRANSACTION_MGR_REGISTER_TRANSACTION_ROLLBACK)
                .arg(uuid_str);
        }
        popTransaction(connection, uuid);
    }
    if (transaction) {
        try {
            CqlTransactionExchange exchange;
            exchange.rollback(*connection, uuid);
            cass_uuid_string(uuid, uuid_str);
            DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_TRANSACTION_MGR_TRANSACTION_ROLLBACK)
                .arg(uuid_str);
        } catch (std::exception const&) {
            transaction_error = true;
        }
    }
    if (transaction_error) {
        isc_throw(TransactionException,
                  "CqlTransactionMgr::rollback() failed on thread " << thread_id);
    }
#else  // TERASTREAM_FULL_TRANSACTIONS
    (void)connection;  // [maybe_unused]
#endif  // TERASTREAM_FULL_TRANSACTIONS
    return true;
}

CassUuid CqlTransactionMgr::getTransactionID(CqlConnection const* connection) {
#ifdef NDEBUG
    // Parameters are only used in assert macro which is disabled when
    //      NDEBUG is enabled.
    (void)connection;  // [maybe_unused]
#endif  // NDEBUG

    CassUuid uuid;
    pthread_t thread_id = pthread_self();
    bool transaction_error = false;
    TransactionContainer::const_iterator it = transactions_.find(thread_id);
    if (it == transactions_.end()) {
        DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_TRANSACTION_MGR_TRANSACTION_NOT_FOUND);
        transaction_error = true;
    } else {
        TransactionDataPtr transaction_data = it->second;
        size_t count = transaction_data->connections_.size();
        assert(count > 0);
        assert(transaction_data->connections_[count - 1] == connection);
        uuid = transaction_data->uuid_;
    }
    if (transaction_error) {
        isc_throw(TransactionException,
                  "CqlTransactionMgr::getTransactionID() failed on thread " << thread_id);
    }
    return uuid;
}

CqlTransaction::CqlTransaction(CqlConnection& connection)
    : connection_(connection), committed_(false) {
    connection_.startTransaction();
}

CqlTransaction::~CqlTransaction() {
    // Rollback if commit() wasn't explicitly called.
    if (!committed_) {
        try {
            connection_.rollback();
        } catch (...) {
        }
    }
}

void CqlTransaction::commit() {
    connection_.commit();
    committed_ = true;
}

Ptr<CassUuid> CqlTransactionExchange::retrieve() {
    return Ptr<CassUuid>(new CassUuid(txn_));
}

void CqlTransactionExchange::createBindForSelect(
    AnyArray& data, StatementTag const& /* statement_tag = StatementTag() */) {
    data.add(&txn_);
}

void CqlTransactionExchange::startTransaction(CqlConnection& connection, CassUuid& uuid) {
    cass_uuid_gen_random(CqlUuid::generator(), &uuid);

    AnyArray data;
    data.add(&uuid);
    Collection<CassUuid> collection(executeSelect(connection, data, BEGIN_TXN(), SINGLE));
    CassUuid new_uuid = *collection.front();
    assert(memcmp(static_cast<void*>(&uuid), static_cast<void*>(&new_uuid), sizeof(uuid)) == 0);
}

void CqlTransactionExchange::commit(CqlConnection& connection, CassUuid& uuid) {
    AnyArray data;
    data.add(&uuid);
    executeMutation(connection, data, COMMIT_TXN(),
                    &CqlTransactionExchange::hasTransactionActionBeenApplied);
}

void CqlTransactionExchange::rollback(CqlConnection& connection, CassUuid& uuid) {
    AnyArray data;
    data.add(&uuid);
    executeMutation(connection, data, ROLLBACK_TXN(),
                    &CqlTransactionExchange::hasTransactionActionBeenApplied);
}

bool CqlTransactionExchange::hasTransactionActionBeenApplied(CassFuturePtr& future,
                                                             uint32_t& status) {
    auto resultCollection = MakePtr(cass_future_get_result(future.get()));
    auto rows = MakePtr(cass_iterator_from_result(resultCollection.get()));
    AnyArray data;
    cass_bool_t applied = cass_false;
    cass_int32_t result = -1;
    while (cass_iterator_next(rows.get())) {
        CassRow const* row = cass_iterator_get_row(rows.get());
        // [committed]: bool
        data.add(&applied);
        data.add(&result);
        CqlCommon::getData(row, data);
    }
    status = static_cast<uint32_t>(result);
    return applied == cass_true;
}

}  // namespace db
}  // namespace isc

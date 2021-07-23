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

#ifndef CQL_LOCK_H
#define CQL_LOCK_H

#include <cql/cql_connection.h>
#include <cql/cql_exchange.h>
#include <dhcp/duid.h>
#include <exceptions/exceptions.h>
#include <util/dhcp.h>

#include <memory>
#include <thread>
#include <vector>

namespace isc {
namespace db {

template <isc::dhcp::DhcpSpaceType>
struct LockData {
    LockData(std::vector<uint8_t> txn_id) : txn_id_(txn_id) {
    }

    /// @brief lock identifier
    std::vector<uint8_t> txn_id_;
};

/// @brief Pointer to a LockData structure.
template <isc::dhcp::DhcpSpaceType D>
using LockDataPtr = std::shared_ptr<LockData<D>>;

template <isc::dhcp::DhcpSpaceType D>
struct CqlLockExchange : isc::db::CqlExchange<LockData<D>> {
    /// @brief Constructor
    ///
    /// The initialization of the variables here is only to satisfy cppcheck -
    /// all variables are initialized/set in the methods before they are used.
    explicit CqlLockExchange(isc::db::CqlConnection& connection) : connection_(connection) {
    }

    void createBindForInsert(LockData<D> const& lock, isc::db::AnyArray& data) {
        try {
            txn_id_ = lock.txn_id_;
            data.clear();
            data.add(&txn_id_);
        } catch (isc::Exception const& ex) {
            isc_throw(isc::db::DbOperationError, "CqlLockExchange::createBindForInsert(): "
                                                 "could not create bind array from Lock: "
                                                     << isc::dhcp::DUID(lock.txn_id_).toText()
                                                     << ", reason: " << ex.what());
        }
    }

    /// @brief Create BIND array to receive data
    ///
    /// Creates a CQL_BIND array to receive LockData data from the database.
    void createBindForSelect(
        isc::db::AnyArray& data,
        isc::db::StatementTag const& /* statement_tag = StatementTag() */) override {
        data.clear();
        data.add(&txn_id_);
    }

    void createBindForUpdate(LockData<D> const& lock, isc::db::AnyArray& data) {
        try {
            txn_id_ = lock.txn_id_;
            data.clear();
            data.add(&txn_id_);
        } catch (isc::Exception const& ex) {
            isc_throw(isc::db::DbOperationError, "CqlLockExchange::createBindForUpdate(): "
                                                 "could not create bind array from Lock: "
                                                     << isc::dhcp::DUID(lock.txn_id_).toText()
                                                     << ", reason: " << ex.what());
        }
    }

    LockDataPtr<D> retrieve() override {
        return std::make_shared<LockData<D>>(txn_id_);
    }

    static isc::db::StatementMap& tagged_statements() {
        static isc::db::StatementMap _{
            {DELETE_LOCK(),
             {DELETE_LOCK(), "DELETE FROM lock" + isc::dhcp::dhcpSpaceToString<D>() +
                                 " WHERE txn_id = ?"
                                 " IF EXISTS"}},

            {INSERT_LOCK(),
             {INSERT_LOCK(), "INSERT INTO lock" + isc::dhcp::dhcpSpaceToString<D>() +
                                 " (txn_id)"
                                 " VALUES ( ? )"
                                 " IF NOT EXISTS"}}};

        return _;
    }

    static isc::db::StatementTag& DELETE_LOCK() {
        static isc::db::StatementTag _(FUNCTION_NAME() + isc::dhcp::dhcpSpaceToString<D>());
        return _;
    }

    static isc::db::StatementTag& INSERT_LOCK() {
        static isc::db::StatementTag _(FUNCTION_NAME() + isc::dhcp::dhcpSpaceToString<D>());
        return _;
    }

protected:
    /// @brief Database connection
    isc::db::CqlConnection& connection_;

    /// @brief lock identifier
    isc::db::CassBlob txn_id_;
};
using CqlLock4Exchange = CqlLockExchange<isc::dhcp::DHCP_SPACE_V4>;
using CqlLock6Exchange = CqlLockExchange<isc::dhcp::DHCP_SPACE_V6>;

template <isc::dhcp::DhcpSpaceType D>
struct CqlLockMgr {
    CqlLockMgr(isc::db::CqlConnection& connection) : connection_(connection) {
        connection_.prepareStatements(CqlLockExchange<D>::tagged_statements());
    }

    void deleteExpired() {
        // Delete all expired locks.
        isc::db::CqlLockExchange<D> exchange(connection_);
        exchange.executeMutation(connection_, {},
                                 isc::db::CqlLockExchange<D>::DELETE_ALL_EXPIRED_LOCK);
    }

    /// @brief Lock
    ///
    /// @return false if lock was taken, true otherwise
    bool lock(std::vector<uint8_t>& txn_id) {
        isc::db::AnyArray where_values{&txn_id};
        isc::db::CqlLockExchange<D> exchange(connection_);
        try {
            exchange.executeMutation(connection_, where_values,
                                     isc::db::CqlLockExchange<D>::INSERT_LOCK());
        } catch (StatementNotApplied const&) {
            // Lock is theirs.
            return false;
        }

        // Lock is ours.
        return true;
    }

    /// @brief Unlock
    ///
    /// @return true if lock was taken, false otherwise
    bool unlock(std::vector<uint8_t>& txn_id) {
        isc::db::AnyArray where_values{&txn_id};
        isc::db::CqlLockExchange<D> exchange(connection_);
        try {
            exchange.executeMutation(connection_, where_values,
                                     isc::db::CqlLockExchange<D>::DELETE_LOCK());
        } catch (Exception const&) {
            // It failed, so lock was not taken.
            return false;
        }

        // It succeeded, so lock was taken.
        return true;
    }

    isc::db::CqlConnection& connection_;

private:
    /// @brief non-copyable
    /// @{
    CqlLockMgr(CqlLockMgr const&) = delete;
    CqlLockMgr& operator=(CqlLockMgr const&) = delete;
    /// @}
};

}  // namespace db
}  // namespace isc

#endif  // CQL_LOCK_H

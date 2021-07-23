// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
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

#ifndef CQL_EXCHANGE_H
#define CQL_EXCHANGE_H

#include <cql/cql_connection.h>

#include <database/common.h>
#include <database/db_exceptions.h>
#include <log/macros.h>
#include <stats/stats_mgr.h>
#include <util/call_once.h>
#include <util/func.h>

#include <any>
#include <chrono>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <typeindex>
#include <vector>

namespace isc {
namespace db {

static size_t constexpr MAJOR = 0;
static size_t constexpr MINOR = 1;

/// @brief Define CQL schema version: 7.0
VersionTuple constexpr CQL_SCHEMA_VERSION = std::make_tuple(7, 0);

struct OperationalVersionTuple : VersionTuple {};

#ifdef ENABLE_DEBUG
extern isc::log::Logger cql_logger;
#endif  // ENABLE_DEBUG

/// @brief function type to check if transaction action has been applied
typedef bool (*has_transaction_action_been_applied_t)(CassFuturePtr&, uint32_t&);

/// @brief Host identifier converted to Cassandra data type
using CassBlob = std::vector<cass_byte_t>;

/// @brief Structure used to bind C++ input values to dynamic CQL parameters
///
/// The structure contains a vector which stores the input values. The vector is
/// passed directly into the CQL execute call. Note that the data values are
/// stored as pointers. These pointers need to be valid for the duration of the
/// CQL statement execution. In other words, populating them with pointers that
/// go out of scope before the statement is executed results in undefined
/// behaviour.
struct AnyArray : std::vector<std::any> {
    /// @brief Constructors
    /// @{
    AnyArray() {
        // 32: the atomic number of germanium
        std::vector<std::any>::reserve(32);
    }
    AnyArray(std::initializer_list<value_type> il, allocator_type const& alloc = allocator_type())
        : std::vector<std::any>(il, alloc) {
    }
    /// @}

    /// @brief Add a value at the end of the vector.
    void add(std::any const& value) {
        std::vector<std::any>::push_back(value);
    }

    /// @brief Remove the void pointer to the data value from a specified
    ///     position inside the vector.
    void remove(size_t const& index) {
        if (std::vector<std::any>::size() <= index) {
            isc_throw(BadValue, "AnyArray::remove(): index " << index << " out of bounds: [0, "
                                                             << (std::vector<std::any>::size() - 1)
                                                             << "]");
        }
        std::vector<std::any>::erase(std::vector<std::any>::begin() + index);
    }
};

/// @brief Binds a C++ object to a Cassandra statement's parameter. Used in all
///     statements.
/// @param value the value to be set or retreived
/// @param index offset of the value being processed
/// @param statement pointer to the parent statement being used
typedef CassError (*CqlBindFunction)(std::any const& value,
                                     size_t const& index,
                                     CassStatementPtr& statement);

/// @brief Converts a single Cassandra column value to a C++ object. Used in
///     SELECT statements.
///
/// @param data the result will be stored here (this pointer will be updated)
/// @param value this value will be converted
typedef CassError (*CqlGetFunction)(std::any const& data, CassValue const* value);

/// @brief Wrapper over the bind and get functions that interface with Cassandra
struct CqlFunction {
    /// @brief Binds a C++ object to a Cassandra statement's parameter. Used in
    ///     all statements.
    CqlBindFunction cqlBindFunction_;
    /// @brief Converts a single Cassandra column value to a C++ object. Used in
    ///     SELECT statements.
    CqlGetFunction cqlGetFunction_;
    /// @brief name of type used in errors
    std::string name_;
};

/// @brief Common operations in Cassandra exchanges
struct CqlCommon {
    /// @brief Assigns values to every column of an INSERT or an UPDATE statement.
    ///
    /// Calls cqlBindFunction_() for every column with its respective type.
    ///
    /// @param data array containing column values to be passed to the statement
    ///     being executed
    /// @param statement internal Cassandra object representing the statement
    ///     being executed
    static void bindData(AnyArray const& data, CassStatementPtr& statement) {
        size_t i = 0u;
        for (std::any const& element : data) {
            CassError cass_error;
            try {
                cass_error =
                    CQL_FUNCTIONS().at(element.type()).cqlBindFunction_(element, i, statement);
            } catch (std::out_of_range const& exception) {
                isc_throw(BadValue, PRETTY_FUNCTION_NAME()
                                        << ": std::out_of_range: " << exception.what()
                                        << " when binding parameter " << i << " out of "
                                        << data.size() << " (starting with 0) of type "
                                        << pretty_print_any(element));
            } catch (std::bad_any_cast const& exception) {
                isc_throw(DbOperationError, PRETTY_FUNCTION_NAME()
                                                << ": std::bad_any_cast: " << exception.what()
                                                << " when binding parameter " << i << " out of "
                                                << data.size() << " (starting with 0) of type "
                                                << pretty_print_any(element) << " to type "
                                                << CQL_FUNCTIONS().at(element.type()).name_);
            }
            checkCassError(cass_error, i, data, element, PRETTY_FUNCTION_NAME());
            ++i;
        }
    }

    /// @brief Retrieves data returned by Cassandra.
    ///
    /// Calls cqlGetFunction_() for every column with its respective type.
    ///
    /// @param row internal Cassandra object containing data returned by
    ///     Cassandra
    /// @param data array containing objects to be populated with results
    static void getData(CassRow const* row, AnyArray& data) {
        size_t i(0);
        for (std::any& element : data) {
            CassValue const* column(cass_row_get_column(row, i));
            CassError cass_error;
            try {
                cass_error = CQL_FUNCTIONS().at(element.type()).cqlGetFunction_(element, column);
            } catch (std::out_of_range const& exception) {
                isc_throw(BadValue, PRETTY_FUNCTION_NAME()
                                        << ": " << exception.what() << " when retrieving parameter "
                                        << i << " out of " << data.size()
                                        << " (starting with 0) of type "
                                        << pretty_print_any(element));
            } catch (std::bad_any_cast& exception) {
                isc_throw(DbOperationError, PRETTY_FUNCTION_NAME()
                                                << ": " << exception.what()
                                                << " when retrieving parameter " << i << " out of "
                                                << data.size() << " (starting with 0) of type "
                                                << pretty_print_any(element) << " to type "
                                                << CQL_FUNCTIONS().at(element.type()).name_);
            }
            checkCassError(cass_error, i, data, element, PRETTY_FUNCTION_NAME());
            ++i;
        }
    }

private:
    static std::string pretty_print_any(std::any const& any) {
        return boost::core::demangle(any.type().name());
    }

    /// @name CqlBind functions for binding data into Cassandra format for
    ///     insertion:
    /// @{
    static CassError
    CqlBindNone(std::any const& /* value */, size_t const& index, CassStatementPtr& statement) {
        return cass_statement_bind_null(statement.get(), index);
    }

    static CassError
    CqlBindBool(std::any const& value, size_t const& index, CassStatementPtr& statement) {
        return cass_statement_bind_bool(statement.get(), index,
                                        *std::any_cast<cass_bool_t*>(value));
    }

    static CassError
    CqlBindInt8(std::any const& value, size_t const& index, CassStatementPtr& statement) {
        return cass_statement_bind_int8(statement.get(), index,
                                        *std::any_cast<cass_int8_t*>(value));
    }

    static CassError
    CqlBindInt16(std::any const& value, size_t const& index, CassStatementPtr& statement) {
        return cass_statement_bind_int16(statement.get(), index,
                                         *std::any_cast<cass_int16_t*>(value));
    }

    static CassError
    CqlBindInt32(std::any const& value, size_t const& index, CassStatementPtr& statement) {
        return cass_statement_bind_int32(statement.get(), index,
                                         *std::any_cast<cass_int32_t*>(value));
    }

    static CassError
    CqlBindInt64(std::any const& value, size_t const& index, CassStatementPtr& statement) {
        return cass_statement_bind_int64(statement.get(), index,
                                         *std::any_cast<cass_int64_t*>(value));
    }

    static CassError
    CqlBindUint8(std::any const& value, size_t const& index, CassStatementPtr& statement) {
        return cass_statement_bind_int8(
            statement.get(), index, static_cast<cass_int8_t>(*std::any_cast<cass_uint8_t*>(value)));
    }

    static CassError
    CqlBindUint16(std::any const& value, size_t const& index, CassStatementPtr& statement) {
        return cass_statement_bind_int16(
            statement.get(), index,
            static_cast<cass_int16_t>(*std::any_cast<cass_uint16_t*>(value)));
    }

    static CassError
    CqlBindUint32(std::any const& value, size_t const& index, CassStatementPtr& statement) {
        return cass_statement_bind_int32(
            statement.get(), index,
            static_cast<cass_int32_t>(*std::any_cast<cass_uint32_t*>(value)));
    }

    static CassError
    CqlBindUint64(std::any const& value, size_t const& index, CassStatementPtr& statement) {
        return cass_statement_bind_int64(
            statement.get(), index,
            static_cast<cass_int64_t>(*std::any_cast<cass_uint64_t*>(value)));
    }

    static CassError
    CqlBindString(std::any const& value, size_t const& index, CassStatementPtr& statement) {
        return cass_statement_bind_string(statement.get(), index,
                                          std::any_cast<std::string*>(value)->c_str());
    }

    static CassError
    CqlBindBytes(std::any const& value, size_t const& index, CassStatementPtr& statement) {
        CassBlob* blob_value = std::any_cast<CassBlob*>(value);
        return cass_statement_bind_bytes(statement.get(), index, blob_value->data(),
                                         blob_value->size());
    }

    static CassError
    CqlBindUuid(std::any const& value, size_t const& index, CassStatementPtr& statement) {
        return cass_statement_bind_uuid(statement.get(), index, *std::any_cast<CassUuid*>(value));
    }
    /// @}

    /// @name CqlGet functions for retrieving data of the proper Cassandra format:
    /// @{
    static CassError CqlGetNone(std::any const& /* data */, CassValue const* /* value */) {
        return CASS_OK;
    }

    static CassError CqlGetBool(std::any const& data, CassValue const* value) {
        cass_bool_t* const result(std::any_cast<cass_bool_t*>(data));
        if (cass_value_is_null(value)) {
            *result = cass_false;
            return CASS_OK;
        }
        return cass_value_get_bool(value, result);
    }

    static CassError CqlGetInt8(std::any const& data, CassValue const* value) {
        cass_int8_t* const result(std::any_cast<cass_int8_t*>(data));
        if (cass_value_is_null(value)) {
            *result = 0;
            return CASS_OK;
        }
        return cass_value_get_int8(value, result);
    }

    static CassError CqlGetInt16(std::any const& data, CassValue const* value) {
        cass_int16_t* const result(std::any_cast<cass_int16_t*>(data));
        if (cass_value_is_null(value)) {
            *result = 0;
            return CASS_OK;
        }
        return cass_value_get_int16(value, result);
    }

    static CassError CqlGetInt32(std::any const& data, CassValue const* value) {
        cass_int32_t* const result(std::any_cast<cass_int32_t*>(data));
        if (cass_value_is_null(value)) {
            *result = 0;
            return CASS_OK;
        }
        return cass_value_get_int32(value, result);
    }

    static CassError CqlGetInt64(std::any const& data, CassValue const* value) {
        cass_int64_t* const result(std::any_cast<cass_int64_t*>(data));
        if (cass_value_is_null(value)) {
            *result = 0;
            return CASS_OK;
        }
        return cass_value_get_int64(value, result);
    }

    static CassError CqlGetUint8(std::any const& data, CassValue const* value) {
        cass_uint8_t* const result(std::any_cast<cass_uint8_t*>(data));
        if (cass_value_is_null(value)) {
            *result = 0;
            return CASS_OK;
        }
        return cass_value_get_int8(value, reinterpret_cast<cass_int8_t*>(result));
    }

    static CassError CqlGetUint16(std::any const& data, CassValue const* value) {
        cass_uint16_t* const result(std::any_cast<cass_uint16_t*>(data));
        if (cass_value_is_null(value)) {
            *result = 0;
            return CASS_OK;
        }
        return cass_value_get_int16(value, reinterpret_cast<cass_int16_t*>(result));
    }

    static CassError CqlGetUint32(std::any const& data, CassValue const* value) {
        cass_uint32_t* const result(std::any_cast<cass_uint32_t*>(data));
        if (cass_value_is_null(value)) {
            *result = 0;
            return CASS_OK;
        }
        return cass_value_get_int32(value, reinterpret_cast<cass_int32_t*>(result));
    }

    static CassError CqlGetUint64(std::any const& data, CassValue const* value) {
        cass_uint64_t* const result(std::any_cast<cass_uint64_t*>(data));
        if (cass_value_is_null(value)) {
            *result = 0;
            return CASS_OK;
        }
        return cass_value_get_int64(value, reinterpret_cast<cass_int64_t*>(result));
    }

    static CassError CqlGetString(std::any const& data, CassValue const* value) {
        std::string* const result(std::any_cast<std::string*>(data));
        if (cass_value_is_null(value)) {
            result->clear();
            return CASS_OK;
        }
        char const* data_value;
        size_t size_value;
        CassError cass_error(
            cass_value_get_string(value, static_cast<char const**>(&data_value), &size_value));
        result->assign(data_value, data_value + size_value);
        return cass_error;
    }

    static CassError CqlGetBytes(std::any const& data, CassValue const* value) {
        CassBlob* const result(std::any_cast<CassBlob*>(data));
        if (cass_value_is_null(value)) {
            *result = {};
            return CASS_OK;
        }
        cass_byte_t const* data_value;
        size_t size_value;
        CassError cass_error =
            cass_value_get_bytes(value, static_cast<cass_byte_t const**>(&data_value), &size_value);
        result->assign(data_value, data_value + size_value);
        return cass_error;
    }

    static CassError CqlGetUuid(std::any const& data, CassValue const* value) {
        CassUuid* const result(std::any_cast<CassUuid*>(data));
        if (cass_value_is_null(value)) {
            *result = {};
            return CASS_OK;
        }
        return cass_value_get_uuid(value, result);
    }
    /// @}

    /// @brief Defines a type for storing aux. Cassandra functions
    using CqlFunctionMap = std::unordered_map<std::type_index, CqlFunction>;

    /// @brief Maps C++ type to functions.
    static CqlFunctionMap& CQL_FUNCTIONS() {
        static CqlFunctionMap _{
            {typeid(nullptr), {CqlBindNone, CqlGetNone, "NULL"}},
            {typeid(cass_bool_t*), {CqlBindBool, CqlGetBool, "cass_bool_t*"}},
            {typeid(cass_int8_t*), {CqlBindInt8, CqlGetInt8, "cass_int8_t*"}},
            {typeid(cass_int16_t*), {CqlBindInt16, CqlGetInt16, "cass_int16_t*"}},
            {typeid(cass_int32_t*), {CqlBindInt32, CqlGetInt32, "cass_int32_t*"}},
            {typeid(cass_int64_t*), {CqlBindInt64, CqlGetInt64, "cass_int64_t*"}},
            {typeid(cass_uint8_t*), {CqlBindUint8, CqlGetUint8, "cass_uint8_t*"}},
            {typeid(cass_uint16_t*), {CqlBindUint16, CqlGetUint16, "cass_uint16_t*"}},
            {typeid(cass_uint32_t*), {CqlBindUint32, CqlGetUint32, "cass_uint32_t*"}},
            {typeid(cass_uint64_t*), {CqlBindUint64, CqlGetUint64, "cass_uint64_t*"}},
            {typeid(std::string*), {CqlBindString, CqlGetString, "std::string*"}},
            {typeid(CassBlob*), {CqlBindBytes, CqlGetBytes, "CassBlob*"}},
            {typeid(CassUuid*), {CqlBindUuid, CqlGetUuid, "CassUuid*"}}};
        return _;
    }

    static void checkCassError(CassError const& cass_error,
                               size_t const i,
                               AnyArray const& data,
                               std::any const& element,
                               std::string const& function_name) {
        if (cass_error != CASS_OK) {
            isc_throw(DbOperationError,
                      function_name << ": " << cass_error_desc(cass_error) << " for parameter " << i
                                    << " out of " << data.size() << " (starting with 0) of type "
                                    << pretty_print_any(element) << " to type "
                                    << CQL_FUNCTIONS().at(element.type()).name_);
        }
    }
};  // namespace db

/// @brief Cassandra Exchange
///
/// Used to convert between Cassandra CQL and C++ data types. A different
/// exchange is made for every distinct set of columns. Multiple tables may use
/// the same exchange if they have the same columns.
template <typename T>
struct CqlExchange : DatabaseExchange {
    /// @brief Destructor
    virtual ~CqlExchange() = 0;

    /// @name Time conversion:
    /// @{
    /// @brief Converts time to Cassandra format
    ///
    /// @param cltt timestamp of last client transmission time to be converted
    /// @param valid_lifetime lifetime of a lease
    /// @param expire expiration time (result will be stored here)
    static void convertToDatabaseTime(time_t const& cltt,
                                      const uint32_t& valid_lifetime,
                                      cass_int64_t& expire) {
        // Calculate expire time. Store it in the 64-bit value so as we can
        // detect overflows.
        cass_int64_t expire_time =
            static_cast<cass_int64_t>(cltt) + static_cast<cass_int64_t>(valid_lifetime);

        if (expire_time > DatabaseConnection::MAX_DB_TIME) {
            isc_throw(BadValue, PRETTY_FUNCTION_NAME()
                                    << ": time value " << expire_time << " is too large");
        }

        expire = expire_time;
    }

    /// @brief Converts time from Cassandra format
    /// @param expire expiration time in Cassandra format
    /// @param valid_lifetime lifetime of a lease
    /// @param cltt client last transmission time (result will be stored here)
    static void convertFromDatabaseTime(const cass_int64_t& expire,
                                        const cass_int64_t& valid_lifetime,
                                        time_t& cltt) {
        /// @todo: Although 2037 is still far away, there are people who use infinite
        /// lifetimes. Cassandra doesn't have to support it right now, but at least
        /// we should be able to detect a problem.

        // Convert to local time
        cltt = static_cast<time_t>(expire - valid_lifetime);
    }
    /// @}

    /// @brief Create BIND array to receive C++ data.
    ///
    /// Used in executeSelect() to retrieve from database
    ///
    /// @param data array of bound objects representing data to be retrieved
    /// @param statement_tag prepared statement being executed; defaults to an
    ///     invalid index
    virtual void createBindForSelect(AnyArray& data,
                                     StatementTag const& statement_tag = StatementTag()) = 0;

    static bool constexpr SINGLE = true;

    /// @brief Executes SELECT statements.
    ///
    /// @param connection connection used to communicate with the Cassandra
    ///     database
    /// @param where_values array of bound objects used to filter the results
    /// @param statement_tag prepared statement being executed
    /// @param single true if a single row should be returned; by default,
    /// multiple rows are allowed
    ///
    /// @return collection of std::any objects
    ///
    /// @throw DbOperationError
    /// @throw MultipleRecords
    /// @throw TransactionException
    /// @throw ZeroRecords
    Collection<T> executeSelect(CqlConnection const& connection,
                                AnyArray const& where_values,
                                StatementTag statement_tag,
                                bool const single = false) {
        CqlTaggedStatement const& tagged_statement(connection.statements_.at(statement_tag));
#ifdef ENABLE_DEBUG
        LOG_DEBUG(cql_logger, isc::log::DBGLVL_TRACE_DETAIL_DATA,
                  "[thread %1] executeSelect(%2) started...")
            .arg(std::this_thread::get_id())
            .arg(tagged_statement.text_);
#endif  // ENABLE_DEBUG
        auto const& start(std::chrono::steady_clock::now());

        // Bind the data before the query is executed
        AnyArray local_where_values(where_values);
        CassStatementPtr statement;
        if (tagged_statement.is_raw_) {
            // The entire query is the first element in data.
            std::string* query(std::any_cast<std::string*>(local_where_values.back()));
            local_where_values.pop_back();
            statement = MakePtr(cass_statement_new(query->c_str(), local_where_values.size()));
        } else {
            statement = MakePtr(cass_prepared_bind(tagged_statement.prepared_statement_.get()));
            if (!statement) {
                isc_throw(DbOperationError, PRETTY_METHOD_NAME() << ": unable to bind statement "
                                                                 << tagged_statement.name_);
            }
        }

        // Set specific level of consistency if we're told to do so.
        if (connection.force_consistency_) {
            CassError rc(cass_statement_set_consistency(statement.get(), connection.consistency_));
            if (rc != CASS_OK) {
                isc_throw(DbOperationError,
                          PRETTY_METHOD_NAME() + ": unable to set consistency for statement "
                              << tagged_statement.name_
                              << ", Cassandra error code: " << cass_error_desc(rc));
            }
        }
        if (connection.serial_consistency_ != CASS_CONSISTENCY_UNKNOWN) {
            CassError rc(cass_statement_set_serial_consistency(statement.get(),
                                                               connection.serial_consistency_));
            if (rc != CASS_OK) {
                isc_throw(DbOperationError,
                          PRETTY_METHOD_NAME() + ": unable to set statement "
                                                 "serial consistency for statement "
                              << tagged_statement.name_
                              << ", Cassandra error code: " << cass_error_desc(rc));
            }
        }

        try {
            CqlCommon::bindData(local_where_values, statement);
        } catch (std::exception const& ex) {
            isc_throw(DbOperationError, ex.what());
        }

#ifdef TERASTREAM_FULL_TRANSACTIONS
        cass_statement_add_key_index(statement.get(), local_where_values.size() - 1);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        // Everything's ready. Call the actual statement.
        Collection<T> collection;
        uint32_t max_statement_tries(connection.max_statement_tries_);
        while (max_statement_tries > 0) {
            CassFuturePtr future(
                MakePtr(cass_session_execute(connection.session_.get(), statement.get())));
            if (!future) {
                isc_throw(DbOperationError, PRETTY_METHOD_NAME() << ": no CassFuture for statement "
                                                                 << tagged_statement.name_);
            }

            // Wait for the statement execution to complete.
            cass_future_wait(future.get());
            auto const& error(connection.checkFutureError(PRETTY_METHOD_NAME() +
                                                              ": cass_session_execute() != CASS_OK",
                                                          future, statement_tag));
            CassError rc(cass_future_error_code(future.get()));

            --max_statement_tries;
            if (rc != CASS_OK) {
                if (max_statement_tries > 0) {
                    continue;
                }
#ifdef TERASTREAM_FULL_TRANSACTIONS
                isc_throw(TransactionException, error);
#else  // TERASTREAM_FULL_TRANSACTIONS
                isc_throw(DbOperationError, error);
#endif  // TERASTREAM_FULL_TRANSACTIONS
            }

            // Get column values.
            CassResultPtr const& result_collection(MakePtr(cass_future_get_result(future.get())));
            if (single) {
                size_t const result_count(cass_result_row_count(result_collection.get()));
                if (result_count > 1) {
                    isc_throw(MultipleRecords,
                              PRETTY_METHOD_NAME() +
                                      ": multiple records were found in "
                                      "the database where only one was expected for statement "
                                  << tagged_statement.name_);
                }
            }

            // Get results.
            AnyArray return_values;
            CassIteratorPtr const& rows(
                MakePtr(cass_iterator_from_result(result_collection.get())));
            while (cass_iterator_next(rows.get())) {
                CassRow const* row(cass_iterator_get_row(rows.get()));
                createBindForSelect(return_values, statement_tag);
                CqlCommon::getData(row, return_values);
                collection.push_back(retrieve());
            }
            break;
        }

        auto const& end(std::chrono::steady_clock::now());
        if (getLoggableTags().contains(statement_tag)) {
            isc::stats::StatsMgr::instance().setValue(
                statement_tag + "-processing-time",
                std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
            isc::stats::StatsMgr::instance().addValue(statement_tag + "-count",
                                                      static_cast<int64_t>(1));
        }
#ifdef ENABLE_DEBUG
        LOG_DEBUG(cql_logger, isc::log::DBGLVL_TRACE_DETAIL_DATA,
                  "[thread %1] executeSelect(%2) ended after %3 ms...")
            .arg(std::this_thread::get_id())
            .arg(tagged_statement.text_)
            .arg(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
#endif  // ENABLE_DEBUG

        return collection;
    }

    /// @brief Executes INSERT, UPDATE or DELETE statements.
    ///
    /// @param connection connection used to communicate with the Cassandra
    ///     database
    /// @param assigned_values array of bound objects to be used when inserting
    ///     values
    /// @param statement_tag prepared statement being executed
    /// @param function function that checks if a certain transaction has been
    ///     applied, could be different for commit than it is for rollback
    ///
    /// @throw DbOperationError
    /// @throw StatementNotApplied
    /// @throw TransactionException
    void executeMutation(CqlConnection const& connection,
                         AnyArray const& assigned_values,
                         StatementTag statement_tag,
                         has_transaction_action_been_applied_t function = NULL) {
        CqlTaggedStatement const& tagged_statement(connection.statements_.at(statement_tag));

#ifdef ENABLE_DEBUG
        LOG_DEBUG(cql_logger, isc::log::DBGLVL_TRACE_DETAIL_DATA,
                  "[thread %1] executeMutation(%2) started...")
            .arg(std::this_thread::get_id())
            .arg(tagged_statement.text_);
#endif  // ENABLE_DEBUG
        auto const& start(std::chrono::steady_clock::now());

        // Bind the statement.
        CassStatementPtr statement(
            MakePtr(cass_prepared_bind(tagged_statement.prepared_statement_.get())));
        if (!statement) {
            isc_throw(DbOperationError, PRETTY_METHOD_NAME() << ": unable to bind statement "
                                                             << tagged_statement.name_);
        }

        // Set specific level of consistency, if told to do so.
        if (connection.force_consistency_) {
            CassError rc(cass_statement_set_consistency(statement.get(), connection.consistency_));
            if (rc != CASS_OK) {
                isc_throw(DbOperationError,
                          PRETTY_METHOD_NAME() + ": unable to set"
                                                 " statement consistency for statement "
                              << tagged_statement.name_
                              << ", Cassandra error code: " << cass_error_desc(rc));
            }
        }

        try {
            CqlCommon::bindData(assigned_values, statement);
        } catch (std::exception const& ex) {
            isc_throw(DbOperationError, ex.what());
        }

#ifdef TERASTREAM_FULL_TRANSACTIONS
        cass_statement_add_key_index(statement.get(), assigned_values.size() - 1);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        uint32_t max_statement_tries(connection.max_statement_tries_);
        while (max_statement_tries > 0) {
            CassFuturePtr future(
                MakePtr(cass_session_execute(connection.session_.get(), statement.get())));
            if (!future) {
                isc_throw(DbOperationError, PRETTY_METHOD_NAME() << ": unable to execute statement "
                                                                 << tagged_statement.name_);
            }
            cass_future_wait(future.get());
            std::string const& error(connection.checkFutureError(
                PRETTY_METHOD_NAME() + ": cass_session_execute() != CASS_OK", future,
                statement_tag));
            CassError rc(cass_future_error_code(future.get()));

            --max_statement_tries;
            if (rc != CASS_OK) {
                if (max_statement_tries > 0) {
                    continue;
                }
#ifdef TERASTREAM_FULL_TRANSACTIONS
                isc_throw(TransactionException, error);
#else  // TERASTREAM_FULL_TRANSACTIONS
                isc_throw(DbOperationError, error);
#endif  // TERASTREAM_FULL_TRANSACTIONS
            }

            // Check if statement has been applied.
            bool applied(statementApplied(future));

            if (function) {
                uint32_t result;
                bool txn_applied(function(future, result));
                applied = applied && txn_applied;
            }

            if (!applied) {
#ifdef TERASTREAM_FULL_TRANSACTIONS
                isc_throw(TransactionException,
                          PRETTY_METHOD_NAME() + ": [applied] is false for statement "
                              << tagged_statement.name_);
#else  // TERASTREAM_FULL_TRANSACTIONS
                isc_throw(StatementNotApplied,
                          PRETTY_METHOD_NAME() + ": [applied] is false for statement "
                              << tagged_statement.name_);
#endif  // TERASTREAM_FULL_TRANSACTIONS
            }
            break;
        }

        auto const& end(std::chrono::steady_clock::now());
#ifdef ENABLE_DEBUG
        LOG_DEBUG(cql_logger, isc::log::DBGLVL_TRACE_DETAIL_DATA,
                  "[thread %1] executeMutation(%2) ended after %3 ms...")
            .arg(std::this_thread::get_id())
            .arg(tagged_statement.text_)
            .arg(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
#endif  // ENABLE_DEBUG

        if (getLoggableTags().contains(statement_tag)) {
            isc::stats::StatsMgr::instance().setValue(
                statement_tag + "-processing-time",
                std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
            isc::stats::StatsMgr::instance().addValue(statement_tag + "-count",
                                                      static_cast<int64_t>(1));
        }
    }

    /// @brief Check if CQL statement has been applied.
    ///
    /// @param future structure used to wait on statement executions
    /// @param row_count number of rows returned
    /// @param column_count number of columns queried
    ///
    /// On insert, a false [applied] means there is a duplicate entry with the
    ///     same priumary key.
    ///
    /// @return true if statement has been successfully applied, false otherwise
    bool statementApplied(CassFuturePtr& future,
                          size_t* row_count = nullptr,
                          size_t* column_count = nullptr) {
        CassResultPtr const& result_collection(MakePtr(cass_future_get_result(future.get())));
        if (!result_collection) {
            isc_throw(DbOperationError,
                      PRETTY_METHOD_NAME() + ": unable to get results collection");
        }
        if (row_count) {
            *row_count = cass_result_row_count(result_collection.get());
        }
        if (column_count) {
            *column_count = cass_result_column_count(result_collection.get());
        }
        CassIteratorPtr const& rows(MakePtr(cass_iterator_from_result(result_collection.get())));
        cass_bool_t applied(cass_true);
        if (cass_iterator_next(rows.get())) {
            AnyArray data;
            CassRow const* row(cass_iterator_get_row(rows.get()));
            // [applied]: bool
            data.add(&applied);
            CqlCommon::getData(row, data);
            if (cass_iterator_next(rows.get())) {
                isc_throw(DbOperationError, PRETTY_METHOD_NAME() << ": double [applied]?");
            }
        }
        return applied == cass_true;
    }

    /// @brief Copy received data into the derived class' object.
    ///
    /// Copies information about the entity to be retrieved into a holistic
    /// object. Called in @ref executeSelect(). Not implemented for base class
    /// CqlExchange. To be implemented in derived classes.
    ///
    /// @return a pointer to the object retrieved.
    virtual Ptr<T> retrieve() = 0;

    static std::set<StatementTag>& getLoggableTags() {
        static std::set<StatementTag> _{
            "INSERT_LOCK4",
            "INSERT_LOCK6",
            "DELETE_LOCK4",
            "DELETE_LOCK6",
            "INSERT_LEASE4",
            "UPDATE_LEASE4",
            "DELETE_LEASE4",
            "GET_LEASE4_EXPIRE",
            "GET_LEASE4",
            "GET_LEASE4_ADDR",
            "GET_LEASE4_CLIENTID",
            "GET_LEASE4_CLIENTID_SUBID",
            "GET_LEASE4_HWADDR",
            "GET_LEASE4_HWADDR_SUBID",
            "GET_LEASE4_LIMIT",
            "GET_LEASE4_PAGE",
            "GET_LEASE4_SUBID",
            "GET_LEASE4_HOSTNAME",
            "INSERT_LEASE6",
            "UPDATE_LEASE6",
            "DELETE_LEASE6",
            "GET_LEASE6_EXPIRE",
            "GET_LEASE6_ADDR",
            "GET_LEASE6_DUID",
            "GET_LEASE6_DUID_IAID",
            "GET_LEASE6_DUID_IAID_SUBID",
            "GET_LEASE6_LIMIT",
            "GET_LEASE6_PAGE",
            "GET_LEASE6_SUBID",
            "GET_LEASE6_HOSTNAME",
            "ALL_LEASE4_STATS",
            "SUBNET_LEASE4_STATS",
            "SUBNET_RANGE_LEASE4_STATS",
            "ALL_LEASE6_STATS",
            "SUBNET_LEASE6_STATS",
            "SUBNET_RANGE_LEASE6_STATS",
        };
        return _;
    }
};

template <typename T>
CqlExchange<T>::~CqlExchange() = default;

//// @brief Exchange used to retrieve schema version from the keyspace.
template <typename version_tuple_t>
struct CqlVersionExchange : CqlExchange<VersionTuple> {
    void validateSchema(CqlConnection& connection) {
        // Check if it was run before in this process's lifetime.
        if (!isc::util::CallOnceStore::get(PRETTY_NAME(this))) {
            return;
        }

        // Prepare the version exchange first.
        connection.prepareStatements(tagged_statements());

        // Validate the schema version.
        VersionTuple const code_version(codeVersion());
        VersionTuplePtr const& db_version_ptr(retrieveVersion(connection));
        if (!db_version_ptr) {
            isc_throw(DbOpenError, "Cassandra schema version mismatch: need version: "
                                       << std::get<MAJOR>(code_version) << "."
                                       << std::get<MINOR>(code_version)
                                       << ", found empty version table in " << table());
        }
        VersionTuple const& db_version(*db_version_ptr);
        if (code_version != db_version) {
            isc_throw(DbOpenError, "Cassandra schema version mismatch: need version: "
                                       << std::get<MAJOR>(code_version) << "."
                                       << std::get<MINOR>(code_version)
                                       << ", found version: " << std::get<MAJOR>(db_version) << "."
                                       << std::get<MINOR>(db_version) << " in " << table());
        }
    }

    /// @brief Standalone method used to retrieve schema version
    ///
    /// @param connection array of bound objects representing data to be
    /// retrieved
    ///
    /// @return version of schema specified in the prepared statement in the
    /// @ref CqlConnection parameter
    VersionTuplePtr retrieveVersion(CqlConnection const& connection) {
        // Run statement.
        Collection<VersionTuple> const& versions(
            executeSelect(connection, {}, GET_VERSION(), SINGLE));
        if (versions.empty()) {
            return nullptr;
        }
        return versions.front();
    }

private:
    static isc::db::VersionTuple const& codeVersion();

    /// @brief Create BIND array to receive C++ data.
    ///
    /// Used in executeSelect() to retrieve from database
    ///
    /// @param data array of bound objects representing data to be retrieved
    /// @param statement_tag prepared statement being executed; defaults to an
    ///     invalid index
    void
    createBindForSelect(AnyArray& data,
                        StatementTag const& /* statement_tag = StatementTag() */) override final {
        data.clear();  // Start with a fresh array.
        data.add(&std::get<0>(data_));  // First column is a major version.
        data.add(&std::get<1>(data_));  // Second column is a minor version.
    }

    /// @brief Copy received data into the <version,minor> pair.
    ///
    /// Copies information about the version to be retrieved into a pair. Called
    /// in executeSelect().
    ///
    /// @return a pointer to the object retrieved.
    VersionTuplePtr retrieve() override final {
        return MakePtr(data_);
    }

    /// @brief The keyspace which this exchange operates on.
    ///
    /// Used in all statements this exchange operates on.
    static std::string const& table();

    /// @brief The initialization function which this exchange.
    ///
    /// Used to initialize the static members of this exchange.
    static StatementMap& tagged_statements() {
        static StatementMap _{
            {GET_VERSION(),
             isc::db::CqlTaggedStatement(GET_VERSION(), "SELECT version, minor FROM " + table())}};

        return _;
    }

    /// @brief Statement tags
    /// @{
    static StatementTag& GET_VERSION() {
        static StatementTag _(FUNCTION_NAME());
        return _;
    }
    /// @}

private:
    version_tuple_t data_;
};

}  // namespace db
}  // namespace isc

#endif  // CQL_EXCHANGE_H

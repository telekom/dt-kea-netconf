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

#ifndef MYSQL_CONFIGURATION_MASTER_MGR_H
#define MYSQL_CONFIGURATION_MASTER_MGR_H

#include <dhcpsrv/dhcpsrv_log.h>
#include <dhcpsrv/master_config_mgr.h>
#include <mysql/mysql_connection.h>
#include <util/types.h>

#include <errmsg.h>
#include <mysql.h>
#include <mysql/mysql_connection.h>
#include <mysqld_error.h>

#include <array>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace isc {
namespace dhcp {

static size_t constexpr INSTANCE_ID_MAX_LEN = 128;
static size_t constexpr SERVER_CONFIG_MAX_LEN = 65535;
static size_t constexpr CONFIG_DATABASE_MAX_LEN = 65535;
static size_t constexpr CONFIG_DATABASE_NAME_MAX_LEN = 128;

/// @brief Forward declaration to the exchange.
/// @brief Exchange MySQL and MasterConfig data
///
/// On any MySQL operation, arrays of MYSQL_BIND structures must be built to
/// describe the parameters in the prepared statements.  Where information is
/// inserted or retrieved - INSERT, UPDATE, SELECT - a large amount of that
/// structure is identical.  This class handles the creation of that array.
///
/// Owing to the MySQL API, the process requires some intermediate variables
/// to hold things like data length etc.  This object holds those variables.
///
/// @note There are no unit tests for this class.  It is tested indirectly
/// in all MySqlMasterConfigMgr::xxx() calls where it is used.
struct MySqlMasterConfigExchange {
    using my_bool = db::my_bool;

    /// @brief Set number of database columns to send data for
    /// the server configuration structure
    static const size_t SERVER_CONFIG_SEND_COLUMNS = 5;

    /// @brief Set number of database columns to read data for
    /// the server configuration structure
    static const size_t SERVER_CONFIG_RECEIVE_COLUMNS = 5;

    MySqlMasterConfigExchange() : config_master_(std::make_shared<MasterConfig>()) {
        size_t i = 0;
        for (std::string const& column : {"instance_id", "timestamp", "server_config",
                                          "config_database", "config_database_name"}) {
            send_columns_[i] = column;
            receive_columns_[i] = column;
            assert(i < SERVER_CONFIG_SEND_COLUMNS);
            assert(i < SERVER_CONFIG_RECEIVE_COLUMNS);
            ++i;
        }

        std::fill(&error_receive[0], &error_receive[SERVER_CONFIG_RECEIVE_COLUMNS],
                  isc::db::MLM_FALSE);
        std::fill(&error_send[0], &error_send[SERVER_CONFIG_SEND_COLUMNS], isc::db::MLM_FALSE);
    }

    virtual ~MySqlMasterConfigExchange() = default;

    /// @brief Create MYSQL_BIND objects for MasterConfig Pointer
    ///
    /// Fills in the MYSQL_BIND array for sending data in the
    /// MasterConfig object to the database.
    ///
    /// @param instance_id the instance identifier of the server being added to the database
    /// @param server_config configuration of the server being added to the database
    /// @param config_database configuration to be used on the shard
    /// @param config_database_name name of the shard whose configuration is being added
    /// @param timestamp read from file, operation is valid only if it is equal to database-side
    ///     timestamp
    ///
    /// @return Vector of MySQL BIND objects representing the data to be added.
    std::vector<MYSQL_BIND> createBindForSend(std::string const& instance_id,
                                              std::string const& server_config,
                                              std::string const& config_database,
                                              std::string const& config_database_name,
                                              int64_t const timestamp) {
        // Store configuration object to ensure it remains valid.
        config_master_->server_specific_configuration(isc::data::Element::fromJSON(server_config));
        config_master_->instance_ID_ = instance_id;
        config_master_->shard_ = config_database_name;
        config_master_->database_credentials(isc::data::Element::fromJSON(config_database));
        config_master_->timestamp_ = static_cast<int64_t>(timestamp);

        // Initialize prior to constructing the array of MYSQL_BIND structures.
        // It sets all fields, including is_null, to zero, so we need to set
        // is_null only if it should be true. This gives up minor performance
        // benefit while being safe approach. For improved readability, the
        // code that explicitly sets is_null is there, but is commented out.
        memset(bind_send, 0, sizeof(bind_send));

        // Set up the structures for the various components of the
        // server configuration structure.
        try {
            // instance_id: VARCHAR(128)
            bind_send[0].buffer_type = MYSQL_TYPE_STRING;
            bind_send[0].buffer = const_cast<char*>(config_master_->instance_ID_.c_str());
            bind_send[0].buffer_length = config_master_->instance_ID_.length();
            // bind_send[0].is_null = &MLM_FALSE; // commented out for performance
            // reasons, see memset() above

            // timestamp: TIMESTAMP
            // TODO
            // isc::db::MySqlConnection::convertToDatabaseTime(config_master_->timestamp_,
            //                                                 timestamp_buffer_);
            // bind_send[1].buffer_type = MYSQL_TYPE_TIMESTAMP;
            // bind_send[1].buffer = reinterpret_cast<char*>(&timestamp_buffer_);
            // bind_send[1].buffer_length = sizeof(timestamp_buffer_);
            // bind_send[1].is_null = &MLM_FALSE; // commented out for performance
            // reasons, see memset() above

            // server_config: TEXT
            bind_send[2].buffer_type = MYSQL_TYPE_STRING;
            bind_send[2].buffer =
                const_cast<char*>(config_master_->server_specific_configuration_string().c_str());
            bind_send[2].buffer_length =
                config_master_->server_specific_configuration_string().length();
            // bind_send[2].is_null = &MLM_FALSE; // commented out for performance
            // reasons, see memset() above

            // config_database: TEXT
            bind_send[3].buffer_type = MYSQL_TYPE_STRING;
            bind_send[3].buffer =
                const_cast<char*>(config_master_->database_credentials_string().c_str());
            bind_send[3].buffer_length = config_master_->database_credentials_string().length();
            // bind_send[3].is_null = &MLM_FALSE; // commented out for performance
            // reasons, see memset() above

            // config_database_name: VARCHAR(128)
            bind_send[4].buffer_type = MYSQL_TYPE_STRING;
            bind_send[4].buffer = const_cast<char*>(config_master_->shard_.c_str());
            bind_send[4].buffer_length = config_master_->shard_.length();
            // bind_send[4].is_null = &MLM_FALSE; // commented out for performance
            // reasons, see memset() above

            // Add the error flags
            setErrorIndicators(bind_send, error_send, SERVER_CONFIG_SEND_COLUMNS);
        } catch (std::exception const& ex) {
            isc_throw(isc::db::DbOperationError,
                      "Could not create bind array from master configuration."
                          << "Reason: " << ex.what());
        }

        // Add the data to the vector.  Note the end element is one after the
        // end of the array.
        return std::vector<MYSQL_BIND>(&bind_send[0], &bind_send[SERVER_CONFIG_SEND_COLUMNS]);
    }

    /// @brief Create BIND array to receive data
    ///
    /// Creates a MYSQL_BIND array to receive MasterConfig data
    /// from the database.
    ///
    std::vector<MYSQL_BIND> createBindForReceive() {
        // Initialize MYSQL_BIND array.
        // It sets all fields, including is_null, to zero, so we need to set
        // is_null only if it should be true. This gives up minor performance
        // benefit while being safe approach. For improved readability, the
        // code that explicitly sets is_null is there, but is commented out.
        memset(bind_receive, 0, sizeof(bind_receive));

        // instance_id: VARCHAR(128)
        instance_ID_length_ = sizeof(instance_ID_buffer_);
        bind_receive[0].buffer_type = MYSQL_TYPE_STRING;
        bind_receive[0].buffer = reinterpret_cast<char*>(instance_ID_buffer_);
        bind_receive[0].buffer_length = instance_ID_length_;
        bind_receive[0].length = &instance_ID_length_;
        // bind_receive[0].is_null = &MLM_FALSE; // commented out for performance
        // reasons, see memset() above

        // expire: timestamp
        bind_receive[1].buffer_type = MYSQL_TYPE_TIMESTAMP;
        bind_receive[1].buffer = reinterpret_cast<char*>(&timestamp_);
        bind_receive[1].buffer_length = sizeof(timestamp_);
        // bind_receive[1].is_null = &MLM_FALSE; // commented out for performance
        // reasons, see memset() above

        // server_config: TEXT
        server_config_length_ = sizeof(server_config_buffer_);
        bind_receive[2].buffer_type = MYSQL_TYPE_STRING;
        bind_receive[2].buffer = reinterpret_cast<char*>(server_config_buffer_);
        bind_receive[2].buffer_length = server_config_length_;
        bind_receive[2].length = &server_config_length_;
        // bind_receive[2].is_null = &MLM_FALSE; // commented out for performance
        // reasons, see memset() above

        // config_database: TEXT
        config_database_length_ = sizeof(config_database_buffer_);
        bind_receive[3].buffer_type = MYSQL_TYPE_STRING;
        bind_receive[3].buffer = reinterpret_cast<char*>(config_database_buffer_);
        bind_receive[3].buffer_length = config_database_length_;
        bind_receive[3].length = &config_database_length_;
        // bind_receive[3].is_null = &MLM_FALSE; // commented out for performance
        // reasons, see memset() above

        // config_database_name: VARCHAR(128)
        config_database_name_length_ = sizeof(config_database_name_buffer_);
        bind_receive[4].buffer_type = MYSQL_TYPE_STRING;
        bind_receive[4].buffer = reinterpret_cast<char*>(config_database_name_buffer_);
        bind_receive[4].buffer_length = config_database_name_length_;
        bind_receive[4].length = &config_database_name_length_;
        // bind_receive[4].is_null = &MLM_FALSE; // commented out for performance
        // reasons, see memset() above

        // Add the error flags
        setErrorIndicators(bind_receive, error_receive, SERVER_CONFIG_RECEIVE_COLUMNS);

        // .. and check that we have the numbers correct at compile time.
        BOOST_STATIC_ASSERT(4 < SERVER_CONFIG_RECEIVE_COLUMNS);

        // Add the data to the vector.  Note the end element is one after the
        // end of the array.
        return std::vector<MYSQL_BIND>(&bind_receive[0],
                                       &bind_receive[SERVER_CONFIG_RECEIVE_COLUMNS]);
    }

    /// @brief Copy Received Data into MasterConfig Object
    ///
    /// Called after the MYSQL_BIND array created by createBindForReceive()
    /// has been used, this copies data from the internal member variables
    /// into a MasterConfig object.
    ///
    /// @return MasterConfigPtr Pointer to a MasterConfig
    ///         object holding the relevant data.
    MasterConfigPtr selectData() {
        MasterConfigPtr config_ptr = std::make_shared<MasterConfig>();
        config_ptr->instance_ID_ = std::string(instance_ID_buffer_, instance_ID_length_);
        config_ptr->timestamp_ = timestamp_;
        config_ptr->server_specific_configuration(isc::data::Element::fromJSON(
            std::string(server_config_buffer_, server_config_length_)));
        config_ptr->database_credentials_string() =
            std::string(config_database_buffer_, config_database_length_);
        config_ptr->shard_ =
            std::string(config_database_name_buffer_, config_database_name_length_);

        return config_ptr;
    }

    /// @brief Return columns in error
    ///
    /// If an error is returned from a fetch (in particular, a truncated
    /// status), this method can be called to get the names of the fields in
    /// error.  It returns a string comprising the names of the fields
    /// separated by commas.  In the case of there being no error indicators
    /// set, it returns the string "(None)".
    ///
    /// @return Comma-separated list of columns in error, or the string
    ///         "(None)".
    std::string getErrorSendColumns() {
        return getColumnsInError(error_send, send_columns_, SERVER_CONFIG_SEND_COLUMNS);
    }

    std::string getErrorReceiveColumns() {
        return getColumnsInError(error_receive, receive_columns_, SERVER_CONFIG_RECEIVE_COLUMNS);
    }

    /// @brief Return columns in error
    ///
    /// If an error is returned from a fetch (in particular, a truncated
    /// status), this method can be called to get the names of the fields in
    /// error.  It returns a string comprising the names of the fields
    /// separated by commas.  In the case of there being no error indicators
    /// set, it returns the string "(None)".
    ///
    /// @param error Array of error elements.  An element is set to MLM_TRUE
    ///        if the corresponding column in the database is the source of
    ///        the error.
    /// @param names Array of column names, the same size as the error array.
    /// @param count Size of each of the arrays.
    static std::string getColumnsInError(my_bool* error, std::string* names, size_t count) {
        std::string result;

        // Accumulate list of column names
        for (size_t i = 0; i < count; ++i) {
            if (error[i] == isc::db::MLM_TRUE) {
                if (!result.empty()) {
                    result += ", ";
                }
                result += names[i];
            }
        }

        if (result.empty()) {
            result = "(None)";
        }

        return result;
    }

private:
    /// @brief Set error indicators
    ///
    /// Sets the error indicator for each of the MYSQL_BIND elements.  It points
    /// the "error" field within an element of the input array to the
    /// corresponding element of the passed error array.
    ///
    /// @param bind Array of BIND elements
    /// @param error Array of error elements.  If there is an error in getting
    ///        data associated with one of the "bind" elements, the
    ///        corresponding element in the error array is set to MLM_TRUE.
    /// @param count Size of each of the arrays.
    static void setErrorIndicators(MYSQL_BIND* bind, my_bool* error, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            error[i] = isc::db::MLM_FALSE;
            bind[i].error = reinterpret_cast<my_bool*>(&error[i]);
        }
    }

    // Bind arays
    MYSQL_BIND bind_send[SERVER_CONFIG_SEND_COLUMNS];
    MYSQL_BIND bind_receive[SERVER_CONFIG_RECEIVE_COLUMNS];

    // Send column names
    std::string send_columns_[SERVER_CONFIG_SEND_COLUMNS];
    // Receive column names
    std::string receive_columns_[SERVER_CONFIG_RECEIVE_COLUMNS];

    // Error indicators
    my_bool error_send[SERVER_CONFIG_SEND_COLUMNS];
    my_bool error_receive[SERVER_CONFIG_RECEIVE_COLUMNS];

    // Pointer to the master configuration object
    MasterConfigPtr config_master_;

    // Instance identifier buffer and length
    char instance_ID_buffer_[INSTANCE_ID_MAX_LEN];
    unsigned long instance_ID_length_;  ///< Instance identifier length

    // Timestamp buffer and length
    // TODO: MYSQL_TIME timestamp_buffer_;
    int64_t timestamp_;

    // Server configuration buffer and length
    char server_config_buffer_[SERVER_CONFIG_MAX_LEN];
    unsigned long server_config_length_;

    // Configuration buffer and length
    char config_database_buffer_[CONFIG_DATABASE_MAX_LEN];
    unsigned long config_database_length_;

    // Database name buffer and length
    char config_database_name_buffer_[CONFIG_DATABASE_NAME_MAX_LEN];
    unsigned long config_database_name_length_;
};

/// @brief Exchange used to interact with the master server configuration table
/// from the database
template <DhcpSpaceType D>
struct MySqlMasterConfigMgr : public MasterConfigMgr {
    using my_bool = db::my_bool;

    /// @brief Statement Tags
    ///
    /// The contents of the enum are indexes into the list of SQL statements
    enum StatementIndex {
        // Delete DHCPv4 server configuration.
        DELETE_CONFIGURATION,
        // Retrieve DHCP server configuration filtered by instance ID.
        GET_CONFIGURATION_BY_INSTANCE_ID,
        // Retrieve DHCP server configuration filtered by shard name.
        GET_CONFIGURATION_BY_SHARD,
        // Retrieve DHCPv4 shard names.
        GET_CONFIGURATION_SHARD_NAMES,
        // Retrieve DHCP synchronization timestamp.
        GET_CONFIGURATION_TIMESTAMP,
        // Insert DHCP server configuration.
        INSERT_CONFIGURATION,
        // Retrieve schema version.
        GET_VERSION,
        // Number of statements
        STATEMENT_COUNT
    };

    explicit MySqlMasterConfigMgr(const db::DatabaseConnection::ParameterMap& parameters)
        : connection_(parameters) {

        // Open the database.
        connection_.openDatabase();

        // Enable autocommit.  To avoid a flush to disk on every commit, the global
        // parameter innodb_flush_log_at_trx_commit should be set to 2.  This will
        // cause the changes to be written to the log, but flushed to disk in the
        // background every second.  Setting the parameter to that value will speed
        // up the system, but at the risk of losing data if the system crashes.
        my_bool result = mysql_autocommit(connection_.mysql_, 1);
        if (result != 0) {
            isc_throw(isc::db::DbOperationError, mysql_error(connection_.mysql_));
        }

        // Prepare all statements likely to be used.
        connection_.prepareStatements(tagged_statements().begin(), tagged_statements().end());
    }

    std::list<std::string> selectInstanceIDs(
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        isc_throw(NotImplemented, PRETTY_METHOD_NAME());
    }

    void insert(
        MasterConfigPtr& master_config,
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        StatementIndex const statement_index(INSERT_CONFIGURATION);
        MySqlMasterConfigExchange exchange;
        std::vector<MYSQL_BIND> bind = exchange.createBindForSend(
            master_config->instance_ID_, master_config->server_specific_configuration_string(),
            master_config->database_credentials_string(), master_config->shard_, 0);
        try {
            int status = mysql_stmt_bind_param(connection_.statements_[statement_index], &bind[0]);
            connection_.checkError(status, statement_index, "unable to bind parameters");

            // Execute the statement
            status = mysql_stmt_execute(connection_.statements_[statement_index]);
            if (status != 0) {
                if (mysql_errno(connection_.mysql_) == ER_DUP_ENTRY) {
                    isc_throw(isc::db::DuplicateEntry, "ER_DUP_ENTRY");
                }
                connection_.checkError(status, statement_index, "unable to execute");
            }
        } catch (std::exception const& ex) {
            isc_throw(isc::db::DbOperationError,
                      "Could not create bind array to insert configuration "
                          << ", reason: " << ex.what());
        }
    }

    void update(MasterConfigPtr& /*master_config*/,
                std::string const& /*config_timestamp_path*/) override final {
        isc_throw(NotImplemented, PRETTY_METHOD_NAME());
    }

    void deleteAll(std::string const& /*config_timestamp_path*/) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_MYSQL_CLEAR_MASTER_CONFIG);

        // Execute the prepared statement
        int status = mysql_stmt_execute(connection_.statements_[DELETE_CONFIGURATION]);
        connection_.checkError(status, DELETE_CONFIGURATION, "unable to execute");
    }

    void del(std::string& /*instance_id*/,
             std::string const& /*config_timestamp_path*/) override final {
        isc_throw(NotImplemented, PRETTY_METHOD_NAME());
    }

    MasterConfigPtr select(
        std::string& instance_id,
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        // Set up the WHERE clause value
        MYSQL_BIND inbind[1];
        memset(inbind, 0, sizeof(inbind));

        inbind[0].buffer_type = MYSQL_TYPE_STRING;
        inbind[0].buffer = const_cast<char*>(instance_id.c_str());
        inbind[0].buffer_length = instance_id.length();
        inbind[0].is_null = 0;

        MasterConfigPtr result;

        StatementIndex const statement_index(GET_CONFIGURATION_BY_INSTANCE_ID);

        // Bind the selection parameters to the statement
        int status = mysql_stmt_bind_param(connection_.statements_[statement_index], inbind);
        connection_.checkError(status, statement_index, "unable to bind WHERE clause parameter");

        // Set up the MYSQL_BIND array for the data being returned and bind it to
        // the statement.
        MySqlMasterConfigExchange exchange;
        std::vector<MYSQL_BIND> outbind = exchange.createBindForReceive();
        status = mysql_stmt_bind_result(connection_.statements_[statement_index], &outbind[0]);
        connection_.checkError(status, statement_index, "unable to bind SELECT clause parameters");

        // Execute the statement
        status = mysql_stmt_execute(connection_.statements_[statement_index]);
        connection_.checkError(status, statement_index, "unable to execute");

        // Ensure that all the information is retrieved in one go to avoid
        // overhead of going back and forth between client and server.
        status = mysql_stmt_store_result(connection_.statements_[statement_index]);
        connection_.checkError(status, statement_index, "unable to set up for storing all results");

        // Set up the fetch "release" object to release resources associated
        // with the call to mysql_stmt_fetch when this method exits, then
        // retrieve the data.
        isc::db::MySqlFreeResult fetch_release(connection_.statements_[statement_index]);
        int count = 0;
        while ((status = mysql_stmt_fetch(connection_.statements_[statement_index])) == 0) {
            try {
                result = exchange.selectData();
            } catch (const isc::BadValue& ex) {
                // Rethrow the exception with a bit more data.
                isc_throw(isc::BadValue, ex.what() << ". Statement is <"
                                                   << connection_.text_statements_[statement_index]
                                                   << ">");
            }

            if (++count > 1) {
                isc_throw(isc::db::MultipleRecords,
                          "multiple records were found in the "
                          "database where only one was expected for query "
                              << connection_.text_statements_[statement_index]);
            }
        }

        // How did the fetch end?
        if (status == 1) {
            // Error - unable to fetch results
            connection_.checkError(status, statement_index, "unable to fetch results");
        } else if (status == MYSQL_DATA_TRUNCATED) {
            // Data truncated - throw an exception indicating what was at fault
            isc_throw(isc::db::DataTruncated,
                      connection_.text_statements_[statement_index]
                          << " returned truncated data: columns affected are "
                          << exchange.getErrorReceiveColumns());
        }

        return result;
    }

    MasterConfigCollection selectByShard(
        std::string const& config_database_name,
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL,
                  DHCPSRV_MYSQL_GET_MASTER_CONFIG_SHARD_DB)
            .arg(config_database_name);

        StatementIndex const statement_index(GET_CONFIGURATION_BY_SHARD);

        // Set up the WHERE clause value
        MYSQL_BIND inbind[1];
        memset(inbind, 0, sizeof(inbind));

        inbind[0].buffer_type = MYSQL_TYPE_STRING;
        inbind[0].buffer = const_cast<char*>(config_database_name.c_str());
        inbind[0].buffer_length = config_database_name.length();
        inbind[0].is_null = 0;

        // Bind the selection parameters to the statement
        int status = mysql_stmt_bind_param(connection_.statements_[statement_index], inbind);
        connection_.checkError(status, statement_index, "unable to bind WHERE clause parameter");

        // Set up the MYSQL_BIND array for the data being returned and bind it to
        // the statement.
        MySqlMasterConfigExchange exchange;
        std::vector<MYSQL_BIND> outbind = exchange.createBindForReceive();
        status = mysql_stmt_bind_result(connection_.statements_[statement_index], &outbind[0]);
        connection_.checkError(status, statement_index, "unable to bind SELECT clause parameters");

        // Execute the statement
        status = mysql_stmt_execute(connection_.statements_[statement_index]);
        connection_.checkError(status, statement_index, "unable to execute");

        // Ensure that all the information is retrieved in one go to avoid
        // overhead of going back and forth between client and server.
        status = mysql_stmt_store_result(connection_.statements_[statement_index]);
        connection_.checkError(status, statement_index, "unable to set up for storing all results");

        // Set up the fetch "release" object to release resources associated
        // with the call to mysql_stmt_fetch when this method exits, then
        // retrieve the data.
        isc::db::MySqlFreeResult fetch_release(connection_.statements_[statement_index]);

        MasterConfigCollection server_configurations;
        while ((status = mysql_stmt_fetch(connection_.statements_[statement_index])) == 0) {
            try {
                server_configurations.push_back(exchange.selectData());
            } catch (const isc::BadValue& ex) {
                // Rethrow the exception with a bit more data.
                isc_throw(BadValue, ex.what()
                                        << ". Statement is <"
                                        << connection_.text_statements_[statement_index] << ">");
            }
        }

        // How did the fetch end?
        if (status == 1) {
            // Error - unable to fetch results
            connection_.checkError(status, statement_index, "unable to fetch results");
        } else if (status == MYSQL_DATA_TRUNCATED) {
            // Data truncated - throw an exception indicating what was at fault
            isc_throw(isc::db::DataTruncated,
                      connection_.text_statements_[statement_index]
                          << " returned truncated data: columns affected are "
                          << exchange.getErrorReceiveColumns());
        }

        return server_configurations;
    }

    MasterConfigPtr selectTimestamp(
        std::string& instance_id,
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        StatementIndex const statement_index(GET_CONFIGURATION_TIMESTAMP);

        // Set up the WHERE clause value
        MYSQL_BIND inbind[1];
        memset(inbind, 0, sizeof(inbind));

        inbind[0].buffer_type = MYSQL_TYPE_STRING;
        inbind[0].buffer = const_cast<char*>(instance_id.c_str());
        inbind[0].buffer_length = instance_id.length();
        inbind[0].is_null = 0;

        // Bind the selection parameters to the statement
        int status = mysql_stmt_bind_param(connection_.statements_[statement_index], inbind);
        connection_.checkError(status, statement_index, "unable to bind WHERE clause parameter");

        MYSQL_BIND outbind[2];
        memset(outbind, 0, sizeof(outbind));

        char instance_ID_buffer[INSTANCE_ID_MAX_LEN];
        unsigned long instance_ID_length = sizeof(instance_ID_buffer);

        // Server Id: VARCHAR(128)
        outbind[0].buffer_type = MYSQL_TYPE_STRING;
        outbind[0].buffer = reinterpret_cast<char*>(instance_ID_buffer);
        outbind[0].buffer_length = instance_ID_length;
        outbind[0].length = &instance_ID_length;
        // outbind[0].is_null = &MLM_FALSE; // commented out for performance
        // reasons, see memset() above

        // Timestamp
        int64_t timestamp = 0;
        outbind[1].buffer_type = MYSQL_TYPE_TIMESTAMP;
        outbind[1].buffer = reinterpret_cast<char*>(&timestamp);
        outbind[1].is_unsigned = isc::db::MLM_TRUE;
        outbind[1].buffer_length = instance_ID_length;
        // outbind[1].is_null = &MLM_FALSE; // commented out for performance
        // reasons, see memset() above

        status = mysql_stmt_bind_result(connection_.statements_[statement_index], &outbind[0]);
        connection_.checkError(status, statement_index, "unable to bind SELECT clause parameters");

        // Execute the statement
        status = mysql_stmt_execute(connection_.statements_[statement_index]);
        connection_.checkError(status, statement_index, "unable to execute");

        // Ensure that all the information is retrieved in one go to avoid
        // overhead of going back and forth between client and server.
        status = mysql_stmt_store_result(connection_.statements_[statement_index]);
        connection_.checkError(status, statement_index, "unable to set up for storing all results");

        // Set up the fetch "release" object to release resources associated
        // with the call to mysql_stmt_fetch when this method exits, then
        // retrieve the data.
        isc::db::MySqlFreeResult fetch_release(connection_.statements_[statement_index]);

        int count = 0;
        MasterConfigPtr result;
        while ((status = mysql_stmt_fetch(connection_.statements_[statement_index])) == 0) {
            try {
                std::string instance_id(instance_ID_buffer, instance_ID_length);
                result.reset(new MasterConfig());
                result->instance_ID_ = instance_id;
                result->timestamp_ = timestamp;

                if (++count > 1) {
                    isc_throw(isc::db::MultipleRecords,
                              "multiple records were found in the "
                              "database where only one was expected for query "
                                  << connection_.text_statements_[statement_index]);
                }
            } catch (const isc::BadValue& ex) {
                // Rethrow the exception with a bit more data.
                isc_throw(BadValue, ex.what()
                                        << ". Statement is <"
                                        << connection_.text_statements_[statement_index] << ">");
            }
        }

        // How did the fetch end?
        if (status == 1) {
            // Error - unable to fetch results
            connection_.checkError(status, statement_index, "unable to fetch results");
        } else if (status == MYSQL_DATA_TRUNCATED) {
            // Data truncated - throw an exception indicating what was at fault
            isc_throw(isc::db::DataTruncated,
                      connection_.text_statements_[statement_index] << " returned truncated data.");
        }

        return result;
    }

    void selectShardNames(
        isc::util::shard_list_t& shards,
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        StatementIndex const statement_index(GET_CONFIGURATION_SHARD_NAMES);

        shards.clear();

        MYSQL_BIND outbind[1];
        memset(outbind, 0, sizeof(outbind));

        char config_database_name_buffer[CONFIG_DATABASE_NAME_MAX_LEN];
        unsigned long config_database_name_length;

        // config_database_name: VARCHAR(128)
        config_database_name_length = sizeof(config_database_name_buffer);
        outbind[0].buffer_type = MYSQL_TYPE_STRING;
        outbind[0].buffer = reinterpret_cast<char*>(config_database_name_buffer);
        outbind[0].buffer_length = config_database_name_length;
        outbind[0].length = &config_database_name_length;
        // outbind[0].is_null = &MLM_FALSE; // commented out for performance
        // reasons, see memset() above

        int status = mysql_stmt_bind_result(connection_.statements_[statement_index], &outbind[0]);
        connection_.checkError(status, statement_index, "unable to bind SELECT clause parameters");

        // Execute the statement
        status = mysql_stmt_execute(connection_.statements_[statement_index]);
        connection_.checkError(status, statement_index, "unable to execute");

        // Ensure that all the information is retrieved in one go to avoid
        // overhead of going back and forth between client and server.
        status = mysql_stmt_store_result(connection_.statements_[statement_index]);
        connection_.checkError(status, statement_index, "unable to set up for storing all results");

        // Set up the fetch "release" object to release resources associated
        // with the call to mysql_stmt_fetch when this method exits, then
        // retrieve the data.
        isc::db::MySqlFreeResult fetch_release(connection_.statements_[statement_index]);

        while ((status = mysql_stmt_fetch(connection_.statements_[statement_index])) == 0) {
            try {
                std::string config_database(config_database_name_buffer,
                                            config_database_name_length);
                shards.push_back(config_database);
            } catch (const isc::BadValue& ex) {
                // Rethrow the exception with a bit more data.
                isc_throw(BadValue, ex.what()
                                        << ". Statement is <"
                                        << connection_.text_statements_[statement_index] << ">");
            }
        }

        // How did the fetch end?
        if (status == 1) {
            // Error - unable to fetch results
            connection_.checkError(status, statement_index, "unable to fetch results");
        } else if (status == MYSQL_DATA_TRUNCATED) {
            // Data truncated - throw an exception indicating what was at fault

            isc_throw(isc::db::DataTruncated,
                      connection_.text_statements_[statement_index] << " returned truncated data.");
        }
    }

    void startTransaction() override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_MYSQL_BEGIN_TRANSACTION);
        if (mysql_query(connection_.mysql_, "START TRANSACTION")) {
            isc_throw(isc::db::DbOperationError,
                      "start transaction failed: " << mysql_error(connection_.mysql_));
        }
    }

    void commit() override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_MYSQL_COMMIT);
        if (mysql_commit(connection_.mysql_) != 0) {
            isc_throw(isc::db::DbOperationError,
                      "commit failed: " << mysql_error(connection_.mysql_));
        }
    }

    void rollback() override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_MYSQL_ROLLBACK);
        if (mysql_rollback(connection_.mysql_) != 0) {
            isc_throw(isc::db::DbOperationError,
                      "rollback failed: " << mysql_error(connection_.mysql_));
        }
    }

    db::VersionTuple getVersion() const {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_MYSQL_GET_VERSION);

        StatementIndex const stindex = GET_VERSION;

        uint32_t major;  // Major version number
        uint32_t minor;  // Minor version number

        // Execute the prepared statement
        int status = mysql_stmt_execute(connection_.statements_[stindex]);
        if (status != 0) {
            isc_throw(isc::db::DbOperationError,
                      "unable to execute <" << connection_.text_statements_[stindex]
                                            << "> - reason: " << mysql_error(connection_.mysql_));
        }

        // Bind the output of the statement to the appropriate variables.
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].is_unsigned = 1;
        bind[0].buffer = &major;
        bind[0].buffer_length = sizeof(major);

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].is_unsigned = 1;
        bind[1].buffer = &minor;
        bind[1].buffer_length = sizeof(minor);

        status = mysql_stmt_bind_result(connection_.statements_[stindex], bind);
        if (status != 0) {
            isc_throw(isc::db::DbOperationError,
                      "unable to bind result set: " << mysql_error(connection_.mysql_));
        }

        // Fetch the data and set up the "release" object to release associated
        // resources when this method exits then retrieve the data.
        isc::db::MySqlFreeResult fetch_release(connection_.statements_[stindex]);
        status = mysql_stmt_fetch(connection_.statements_[stindex]);
        if (status != 0) {
            isc_throw(isc::db::DbOperationError,
                      "unable to obtain result set: " << mysql_error(connection_.mysql_));
        }

        return std::make_pair(major, minor);
    }

    std::string getType() const override final {
        return "mysql";
    }

private:
    static std::array<isc::db::TaggedStatement, STATEMENT_COUNT>& tagged_statements() {
        static std::array<isc::db::TaggedStatement, STATEMENT_COUNT> _{
            {{MySqlMasterConfigMgr<D>::DELETE_CONFIGURATION,
              "TRUNCATE server_configuration" + isc::dhcp::dhcpSpaceToString<D>()},

             {MySqlMasterConfigMgr<D>::GET_CONFIGURATION_BY_INSTANCE_ID,
              "SELECT instance_id, timestamp, server_config, config_database, config_database_name"
              " FROM server_configuration" +
                  isc::dhcp::dhcpSpaceToString<D>() + " WHERE instance_id = ? "},

             {MySqlMasterConfigMgr<D>::GET_CONFIGURATION_BY_SHARD,
              "SELECT instance_id, timestamp, server_config, config_database, config_database_name"
              " FROM server_configuration" +
                  isc::dhcp::dhcpSpaceToString<D>() + " WHERE config_database_name = ?"},

             {MySqlMasterConfigMgr<D>::GET_CONFIGURATION_SHARD_NAMES,
              "SELECT config_database_name"
              " FROM server_configuration" +
                  isc::dhcp::dhcpSpaceToString<D>()},

             {MySqlMasterConfigMgr<D>::GET_CONFIGURATION_TIMESTAMP,
              "SELECT instance_id, timestamp"
              " FROM server_configuration" +
                  isc::dhcp::dhcpSpaceToString<D>() + " WHERE instance_id = ? "},

             {MySqlMasterConfigMgr<D>::INSERT_CONFIGURATION,
              "INSERT INTO server_configuration" + isc::dhcp::dhcpSpaceToString<D>() +
                  " (instance_id, timestamp, server_config, config_database, config_database_name)"
                  " VALUES (?, ?, ?, ?, ?)"},

             {MySqlMasterConfigMgr<D>::GET_VERSION, "SELECT version, minor"
                                                    " FROM master_schema_version"}}};
        return _;
    }

    /// @brief Database connection
    db::MySqlConnection connection_;
};

}  // namespace dhcp
}  // namespace isc

#endif  // MYSQL_CONFIGURATION_MASTER_MGR_H

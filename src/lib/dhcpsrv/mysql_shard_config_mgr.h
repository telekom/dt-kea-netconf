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

#ifndef MYSQL_SHARD_CONFIGURATION_MGR_H
#define MYSQL_SHARD_CONFIGURATION_MGR_H

#include <database/timestamp_store.h>
#include <dhcpsrv/dhcpsrv_log.h>
#include <dhcpsrv/shard_config_mgr.h>
#include <mysql/mysql_connection.h>
#include <util/hash.h>

#include <errmsg.h>
#include <mysql.h>
#include <mysqld_error.h>

#include <array>
#include <string>
#include <utility>
#include <vector>

namespace isc {
namespace dhcp {

template <DhcpSpaceType D>
struct MySqlShardConfigMgr : public ShardConfigMgr {
    using my_bool = db::my_bool;
    /// @brief Statement Tags
    ///
    /// The contents of the enum are indexes into the list of SQL statements
    enum StatementIndex {
        GET_CONFIGURATION_TIMESTAMP,  // Get the timestamp of the stored configuration
        GET_JSON_CONFIGURATION,  // Get only JSON server configuration
        INSERT_CONFIGURATION,  // Add server configuration
        UPDATE_CONFIGURATION,  // Update server configuration
        GET_VERSION,  // Obtain version number
        STATEMENT_COUNT  // Number of statements
    };

    static size_t constexpr CONFIG_DATA_SIZE = 1073741824u;

    /// @brief Constructor
    ///
    /// Uses the following keywords in the parameters passed to it to
    /// connect to the database:
    /// - name - Name of the database to which to connect (mandatory)
    /// - host - Host to which to connect (optional, defaults to "localhost")
    /// - user - Username under which to connect (optional)
    /// - password - Password for "user" on the database (optional)
    ///
    /// If the database is successfully opened, the version number in the
    /// schema_version table will be checked against hard-coded value in
    /// the implementation file.
    ///
    /// Finally, all the SQL commands are pre-compiled.
    ///
    /// @param parameters A data structure relating keywords and values
    ///        concerned with the database.
    ///
    /// @throw isc::dhcp::NoDatabaseName Mandatory database name not given
    /// @throw isc::dhcp::DbOpenError Error opening the database
    /// @throw isc::db::DbOperationError An operation on the open database has
    ///        failed.
    MySqlShardConfigMgr(isc::db::DatabaseConnection::ParameterMap const& parameters)
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

    /// @brief Destructor (closes database)
    virtual ~MySqlShardConfigMgr() = default;

    void del(std::string const& /*config_timestamp_path*/) override final {
        isc_throw(NotImplemented, PRETTY_METHOD_NAME());
    }

    /// @brief Adds if not exists or updates and existing DHCP V6 server
    /// configuration
    ///
    /// One database contains server configuration data only for one kea server.
    ///
    /// @param json_data The server configuration to be written in json format
    /// @param config_timestamp_path path to the config.timestamp file
    ///
    /// @throw isc::db::DbOperationError Connection is ok, so it must be an
    /// SQL error.
    ///
    /// @return true if the configuration was added/updated, false if not.
    void upsert(ShardConfigPtr const& shard_config,
                std::string const& config_timestamp_path) override final {
        ShardConfigPtr current_config(selectTimestamp());

        if (!current_config) {
            insert(shard_config, config_timestamp_path);
        }

        try {
            update(shard_config, config_timestamp_path);
        } catch (std::exception const& e) {
            // The configuration timestamp has been changed since the last
            // configuration read until this update.
            LOG_WARN(dhcpsrv_logger, DHCPSRV_MYSQL_UPDATE_SHARD_CONFIG_TIMESTAMP_CHANGED)
                .arg("N/A")
                .arg(current_config->timestamp_);

            isc_throw(isc::db::TimestampHasChanged,
                      "could not update the database server configuration because the "
                      "timestamp of the database record has been changed and "
                          << e.what());
        }
    }

    /// @brief Retrieves timestamp for the DHCPv4 shard server configuration.
    ///
    /// @return pointer to the generic structure containing the timestamp
    ShardConfigPtr selectTimestamp(
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        StatementIndex const statement_index(GET_CONFIGURATION_TIMESTAMP);
        char config_id[37];  // SQL type CHAR(36) + NULL character
        MYSQL_TIME db_timestamp;

        // Execute the statement
        int status = mysql_stmt_execute(connection_.statements_[statement_index]);
        connection_.checkError(status, GET_CONFIGURATION_TIMESTAMP, "unable to execute");

        // Bind the output of the statement to the appropriate variables.
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        unsigned long config_id_length = sizeof(config_id);
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = &config_id[0];
        bind[0].buffer_length = config_id_length;
        bind[0].length = &config_id_length;

        bind[1].buffer_type = MYSQL_TYPE_TIMESTAMP;
        bind[1].buffer = reinterpret_cast<char*>(&db_timestamp);
        bind[1].buffer_length = sizeof(db_timestamp);

        status = mysql_stmt_bind_result(connection_.statements_[statement_index], bind);
        connection_.checkError(status, GET_CONFIGURATION_TIMESTAMP,
                               "unable to set up for storing all results");

        // Fetch the data and set up the "release" object to release associated
        // resources when this method exits then retrieve the data.
        isc::db::MySqlFreeResult fetch_release(connection_.statements_[statement_index]);
        status = mysql_stmt_fetch(connection_.statements_[statement_index]);
        if (status == MYSQL_NO_DATA) {
            return ShardConfigPtr();
        }
        connection_.checkError(status, statement_index, "unable to obtain result set");

        int64_t timestamp;
        isc::db::MySqlConnection::convertFromDatabaseTime(db_timestamp, 0, timestamp);

        ShardConfigPtr srvConf = std::make_shared<ShardConfig>();
        srvConf->timestamp_ = timestamp;

        return srvConf;
    }

    /// @brief Retrieves the configuration of the DHCPv4 server belonging to the
    ///     current shard in JSON format.
    ///
    /// @return server configuration being retrieved
    ShardConfigPtr select(
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_MYSQL_GET_SHARD_CONFIG_JSON);
        return getConfigCommon(GET_JSON_CONFIGURATION);
    }

    /// @brief Returns backend version.
    ///
    /// @return Version number as a pair of unsigned integers.  "first" is the
    ///         major version number, "second" the minor number.
    ///
    /// @throw isc::db::DbOperationError An operation on the open database has
    ///        failed.
    isc::db::VersionTuple getVersion() const {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_MYSQL_GET_VERSION);
        StatementIndex const statement_index(GET_VERSION);

        uint32_t major;  // Major version number
        uint32_t minor;  // Minor version number

        // Execute the prepared statement
        int status = mysql_stmt_execute(connection_.statements_[statement_index]);
        if (status != 0) {
            isc_throw(isc::db::DbOperationError,
                      "unable to execute <" << connection_.text_statements_[statement_index]
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

        status = mysql_stmt_bind_result(connection_.statements_[statement_index], bind);
        if (status != 0) {
            isc_throw(isc::db::DbOperationError,
                      "unable to bind result set: " << mysql_error(connection_.mysql_));
        }

        // Fetch the data and set up the "release" object to release associated
        // resources when this method exits then retrieve the data.
        isc::db::MySqlFreeResult fetch_release(connection_.statements_[statement_index]);
        status = mysql_stmt_fetch(connection_.statements_[statement_index]);
        if (status != 0) {
            isc_throw(isc::db::DbOperationError,
                      "unable to obtain result set: " << mysql_error(connection_.mysql_));
        }

        return std::make_pair(major, minor);
    }

    /// @brief Return backend type
    ///
    /// Returns the type of the backend (e.g. "mysql", "memfile" etc.)
    ///
    /// @return Type of the backend.
    std::string getType() const override final {
        return "mysql";
    }

    /// @brief Start Transaction
    ///
    /// Start transaction for database operations. On databases that don't
    /// support transactions, this is a no-op.
    void startTransaction() override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_MYSQL_BEGIN_TRANSACTION);
        if (mysql_query(connection_.mysql_, "START TRANSACTION")) {
            isc_throw(isc::db::DbOperationError,
                      "start transaction failed: " << mysql_error(connection_.mysql_));
        }
    }

    /// @brief Commit Transactions
    ///
    /// Commits all pending database operations.  On databases that don't
    /// support transactions, this is a no-op.
    ///
    /// @throw DbOperationError If the commit failed.
    void commit() override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_MYSQL_COMMIT);
        if (mysql_commit(connection_.mysql_) != 0) {
            isc_throw(isc::db::DbOperationError,
                      "commit failed: " << mysql_error(connection_.mysql_));
        }
    }

    /// @brief Rollback Transactions
    ///
    /// Rolls back all pending database operations.  On databases that don't
    /// support transactions, this is a no-op.
    ///
    /// @throw DbOperationError If the rollback failed.
    void rollback() override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_MYSQL_ROLLBACK);
        if (mysql_rollback(connection_.mysql_) != 0) {
            isc_throw(isc::db::DbOperationError,
                      "rollback failed: " << mysql_error(connection_.mysql_));
        }
    }

private:
    static std::array<isc::db::TaggedStatement, STATEMENT_COUNT>& tagged_statements() {
        static std::array<isc::db::TaggedStatement, STATEMENT_COUNT> _{
            {{GET_CONFIGURATION_TIMESTAMP,  //
              "SELECT config_id, timestamp"
              " FROM server_configuration" +
                  isc::dhcp::dhcpSpaceToString<D>()},

             {GET_JSON_CONFIGURATION,  //
              "SELECT config_id, timestamp, json_data"
              " FROM server_configuration" +
                  isc::dhcp::dhcpSpaceToString<D>()},

             {INSERT_CONFIGURATION,  //
              "INSERT INTO server_configuration" + isc::dhcp::dhcpSpaceToString<D>() +
                  " (config_id, timestamp, json_data)"
                  " VALUES (?, ?, ?, ?)"},

             {UPDATE_CONFIGURATION,  //
              "UPDATE server_configuration" + isc::dhcp::dhcpSpaceToString<D>() +
                  " SET timestamp=?, json_data=?"
                  " WHERE config_id=?"},

             {GET_VERSION,  //
              "SELECT version, minor"
              " FROM config_schema_version"}}};
        return _;
    }

    /// @brief Retrieves a single server configuration from the current shard.
    ///
    /// @param statement_index index of the prepared statement being executed in
    ///     the database
    ///
    /// @return the single retrieved configuration
    ShardConfigPtr getConfigCommon(StatementIndex statement_index) {
        char config_id[37];  // SQL type CHAR(36) + NULL character
        MYSQL_TIME db_timestamp;
        // The configuration data is too big to keep it on the thread's stack
        std::vector<char> config_data(
            CONFIG_DATA_SIZE);  // SQL type: LONGTEXT(4 GiB) + NULL character
                                // We limit this data to 60 MB + NULL character

        // Execute the statement
        int status = mysql_stmt_execute(connection_.statements_[statement_index]);
        connection_.checkError(status, statement_index, "unable to execute");

        // Bind the output of the statement to the appropriate variables.
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        unsigned long config_id_length = sizeof(config_id);
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = &config_id[0];
        bind[0].buffer_length = config_id_length;
        bind[0].length = &config_id_length;

        bind[1].buffer_type = MYSQL_TYPE_TIMESTAMP;
        bind[1].buffer = reinterpret_cast<char*>(&db_timestamp);
        bind[1].buffer_length = sizeof(db_timestamp);

        unsigned long config_data_length = config_data.size();
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = &config_data[0];
        bind[2].buffer_length = config_data_length;
        bind[2].length = &config_data_length;

        // overhead of going back and forth between client and server.
        // status = mysql_stmt_store_result(connection_.statements_[statement_index]);
        status = mysql_stmt_bind_result(connection_.statements_[statement_index], bind);
        connection_.checkError(status, statement_index, "unable to set up for storing all results");

        // Fetch the data and set up the "release" object to release associated
        // resources when this method exits then retrieve the data.
        isc::db::MySqlFreeResult fetch_release(connection_.statements_[statement_index]);
        status = mysql_stmt_fetch(connection_.statements_[statement_index]);
        if (status == MYSQL_NO_DATA) {
            return ShardConfigPtr();
        }
        connection_.checkError(status, statement_index, "unable to obtain result set");

        int64_t timestamp;
        isc::db::MySqlConnection::convertFromDatabaseTime(db_timestamp, 0, timestamp);

        ShardConfigPtr srvConf(std::make_shared<ShardConfig>());
        srvConf->configuration(std::string(&config_data[0], config_data_length));
        srvConf->timestamp_ = timestamp;

        return srvConf;
    }

    /// @brief Adds server configuration.
    ///
    /// @param json_data JSON configuration being inserted
    /// @param config_timestamp_path path to the config.timestamp file
    ///
    /// @return true, if configuration has been successfully added, false
    ///     otherwise
    void insert(ShardConfigPtr const& shard_config,
                std::string const& /*config_timestamp_path*/) override final {
        std::string config_id(std::to_string(isc::db::TimestampStore::generate()));
        std::string json_data(shard_config->configuration_string());

        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_MYSQL_INSERT_SHARD_CONFIG)
            .arg(config_id);
        try {
            MYSQL_BIND bind[4];  // Bind array
            memset(bind, 0, sizeof(bind));

            bind[0].buffer_type = MYSQL_TYPE_STRING;
            bind[0].buffer = const_cast<char*>(config_id.c_str());
            bind[0].buffer_length = config_id.length();
            bind[0].is_null = 0;

            MYSQL_TIME expire_time;
            isc::db::timestamp_t new_timestamp(isc::db::TimestampStore::generate());
            connection_.convertToDatabaseTime(new_timestamp, expire_time);
            bind[1].buffer_type = MYSQL_TYPE_TIMESTAMP;
            bind[1].buffer = reinterpret_cast<char*>(&expire_time);
            bind[1].buffer_length = sizeof(expire_time);
            bind[1].is_null = 0;

            bind[2].buffer_type = MYSQL_TYPE_STRING;
            bind[2].buffer = const_cast<char*>(json_data.c_str());
            bind[2].buffer_length = json_data.length();
            bind[2].is_null = 0;

            StatementIndex const statement_index(INSERT_CONFIGURATION);
            int status = mysql_stmt_bind_param(connection_.statements_[statement_index], &bind[0]);
            connection_.checkError(status, INSERT_CONFIGURATION, "unable to bind parameters");

            // Execute the statement
            status = mysql_stmt_execute(connection_.statements_[statement_index]);
            if (status != 0) {
                // Failure: check for the special case of duplicate entry.  If this is
                // the case, we return false to indicate that the row was not added.
                // Otherwise we throw an exception.
                if (mysql_errno(connection_.mysql_) == ER_DUP_ENTRY) {
                    isc_throw(isc::db::DuplicateEntry,
                              "MySqlShardConfigMgr::insert(): duplicate entry");
                }
                connection_.checkError(status, statement_index, "unable to execute");
            }
        } catch (std::exception const& ex) {
            isc_throw(isc::db::DbOperationError,
                      "Could not create bind array to insert configuration "
                          << ", reason: " << ex.what());
        }
    }

    /// @brief Updates server configuration.
    ///
    /// @param statement_index index of the prepared statement being executed in
    ///     the database
    /// @param config_id unique ID for configuration
    /// @param json_data JSON configuration to update the current configuration
    /// @param config_timestamp_path path to the config.timestamp file
    ///
    /// @return true, if configuration has been successfully updated, false
    ///     otherwise
    void update(ShardConfigPtr const& shard_config,
                std::string const& /*config_timestamp_path*/) override final {
        StatementIndex const statement_index(UPDATE_CONFIGURATION);
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_MYSQL_UPDATE_SHARD_CONFIG)
            .arg("0");
        std::string json_data(shard_config->configuration_string());

        try {
            MYSQL_BIND bind[4];  // Bind array
            memset(bind, 0, sizeof(bind));

            MYSQL_TIME expire_time;
            isc::db::timestamp_t new_timestamp(isc::db::TimestampStore::generate());
            connection_.convertToDatabaseTime(new_timestamp, expire_time);
            bind[0].buffer_type = MYSQL_TYPE_TIMESTAMP;
            bind[0].buffer = reinterpret_cast<char*>(&expire_time);
            bind[0].buffer_length = sizeof(expire_time);
            bind[0].is_null = 0;

            bind[1].buffer_type = MYSQL_TYPE_STRING;
            bind[1].buffer = const_cast<char*>(json_data.c_str());
            bind[1].buffer_length = json_data.length();
            bind[1].is_null = 0;

            std::string config_id("0");
            bind[3].buffer_type = MYSQL_TYPE_STRING;
            bind[3].buffer = const_cast<char*>(config_id.c_str());
            bind[3].buffer_length = config_id.length();
            bind[3].is_null = 0;

            int status = mysql_stmt_bind_param(connection_.statements_[statement_index], &bind[0]);
            connection_.checkError(status, statement_index, "unable to bind parameters");

            // Execute the statement
            status = mysql_stmt_execute(connection_.statements_[statement_index]);
            if (status != 0) {
                // Failure: check for the special case of duplicate entry.  If this is
                // the case, we return false to indicate that the row was not added.
                // Otherwise we throw an exception.
                if (mysql_errno(connection_.mysql_) == ER_DUP_ENTRY) {
                    isc_throw(isc::db::DuplicateEntry, "duplicate entry in updateConfigCommon()");
                }
                connection_.checkError(status, statement_index, "unable to execute");
            }
        } catch (std::exception const& ex) {
            isc_throw(isc::db::DbOperationError, "Could not create bind to update configuration: "
                                                     << ", reason: " << ex.what());
        }
    }

    void updateTimestamp(std::string const& /* config_timestamp_path */) override final {
        isc_throw(NotImplemented, PRETTY_METHOD_NAME());
    }

    /// @brief MySQL connection
    isc::db::MySqlConnection connection_;
};

}  // namespace dhcp
}  // namespace isc

#endif  // MYSQL_SHARD_CONFIGURATION_MGR_H

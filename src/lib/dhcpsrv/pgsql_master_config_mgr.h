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

#ifndef PGSQL_MASTER_CONFIG_MGR_H
#define PGSQL_MASTER_CONFIG_MGR_H

#include <dhcpsrv/dhcpsrv_log.h>
#include <dhcpsrv/master_config_mgr.h>
#include <pgsql/pgsql_connection.h>
#include <pgsql/pgsql_exchange.h>
#include <util/types.h>

#include <boost/lexical_cast.hpp>

#include <time.h>

#include <array>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace isc {
namespace dhcp {

/// @brief Manages exchanging of master configurations with PostgreSQL.
struct PgSqlMasterConfigExchange : public isc::db::PgSqlExchange {
    /// @brief Default constructor
    PgSqlMasterConfigExchange() : config_(std::make_shared<MasterConfig>()) {
        BOOST_STATIC_ASSERT(COLUMN_COUNT == 5);

        // Set column names (for error reporting).
        columns_.push_back("instance_id");
        columns_.push_back("timestamp");
        columns_.push_back("server_config");
        columns_.push_back("config_database");
        columns_.push_back("config_database_name");
    }

    virtual ~PgSqlMasterConfigExchange() = default;

    /// @brief Creates the bind array for sending @ref MasterConfig data
    ///     to the database.
    ///
    /// Converts each @ref MasterConfig member into a
    /// PostgreSQL-compatible type and adds it to the bind array. Note that the
    /// array additions must occur in the same order that the SQL statement
    /// specifies. By convention all columns in the table are explicitly listed
    /// in the SQL statements in the same order as they occur in the table.
    ///
    /// @param instance_id ID of the server that has it's configuration written to the database
    /// @param server_config actual configuration of the server that is written to the database
    /// @param config_database configuration to be used on the shard
    /// @param config_database_name name of the shard whose configuration is being added
    /// @param timestamp read from file, operation is valid only if it is equal to database-side
    ///     timestamp
    /// @param[out] bind_array array to populate with the configuration data
    ///
    /// @throw DbOperationError if bind_array cannot be populated
    void createBindForSend(std::string const& instance_id,
                           std::string const& server_config,
                           std::string const& config_database,
                           std::string const& config_database_name,
                           int64_t const timestamp,
                           isc::db::PsqlBindArray& bind_array) {
        // Convert to PostgreSQL-compatible types.
        config_->instance_ID_ = instance_id;
        timestamp_ = timestamp;
        server_config_.assign(server_config.c_str(), server_config.c_str() + server_config.size());
        config_database_.assign(config_database.c_str(),
                                config_database.c_str() + config_database.size());
        config_->shard_ = config_database_name;

        try {
            // Bind to array.
            bind_array.add(config_->instance_ID_);
            bind_array.add(timestamp_);
            bind_array.add(server_config_);
            bind_array.add(config_database_);
            bind_array.add(config_->shard_);
        } catch (std::exception const& ex) {
            isc_throw(isc::db::DbOperationError, "Could not create bind array for "
                                                 "server_configuration[4|6] with server ID: "
                                                     << instance_id
                                                     << ", shard name: " << config_database_name
                                                     << ", server configuration: " << server_config
                                                     << ", reason: " << ex.what());
        }
    }

    /// @brief Creates a @ref MasterConfig object from a given row in a
    ///     result set.
    ///
    /// @param r result set containing one or rows from the server_configuration[4|6] table
    /// @param row row number within the result set from to create the @ref MasterConfig
    ///     object
    ///
    /// @return MasterConfigPtr to the newly created @ref MasterConfig object
    ///
    /// @throw DbOperationError if the server configuration object cannot be created
    MasterConfigPtr convertFromDatabase(isc::db::PgSqlResult const& r, const int row) {
        MasterConfigPtr config = std::make_shared<MasterConfig>();
        try {
            // Retrieve column values.
            size_t converted_bytes;
            unsigned char* bytes;

            // instance_id
            getColumnValue(r, row, INSTANCE_ID, config->instance_ID_);

            // timestamp
            std::string timestamp_s = getRawColumnValue(r, row, TIME_STAMP);
            config->timestamp_ = boost::lexical_cast<int64_t>(timestamp_s);

            // server_config
            bytes = PQunescapeBytea(
                reinterpret_cast<const unsigned char*>(PQgetvalue(r, row, SERVER_CONFIG)),
                &converted_bytes);

            if (bytes) {
                config->server_specific_configuration(std::string(bytes, bytes + converted_bytes));
            }
            PQfreemem(bytes);

            // config_database
            bytes = PQunescapeBytea(
                reinterpret_cast<const unsigned char*>(PQgetvalue(r, row, CONFIG_DATABASE)),
                &converted_bytes);

            if (bytes) {
                config->database_credentials(std::string(bytes, bytes + converted_bytes));
            }
            PQfreemem(bytes);

            // config_database_name
            getColumnValue(r, row, CONFIG_DATABASE_NAME, config->shard_);
        } catch (std::exception const& ex) {
            isc_throw(isc::db::DbOperationError, "Could not convert data to server "
                                                 "configuration object, reason: "
                                                     << ex.what());
        }
        return config;
    }

    // C++98 standard ensures enum values are consecutive starting with 0.
    // Don't add ordinal numbers in order to facilitate further addition of enum
    // values. N_OF_COLUMNS is always last.
    enum Columns {
        INSTANCE_ID,
        TIME_STAMP,
        SERVER_CONFIG,
        CONFIG_DATABASE,
        CONFIG_DATABASE_NAME,
        COLUMN_COUNT
    };

    /// @brief Configuration object used when sending to the database.
    /// Being a class member ensures that it remains in scope while any bindings
    /// that refer to its contents are in use.
    MasterConfigPtr config_;

    /// @brief Configuration specific members used for binding and conversion.
    /// @{
    std::vector<uint8_t> config_database_;
    std::vector<uint8_t> server_config_;
    std::string timestamp_;
    /// @}
};

/// @brief Exchange used to interact with the master server configuration table
/// from the database
template <isc::dhcp::DhcpSpaceType D>
struct PgSqlMasterConfigMgr : public MasterConfigMgr {
    /// @brief Statement Tags
    ///
    /// The contents of the enum are indexes into the list of SQL statements
    enum StatementIndex {
        // Insert DHCP server configuration.
        INSERT_CONFIGURATION,
        // Retrieve DHCP server configuration filtered by server ID.
        GET_CONFIGURATION_BY_INSTANCE_ID,
        // Retrieve DHCP server configuration filtered by shard name.
        GET_CONFIGURATION_BY_SHARD,
        // Retrieve DHCP synchronization timestamp.
        GET_CONFIGURATION_TIMESTAMP,
        // Delete DHCP server configuration.
        DELETE_CONFIGURATION,
        // Retrieve DHCP shard names.
        GET_CONFIGURATION_SHARD_NAMES,
        // Retrieve schema version.
        GET_VERSION,
        // Number of statements
        STATEMENT_COUNT
    };

    /// @brief Constructor
    ///
    /// @param parameters database connection parameters
    explicit PgSqlMasterConfigMgr(const db::DatabaseConnection::ParameterMap& parameters)
        : connection_(parameters), exchange_(new PgSqlMasterConfigExchange()) {

        // Open the database.
        connection_.openDatabase();
        int i = 0;
        for (auto const& it : tagged_statements()) {
            connection_.prepareStatement(it);
            ++i;
        }
    }

    std::list<std::string> selectInstanceIDs(
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        isc_throw(NotImplemented, PRETTY_METHOD_NAME());
    }

    void insert(MasterConfigPtr& master_config,
                std::string const& /*config_timestamp_path*/) override final {
        StatementIndex const statement_index(INSERT_CONFIGURATION);
        try {
            // Bind to array.
            isc::db::PsqlBindArray bind_array;
            exchange_->createBindForSend(
                master_config->instance_ID_, master_config->server_specific_configuration_string(),
                master_config->database_credentials_string(), master_config->shard_, 0, bind_array);

            // Execute query.
            isc::db::PgSqlResult r(PQexecPrepared(
                connection_, tagged_statements()[statement_index].name,
                tagged_statements()[statement_index].nbparams, &bind_array.values_[0],
                &bind_array.lengths_[0], &bind_array.formats_[0], 0));

            // Check for errors.
            connection_.checkStatementError(r, tagged_statements()[statement_index]);
        } catch (std::exception const& ex) {
            isc_throw(isc::db::DbOperationError, "Cannot add server configuration into the "
                                                 "master database, reason: "
                                                     << ex.what());
        }
    }

    void update(MasterConfigPtr& /*master_config*/,
                std::string const& /*config_timestamp_path*/) override final {
        isc_throw(NotImplemented, PRETTY_METHOD_NAME());
    }

    void deleteAll(std::string const& /*config_timestamp_path*/) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_PGSQL_CLEAR_MASTER_CONFIG);

        StatementIndex const statement_index(DELETE_CONFIGURATION);
        isc::db::PgSqlResult r(PQexecPrepared(
            connection_, tagged_statements()[statement_index].name, 0, NULL, NULL, NULL, 0));
        connection_.checkStatementError(r, tagged_statements()[statement_index]);
    }

    void del(std::string& /*instance_id*/,
             std::string const& /*config_timestamp_path*/) override final {
        isc_throw(NotImplemented, PRETTY_METHOD_NAME());
    }

    MasterConfigPtr select(
        std::string& instance_id,
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        StatementIndex const statement_index(GET_CONFIGURATION_BY_INSTANCE_ID);
        MasterConfigPtr config;
        try {
            // Bind to array.
            isc::db::PsqlBindArray bind_array;
            bind_array.add(instance_id);

            // Execute query.
            isc::db::PgSqlResult r(PQexecPrepared(
                connection_, tagged_statements()[statement_index].name,
                tagged_statements()[statement_index].nbparams, &bind_array.values_[0],
                &bind_array.lengths_[0], &bind_array.formats_[0], 0));

            // Check for errors.
            connection_.checkStatementError(r, tagged_statements()[statement_index]);

            // Populate shard configurations.
            int rows = PQntuples(r);
            for (int i = 0; i < rows; ++i) {
                config = exchange_->convertFromDatabase(r, i);
            }
        } catch (std::exception const& ex) {
            isc_throw(isc::db::DbOperationError, "Cannot retrieve server configuration from "
                                                 "the master database, reason: "
                                                     << ex.what());
        }

        return config;
    }

    MasterConfigCollection selectByShard(
        std::string const& config_database_name,
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        StatementIndex const statement_index(GET_CONFIGURATION_BY_SHARD);

        MasterConfigCollection server_configurations;
        try {
            // Bind to array.
            isc::db::PsqlBindArray bind_array;
            bind_array.add(config_database_name);

            // Execute query.
            isc::db::PgSqlResult r(PQexecPrepared(
                connection_, tagged_statements()[statement_index].name,
                tagged_statements()[statement_index].nbparams, &bind_array.values_[0],
                &bind_array.lengths_[0], &bind_array.formats_[0], 0));

            // Check for errors.
            connection_.checkStatementError(r, tagged_statements()[statement_index]);

            // Populate configuration object.
            int rows = PQntuples(r);
            for (int i = 0; i < rows; ++i) {
                server_configurations.push_back(exchange_->convertFromDatabase(r, i));
            }
        } catch (std::exception const& ex) {
            isc_throw(isc::db::DbOperationError, "Cannot retrieve server configuration from "
                                                 "the master database, reason: "
                                                     << ex.what());
        }

        return server_configurations;
    }

    MasterConfigPtr selectTimestamp(
        std::string& instance_id,
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        StatementIndex const statement_index(GET_CONFIGURATION_TIMESTAMP);
        MasterConfigPtr config = std::make_shared<MasterConfig>();
        try {
            // Bind to array.
            isc::db::PsqlBindArray bind_array;
            bind_array.add(instance_id);

            // Execute query.
            isc::db::PgSqlResult r(PQexecPrepared(
                connection_, tagged_statements()[statement_index].name,
                tagged_statements()[statement_index].nbparams, &bind_array.values_[0],
                &bind_array.lengths_[0], &bind_array.formats_[0], 0));

            // Check for errors.
            connection_.checkStatementError(r, tagged_statements()[statement_index]);
            int affected_rows = boost::lexical_cast<int>(PQcmdTuples(r));
            if (affected_rows < 1) {
                return config;
            }

            // Populate timestamp.
            std::istringstream tmp;
            std::string timestamp_s;

            tmp.str(PQgetvalue(r, 0, 0));
            tmp >> timestamp_s;

            int64_t timestamp = boost::lexical_cast<int64_t>(timestamp_s);

            config->timestamp_ = timestamp;
        } catch (std::exception const& ex) {
            isc_throw(isc::db::DbOperationError,
                      "Cannot retrieve timestamp from the master database, reason: " << ex.what());
        }

        return config;
    }

    void selectShardNames(
        isc::util::shard_list_t& shards,
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        shards.clear();

        StatementIndex const statement_index(GET_CONFIGURATION_SHARD_NAMES);

        try {
            // Execute query.
            isc::db::PgSqlResult r(PQexecPrepared(
                connection_, tagged_statements()[statement_index].name, 0, NULL, NULL, NULL, 0));

            // Check for errors.
            connection_.checkStatementError(r, tagged_statements()[statement_index]);
            int affected_rows = boost::lexical_cast<int>(PQcmdTuples(r));
            if (affected_rows < 1) {
                isc_throw(isc::db::DbOperationError, "affected_rows < 1");
            }

            // Populate shard names.
            int rows = PQntuples(r);
            for (int i = 0; i < rows; ++i) {
                std::string config_database_name;
                exchange_->getColumnValue(r, i, 0, config_database_name);
                shards.push_back(config_database_name);
            }
        } catch (std::exception const& ex) {
            isc_throw(
                isc::db::DbOperationError,
                "Cannot retrieve shard names from the master database, reason: " << ex.what());
        }
    }

    db::VersionTuple getVersion() const {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_PGSQL_GET_VERSION);
        StatementIndex const statement_index(GET_VERSION);

        isc::db::PgSqlResult r(PQexecPrepared(
            connection_, tagged_statements()[statement_index].name, 0, NULL, NULL, NULL, 0));
        connection_.checkStatementError(r, tagged_statements()[statement_index]);

        std::istringstream tmp;
        uint32_t major;
        tmp.str(PQgetvalue(r, 0, 0));
        tmp >> major;
        tmp.str(std::string());
        tmp.clear();

        uint32_t minor;
        tmp.str(PQgetvalue(r, 0, 1));
        tmp >> minor;

        return std::make_pair(major, minor);
    }

    std::string getType() const override final {
        return "postgresql";
    }

    void startTransaction() override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_PGSQL_BEGIN_TRANSACTION);
        PGresult* r = PQexec(connection_, "START TRANSACTION");
        if (PQresultStatus(r) != PGRES_COMMAND_OK) {
            const char* error_message = PQerrorMessage(connection_);
            PQclear(r);
            isc_throw(isc::db::DbOperationError, "start transaction failed: " << error_message);
        }

        PQclear(r);
    }

    void commit() override final {
        connection_.commit();
    }

    void rollback() override final {
        connection_.rollback();
    }

private:
    static std::array<isc::db::PgSqlTaggedStatement, STATEMENT_COUNT>& tagged_statements() {
        static std::string const DELETE_CONFIGURATION("TRUNCATE server_configuration" +
                                                      dhcpSpaceToString<D>());
        static std::string const GET_CONFIGURATION_BY_INSTANCE_ID(
            "SELECT instance_id, extract(epoch from timestamp)::bigint, server_config, "
            "config_database, "
            "config_database_name"
            " FROM server_configuration" +
            dhcpSpaceToString<D>() + " WHERE instance_id = $1");
        static std::string const GET_CONFIGURATION_BY_SHARD(
            "SELECT instance_id, extract(epoch from timestamp)::bigint, server_config, "
            "config_database, "
            "config_database_name"
            " FROM server_configuration" +
            dhcpSpaceToString<D>() + " WHERE config_database_name = $1");
        static std::string const GET_CONFIGURATION_SHARD_NAMES(("SELECT config_database_name"
                                                                " FROM server_configuration" +
                                                                dhcpSpaceToString<D>()));
        static std::string const GET_CONFIGURATION_TIMESTAMP(
            "SELECT extract(epoch from timestamp)::bigint"
            " FROM server_configuration" +
            dhcpSpaceToString<D>() + " WHERE instance_id = $1");
        static std::string const INSERT_CONFIGURATION(
            "INSERT INTO server_configuration" + dhcpSpaceToString<D>() +
            " (instance_id, timestamp, server_config, config_database, "
            "config_database_name)"
            " VALUES ($1, $2, $3, $4, $5)");

        static std::array<isc::db::PgSqlTaggedStatement, STATEMENT_COUNT> _{
            {// DELETE_CONFIGURATION
             {0, {isc::db::OID_NONE}, "DELETE_CONFIGURATION", DELETE_CONFIGURATION.c_str()},

             // GET_CONFIGURATION_BY_INSTANCE_ID
             {1,
              {isc::db::OID_VARCHAR},
              "GET_CONFIGURATION_BY_INSTANCE_ID",
              GET_CONFIGURATION_BY_INSTANCE_ID.c_str()},

             // GET_CONFIGURATION_BY_SHARD
             {1,
              {isc::db::OID_VARCHAR},
              "GET_CONFIGURATION_BY_SHARD_DB",
              GET_CONFIGURATION_BY_SHARD.c_str()},

             // GET_CONFIGURATION_SHARD_NAMES
             {0,
              {isc::db::OID_NONE},
              "GET_CONFIGURATION_SHARD_NAMES",
              GET_CONFIGURATION_SHARD_NAMES.c_str()},

             // GET_CONFIGURATION_TIMESTAMP
             {1,
              {isc::db::OID_VARCHAR},
              "GET_CONFIGURATION_TIMESTAMP",
              GET_CONFIGURATION_TIMESTAMP.c_str()},

             // INSERT_CONFIGURATION
             {5,
              {isc::db::OID_VARCHAR, isc::db::OID_TIMESTAMP, isc::db::OID_BYTEA, isc::db::OID_BYTEA,
               isc::db::OID_VARCHAR},
              "INSERT_CONFIGURATION",
              INSERT_CONFIGURATION.c_str()},

             // GET_VERSION
             {0,
              {isc::db::OID_NONE},
              "GET_VERSION",
              "SELECT version, minor"
              " FROM master_schema_version"}}};
        return _;
    }

    /// @brief Database connection
    db::PgSqlConnection connection_;
    // @brief Exchange in charge of data conversion to and from PostgreSQL types
    std::shared_ptr<PgSqlMasterConfigExchange> exchange_;
};

}  // namespace dhcp
}  // namespace isc

#endif  // PGSQL_MASTER_CONFIG_MGR_H

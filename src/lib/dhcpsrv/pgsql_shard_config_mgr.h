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

#ifndef PGSQL_SHARD_CONFIG_MGR_H
#define PGSQL_SHARD_CONFIG_MGR_H

#include <database/timestamp_store.h>
#include <dhcpsrv/dhcpsrv_log.h>
#include <dhcpsrv/shard_config_mgr.h>
#include <pgsql/pgsql_connection.h>
#include <pgsql/pgsql_exchange.h>
#include <util/hash.h>

#include <boost/lexical_cast.hpp>

#include <array>
#include <string>
#include <utility>
#include <vector>

#include <time.h>

namespace isc {
namespace dhcp {

template <DhcpSpaceType D>
struct PgSqlShardConfigMgr : public ShardConfigMgr {
    /// @brief Statement Tags
    ///
    /// The contents of the enum are indexes into the list of SQL statements
    enum StatementIndex {
        GET_VERSION,  // Obtain version number
        GET_CONFIGURATION_TIMESTAMP,  // Get the timestamp of the stored configuration
        GET_JSON_CONFIGURATION,  // Get only JSON server configuration v6
        INSERT_CONFIGURATION,  // Add server configuration v6
        UPDATE_CONFIGURATION,  // Update server configuration v6
        STATEMENT_COUNT  // Number of statements
    };

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
    PgSqlShardConfigMgr(isc::db::DatabaseConnection::ParameterMap const& parameters)
        : connection_(parameters) {

        // Open the database.
        connection_.openDatabase();
        int i = 0;
        for (auto const& it : tagged_statements()) {
            connection_.prepareStatement(it);
            ++i;
        }

        // Check that all statements have been prepared.
        assert(i == STATEMENT_COUNT);
    }

    void del(std::string const& /*config_timestamp_path*/) override final {
        isc_throw(isc::NotImplemented, PRETTY_METHOD_NAME());
    }

    ShardConfigPtr selectTimestamp(
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        StatementIndex const statement_index(GET_CONFIGURATION_TIMESTAMP);

        isc::db::PgSqlResult r(PQexecPrepared(
            connection_, tagged_statements()[statement_index].name, 0, NULL, NULL, NULL, 0));
        connection_.checkStatementError(r, tagged_statements()[statement_index]);

        int affected_rows = boost::lexical_cast<int>(PQcmdTuples(r));
        if (affected_rows < 1) {
            return ShardConfigPtr();
        }

        std::istringstream tmp;
        std::string config_id;
        std::string db_timestamp;

        tmp.str(PQgetvalue(r, 0, 0));
        tmp >> config_id;
        tmp.str(std::string());
        tmp.clear();

        tmp.str(PQgetvalue(r, 0, 1));
        tmp >> db_timestamp;

        int64_t const timestamp = boost::lexical_cast<int64_t>(db_timestamp);

        ShardConfigPtr srvConf = std::make_shared<ShardConfig>();
        srvConf->timestamp_ = timestamp;

        return srvConf;
    }

    ShardConfigPtr select(
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_PGSQL_GET_SHARD_CONFIG_JSON);
        StatementIndex const statement_index(GET_JSON_CONFIGURATION);
        return getConfigCommon(statement_index);
    }

    void insert(ShardConfigPtr const& shard_config,
                std::string const& /*config_timestamp_path*/) override final {
        StatementIndex const statement_index(INSERT_CONFIGURATION);
        std::string json_data(shard_config->configuration_string());
        isc::db::PsqlBindArray bind_array;

        std::string config_id("0");

        std::vector<uint8_t> json;
        json.assign(json_data.c_str(), json_data.c_str() + json_data.size());

        isc::db::timestamp_t new_timestamp(isc::db::TimestampStore::generate());
        std::string timestamp_str = isc::db::PgSqlExchange::convertToDatabaseTime(new_timestamp);

        try {
            bind_array.add(config_id);
            bind_array.add(timestamp_str);
            bind_array.add(json);
            isc::db::PgSqlResult r(PQexecPrepared(
                connection_, tagged_statements()[statement_index].name,
                tagged_statements()[statement_index].nbparams, &bind_array.values_[0],
                &bind_array.lengths_[0], &bind_array.formats_[0], 0));

            connection_.checkStatementError(r, tagged_statements()[statement_index]);
        } catch (std::exception const& ex) {
            isc_throw(isc::db::DbOperationError,
                      "Could not create bind array to insert configuration "
                          << ", reason: " << ex.what());
        }
    }

    void upsert(ShardConfigPtr const& shard_config,
                std::string const& config_timestamp_path) override final {
        ShardConfigPtr current_config = selectTimestamp();

        if (!current_config) {
            try {
                insert(shard_config, config_timestamp_path);
            } catch (std::exception const& e) {
                throw;
            }
            return;
        }

        try {
            update(shard_config, config_timestamp_path);
        } catch (std::exception const& e) {
            // The configuration timestamp has been changed since the last
            // configuration read until this update.
            LOG_WARN(dhcpsrv_logger, DHCPSRV_PGSQL_UPDATE_SHARD_CONFIG_TIMESTAMP_CHANGED)
                .arg("N/A")
                .arg(current_config->timestamp_);

            isc_throw(isc::db::TimestampHasChanged,
                      "could not update the database server configuration because the "
                      "timestamp of the database record has been changed and "
                          << e.what());
        }
    }

    /// @brief Returns backend version.
    ///
    /// @return Version number as a pair of unsigned integers.  "first" is the
    ///         major version number, "second" the minor number.
    ///
    /// @throw isc::db::DbOperationError An operation on the open database has
    ///        failed.
    isc::db::VersionTuple getVersion() const {
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

    /// @brief Return backend type
    ///
    /// @return the type of the backend ('postgresql').
    std::string getType() const override final {
        return "postgresql";
    }

    /// @brief Start Transaction
    ///
    /// Start transaction for database operations. On databases that don't
    /// support transactions, this is a no-op.
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

    /// @brief Commit Transactions
    ///
    /// Commits all pending database operations.
    ///
    /// @throw DbOperationError If the commit failed.
    void commit() override final {
        connection_.commit();
    }

    /// @brief Rollback Transactions
    ///
    /// Rolls back all pending database operations.
    ///
    /// @throw DbOperationError If the rollback failed.
    void rollback() override final {
        connection_.rollback();
    }

private:
    static std::array<isc::db::PgSqlTaggedStatement, STATEMENT_COUNT>& tagged_statements() {
        static std::string const GET_CONFIGURATION_TIMESTAMP(
            ("INSERT INTO server_configuration" + dhcpSpaceToString<D>() +
             " (instance_id, timestamp, server_config, config_database, "
             "config_database_name)"
             " VALUES ($1, $2, $3, $4, $5)"));
        static std::string const GET_JSON_CONFIGURATION(
            "SELECT config_id, extract(epoch from timestamp)::bigint, json_data"
            " FROM server_configuration" +
            dhcpSpaceToString<D>());
        static std::string const INSERT_CONFIGURATION("INSERT INTO server_configuration" +
                                                      dhcpSpaceToString<D>() +
                                                      " (config_id, timestamp, json_data)"
                                                      " VALUES ($1, $2, $3, $4)");
        static std::string const UPDATE_CONFIGURATION("UPDATE server_configuration" +
                                                      dhcpSpaceToString<D>() +
                                                      " SET timestamp = $1, json_data = $2"
                                                      " WHERE config_id = $3");

        static std::array<isc::db::PgSqlTaggedStatement, STATEMENT_COUNT> _{
            {// GET_CONFIGURATION_TIMESTAMP
             {0,
              {isc::db::OID_NONE},
              "GET_CONFIGURATION_TIMESTAMP",
              GET_CONFIGURATION_TIMESTAMP.c_str()},

             // GET_JSON_CONFIGURATION
             {0, {isc::db::OID_NONE}, "GET_JSON_CONFIGURATION", GET_JSON_CONFIGURATION.c_str()},

             // INSERT_CONFIGURATION
             {4,
              {isc::db::OID_VARCHAR, isc::db::OID_TIMESTAMP, isc::db::OID_BYTEA,
               isc::db::OID_BYTEA},
              "INSERT_CONFIGURATION",
              INSERT_CONFIGURATION.c_str()},

             // UPDATE_CONFIGURATION6
             {4,
              {isc::db::OID_TIMESTAMP, isc::db::OID_BYTEA, isc::db::OID_BYTEA,
               isc::db::OID_VARCHAR},
              "UPDATE_CONFIGURATION",
              UPDATE_CONFIGURATION.c_str()},

             // GET_VERSION
             {0,
              {isc::db::OID_NONE},
              "GET_VERSION",
              "SELECT version, minor"
              " FROM config_schema_version"}}};
        return _;
    }

    ShardConfigPtr getConfigCommon(StatementIndex statement_index) {
        isc::db::PgSqlResult r(PQexecPrepared(
            connection_, tagged_statements()[statement_index].name, 0, NULL, NULL, NULL, 0));

        connection_.checkStatementError(r, tagged_statements()[statement_index]);
        int affected_rows = boost::lexical_cast<int>(PQcmdTuples(r));
        if (affected_rows < 1) {
            return ShardConfigPtr();
        }

        std::istringstream tmp;
        std::string config_id;
        std::string db_timestamp;
        std::string config_data;

        tmp.str(PQgetvalue(r, 0, 0));
        tmp >> config_id;
        tmp.str(std::string());
        tmp.clear();

        tmp.str(PQgetvalue(r, 0, 1));
        tmp >> db_timestamp;
        tmp.str(std::string());
        tmp.clear();

        int64_t const timestamp = boost::lexical_cast<int64_t>(db_timestamp);

        size_t bytes_converted;
        unsigned char* bytes = PQunescapeBytea(
            reinterpret_cast<const unsigned char*>(PQgetvalue(r, 0, 2)), &bytes_converted);

        if (bytes) {
            config_data.assign(bytes, bytes + bytes_converted);
        }
        PQfreemem(bytes);

        ShardConfigPtr srvConf = std::make_shared<ShardConfig>();
        srvConf->timestamp_ = timestamp;
        srvConf->configuration(config_data);

        return srvConf;
    }

    void update(ShardConfigPtr const& shard_config,
                std::string const& /*config_timestamp_path*/) override final {
        StatementIndex const statement_index(UPDATE_CONFIGURATION);
        std::string json_data(shard_config->configuration_string());
        isc::db::PsqlBindArray bind_array;

        std::vector<uint8_t> json;
        json.assign(json_data.c_str(), json_data.c_str() + json_data.size());

        isc::db::timestamp_t new_timestamp(isc::db::TimestampStore::generate());
        std::string timestamp_str = isc::db::PgSqlExchange::convertToDatabaseTime(new_timestamp);

        std::string config_id("0");
        try {
            bind_array.add(timestamp_str);
            bind_array.add(json);
            bind_array.add(config_id);
            isc::db::PgSqlResult r(PQexecPrepared(
                connection_, tagged_statements()[statement_index].name,
                tagged_statements()[statement_index].nbparams, &bind_array.values_[0],
                &bind_array.lengths_[0], &bind_array.formats_[0], 0));

            connection_.checkStatementError(r, tagged_statements()[statement_index]);
        } catch (std::exception const& ex) {
            isc_throw(isc::db::DbOperationError,
                      "Could not create bind array to insert configuration "
                          << ", reason: " << ex.what());
        }
    }

    void updateTimestamp(std::string const& /* config_timestamp_path */) override final {
        isc_throw(NotImplemented, PRETTY_METHOD_NAME());
    }

    /// @brief PostgreSQL connection handle
    isc::db::PgSqlConnection connection_;
};

}  // namespace dhcp
}  // namespace isc

#endif  // PGSQL_SHARD_CONFIG_MGR_H

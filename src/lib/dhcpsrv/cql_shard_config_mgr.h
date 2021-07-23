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

#ifndef CQL_SHARD_CONFIGURATION_MGR_H
#define CQL_SHARD_CONFIGURATION_MGR_H

#include <cql/cql_connection.h>
#include <cql/cql_exchange.h>
#include <cql/cql_transaction.h>
#include <database/granular_retry.h>
#include <database/timestamp_store.h>
#include <dhcpsrv/configuration.h>
#include <dhcpsrv/dhcpsrv_log.h>
#include <dhcpsrv/shard_config_mgr.h>
#include <util/hash.h>

#include <stddef.h>  // for NULL
#include <sys/types.h>  // for int32_t, int64_t

#include <mutex>
#include <string>
#include <utility>

namespace isc {
namespace dhcp {

struct ShardConfigVersionTuple : isc::db::VersionTuple {};

template <typename T>
struct CqlConfigExchange : isc::db::CqlExchange<T> {
    using CM = isc::dhcp::ConfigurationManager<T::DHCP_SPACE, ConfigurationConstants::NOT_IETF>;

    /// @brief Constructor
    ///
    /// Specifies table columns.
    CqlConfigExchange() : configuration_(std::make_shared<T>()) {
    }

    /// @brief Destructor
    virtual ~CqlConfigExchange() = default;

    /// @brief Create BIND array to receive server configuration data.
    ///
    /// Used in executeSelect() to retrieve server configuration
    /// from database
    ///
    /// @param data array of bound objects representing data to be retrieved.
    /// @param statement_tag prepared statement being executed; defaults to an
    /// invalid index
    void createBindForSelect(
        db::AnyArray& data,
        isc::db::StatementTag const& statement_tag = isc::db::StatementTag()) override final {
        // Start with a fresh array.
        data.clear();

        if (statement_tag == GET_JSON_CONFIGURATION()) {
            data.add(configuration_->configuration_string_pointer());
        }
        data.add(&configuration_->timestamp_);
    }

    void exclude(ShardConfigPtr const& shard_config) {
        isc::data::ElementPtr const& config(isc::data::copy(shard_config->configuration()));
        if (config) {
            isc::data::ElementPtr const& dhcp(CM::getDhcpExceptionSafe(config));
            if (dhcp) {
                // Exclude subnets.
                std::string const& subnet("subnet" + isc::dhcp::dhcpSpaceToString<T::DHCP_SPACE>());
                if (dhcp->contains(subnet)) {
                    dhcp->remove(subnet);
                }

                // Exclude hook libraries.
                CM::excludeHookLibraries(dhcp);
            }

            // Exclude network topologies for hook library configurations.
            CM::excludeNetworkTopologies(config);

            shard_config->configuration(config);
        }
    }

    /// @brief Copy received data into @ref ShardConfig object
    ///
    /// Copies information about the shard configuration into a newly created
    /// @ref ShardConfig object. Called in @ref executeSelect().
    ///
    /// @return pointer to a @ref std::shared_ptr<T> object holding a
    /// pointer to the @ref ShardConfig object returned.
    isc::db::Ptr<T> retrieve() override final {
        std::shared_ptr<T> result(std::make_shared<T>(*configuration_));
        result->sanitize();
        return result;
    }

    void deleteCommon(isc::db::CqlConnection& connection,
                      std::string const& config_timestamp_path,
                      isc::db::StatementTag statement_tag) {
        // Bind to array.
        isc::db::AnyArray assigned_values;

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection.getTransactionID());
        assigned_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        isc::db::CqlExchange<T>::executeMutation(connection, assigned_values, statement_tag);
        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        config_timestamp.del(table(), std::to_string(universalID()));
    }

    /// @brief Common method of executing SELECT statements for the Cassandra
    ///     database.
    ///
    /// @param connection connection used to communicate with the Cassandra
    ///     database
    /// @param statement_tag prepared Cassandra statement being executed
    /// @param single is the result expected to consist of a single row?
    ///
    /// @return true if statement has been [applied], false otherwise.
    isc::db::Collection<T> getCommon(isc::db::CqlConnection& connection,
                                     std::string const& config_timestamp_path,
                                     isc::db::StatementTag statement_tag,
                                     bool const single = false) {
        // Bind to array.
        isc::db::AnyArray where_values;

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection.getTransactionID());
        where_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        isc::db::Collection<T> collection(isc::db::CqlExchange<T>::executeSelect(
            connection, where_values, statement_tag, single));

        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        for (auto const& record : collection) {
            config_timestamp.write(table(), std::to_string(universalID()), record->timestamp_);
        }

        return collection;
    }

    /// @brief Common method of executing INSERT statements for the shard
    ///     database in the Cassandra database.
    ///
    /// @param connection connection used to communicate with the Cassandra
    ///     database
    /// @param ID identifier of the configuration being inserted
    /// @param configuration JSON being inserted
    /// @param statement_tag prepared Cassandra statement being executed
    ///
    /// @return true if statement has been [applied], false otherwise.
    void insertCommon(isc::db::CqlConnection& connection,
                      ShardConfigPtr const& shard_config,
                      std::string const& config_timestamp_path,
                      isc::db::StatementTag statement_tag) {
        cass_int64_t new_timestamp(isc::db::TimestampStore::now());

        exclude(shard_config);

        // Bind to array.
        isc::db::AnyArray assigned_values{
            shard_config->configuration_string_pointer(),
            &new_timestamp,
        };

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection.getTransactionID());
        assigned_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        isc::db::CqlExchange<T>::executeMutation(connection, assigned_values, statement_tag);
        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        config_timestamp.write(table(), std::to_string(universalID()), new_timestamp);
    }

    /// @brief Common method of executing SELECT statements for the Cassandra
    ///     database.
    /// @param connection connection used to communicate with the Cassandra
    ///     database
    /// @param ID identifier of the configuration being inserted
    /// @param configuration JSON being inserted
    /// @param statement_tag prepared Cassandra statement being executed
    ///
    /// @return true if statement has been [applied], false otherwise.
    void updateCommon(isc::db::CqlConnection& connection,
                      ShardConfigPtr const& shard_config,
                      std::string const& config_timestamp_path,
                      isc::db::StatementTag statement_tag) {
        cass_int64_t new_timestamp(isc::db::TimestampStore::now());

        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        isc::db::timestamp_t old_timestamp(
            config_timestamp.read(table(), std::to_string(universalID())));

        exclude(shard_config);

        // Bind to array.
        isc::db::AnyArray data{shard_config->configuration_string_pointer(), &new_timestamp,
                               &old_timestamp};

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection.getTransactionID());
        data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        isc::db::CqlExchange<T>::executeMutation(connection, data, statement_tag);
        config_timestamp.write(table(), std::to_string(universalID()), new_timestamp);
    }

    void updateTimestamp(isc::db::CqlConnection& connection,
                         std::string const& config_timestamp_path) {
        // Bind to array.
        cass_int64_t new_timestamp(isc::db::TimestampStore::now());
        isc::db::AnyArray data{&new_timestamp};
#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection.getTransactionID());
        data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        // Execute.
        isc::db::CqlExchange<T>::executeMutation(connection, data,
                                                 CqlConfigExchange<T>::UPDATE_TIMESTAMP());

        // Write to timestamp store.
        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        config_timestamp.write(table(), std::to_string(universalID()), new_timestamp);
    }

    /// @brief The keyspace which this exchange operates on.
    ///
    /// Used in all statements this exchange operates on.
    static std::string const& table() {
        static std::string const _("server_configuration" +
                                   isc::dhcp::dhcpSpaceToString<T::DHCP_SPACE>());
        return _;
    }

    /// @brief The initialization function which this exchange.
    ///
    /// Used to initialize the static members of this exchange.
    static isc::db::StatementMap& tagged_statements() {
        static isc::db::StatementMap _;

        static std::once_flag flag;
        std::call_once(flag, [&] {
            isc::db::StatementText text;

            text = "DELETE FROM " + table() + " WHERE id = " + std::to_string(universalID());
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text = "@free = " + text + " IN TXN ?";
#else  // TERASTREAM_FULL_TRANSACTIONS
        text += " IF EXISTS";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(DELETE_SHARD_CONFIGURATION(),
                          isc::db::CqlTaggedStatement(DELETE_SHARD_CONFIGURATION(), text));

            text = "SELECT timestamp FROM " + table();
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text = "@free = " + text + " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(GET_CONFIGURATION_TIMESTAMP(),
                          isc::db::CqlTaggedStatement(GET_CONFIGURATION_TIMESTAMP(), text));

            text = "SELECT configuration, timestamp FROM " + table();
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text = "@free = " + text + " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(GET_JSON_CONFIGURATION(),
                          isc::db::CqlTaggedStatement(GET_JSON_CONFIGURATION(), text));

            text = "INSERT INTO " + table() + " (id, configuration, timestamp) VALUES (" +
                   std::to_string(universalID()) + ", ?, ?)";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text += " IN TXN ?";
#else  // TERASTREAM_FULL_TRANSACTIONS
        text += " IF NOT EXISTS";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(INSERT_CONFIGURATION(),
                          isc::db::CqlTaggedStatement(INSERT_CONFIGURATION(), text));

            text = "UPDATE " + table() + " SET configuration = ?, timestamp = ? WHERE id = " +
                   std::to_string(universalID()) + " IF timestamp = ?";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text += " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(UPDATE_CONFIGURATION(),
                          isc::db::CqlTaggedStatement(UPDATE_CONFIGURATION(), text));

            text = "UPDATE " + table() +
                   " SET timestamp = ? WHERE id = " + std::to_string(universalID());
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text += " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(UPDATE_TIMESTAMP(),
                          isc::db::CqlTaggedStatement(UPDATE_TIMESTAMP(), text));
        });
        return _;
    }

    static int16_t constexpr universalID() {
        return 0;
    }

    /// @brief Statement tags
    /// @{
    // Delete configuration.
    static isc::db::StatementTag& DELETE_SHARD_CONFIGURATION() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }

    // Get the timestamp of the stored configuration.
    static isc::db::StatementTag& GET_CONFIGURATION_TIMESTAMP() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }

    // Get only JSON server configuration.
    static isc::db::StatementTag& GET_JSON_CONFIGURATION() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }

    // Add configuration.
    static isc::db::StatementTag& INSERT_CONFIGURATION() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }

    // Update configuration.
    static isc::db::StatementTag& UPDATE_CONFIGURATION() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }

    // Update timestamp.
    static isc::db::StatementTag& UPDATE_TIMESTAMP() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }
    /// @}

private:
    /// @brief Structure containing configuration data
    std::shared_ptr<T> configuration_;
};  // CqlConfigExchange

template <typename shard_config_t>
struct CqlShardConfigMgr : ShardConfigMgr {
    /// @brief Constructor
    explicit CqlShardConfigMgr(const isc::db::DatabaseConnection::ParameterMap& parameters)
        : ShardConfigMgr(), connection_(parameters) {
        isc::db::CqlVersionExchange<ShardConfigVersionTuple> version_exchange;
        version_exchange.validateSchema(connection_);

#ifdef TERASTREAM_FULL_TRANSACTIONS
        connection_.setTransactionOperations(CqlTransactionExchange::BEGIN_TXN,
                                             CqlTransactionExchange::COMMIT_TXN,
                                             CqlTransactionExchange::ROLLBACK_TXN);
#endif  // TERASTREAM_FULL_TRANSACTIONS
        connection_.prepareStatements(CqlConfigExchange<shard_config_t>::tagged_statements());
#ifdef TERASTREAM_FULL_TRANSACTIONS
        connection_.prepareStatements(CqlTransactionExchange::tagged_statements());
#endif  // TERASTREAM_FULL_TRANSACTIONS
    }

    void del(std::string const& config_timestamp_path) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_SHARD_CONFIGURATION_DELETE)
            .arg(CqlConfigExchange<shard_config_t>::universalID());

        CqlConfigExchange<shard_config_t> exchange;
        exchange.deleteCommon(connection_, config_timestamp_path,
                              CqlConfigExchange<shard_config_t>::DELETE_SHARD_CONFIGURATION());
    }

    /// @brief Retrieves DHCP timestamp.
    ShardConfigPtr selectTimestamp(std::string const& config_timestamp_path) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_SHARD_CONFIG_TIMESTAMP);

        // Query.
        isc::db::CqlTransaction transaction(connection_);
        CqlConfigExchange<shard_config_t> exchange;
        isc::db::Collection<shard_config_t> collection(
            exchange.getCommon(connection_, config_timestamp_path,
                               CqlConfigExchange<shard_config_t>::GET_CONFIGURATION_TIMESTAMP()));
        transaction.commit();

        // Get the singular result.
        std::shared_ptr<shard_config_t> result;
        if (!collection.empty()) {
            result = collection.front();
        }

        return result;
    }

    /// @brief Retrieves DHCP configuration in JSON format.
    ShardConfigPtr select(std::string const& config_timestamp_path) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_SHARD_CONFIG_JSON);

        isc::db::CqlTransaction transaction(connection_);
        CqlConfigExchange<shard_config_t> exchange;
        isc::db::Collection<shard_config_t> collection(
            exchange.getCommon(connection_, config_timestamp_path,
                               CqlConfigExchange<shard_config_t>::GET_JSON_CONFIGURATION()));
        transaction.commit();

        std::shared_ptr<shard_config_t> result;
        if (!collection.empty()) {
            result = collection.front();
        }

        return result;
    }

    /// @brief Retrieves database type
    std::string getType() const override final {
        return "cql";
    }

    /// @brief Inserts a new DHCPv6 server configuration.
    ///
    ///
    /// @param configuration The server configuration to be written in json format
    /// @param config_timestamp_path path to the config.timestamp file
    ///
    /// @throw isc::db::DbOperationError Connection is ok, so it must be a
    ///     CQL error.
    ///
    /// @return true if the configuration was added, false if not.
    void insert(ShardConfigPtr const& shard_config,
                std::string const& config_timestamp_path) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_INSERT_SHARD_CONFIG)
            .arg(CqlConfigExchange<shard_config_t>::universalID());

        CqlConfigExchange<shard_config_t> exchange;
        exchange.insertCommon(connection_, shard_config, config_timestamp_path,
                              CqlConfigExchange<shard_config_t>::INSERT_CONFIGURATION());
    }

    /// @brief Updates an existing DHCPv6 server configuration.
    ///
    ///
    /// @param ID the new configuration's id.
    /// @param configuration The server configuration to be written in json format
    /// @param config_timestamp_path path to the config.timestamp file
    ///
    /// @throw isc::db::DbOperationError Connection is ok, so it must be a
    ///     CQL error.
    ///
    /// @return true if the configuration was added/updated, false if not.
    void update(ShardConfigPtr const& shard_config,
                std::string const& config_timestamp_path) override final {
#ifdef GRANULAR_RETRIES
        updateConfigWithRetry(shard_config, config_timestamp_path);
#else  // GRANULAR_RETRIES
        updateConfigWithoutRetry(shard_config, config_timestamp_path);
#endif  // GRANULAR_RETRIES
    }

    void upsert(ShardConfigPtr const& shard_config,
                std::string const& config_timestamp_path) override final {
        try {
            update(shard_config, config_timestamp_path);
        } catch (isc::db::StatementNotApplied const& exception) {
            insert(shard_config, config_timestamp_path);
        }
    }

    /// @brief Start Transaction
    ///
    /// Start transaction for database operations. On databases that don't
    /// support transactions, this is a no-op.
    void startTransaction() override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_BEGIN_TRANSACTION);
        connection_.startTransaction();
    }

    /// @brief Commit Transactions
    ///
    /// This is a no-op for Cassandra.
    void commit() override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_COMMIT);
        connection_.commit();
    }

    /// @brief Rollback Transactions
    ///
    /// This is a no-op for Cassandra.
    void rollback() override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_ROLLBACK);
        connection_.rollback();
    }

private:
    void updateConfigWithRetry(ShardConfigPtr shard_config,
                               std::string const& config_timestamp_path) {
        isc::db::Granular::retry(
            [&] { select(config_timestamp_path); },
            [&] { updateConfigWithoutRetry(shard_config, config_timestamp_path); });
    }

    void updateConfigWithoutRetry(ShardConfigPtr shard_config,
                                  std::string const& config_timestamp_path) {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_UPDATE_SHARD_CONFIG)
            .arg(CqlConfigExchange<shard_config_t>::universalID());

        CqlConfigExchange<shard_config_t> exchange;
        exchange.updateCommon(connection_, shard_config, config_timestamp_path,
                              CqlConfigExchange<shard_config_t>::UPDATE_CONFIGURATION());
    }

    void updateTimestamp(std::string const& config_timestamp_path) override final {
        CqlConfigExchange<shard_config_t> exchange;
        exchange.updateTimestamp(connection_, config_timestamp_path);
    }

    /// @brief Database connection
    isc::db::CqlConnection connection_;
};

}  // namespace dhcp
}  // namespace isc

#endif  // CQL_SHARD_CONFIGURATION_MGR_H

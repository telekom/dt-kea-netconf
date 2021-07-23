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

#ifndef CQL_CONFIGURATION_MASTER_MGR_H
#define CQL_CONFIGURATION_MASTER_MGR_H

#include <cql/cql_connection.h>
#include <cql/cql_exchange.h>
#include <cql/cql_transaction.h>
#include <database/granular_retry.h>
#include <database/timestamp_store.h>
#include <dhcpsrv/dhcpsrv_log.h>
#include <dhcpsrv/master_config_mgr.h>
#include <util/dhcp.h>
#include <util/hash.h>
#include <util/types.h>

#include <sys/types.h>  // for int32_t

#include <list>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace isc {
namespace dhcp {

struct MasterConfigVersionTuple : isc::db::VersionTuple {};

/// @brief Exchange used to interact with the master server configuration table
/// from the database
template <DhcpSpaceType D>
struct CqlMasterConfigExchange : isc::db::CqlExchange<MasterConfigT<D>> {
    /// @brief Constructor
    ///
    /// Specifies table columns.
    CqlMasterConfigExchange() : master_config_info_(std::make_shared<MasterConfigT<D>>()) {
    }

    /// @brief Destructor
    virtual ~CqlMasterConfigExchange() = default;

    /// @brief Create BIND array to receive master configuration data.
    ///
    /// Method used in executeSelect() to retrieve std::shared_ptr<MasterConfigT<D>> object
    /// from database
    ///
    /// @param data array of bound objects representing data to be retrieved.
    /// @param statement_tag prepared statement being executed; defaults to an
    /// invalid index
    void createBindForSelect(
        isc::db::AnyArray &data,
        isc::db::StatementTag const &statement_tag = isc::db::StatementTag()) override final {
        // Start with a fresh array.
        data.clear();

        if (statement_tag == GET_CONFIGURATION_TIMESTAMP()) {
            data.add(&master_config_info_->instance_ID_);
            data.add(&master_config_info_->timestamp_);
        } else if (statement_tag == GET_CONFIGURATION_SHARDS()) {
            data.add(&master_config_info_->shard_);
        } else if (statement_tag == GET_MASTER_INSTANCE_IDS()) {
            data.add(&master_config_info_->instance_ID_);
        } else {
            data.add(&master_config_info_->instance_ID_);
            data.add(master_config_info_->database_credentials_string_pointer());
            data.add(&master_config_info_->shard_);
            data.add(master_config_info_->server_specific_configuration_string_pointer());
            data.add(&master_config_info_->timestamp_);
        }
    }

    /// @brief Copy received data into @ref MasterConfig object
    ///
    /// This method copies information about the master server configuration
    /// into a newly created @ref MasterConfig object. This method is
    /// called in @ref executeSelect().
    ///
    /// @return pointer to a @ref std::shared_ptr<MasterConfigT<D>> object holding a
    /// pointer to the @ref MasterConfig object returned.
    isc::db::Ptr<MasterConfigT<D>> retrieve() override final {
        std::shared_ptr<MasterConfigT<D>> result(
            std::make_shared<MasterConfigT<D>>(*master_config_info_));
        result->sanitize();
        return result;
    }

    /// @brief Common method for inserting configuration information into the
    ///     database.
    ///
    /// @param connection connection used to communicate with the Cassandra
    /// database
    /// @param master_config configuration being added
    /// @param timestamp read from file, operation is valid only if it is equal to database-side
    ///     timestamp
    /// @param statement_tag prepared statement being executed
    void insertCommon(isc::db::CqlConnection &connection,
                      std::string const &config_timestamp_path,
                      MasterConfigPtr &master_config,
                      isc::db::StatementTag statement_tag) {
        cass_int64_t new_timestamp(isc::db::TimestampStore::now());

        isc::db::AnyArray assigned_values{
            &master_config->instance_ID_, master_config->database_credentials_string_pointer(),
            &master_config->shard_, master_config->server_specific_configuration_string_pointer(),
            &new_timestamp};

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection.getTransactionID());
        assigned_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        isc::db::CqlExchange<MasterConfigT<D>>::executeMutation(connection, assigned_values,
                                                                statement_tag);
        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        config_timestamp.write(table(), master_config->instance_ID_, new_timestamp);
    }

    void updateCommon(isc::db::CqlConnection &connection,
                      std::string const &config_timestamp_path,
                      MasterConfigPtr &master_config,
                      isc::db::StatementTag statement_tag) {
        cass_int64_t new_timestamp(isc::db::TimestampStore::now());

        isc::db::TimestampStore config_timestamp(config_timestamp_path);

        isc::db::AnyArray data{master_config->server_specific_configuration_string_pointer(),
                               master_config->database_credentials_string_pointer(),
                               &master_config->shard_, &new_timestamp,
                               &master_config->instance_ID_};

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection.getTransactionID());
        data.add(&txid);
#else  // TERASTREAM_FULL_TRANSACTIONS
        isc::db::timestamp_t old_timestamp(
            config_timestamp.read(table(), master_config->instance_ID_));
        data.add(&old_timestamp);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        isc::db::CqlExchange<MasterConfigT<D>>::executeMutation(connection, data, statement_tag);
        config_timestamp.write(table(), master_config->instance_ID_, new_timestamp);
    }

    isc::db::Collection<MasterConfigT<D>> getCommon(isc::db::CqlConnection &connection,
                                                    std::string const &config_timestamp_path,
                                                    isc::db::AnyArray &where_values,
                                                    isc::db::StatementTag statement_tag,
                                                    bool const &single = false) {
#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection.getTransactionID());
        where_values.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        auto collection(isc::db::CqlExchange<MasterConfigT<D>>::executeSelect(
            connection, where_values, statement_tag, single));

        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        for (auto const &record : collection) {
            config_timestamp.write(table(), record->instance_ID_, record->timestamp_);
        }

        return collection;
    }

    /// @brief The keyspace which this exchange operates on.
    ///
    /// Used in all statements this exchange operates on.
    static std::string const &table() {
        static std::string const _("server_configuration" + dhcpSpaceToString<D>());
        return _;
    }

    /// @brief The initialization function which this exchange.
    ///
    /// Used to initialize the static members of this exchange.
    static isc::db::StatementMap &tagged_statements() {
        static isc::db::StatementMap _;

        static std::once_flag flag;
        std::call_once(flag, [&] {
            std::string text;

            text =
                "SELECT instance_ID, database_credentials, shard, server_specific_configuration, "
                "timestamp FROM " +
                table() + " WHERE instance_ID = ?";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text = "@free = " + text + " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(GET_CONFIGURATION_BY_ID(),
                          isc::db::CqlTaggedStatement(GET_CONFIGURATION_BY_ID(), text));

            text =
                "SELECT instance_ID, database_credentials, shard, server_specific_configuration, "
                "timestamp FROM " +
                table() + " WHERE shard = ?";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text = "@free = " + text + " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(GET_CONFIGURATION_BY_SHARD(),
                          isc::db::CqlTaggedStatement(GET_CONFIGURATION_BY_SHARD(), text));

            text = "SELECT shard FROM " + table();
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text = "@free = " + text + " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(GET_CONFIGURATION_SHARDS(),
                          isc::db::CqlTaggedStatement(GET_CONFIGURATION_SHARDS(), text));

            text = "SELECT instance_ID, timestamp FROM " + table() + " WHERE instance_ID = ?";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text = "@free = " + text + " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(GET_CONFIGURATION_TIMESTAMP(),
                          isc::db::CqlTaggedStatement(GET_CONFIGURATION_TIMESTAMP(), text));
            text = "SELECT instance_ID FROM " + table();
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text = "@free = " + text + " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(GET_MASTER_INSTANCE_IDS(),
                          isc::db::CqlTaggedStatement(GET_MASTER_INSTANCE_IDS(), text));

            text = "DELETE FROM " + table() + " WHERE instance_ID = ?";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text += " IN TXN ?";
#else
            text += " IF timestamp = ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(DELETE_CONFIGURATION(),
                          isc::db::CqlTaggedStatement(DELETE_CONFIGURATION(), text));

            text = "INSERT INTO " + table() +
                   " (instance_ID, database_credentials, shard, server_specific_configuration, "
                   "timestamp) "
                   "VALUES (?, ?, ?, ?, ?)";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text += " IN TXN ?";
#else  // TERASTREAM_FULL_TRANSACTIONS
            text += " IF NOT EXISTS";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(INSERT_CONFIGURATION(),
                          isc::db::CqlTaggedStatement(INSERT_CONFIGURATION(), text));

            text = "UPDATE " + table() +
                   " SET server_specific_configuration = ?, database_credentials = ?, shard = ?, "
                   "timestamp = ? WHERE instance_ID = ?";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text += " IN TXN ? ";
#else  // TERASTREAM_FULL_TRANSACTIONS
            text += " IF timestamp = ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(UPDATE_CONFIGURATION(),
                          isc::db::CqlTaggedStatement(UPDATE_CONFIGURATION(), text));
        });

        return _;
    }

    /// @brief Statement tags
    /// @{
    // Retrieve configuration filtered by ID.
    static isc::db::StatementTag &GET_CONFIGURATION_BY_ID() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }

    // Retrieve configuration filtered by shard name.
    static isc::db::StatementTag &GET_CONFIGURATION_BY_SHARD() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }

    // Retrieve shard names.
    static isc::db::StatementTag &GET_CONFIGURATION_SHARDS() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }

    // Retrieve configuration timestamp.
    static isc::db::StatementTag &GET_CONFIGURATION_TIMESTAMP() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }

    // Retrieve configurations.
    static isc::db::StatementTag &GET_MASTER_INSTANCE_IDS() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }

    // Delete configuration.
    static isc::db::StatementTag &DELETE_CONFIGURATION() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }

    // Insert configuration.
    static isc::db::StatementTag &INSERT_CONFIGURATION() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }

    // Update configuration.
    static isc::db::StatementTag &UPDATE_CONFIGURATION() {
        static isc::db::StatementTag _(PRETTY_FUNCTION_NAME());
        return _;
    }
    /// @}

private:
    /// @brief Object that this exchange reads from when writing to the database
    /// and writes to when reading from the database.
    std::shared_ptr<MasterConfigT<D>> master_config_info_;
};  // CqlMasterConfigExchange

/// @brief Exchange used to interact with the master server configuration table
/// from the database
template <DhcpSpaceType D>
struct CqlMasterConfigMgr : MasterConfigMgr {
    /// @brief Constructor
    ///
    /// @param parameters database connection parameters
    explicit CqlMasterConfigMgr(isc::db::DatabaseConnection::ParameterMap const &parameters)
        : connection_(parameters) {
        isc::db::CqlVersionExchange<MasterConfigVersionTuple> version_exchange;
        version_exchange.validateSchema(connection_);

#ifdef TERASTREAM_FULL_TRANSACTIONS
        connection_.setTransactionOperations(CqlTransactionExchange::BEGIN_TXN(),
                                             CqlTransactionExchange::COMMIT_TXN(),
                                             CqlTransactionExchange::ROLLBACK_TXN());
#endif  // TERASTREAM_FULL_TRANSACTIONS
        connection_.prepareStatements(CqlMasterConfigExchange<D>::tagged_statements());
#ifdef TERASTREAM_FULL_TRANSACTIONS
        connection_.prepareStatements(CqlTransactionExchange::tagged_statements());
#endif  // TERASTREAM_FULL_TRANSACTIONS
    }

    void insert(MasterConfigPtr &master_config,
                std::string const &config_timestamp_path) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_INSERT_MASTER_CONFIG)
            .arg(master_config->instance_ID_)
            .arg(master_config->shard_);

        isc::db::CqlTransaction transaction(connection_);
        CqlMasterConfigExchange<D> exchange;
        exchange.insertCommon(connection_, config_timestamp_path, master_config,
                              CqlMasterConfigExchange<D>::INSERT_CONFIGURATION());
        transaction.commit();
    }

    void deleteAll(std::string const &config_timestamp_path) override final {
        // Get master instance IDs.
        isc::db::CqlTransaction transaction(connection_);
        std::list<std::string> master_IDs(selectInstanceIDs(config_timestamp_path));

        // Iterate through all of them and delete each one.
        for (std::string &master_ID : master_IDs) {
            del(master_ID, config_timestamp_path);
        }
        transaction.commit();
    }

    void del(std::string &instance_ID, std::string const &config_timestamp_path) override final {
#ifdef GRANULAR_RETRIES
        delWithRetry(instance_ID, config_timestamp_path);
#else  // GRANULAR_RETRIES
        delWithoutRetry(instance_ID, config_timestamp_path);
#endif  // GRANULAR_RETRIES
    }

    void delWithRetry(std::string &instance_ID, std::string const &config_timestamp_path) {
        isc::db::Granular::retry([&] { select(instance_ID, config_timestamp_path); },
                                 [&] { delWithoutRetry(instance_ID, config_timestamp_path); });
    }

    void delWithoutRetry(std::string &instance_ID, std::string const &config_timestamp_path) {
        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        isc::db::timestamp_t old_timestamp(
            config_timestamp.read(CqlMasterConfigExchange<D>::table(), instance_ID));

        isc::db::AnyArray data{&instance_ID, &old_timestamp};

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection_.getTransactionID());
        data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        isc::db::CqlTransaction transaction(connection_);
        CqlMasterConfigExchange<D> exchange;
        exchange.executeMutation(connection_, data,
                                 CqlMasterConfigExchange<D>::DELETE_CONFIGURATION());
        transaction.commit();

        config_timestamp.del(CqlMasterConfigExchange<D>::table(), instance_ID);
    }

    MasterConfigPtr select(std::string &instance_ID,
                           std::string const &config_timestamp_path) override final {
        isc::db::CqlTransaction transaction(connection_);
        isc::db::AnyArray where_values{&instance_ID};
        CqlMasterConfigExchange<D> exchange;
        isc::db::Collection<MasterConfigT<D>> collection(
            exchange.getCommon(connection_, config_timestamp_path, where_values,
                               CqlMasterConfigExchange<D>::GET_CONFIGURATION_BY_ID(), true));
        transaction.commit();

        if (collection.empty()) {
            return nullptr;
        }

        return collection.front();
    }

    MasterConfigCollection selectByShard(std::string const &shard,
                                         std::string const &config_timestamp_path) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_MASTER_CONFIG_SHARD_DB)
            .arg(shard);

        MasterConfigCollection master_configurations;

        // Prepare where values.
        std::string database_name(shard);
        isc::db::AnyArray where_values{&database_name};

        // Query.
        isc::db::CqlTransaction transaction(connection_);
        CqlMasterConfigExchange<D> exchange;
        isc::db::Collection<MasterConfigT<D>> collection_data(
            exchange.getCommon(connection_, config_timestamp_path, where_values,
                               CqlMasterConfigExchange<D>::GET_CONFIGURATION_BY_SHARD()));
        transaction.commit();

        // Copy results.
        copy(collection_data.begin(), collection_data.end(), back_inserter(master_configurations));

        return master_configurations;
    }

    MasterConfigPtr selectTimestamp(std::string &instance_ID,
                                    std::string const &config_timestamp_path) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL, DHCPSRV_CQL_GET_MASTER_CONFIG_TIMESTAMP)
            .arg(instance_ID);

        // Prepare where values.
        isc::db::AnyArray where_values{&instance_ID};

        // Query.
        isc::db::CqlTransaction transaction(connection_);
        CqlMasterConfigExchange<D> exchange;
        isc::db::Collection<MasterConfigT<D>> collection(
            exchange.getCommon(connection_, config_timestamp_path, where_values,
                               CqlMasterConfigExchange<D>::GET_CONFIGURATION_TIMESTAMP(), true));
        transaction.commit();

        return collection.front();
    }

    void selectShardNames(isc::util::shard_list_t &shards,
                          std::string const &config_timestamp_path) override final {
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL,
                  DHCPSRV_CQL_GET_MASTER_CONFIG_SHARDS_NAME);

        // Prepare where values.
        isc::db::AnyArray where_values;

        // Clear old results.
        shards.clear();

        // Query.
        isc::db::CqlTransaction transaction(connection_);
        CqlMasterConfigExchange<D> exchange;
        isc::db::Collection<MasterConfigT<D>> collection_data(
            exchange.getCommon(connection_, config_timestamp_path, where_values,
                               CqlMasterConfigExchange<D>::GET_CONFIGURATION_SHARDS()));
        transaction.commit();

        // Copy results.
        for (std::shared_ptr<MasterConfigT<D>> const &srvInfo : collection_data) {
            shards.push_back(srvInfo->shard_);
        }
    }

    std::list<std::string>
    selectInstanceIDs(std::string const &config_timestamp_path) override final {
        // Prepare where values.
        isc::db::AnyArray where_values;

        // Query.
        isc::db::CqlTransaction transaction(connection_);
        CqlMasterConfigExchange<D> exchange;
        auto collection(exchange.getCommon(connection_, config_timestamp_path, where_values,
                                           CqlMasterConfigExchange<D>::GET_MASTER_INSTANCE_IDS()));
        transaction.commit();

        // Copy results.
        std::list<std::string> result;
        for (auto const &item : collection) {
            result.push_back(item->instance_ID_);
        }

        return result;
    }

    std::list<std::string> getAllMasters(std::string const &config_timestamp_path) {
        // Prepare where values.
        isc::db::AnyArray where_values;

        // Query.
        isc::db::CqlTransaction transaction(connection_);
        CqlMasterConfigExchange<D> exchange;
        auto collection(exchange.getCommon(connection_, config_timestamp_path, where_values,
                                           CqlMasterConfigExchange<D>::GET_MASTER_INSTANCE_IDS));
        transaction.commit();

        // Copy results.
        std::list<std::string> result;
        for (auto const &item : collection) {
            result.push_back(item->instance_ID_);
        }

        return result;
    }

    void update(MasterConfigPtr &master_config,
                std::string const &config_timestamp_path) override final {
#ifdef GRANULAR_RETRIES
        updateWithRetry(master_config, config_timestamp_path);
#else  // GRANULAR_RETRIES
        updateWithoutRetry(master_config, config_timestamp_path);
#endif  // GRANULAR_RETRIES
    }

    void updateWithRetry(MasterConfigPtr &master_config, std::string const &config_timestamp_path) {
        isc::db::Granular::retry(
            [&] { select(master_config->instance_ID_, config_timestamp_path); },
            [&] { updateWithoutRetry(master_config, config_timestamp_path); });
    }

    void updateWithoutRetry(MasterConfigPtr &master_config,
                            std::string const &config_timestamp_path) {
        isc::db::CqlTransaction transaction(connection_);
        CqlMasterConfigExchange<D> exchange;
        exchange.updateCommon(connection_, config_timestamp_path, master_config,
                              CqlMasterConfigExchange<D>::UPDATE_CONFIGURATION());
        transaction.commit();
    }

    /// @brief Returns the type of backend used, in this case "cql". Useful
    ///     when called from the base class.
    std::string getType() const override final {
        return "cql";
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
    /// @brief non-copyable
    /// @{
    CqlMasterConfigMgr(CqlMasterConfigMgr const &) = delete;

    CqlMasterConfigMgr &operator=(CqlMasterConfigMgr const &) = delete;
    /// @}

    /// Database connection
    isc::db::CqlConnection connection_;
};  // CqlMasterConfigMgr

}  // namespace dhcp
}  // namespace isc

#endif  // CQL_CONFIGURATION_MASTER_MGR_H

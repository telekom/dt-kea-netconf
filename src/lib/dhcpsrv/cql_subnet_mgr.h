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

#ifndef CQL_SUBNET_MGR_H
#define CQL_SUBNET_MGR_H

#include <cql/cql_connection.h>
#include <cql/cql_exchange.h>

#ifdef TERASTREAM_FULL_TRANSACTIONS
#include <cql/cql_transaction.h>
#endif  // TERASTREAM_FULL_TRANSACTIONS

#include <dhcpsrv/cql_shard_config_mgr.h>
#include <dhcpsrv/subnet_mgr.h>
#include <util/dhcp.h>

#include <mutex>
#include <stack>

namespace isc {
namespace dhcp {

template <DhcpSpaceType D>
struct CqlSubnetExchange : isc::db::CqlExchange<SubnetInfo<D>> {
    explicit CqlSubnetExchange() = default;

    static std::string const &table() {
        static std::string const _("subnet" + dhcpSpaceToString<D>());
        return _;
    }

    static isc::db::StatementMap &tagged_statements() {
        static isc::db::StatementMap _;

        static std::once_flag flag;
        std::call_once(flag, [&] {
            isc::db::StatementTag tag;
            isc::db::StatementText text;
            // Column-oriented statements
            // @{
            std::string columns;
            std::string values;
            std::string assignments;
            bool first(true);
            for (std::string const &column : SubnetInfo<D>::columns()) {
                if (column == "subnet") {
                    continue;
                }

                // Form columns, values and assignments used in universal statements.
                if (first) {
                    first = false;
                } else {
                    columns += ", ";
                    values += ", ";
                    assignments += ", ";
                }
                columns += column;
                values += "?";
                assignments += column + " = ?";

                tag = "INSERT_SUBNET" + dhcpSpaceToString<D>() + "_" + column;
                text = "INSERT INTO " + table() + " (subnet, " + column + ") VALUES (?, ?)";
#ifdef TERASTREAM_FULL_TRANSACTIONS
                text += " IN TXN ?";
#else  // TERASTREAM_FULL_TRANSACTIONS
            text += " IF NOT EXISTS";
#endif  // TERASTREAM_FULL_TRANSACTIONS
                _.try_emplace(tag, isc::db::CqlTaggedStatement(tag, text));

                tag = "SELECT_SUBNET" + dhcpSpaceToString<D>() + "_" + column;
                text = "SELECT subnet, " + column + " FROM " + table() + " WHERE subnet = ?";
#ifdef TERASTREAM_FULL_TRANSACTIONS
                text = "@free = " + text + " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
                _.try_emplace(tag, isc::db::CqlTaggedStatement(tag, text));

                tag = "SELECT_SUBNET" + dhcpSpaceToString<D>() + "_" + column + "_UNFILTERED";
                text = "SELECT subnet, " + column + " FROM " + table();
#ifdef TERASTREAM_FULL_TRANSACTIONS
                text = "@free = " + text + " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
                _.try_emplace(tag, isc::db::CqlTaggedStatement(tag, text));

                tag = "UPDATE_SUBNET" + dhcpSpaceToString<D>() + "_" + column;
                text = "UPDATE " + table() + " SET " + column +
                       " = ? WHERE subnet = ? IF timestamp = ?";
#ifdef TERASTREAM_FULL_TRANSACTIONS
                text += " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
                _.try_emplace(tag, isc::db::CqlTaggedStatement(tag, text));
            }
            // @}

            std::string network_topology_columns;
            std::string network_topology_values;
            for (auto const &[column, value] : SubnetInfo<D>::networkTopologyColumnsAndValues()) {
                network_topology_columns += ", " + column;
                network_topology_values += ", " + value;
            }

            // Universal statements
            // @{
            tag = "DELETE_SUBNET" + dhcpSpaceToString<D>() + "_ALL";
            text = "DELETE FROM " + table() + " WHERE subnet = ?";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text += " IN TXN ?";
#else  // TERASTREAM_FULL_TRANSACTIONS
        text += " IF timestamp = ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(tag, isc::db::CqlTaggedStatement(tag, text));

            tag = "INSERT_SUBNET" + dhcpSpaceToString<D>() + "_ALL";
            text = "INSERT INTO " + table() + " (subnet, " + columns + network_topology_columns +
                   ") VALUES (?, " + values + network_topology_values + ")";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text += " IN TXN ?";
#else  // TERASTREAM_FULL_TRANSACTIONS
            text += " IF NOT EXISTS";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(tag, isc::db::CqlTaggedStatement(tag, text));

            tag = "SELECT_SUBNET" + dhcpSpaceToString<D>() + "_ALL";
            text = "SELECT subnet, " + columns + " FROM " + table() + " WHERE subnet = ?";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text = "@free = " + text + " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(tag, isc::db::CqlTaggedStatement(tag, text));

            tag = "SELECT_SUBNETID" + dhcpSpaceToString<D>() + "_ALL";
            text = "SELECT subnet, " + columns + " FROM " + table() + " WHERE id = ? ";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text = "@free = " + text + " IN TXN ? ";
#endif  // TERASTREAM_FULL_TRANSACTIONS
#ifdef CASSANDRA_DENORMALIZED_TABLES
#else  // CASSANDRA_DENORMALIZED_TABLES
            text += "ALLOW FILTERING";
#endif  // CASSANDRA_DENORMALIZED_TABLES
            _.try_emplace(tag, isc::db::CqlTaggedStatement(tag, text));

            tag = "SELECT_SUBNET" + dhcpSpaceToString<D>() + "_ALL_UNFILTERED";
            text = "SELECT subnet, " + columns + " FROM " + table();
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text = "@free = " + text + " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(tag, isc::db::CqlTaggedStatement(tag, text));

            tag = "UPDATE_SUBNET" + dhcpSpaceToString<D>() + "_ALL";
            text =
                "UPDATE " + table() + " SET " + assignments + " WHERE subnet = ? IF timestamp = ?";
#ifdef TERASTREAM_FULL_TRANSACTIONS
            text += " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS
            _.try_emplace(tag, isc::db::CqlTaggedStatement(tag, text));
            // @}
        });

        return _;
    }

    inline void bindCommon(isc::db::AnyArray &data, SubnetInfo<D> &subnet) {
        data.add(&subnet.id_);
        data.add(&subnet.timestamp_);
        data.add(&subnet.renew_timer_);
        data.add(&subnet.rebind_timer_);
        data.add(&subnet.valid_lifetime_);
        data.add(&subnet.allocation_type_);
        data.add(&subnet.strings_.client_class_);
        data.add(&subnet.strings_.interface_);
        data.add(&subnet.strings_.interface_id_);
        data.add(&subnet.strings_.option_data_list_);
        data.add(&subnet.strings_.pools_list_);
        data.add(&subnet.rapid_commit_);
        data.add(&subnet.strings_.relay_);
        data.add(&subnet.strings_.require_client_classes_);
        data.add(&subnet.strings_.reservations_);
        data.add(&subnet.reservation_mode_);
        data.add(&subnet.strings_.user_context_);
    }

    void bindDelta(isc::db::AnyArray &data, SubnetInfo<D> &subnet);

private:
    std::vector<std::tuple<std::string, std::string>> networkTopologyColumnsAndValues();

    inline void
    createBindForSelect(isc::db::AnyArray &data,
                        isc::db::StatementTag const & /* statement_tag = StatementTag() */)

        override final {
        data.clear();
        data.add(&subnet_.strings_.subnet_);
        bindCommon(data, subnet_);
        bindDelta(data, subnet_);
    }

    SubnetInfoPtr<D> retrieve()

        override final {
        return std::make_shared<SubnetInfo<D>>(subnet_);
    }

    SubnetInfo<D> subnet_;
};  // CqlSubnetExchange
using CqlSubnet4Exchange = CqlSubnetExchange<DHCP_SPACE_V4>;
using CqlSubnet6Exchange = CqlSubnetExchange<DHCP_SPACE_V6>;

template <DhcpSpaceType D>
struct CqlMaxSubnetIdExchange : isc::db::CqlExchange<SubnetID> {
    static isc::db::StatementTag &SELECT_MAX_SUBNET_ID() {
        static isc::db::StatementTag _("SELECT_MAX_SUBNET_ID_" + dhcpSpaceToString<D>());
        return _;
    }

    static isc::db::StatementMap &tagged_statements() {
        static isc::db::StatementMap _;

        std::string const &text("SELECT MAX(id) FROM subnet" + dhcpSpaceToString<D>());
#ifdef TERASTREAM_FULL_TRANSACTIONS
        text = "@free = " + text + " IN TXN ?";
#endif  // TERASTREAM_FULL_TRANSACTIONS

        _.try_emplace(SELECT_MAX_SUBNET_ID(),
                      isc::db::CqlTaggedStatement(SELECT_MAX_SUBNET_ID(), text));

        return _;
    }

private:
    inline void createBindForSelect(
        isc::db::AnyArray &data,
        isc::db::StatementTag const & /* statement_tag = StatementTag() */) override final {
        data.clear();
        data.add(&max_);
    }

    isc::db::Ptr<uint32_t> retrieve() override final {
        return std::make_shared<uint32_t>(max_);
    }

    uint32_t max_;
};

template <DhcpSpaceType D>
struct CqlSubnetMgr : SubnetMgr<D> {
    explicit CqlSubnetMgr(isc::db::DatabaseConnection::ParameterMap const &parameters)
        : connection_(parameters) {
        isc::db::CqlVersionExchange<ShardConfigVersionTuple> version_exchange;
        version_exchange.validateSchema(connection_);

#ifdef TERASTREAM_FULL_TRANSACTIONS
        connection_.setTransactionOperations(CqlTransactionExchange::BEGIN_TXN,
                                             CqlTransactionExchange::COMMIT_TXN,
                                             CqlTransactionExchange::ROLLBACK_TXN);
#endif  // TERASTREAM_FULL_TRANSACTIONS
        connection_.prepareStatements(CqlSubnetExchange<D>::tagged_statements());
        connection_.prepareStatements(CqlMaxSubnetIdExchange<D>::tagged_statements());
#ifdef TERASTREAM_FULL_TRANSACTIONS
        connection_.prepareStatements(CqlTransactionExchange::tagged_statements());
#endif  // TERASTREAM_FULL_TRANSACTIONS
    }

    explicit CqlSubnetMgr(isc::db::CqlConnection const &connection)
        : connection_(connection.parameters()) {
        isc::db::CqlVersionExchange<ShardConfigVersionTuple> version_exchange;
        version_exchange.validateSchema(connection_);

#ifdef TERASTREAM_FULL_TRANSACTIONS
        connection_.setTransactionOperations(CqlTransactionExchange::BEGIN_TXN,
                                             CqlTransactionExchange::COMMIT_TXN,
                                             CqlTransactionExchange::ROLLBACK_TXN);
#endif  // TERASTREAM_FULL_TRANSACTIONS
        connection_.prepareStatements(CqlSubnetExchange<D>::tagged_statements());
        connection_.prepareStatements(CqlMaxSubnetIdExchange<D>::tagged_statements());
#ifdef TERASTREAM_FULL_TRANSACTIONS
        connection_.prepareStatements(CqlTransactionExchange::tagged_statements());
#endif  // TERASTREAM_FULL_TRANSACTIONS
    }

    virtual ~CqlSubnetMgr() = default;

    std::string getName() const override final {
        std::string name;
        try {
            name = connection_.getParameter("name");
        } catch (...) {
            // Return an empty name
        }
        return name;
    }

    std::string getType() const override final {
        return "cql";
    }

    void del(SubnetInfoPtr<D> &subnet, std::string const &config_timestamp_path) override final {
#ifdef GRANULAR_RETRIES
        delWithRetry(subnet, config_timestamp_path);
#else  // GRANULAR_RETRIES
        delWithoutRetry(subnet, config_timestamp_path);
#endif  // GRANULAR_RETRIES
    }

    void delWithRetry(SubnetInfoPtr<D> &subnet, std::string const &config_timestamp_path) {
        isc::db::Granular::retry([&] { selectBySubnet(subnet, config_timestamp_path); },
                                 [&] { delWithoutRetry(subnet, config_timestamp_path); });
    }

    void delWithoutRetry(SubnetInfoPtr<D> &subnet, std::string const &config_timestamp_path) {
        CqlSubnetExchange<D> exchange;
        std::string const &statement_tag("DELETE_SUBNET" + dhcpSpaceToString<D>() + "_ALL");

        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        isc::db::timestamp_t old_timestamp(
            config_timestamp.read(exchange.table(), subnet->strings_.subnet_));
        isc::db::AnyArray data{&subnet->strings_.subnet_, &old_timestamp};

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection_.getTransactionID());
        data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        exchange.executeMutation(connection_, data, statement_tag);
        config_timestamp.del(exchange.table(), subnet->strings_.subnet_);
    }

    void insert(SubnetInfoPtr<D> &subnet, std::string const &config_timestamp_path) override final {
        CqlSubnetExchange<D> exchange;
        std::string const &statement_tag("INSERT_SUBNET" + dhcpSpaceToString<D>() + "_ALL");
        subnet->timestamp_ = isc::db::TimestampStore::now();
        isc::db::AnyArray data{&subnet->strings_.subnet_};
        exchange.bindCommon(data, *subnet);
        exchange.bindDelta(data, *subnet);

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection_.getTransactionID());
        data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        exchange.executeMutation(connection_, data, statement_tag);
        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        config_timestamp.write(exchange.table(), subnet->strings_.subnet_, subnet->timestamp_);
    }

    SubnetInfoCollection<D>
    select(std::string const &config_timestamp_path = std::string()) override final {
        CqlSubnetExchange<D> exchange;
        std::string const &statement_tag("SELECT_SUBNET" + dhcpSpaceToString<D>() +
                                         "_ALL_UNFILTERED");
        isc::db::AnyArray data;

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection_.getTransactionID());
        data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        SubnetInfoCollection<D> collection(
            exchange.executeSelect(connection_, data, statement_tag));

        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        for (auto const &record : collection) {
            config_timestamp.write(exchange.table(), record->strings_.subnet_, record->timestamp_);
        }

        return collection;
    }

    /// @brief include overloaded @ref selectBySubnet
    using SubnetMgr<D>::selectBySubnet;

    SubnetInfoPtr<D>
    selectBySubnet(std::string &subnet,
                   std::string const &config_timestamp_path = std::string()) override final {
        CqlSubnetExchange<D> exchange;
        std::string const &statement_tag("SELECT_SUBNET" + dhcpSpaceToString<D>() + "_ALL");
        isc::db::AnyArray data{&subnet};

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection_.getTransactionID());
        data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        SubnetInfoCollection<D> collection(exchange.executeSelect(
            connection_, data, statement_tag, isc::db::CqlExchange<SubnetInfo<D>>::SINGLE));

        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        for (auto const &record : collection) {
            config_timestamp.write(exchange.table(), record->strings_.subnet_, record->timestamp_);
        }

        if (collection.empty()) {
            return nullptr;
        }

        return collection.front();
    }

    /// @brief include overloaded @ref selectBySubnet
    using SubnetMgr<D>::selectBySubnetID;

    SubnetInfoPtr<D> selectBySubnetID(SubnetID subnet_id) override final {
        CqlSubnetExchange<D> exchange;
        std::string const &statement_tag("SELECT_SUBNETID" + dhcpSpaceToString<D>() + "_ALL");
        isc::db::AnyArray data{&subnet_id};

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection_.getTransactionID());
        data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        SubnetInfoCollection<D> collection(exchange.executeSelect(
            connection_, data, statement_tag, isc::db::CqlExchange<SubnetInfo<D>>::SINGLE));

        if (collection.empty()) {
            return nullptr;
        }

        return collection.front();
    }

    void update(SubnetInfoPtr<D> &subnet, std::string const &config_timestamp_path) override final {
#ifdef GRANULAR_RETRIES
        updateWithRetry(subnet, config_timestamp_path);
#else  // GRANULAR_RETRIES
        updateWithoutRetry(subnet, config_timestamp_path);
#endif  // GRANULAR_RETRIES
    }

    void updateWithRetry(SubnetInfoPtr<D> &subnet, std::string const &config_timestamp_path) {
        isc::db::Granular::retry([&] { selectBySubnet(subnet, config_timestamp_path); },
                                 [&] { updateWithoutRetry(subnet, config_timestamp_path); });
    }

    void updateWithoutRetry(SubnetInfoPtr<D> &subnet, std::string const &config_timestamp_path) {
        CqlSubnetExchange<D> exchange;
        std::string const &statement_tag("UPDATE_SUBNET" + dhcpSpaceToString<D>() + "_ALL");

        isc::db::TimestampStore config_timestamp(config_timestamp_path);
        isc::db::timestamp_t old_timestamp(
            config_timestamp.read(exchange.table(), subnet->strings_.subnet_));

        isc::db::AnyArray data;
        subnet->timestamp_ = isc::db::TimestampStore::now();
        exchange.bindCommon(data, *subnet);
        exchange.bindDelta(data, *subnet);
        data.add(&subnet->strings_.subnet_);
        data.add(&old_timestamp);

#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection_.getTransactionID());
        data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        exchange.executeMutation(connection_, data, statement_tag);
        config_timestamp.write(exchange.table(), subnet->strings_.subnet_, subnet->timestamp_);
    }

    SubnetID maxSubnetID() override final {
        CqlMaxSubnetIdExchange<D> exchange;

        isc::db::AnyArray data;
#ifdef TERASTREAM_FULL_TRANSACTIONS
        CassUuid txid(connection_.getTransactionID());
        data.add(&txid);
#endif  // TERASTREAM_FULL_TRANSACTIONS

        SubnetID max_subnet_id;

        try {
            max_subnet_id = *exchange
                                 .executeSelect(connection_, data,
                                                CqlMaxSubnetIdExchange<D>::SELECT_MAX_SUBNET_ID())
                                 .front();
        } catch (isc::db::DbOperationError const &exception) {
            if (std::string(exception.what())
                    .find("NULL value specified when retrieving parameter") == std::string::npos) {
                throw;
            } else {
                // No subnet ID in the database so consider the max to be zero.
                max_subnet_id = 0;
            }
        }
        return max_subnet_id;
    }

    void commit() override final {
        connection_.commit();
    }

    void rollback() override final {
        connection_.rollback();
    }

private:
    /// @brief Database connection
    isc::db::CqlConnection connection_;
};

using CqlSubnet4Mgr = CqlSubnetMgr<DHCP_SPACE_V4>;
using CqlSubnet6Mgr = CqlSubnetMgr<DHCP_SPACE_V6>;

}  // namespace dhcp
}  // namespace isc

#endif  // CQL_SUBNET_MGR_H

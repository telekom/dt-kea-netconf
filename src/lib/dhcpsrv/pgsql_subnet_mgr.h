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

#ifndef PGSQL_SUBNET_MGR_H
#define PGSQL_SUBNET_MGR_H

#include <dhcpsrv/subnet_mgr.h>
#include <util/dhcp.h>

#include <database/db_exceptions.h>
#include <pgsql/pgsql_connection.h>
#include <pgsql/pgsql_exchange.h>

namespace isc {
namespace dhcp {

template <DhcpSpaceType D>
struct PgSqlSubnetMgr : public SubnetMgr<D> {
    explicit PgSqlSubnetMgr(db::DatabaseConnection::ParameterMap const& parameters)
        : connection_(parameters) {

        // Open the database.
        connection_.openDatabase();

        // Validate schema version first.
        db::VersionTuple const code_version(db::PG_CONFIG_SCHEMA_VERSION_MAJOR,
                                            db::PG_CONFIG_SCHEMA_VERSION_MINOR);
        db::VersionTuple const& db_version(getVersion());
        if (code_version != db_version) {
            isc_throw(db::DbOpenError, "PostgreSQL schema version mismatch: need version: "
                                           << std::get<0>(code_version) << "."
                                           << std::get<1>(code_version)
                                           << ", found version: " << std::get<0>(db_version) << "."
                                           << std::get<1>(db_version));
        }

        for (auto const& tagged_statement : tagged_statements()) {
            connection_.prepareStatement(tagged_statement.second);
        }
    }

    void del(SubnetInfoPtr<D>& subnet,
             std::string const& /* config_timestamp_path */) override final {
        std::string const& index("DELETE_SUBNET" + dhcpSpaceToString<D>() + "_ALL");
        db::PgSqlTaggedStatement& statement = tagged_statements().at(index);

        // Set up the WHERE clause values.
        db::PsqlBindArray bind_array;
        bind_array.add(subnet->strings_.subnet_);

        db::PgSqlResult r(PQexecPrepared(connection_, statement.name, statement.nbparams,
                                         &bind_array.values_[0], &bind_array.lengths_[0],
                                         &bind_array.formats_[0], 0));

        connection_.checkStatementError(r, statement);
        int affected_rows = boost::lexical_cast<int>(PQcmdTuples(r));

        // Check success case first as it is the most likely outcome.
        if (affected_rows == 1) {
            return;
        }

        // If no rows affected, subnet doesn't exist.
        if (affected_rows == 0) {
            isc_throw(db::DbOperationError, "PgSqlSubnet"
                                                << dhcpSpaceToString<D>()
                                                << "Mgr::del(): deleted none, expected one");
        }

        // Should not have happened, primary key constraint should only have filtered one row.
        isc_throw(db::DbOperationError, "PgSqlSubnet"
                                            << dhcpSpaceToString<D>()
                                            << "Mgr::del(): deleted multiple, expected one");
    }

    void insert(SubnetInfoPtr<D>& subnet,
                std::string const& /* config_timestamp_path */) override final {
        std::string const& index("INSERT_SUBNET" + dhcpSpaceToString<D>() + "_ALL");
        db::PgSqlTaggedStatement& statement = tagged_statements().at(index);

        // Set up the WHERE clause values.
        db::PsqlBindArray bind_array;
        bindCommon(bind_array, subnet);
        bindDelta(bind_array, subnet);
        bind_array.add(subnet->strings_.subnet_);

        db::PgSqlResult r(PQexecPrepared(connection_, statement.name, statement.nbparams,
                                         &bind_array.values_[0], &bind_array.lengths_[0],
                                         &bind_array.formats_[0], 0));

        int s = PQresultStatus(r);
        if (s != PGRES_COMMAND_OK) {
            // Failure: check for the special case of duplicate entry. If this is
            // the case, we return false to indicate that the row was not added.
            // Otherwise we throw an exception.
            if (connection_.compareError(r, db::PgSqlConnection::DUPLICATE_KEY)) {
                isc_throw(db::DuplicateEntry, "Duplicate entry");
            }

            connection_.checkStatementError(r, statement);
        }
    }

    SubnetInfoCollection<D> select(
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        std::string const& index("SELECT_SUBNET" + dhcpSpaceToString<D>() + "_ALL_UNFILTERED");
        db::PgSqlTaggedStatement& statement = tagged_statements().at(index);

        // Set up the WHERE clause values.
        db::PsqlBindArray bind_array;

        db::PgSqlResult r(
            PQexecPrepared(connection_, statement.name, statement.nbparams, NULL, NULL, NULL, 0));

        return selectCommon(statement, r);
    }

    SubnetInfoPtr<D> selectBySubnet(
        std::string& subnet,
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        std::string const& index("SELECT_SUBNET" + dhcpSpaceToString<D>() + "_ALL");
        db::PgSqlTaggedStatement& statement = tagged_statements().at(index);

        // Set up the WHERE clause values.
        db::PsqlBindArray bind_array;
        bind_array.add(subnet);

        db::PgSqlResult r(PQexecPrepared(connection_, statement.name, statement.nbparams,
                                         &bind_array.values_[0], &bind_array.lengths_[0],
                                         &bind_array.formats_[0], 0));

        return selectCommon(statement, r, true).at(0);
    }

    SubnetInfoPtr<D> selectBySubnetID(SubnetID) override final {
        isc_throw(NotImplemented, "not yet implemented.");
    };

    SubnetInfoCollection<D> selectCommon(db::PgSqlTaggedStatement& statement,
                                         db::PgSqlResult& result,
                                         bool single = false) {
        connection_.checkStatementError(result, statement);

        int rows = PQntuples(result);
        if (single && rows > 1) {
            isc_throw(db::MultipleRecords, "multiple records were found in the "
                                           "database where only one was expected for query "
                                               << statement.name);
        }

        SubnetInfoCollection<D> collection;
        for (int i = 0; i < rows; ++i) {
            collection.push_back(retrieve(result, i));
        }
        return collection;
    }

    void update(SubnetInfoPtr<D>& subnet,
                std::string const& /* config_timestamp_path */) override final {
        std::string const& index("UPDATE_SUBNET" + dhcpSpaceToString<D>() + "_ALL");
        db::PgSqlTaggedStatement& statement = tagged_statements().at(index);

        // Set up the WHERE clause values.
        db::PsqlBindArray bind_array;
        bindCommon(bind_array, subnet);
        bindDelta(bind_array, subnet);
        bind_array.add(subnet->strings_.subnet_);

        db::PgSqlResult r(PQexecPrepared(connection_, statement.name, statement.nbparams,
                                         &bind_array.values_[0], &bind_array.lengths_[0],
                                         &bind_array.formats_[0], 0));

        connection_.checkStatementError(r, statement);

        int affected_rows = boost::lexical_cast<int>(PQcmdTuples(r));

        // Check success case first as it is the most likely outcome.
        if (affected_rows == 1) {
            return;
        }

        // If no rows affected, subnet doesn't exist.
        if (affected_rows == 0) {
            isc_throw(db::DbOperationError, "PgSqlSubnet"
                                                << dhcpSpaceToString<D>()
                                                << "Mgr::update(): updated none, expected one");
        }

        // Should not have happened, primary key constraint should only have filtered one row.
        isc_throw(db::DbOperationError, "PgSqlSubnet"
                                            << dhcpSpaceToString<D>()
                                            << "Mgr::update(): updated multiple, expected one");
    }

    SubnetID maxSubnetID() override final {
        std::string const& index("SELECT_MAX_SUBNET_ID_" + dhcpSpaceToString<D>());
        db::PgSqlTaggedStatement& statement = tagged_statements().at(index);

        // Set up the WHERE clause values.
        db::PsqlBindArray bind_array;

        db::PgSqlResult result(PQexecPrepared(connection_, statement.name, statement.nbparams,
                                              &bind_array.values_[0], &bind_array.lengths_[0],
                                              &bind_array.formats_[0], 0));

        connection_.checkStatementError(result, statement);

        int rows = PQntuples(result);
        if (rows > 1) {
            isc_throw(db::MultipleRecords, "multiple records were found in the "
                                           "database where only one was expected for query "
                                               << statement.name);
        }

        SubnetID max_subnet_id;

        // MAX(id) returns empty string when no IDs are in the database,
        // boost::lexical_cast doesn't know how to convert it so check here.
        char const* data = db::PgSqlExchange::getRawColumnValue(result, 0, 0);
        if (data[0] == '\0') {
            return 0;
        }

        db::PgSqlExchange::getColumnValue(result, 0, 0, max_subnet_id);
        return max_subnet_id;
    }

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
        return "postgresql";
    }

    db::VersionTuple getVersion() const {
        char const* version_sql = "SELECT version, minor FROM config_schema_version;";
        db::PgSqlResult r(PQexec(connection_, version_sql));
        if (PQresultStatus(r) != PGRES_TUPLES_OK) {
            isc_throw(db::DbOperationError, "unable to execute PostgreSQL statement <"
                                                << version_sql
                                                << ", reason: " << PQerrorMessage(connection_));
        }

        uint32_t major;
        uint32_t minor;

        db::PgSqlExchange::getColumnValue(r, 0, 0, major);
        db::PgSqlExchange::getColumnValue(r, 0, 1, minor);

        return std::make_pair(major, minor);
    }

    using StatementMap = std::unordered_map<std::string, db::PgSqlTaggedStatement>;
    static StatementMap& tagged_statements() {
        static StatementMap tagged_statements;

        TypesMap const& t = PgSqlSubnetMgr<D>::getTypes();
        static std::vector<std::string> tags, texts;

        tags.push_back("GET_VERSION");
        texts.push_back("SELECT version, minor FROM config_schema_version");
        tagged_statements.insert(
            {tags.back(), db::PgSqlTaggedStatement{
                              0, {db::OID_NONE}, tags.back().c_str(), texts.back().c_str()}});

        // Column-oriented statements
        // @{
        std::string columns;
        std::string values;
        std::string assignments;
        bool first = true;
        size_t position = 1;
        std::vector<Oid> types;
        for (std::string const& column : SubnetInfo<D>::columns()) {
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
            values += "$" + boost::lexical_cast<std::string>(position);
            assignments += column + " = $" + boost::lexical_cast<std::string>(position);
            types.push_back(t.at(column));
            ++position;

            tags.push_back("INSERT_SUBNET" + dhcpSpaceToString<D>() + "_" + column);
            texts.push_back("INSERT INTO subnet" + dhcpSpaceToString<D>() + " (subnet, " + column +
                            ") VALUES ($1, $2)");
            tagged_statements.insert(
                {tags.back(), db::PgSqlTaggedStatement{2,
                                                       {t.at("subnet"), t.at(column)},
                                                       tags.back().c_str(),
                                                       texts.back().c_str()}});

            tags.push_back("SELECT_SUBNET" + dhcpSpaceToString<D>() + "_" + column);
            texts.push_back("SELECT subnet, " + column + " FROM subnet" + dhcpSpaceToString<D>() +
                            " WHERE subnet = $1");
            tagged_statements.insert(
                {tags.back(), db::PgSqlTaggedStatement{
                                  1, {t.at("subnet")}, tags.back().c_str(), texts.back().c_str()}});

            tags.push_back("SELECT_SUBNET" + dhcpSpaceToString<D>() + "_" + column + "_UNFILTERED");
            texts.push_back("SELECT subnet, " + column + " FROM subnet" + dhcpSpaceToString<D>());
            tagged_statements.insert(
                {tags.back(), db::PgSqlTaggedStatement{
                                  0, {db::OID_NONE}, tags.back().c_str(), texts.back().c_str()}});

            tags.push_back("UPDATE_SUBNET" + dhcpSpaceToString<D>() + "_" + column);
            texts.push_back("UPDATE subnet" + dhcpSpaceToString<D>() + " SET " + column +
                            " = $1 WHERE subnet = $2");
            tagged_statements.insert(
                {tags.back(), db::PgSqlTaggedStatement{2,
                                                       {t.at(column), t.at("subnet")},
                                                       tags.back().c_str(),
                                                       texts.back().c_str()}});
        }
        types.push_back(t.at("subnet"));
        std::string latest_position = "$" + boost::lexical_cast<std::string>(position);
        // @}

        // Universal statements
        // @{
        tags.push_back("DELETE_SUBNET" + dhcpSpaceToString<D>() + "_ALL");
        texts.push_back("DELETE FROM subnet" + dhcpSpaceToString<D>() + " WHERE subnet = $1");
        tagged_statements.insert(
            {tags.back(), db::PgSqlTaggedStatement{
                              1, {t.at("subnet")}, tags.back().c_str(), texts.back().c_str()}});

        tags.push_back("DELETE_SUBNETS" + dhcpSpaceToString<D>() + "_ALL");
        texts.push_back("TRUNCATE subnet" + dhcpSpaceToString<D>());
        tagged_statements.insert(
            {tags.back(), db::PgSqlTaggedStatement{
                              0, {db::OID_NONE}, tags.back().c_str(), texts.back().c_str()}});

        tags.push_back("INSERT_SUBNET" + dhcpSpaceToString<D>() + "_ALL");
        texts.push_back("INSERT INTO subnet" + dhcpSpaceToString<D>() + " (" + columns +
                        ", subnet) VALUES (" + values + ", " + latest_position + ")");
        db::PgSqlTaggedStatement insert_statement{static_cast<int>(types.size()), types,
                                                  tags.back().c_str(), texts.back().c_str()};
        tagged_statements.insert({tags.back(), insert_statement});

        tags.push_back("SELECT_SUBNET" + dhcpSpaceToString<D>() + "_ALL");
        texts.push_back("SELECT subnet, " + columns + " FROM subnet" + dhcpSpaceToString<D>() +
                        " WHERE subnet = $1");
        tagged_statements.insert(
            {tags.back(), db::PgSqlTaggedStatement{
                              1, {t.at("subnet")}, tags.back().c_str(), texts.back().c_str()}});

        tags.push_back("SELECT_SUBNET" + dhcpSpaceToString<D>() + "_ALL_UNFILTERED");
        texts.push_back("SELECT subnet, " + columns + " FROM subnet" + dhcpSpaceToString<D>());
        tagged_statements.insert(
            {tags.back(), db::PgSqlTaggedStatement{
                              0, {db::OID_NONE}, tags.back().c_str(), texts.back().c_str()}});

        tags.push_back("UPDATE_SUBNET" + dhcpSpaceToString<D>() + "_ALL");
        texts.push_back("UPDATE subnet" + dhcpSpaceToString<D>() + " SET " + assignments +
                        " WHERE subnet = " + latest_position);
        db::PgSqlTaggedStatement update_statement{static_cast<int>(types.size()), types,
                                                  tags.back().c_str(), texts.back().c_str()};
        tagged_statements.insert({tags.back(), update_statement});

        tags.push_back("SELECT_MAX_SUBNET_ID_" + dhcpSpaceToString<D>());
        texts.push_back("SELECT MAX(id) FROM subnet" + dhcpSpaceToString<D>());
        tagged_statements.insert(
            {tags.back(), db::PgSqlTaggedStatement{
                              0, {db::OID_NONE}, tags.back().c_str(), texts.back().c_str()}});
        // @}

        return tagged_statements;
    }

private:
    SubnetInfoPtr<D> retrieve(db::PgSqlResult const& result, int row);

    SubnetInfoPtr<D> retrieveCommon(db::PgSqlResult const& result, int row) {
        SubnetInfoPtr<D> subnet(std::make_shared<SubnetInfo<D>>());

        // Position of column in result
        int p(0);

        subnet->strings_.subnet_ = db::PgSqlExchange::getRawColumnValue(result, row, p++);
        db::PgSqlExchange::getColumnValue(result, row, p++, subnet->id_);
        db::PgSqlExchange::getColumnValue(result, row, p++, subnet->timestamp_);
        db::PgSqlExchange::getColumnValue(result, row, p++, subnet->renew_timer_);
        db::PgSqlExchange::getColumnValue(result, row, p++, subnet->rebind_timer_);
        db::PgSqlExchange::getColumnValue(result, row, p++, subnet->valid_lifetime_);
        db::PgSqlExchange::getColumnValue(result, row, p++, subnet->allocation_type_);
        subnet->strings_.client_class_ = db::PgSqlExchange::getRawColumnValue(result, row, p++);
        subnet->strings_.interface_ = db::PgSqlExchange::getRawColumnValue(result, row, p++);
        subnet->strings_.interface_id_ = db::PgSqlExchange::getRawColumnValue(result, row, p++);
        subnet->strings_.option_data_list_ = db::PgSqlExchange::getRawColumnValue(result, row, p++);
        subnet->strings_.pools_list_ = db::PgSqlExchange::getRawColumnValue(result, row, p++);
        db::PgSqlExchange::getColumnValue(result, row, p++, subnet->rapid_commit_);
        subnet->strings_.relay_ = db::PgSqlExchange::getRawColumnValue(result, row, p++);
        subnet->strings_.require_client_classes_ =
            db::PgSqlExchange::getRawColumnValue(result, row, p++);
        subnet->strings_.reservations_ = db::PgSqlExchange::getRawColumnValue(result, row, p++);
        db::PgSqlExchange::getColumnValue(result, row, p++, subnet->reservation_mode_);
        subnet->strings_.user_context_ = db::PgSqlExchange::getRawColumnValue(result, row, p++);

        return subnet;
    }

    void commit() override final {
        connection_.commit();
    }

    void rollback() override final {
        connection_.rollback();
    }

    inline void bindCommon(db::PsqlBindArray& bind, SubnetInfoPtr<D> const& subnet) {
        bind.add(subnet->id_);
        bind.add(subnet->timestamp_);
        bind.add(subnet->renew_timer_);
        bind.add(subnet->rebind_timer_);
        bind.add(subnet->valid_lifetime_);
        bind.add(subnet->allocation_type_);
        bind.add(subnet->strings_.client_class_);
        bind.add(subnet->strings_.interface_);
        bind.add(subnet->strings_.interface_id_);
        bind.add(subnet->strings_.option_data_list_);
        bind.add(subnet->strings_.pools_list_);
        bind.add(subnet->rapid_commit_);
        bind.add(subnet->strings_.relay_);
        bind.add(subnet->strings_.require_client_classes_);
        bind.add(subnet->strings_.reservations_);
        bind.add(subnet->reservation_mode_);
        bind.add(subnet->strings_.user_context_);
    }

    void bindDelta(db::PsqlBindArray& bind, SubnetInfoPtr<D> const& subnet);

    using TypesMap = std::unordered_map<std::string, Oid const>;
    static TypesMap const& getTypes() {
        static TypesMap types = {// common
                                 {"id", db::OID_INT4},
                                 {"timestamp", db::OID_INT8},
                                 {"renew_timer", db::OID_INT8},
                                 {"rebind_timer", db::OID_INT8},
                                 {"valid_lifetime", db::OID_INT8},
                                 {"allocation_type", db::OID_INT2},
                                 {"client_class", db::OID_VARCHAR},
                                 {"interface", db::OID_VARCHAR},
                                 {"interface_id", db::OID_VARCHAR},
                                 {"option_data_list", db::OID_VARCHAR},
                                 {"pools_list", db::OID_VARCHAR},
                                 {"rapid_commit", db::OID_INT2},
                                 {"relay", db::OID_VARCHAR},
                                 {"require_client_classes", db::OID_VARCHAR},
                                 {"reservations", db::OID_VARCHAR},
                                 {"reservation_mode", db::OID_INT2},
                                 {"subnet", db::OID_VARCHAR},
                                 {"user_context", db::OID_VARCHAR},
                                 // v4
                                 {"boot_file_name", db::OID_VARCHAR},
                                 {"match_client_id", db::OID_INT2},
                                 {"next_server", db::OID_VARCHAR},
                                 {"server_hostname", db::OID_VARCHAR},
                                 {"subnet_4o6_interface", db::OID_VARCHAR},
                                 {"subnet_4o6_interface_id", db::OID_VARCHAR},
                                 {"subnet_4o6_subnet", db::OID_VARCHAR},
                                 {"subnet_v4_psid_offset", db::OID_INT2},
                                 {"subnet_v4_psid_len", db::OID_INT2},
                                 {"subnet_v4_excluded_psids", db::OID_VARCHAR},
                                 // v6
                                 {"pd_pools_list", db::OID_VARCHAR},
                                 {"preferred_lifetime", db::OID_INT8}};
        return types;
    }

    /// @brief subnet information
    SubnetInfo<D> subnet_;

    /// @brief Database connection to use to execute the query
    db::PgSqlConnection connection_;
};
using PgSqlSubnet4Mgr = PgSqlSubnetMgr<DHCP_SPACE_V4>;
using PgSqlSubnet6Mgr = PgSqlSubnetMgr<DHCP_SPACE_V6>;

}  // namespace dhcp
}  // namespace isc

#endif  // PGSQL_SUBNET_MGR_H

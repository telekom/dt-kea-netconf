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

#ifndef MYSQL_SUBNET_MGR_H
#define MYSQL_SUBNET_MGR_H

#include <dhcpsrv/subnet_mgr.h>
#include <util/dhcp.h>

#include <database/db_exceptions.h>
#include <mysql/mysql_connection.h>

namespace isc {
namespace dhcp {

template <DhcpSpaceType D>
struct MySqlSubnetMgr : public SubnetMgr<D> {
    explicit MySqlSubnetMgr(db::DatabaseConnection::ParameterMap const& parameters)
        : connection_(parameters) {
        // Open the database.
        connection_.openDatabase();

        // Prepare all statements.
        for (auto const& pair : tagged_statements()) {
            if (pair.second.index >= connection_.statements_.size()) {
                connection_.statements_.resize(pair.second.index + 1, NULL);
                connection_.text_statements_.resize(pair.second.index + 1, std::string());
            }
            connection_.prepareStatement(pair.second.index, pair.second.text);
        }

        // Test schema version before we try to prepare statements.
        db::VersionTuple const code_version(db::MYSQL_CONFIG_SCHEMA_VERSION_MAJOR,
                                            db::MYSQL_CONFIG_SCHEMA_VERSION_MINOR);
        db::VersionTuple const& db_version(getVersion());
        if (code_version != db_version) {
            isc_throw(db::DbOpenError, "MySQL schema version mismatch: need version: "
                                           << std::get<0>(code_version) << "."
                                           << std::get<1>(code_version)
                                           << ", found version: " << std::get<0>(db_version) << "."
                                           << std::get<1>(db_version));
        }
    }

    db::VersionTuple getVersion() const {
        MYSQL_STMT* const& statement =
            connection_.statements_.at(tagged_statements().at("GET_VERSION").index);

        // Execute the prepared statement.
        int status = mysql_stmt_execute(statement);
        if (status != 0) {
            isc_throw(db::DbOperationError,
                      "unable to execute <" << tagged_statements().at("GET_VERSION").text
                                            << "> - reason: " << mysql_error(connection_.mysql_));
        }

        // Bind the output of the statement to the appropriate variables.
        // @{
        uint32_t major;  // Major version number
        uint32_t minor;  // Minor version number
        MySqlBind bind;

        bind.add(&major);
        bind.add(&minor);

        status = mysql_stmt_bind_result(statement, bind.data());
        if (status != 0) {
            isc_throw(db::DbOperationError,
                      "unable to bind result set: " << mysql_error(connection_.mysql_));
        }
        // @}

        // Fetch the data and set up the "release" object to release associated
        // resources when this method exits then retrieve the data.
        // mysql_stmt_fetch return value other than 0 means error occurrence.
        db::MySqlFreeResult fetch_release(statement);
        status = mysql_stmt_fetch(statement);
        if (status != 0) {
            isc_throw(db::DbOperationError,
                      "unable to obtain result set: " << mysql_error(connection_.mysql_));
        }

        return std::make_pair(major, minor);
    }

    void del(SubnetInfoPtr<D>& subnet,
             std::string const& /* config_timestamp_path */) override final {
        uint32_t const index =
            tagged_statements().at("DELETE_SUBNET" + dhcpSpaceToString<D>() + "_ALL").index;
        MYSQL_STMT* const& statement = connection_.statements_.at(index);

        // Set up the WHERE clause values.
        MySqlBind bind;
        bind.add(&subnet->strings_.subnet_);

        // Bind the input parameters to the statement.
        int status = mysql_stmt_bind_param(statement, bind.data());
        connection_.checkError(status, index, "unable to bind WHERE clause parameter");

        // Execute the prepared statement.
        status = mysql_stmt_execute(statement);
        connection_.checkError(status, index, "unable to execute");

        // See how many rows were affected. Note that the statement may delete
        // multiple rows.
        auto affected_rows = mysql_stmt_affected_rows(statement);

        // Check success case first as it is the most likely outcome.
        if (affected_rows == 1) {
            return;
        }

        // If no rows affected, subnet doesn't exist.
        if (affected_rows == 0) {
            isc_throw(db::DbOperationError, "MySqlSubnet"
                                                << dhcpSpaceToString<D>()
                                                << "Mgr::del(): deleted none, expected one");
        }

        // Should not have happened, primary key constraint should only have filtered one row.
        isc_throw(db::DbOperationError, "MySqlSubnet"
                                            << dhcpSpaceToString<D>()
                                            << "Mgr::del(): deleted multiple, expected one");
    }

    void insert(SubnetInfoPtr<D>& subnet,
                std::string const& /* config_timestamp_path */) override final {
        uint32_t const index =
            tagged_statements().at("INSERT_SUBNET" + dhcpSpaceToString<D>() + "_ALL").index;
        MYSQL_STMT* const& statement = connection_.statements_.at(index);

        // Set up the WHERE clause values.
        MySqlBind bind;
        bind.add(&subnet->strings_.subnet_);
        bindCommon(bind, subnet);
        bindDelta(bind, subnet);

        // Bind the input parameters to the statement.
        int status = mysql_stmt_bind_param(statement, bind.data());
        connection_.checkError(status, index, "unable to bind parameters");

        // Execute the prepared statement.
        status = mysql_stmt_execute(statement);
        if (status != 0) {
            // Failure: check for the special case of duplicate entry. If this
            // is the case, we return false to indicate that the row was not
            // added. Otherwise we throw an exception.
            if (mysql_errno(connection_.mysql_) == ER_DUP_ENTRY) {
                isc_throw(db::DuplicateEntry,
                          "MySqlSubnet" + dhcpSpaceToString<D>() + "Mgr::insert()");
            }
            connection_.checkError(status, index, "unable to execute");
        }
    }

    SubnetInfoCollection<D> select(
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        uint32_t const index = tagged_statements()
                                   .at("SELECT_SUBNET" + dhcpSpaceToString<D>() + "_ALL_UNFILTERED")
                                   .index;
        MYSQL_STMT* const& statement = connection_.statements_.at(index);

        // Set up the WHERE clause values.
        MySqlBind bind;

        return selectCommon(index, statement, bind);
    }

    SubnetInfoPtr<D> selectBySubnet(
        std::string& subnet,
        [[maybe_unused]] std::string const& config_timestamp_path = std::string()) override final {
        uint32_t const index =
            tagged_statements().at("SELECT_SUBNET" + dhcpSpaceToString<D>() + "_ALL").index;
        MYSQL_STMT* const& statement = connection_.statements_.at(index);

        // Set up the WHERE clause values.
        MySqlBind bind;
        bind.add(&subnet);

        return selectCommon(index, statement, bind, true).at(0);
    }

    SubnetInfoPtr<D> selectBySubnetID(SubnetID) override final {
        isc_throw(NotImplemented, "not yet implemented.");
    };

    SubnetInfoCollection<D> selectCommon(uint32_t const index,
                                         MYSQL_STMT* const statement,
                                         MySqlBind& bind,
                                         bool single = false) {
        SubnetInfoCollection<D> collection;

        // Set up the WHERE clause.
        int status = mysql_stmt_bind_param(statement, bind.data());
        connection_.checkError(status, index, "unable to bind SELECT clause parameters");

        // Set up the results.
        MySqlBind result_bind;
        SubnetInfoWithCStrings<D> subnet;
        bindCommonForSelect(result_bind, subnet);
        bindDeltaForSelect(result_bind, subnet);
        status = mysql_stmt_bind_result(statement, result_bind.data());

        // Execute the statement.
        status = mysql_stmt_execute(statement);
        connection_.checkError(status, index, "unable to execute");

        // Ensure that all the subnet information is retrieved in one go to avoid
        // overhead of going back and forth between client and server.
        status = mysql_stmt_store_result(statement);
        connection_.checkError(status, index, "unable to set up for storing all results");

        // Set up the fetch "release" object to release resources associated
        // with the call to mysql_stmt_fetch when this method exits, then
        // retrieve the data.
        db::MySqlFreeResult fetch_release(statement);
        int count = 0;
        while ((status = mysql_stmt_fetch(statement)) == 0) {
            try {
                collection.push_back(retrieve(subnet));
            } catch (isc::BadValue const& ex) {
                // Rethrow the exception with a bit more data.
                isc_throw(BadValue, ex.what() << ". Statement is <"
                                              << connection_.text_statements_[index] << ">");
            }

            if (single && (++count > 1)) {
                isc_throw(db::MultipleRecords, "multiple records were found in the "
                                               "database where only one was expected for query "
                                                   << connection_.text_statements_[index]);
            }
        }

        // How did the fetch end?
        if (status == 1) {
            // Error - unable to fetch results
            connection_.checkError(status, index, "unable to fetch results");
        } else if (status == MYSQL_DATA_TRUNCATED) {
            // Data truncated - throw an exception indicating what was at fault
            isc_throw(db::DataTruncated,
                      connection_.text_statements_[index] << " returned truncated data");
        }
        return collection;
    }

    void update(SubnetInfoPtr<D>& subnet,
                std::string const& /* config_timestamp_path */) override final {
        uint32_t const index =
            tagged_statements().at("UPDATE_SUBNET" + dhcpSpaceToString<D>() + "_ALL").index;
        MYSQL_STMT* const& statement = connection_.statements_.at(index);

        // Set up the WHERE clause values.
        MySqlBind bind;
        bindCommon(bind, subnet);
        bindDelta(bind, subnet);
        bind.add(&subnet->strings_.subnet_);

        // Bind the input parameters to the statement.
        int status = mysql_stmt_bind_param(statement, bind.data());
        connection_.checkError(status, index, "unable to bind parameters");

        // Execute the prepared statement.
        status = mysql_stmt_execute(statement);
        if (status != 0) {
            connection_.checkError(status, index, "unable to execute");
        }

        // See how many rows were affected. Note that the statement may delete
        // multiple rows.
        auto affected_rows = mysql_stmt_affected_rows(statement);

        // Check success case first as it is the most likely outcome.
        if (affected_rows == 1) {
            return;
        }

        // If no rows affected, subnet doesn't exist.
        if (affected_rows == 0) {
            isc_throw(db::DbOperationError, "MySqlSubnet"
                                                << dhcpSpaceToString<D>()
                                                << "Mgr::update(): updated none, expected one");
        }

        // Should not have happened, primary key constraint should only have filtered one row.
        isc_throw(db::DbOperationError, "MySqlSubnet"
                                            << dhcpSpaceToString<D>()
                                            << "Mgr::update(): updated multiple, expected one");
    }

    SubnetID maxSubnetID() override final {
        uint32_t const index =
            tagged_statements().at("SELECT_MAX_SUBNET_ID_" + dhcpSpaceToString<D>()).index;
        MYSQL_STMT* const& statement = connection_.statements_.at(index);

        // Set up the WHERE clause values.
        MySqlBind bind;

        SubnetID max_subnet_id;

        // Set up the WHERE clause.
        int status = mysql_stmt_bind_param(statement, bind.data());
        connection_.checkError(status, index, "unable to bind SELECT clause parameters");

        // Set up the results.
        MySqlBind result_bind;
        result_bind.add(&max_subnet_id);
        status = mysql_stmt_bind_result(statement, result_bind.data());

        // Execute the statement.
        status = mysql_stmt_execute(statement);
        connection_.checkError(status, index, "unable to execute");

        // Ensure that all the subnet information is retrieved in one go to avoid
        // overhead of going back and forth between client and server.
        status = mysql_stmt_store_result(statement);
        connection_.checkError(status, index, "unable to set up for storing all results");

        // Set up the fetch "release" object to release resources associated
        // with the call to mysql_stmt_fetch when this method exits, then
        // retrieve the data.
        db::MySqlFreeResult fetch_release(statement);
        int count = 0;
        while ((status = mysql_stmt_fetch(statement)) == 0) {
            if (++count > 1) {
                isc_throw(db::MultipleRecords, "multiple records were found in the "
                                               "database where only one was expected for query "
                                                   << connection_.text_statements_[index]);
            }
        }

        // How did the fetch end?
        if (status == 1) {
            // Error - unable to fetch results
            connection_.checkError(status, index, "unable to fetch results");
        } else if (status == MYSQL_DATA_TRUNCATED) {
            // Data truncated - throw an exception indicating what was at fault
            isc_throw(db::DataTruncated,
                      connection_.text_statements_[index] << " returned truncated data");
        }
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
        return "mysql";
    }

    void commit() override final {
        if (mysql_commit(connection_.mysql_) != 0) {
            isc_throw(db::DbOperationError, "commit failed: " << mysql_error(connection_.mysql_));
        }
    }

    void rollback() override final {
        if (mysql_rollback(connection_.mysql_) != 0) {
            isc_throw(db::DbOperationError, "rollback failed: " << mysql_error(connection_.mysql_));
        }
    }

    using StatementMap = std::unordered_map<std::string, db::TaggedStatement>;

    static StatementMap& tagged_statements() {
        static StatementMap tagged_statements;

        std::string tag;
        std::string text;
        tag = "GET_VERSION";
        text = "SELECT version, minor FROM config_schema_version";
        tagged_statements.insert(
            {tag, db::TaggedStatement{static_cast<uint32_t>(tagged_statements.size()), text}});

        // Column-oriented statements
        // @{
        std::string columns;
        std::string values;
        std::string assignments;
        bool first = true;
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
            values += "?";
            assignments += column + " = ?";

            tag = "INSERT_SUBNET" + dhcpSpaceToString<D>() + "_" + column;
            text = "INSERT INTO subnet" + dhcpSpaceToString<D>() + " (subnet, " + column +
                   ") VALUES (?, ?)";
            tagged_statements.insert(
                {tag, db::TaggedStatement{static_cast<uint32_t>(tagged_statements.size()), text}});

            tag = "SELECT_SUBNET" + dhcpSpaceToString<D>() + "_" + column;
            text = "SELECT subnet, " + column + " FROM subnet" + dhcpSpaceToString<D>() +
                   " WHERE subnet = ?";
            tagged_statements.insert(
                {tag, db::TaggedStatement{static_cast<uint32_t>(tagged_statements.size()), text}});

            tag = "SELECT_SUBNET" + dhcpSpaceToString<D>() + "_" + column + "_UNFILTERED";
            text = "SELECT subnet, " + column + " FROM subnet" + dhcpSpaceToString<D>();
            tagged_statements.insert(
                {tag, db::TaggedStatement{static_cast<uint32_t>(tagged_statements.size()), text}});

            tag = "UPDATE_SUBNET" + dhcpSpaceToString<D>() + "_" + column;
            text = "UPDATE subnet" + dhcpSpaceToString<D>() + " SET " + column +
                   " = ? WHERE subnet = ?";
            tagged_statements.insert(
                {tag, db::TaggedStatement{static_cast<uint32_t>(tagged_statements.size()), text}});
        }
        // @}

        // Universal statements
        // @{
        tag = "DELETE_SUBNET" + dhcpSpaceToString<D>() + "_ALL";
        text = "DELETE FROM subnet" + dhcpSpaceToString<D>() + " WHERE subnet = ?";
        tagged_statements.insert(
            {tag, db::TaggedStatement{static_cast<uint32_t>(tagged_statements.size()), text}});

        tag = "DELETE_SUBNETS" + dhcpSpaceToString<D>() + "_ALL";
        text = "TRUNCATE subnet" + dhcpSpaceToString<D>();
        tagged_statements.insert(
            {tag, db::TaggedStatement{static_cast<uint32_t>(tagged_statements.size()), text}});

        tag = "INSERT_SUBNET" + dhcpSpaceToString<D>() + "_ALL";
        text = "INSERT INTO subnet" + dhcpSpaceToString<D>() + " (subnet, " + columns +
               ") VALUES (?, " + values + ")";
        tagged_statements.insert(
            {tag, db::TaggedStatement{static_cast<uint32_t>(tagged_statements.size()), text}});

        tag = "SELECT_SUBNET" + dhcpSpaceToString<D>() + "_ALL";
        text = "SELECT subnet, " + columns + " FROM subnet" + dhcpSpaceToString<D>() +
               " WHERE subnet = ?";
        tagged_statements.insert(
            {tag, db::TaggedStatement{static_cast<uint32_t>(tagged_statements.size()), text}});

        tag = "SELECT_SUBNET" + dhcpSpaceToString<D>() + "_ALL_UNFILTERED";
        text = "SELECT subnet, " + columns + " FROM subnet" + dhcpSpaceToString<D>();
        tagged_statements.insert(
            {tag, db::TaggedStatement{static_cast<uint32_t>(tagged_statements.size()), text}});

        tag = "UPDATE_SUBNET" + dhcpSpaceToString<D>() + "_ALL";
        text =
            "UPDATE subnet" + dhcpSpaceToString<D>() + " SET " + assignments + " WHERE subnet = ?";
        tagged_statements.insert(
            {tag, db::TaggedStatement{static_cast<uint32_t>(tagged_statements.size()), text}});

        tag = "SELECT_MAX_SUBNET_ID_" + dhcpSpaceToString<D>();
        text = "SELECT MAX(id) FROM subnet" + dhcpSpaceToString<D>();
        tagged_statements.insert(
            {tag, db::TaggedStatement{static_cast<uint32_t>(tagged_statements.size()), text}});
        // @}

        return tagged_statements;
    }

private:
    inline void bindCommon(MySqlBind& bind, SubnetInfoPtr<D> const& subnet) {
        bind.add(&subnet->id_);
        bind.add(&subnet->timestamp_);
        bind.add(&subnet->renew_timer_);
        bind.add(&subnet->rebind_timer_);
        bind.add(&subnet->valid_lifetime_);
        bind.add(&subnet->allocation_type_);
        bind.add(&subnet->strings_.client_class_);
        bind.add(&subnet->strings_.interface_);
        bind.add(&subnet->strings_.interface_id_);
        bind.add(&subnet->strings_.option_data_list_);
        bind.add(&subnet->strings_.pools_list_);
        bind.add(&subnet->rapid_commit_);
        bind.add(&subnet->strings_.relay_);
        bind.add(&subnet->strings_.require_client_classes_);
        bind.add(&subnet->strings_.reservations_);
        bind.add(&subnet->reservation_mode_);
        bind.add(&subnet->strings_.user_context_);
    }

    inline void bindCommonForSelect(MySqlBind& bind, SubnetInfoWithCStrings<D>& subnet) {
        bind.add(subnet.strings_.subnet_, subnet.strings_.subnet_length_);
        bind.add(&subnet.id_);
        bind.add(&subnet.timestamp_);
        bind.add(&subnet.renew_timer_);
        bind.add(&subnet.rebind_timer_);
        bind.add(&subnet.valid_lifetime_);
        bind.add(&subnet.allocation_type_);
        bind.add(subnet.strings_.client_class_, subnet.strings_.client_class_length_);
        bind.add(subnet.strings_.interface_, subnet.strings_.interface_length_);
        bind.add(subnet.strings_.interface_id_, subnet.strings_.interface_id_length_);
        bind.add(subnet.strings_.option_data_list_, subnet.strings_.option_data_list_length_);
        bind.add(subnet.strings_.pools_list_, subnet.strings_.pools_list_length_);
        bind.add(&subnet.rapid_commit_);
        bind.add(subnet.strings_.relay_, subnet.strings_.relay_length_);
        bind.add(subnet.strings_.require_client_classes_,
                 subnet.strings_.require_client_classes_length_);
        bind.add(subnet.strings_.reservations_, subnet.strings_.reservations_length_);
        bind.add(&subnet.reservation_mode_);
        bind.add(subnet.strings_.user_context_, subnet.strings_.user_context_length_);
    }

    void bindDelta(MySqlBind& data, SubnetInfoPtr<D> const& subnet);

    void bindDeltaForSelect(MySqlBind& data, SubnetInfoWithCStrings<D>& subnet);

    /// @brief Copy received data into Ptr<SubnetInfo<D>> object.
    ///
    /// Called after the MYSQL_BIND array created by createBindForReceive()
    /// has been used, this copies data from the internal member variables
    /// into a Ptr<SubnetInfo<D>> object.
    ///
    /// @return Ptr<SubnetInfo<D>> Pointer to an object holding the relevant data.
    SubnetInfoPtr<D> retrieve(SubnetInfoWithCStrings<D>& subnet) {
        return std::make_shared<SubnetInfo<D>>(subnet);
    }

    /// @brief MySQL connection
    db::MySqlConnection connection_;
};
using MySqlSubnet4Mgr = MySqlSubnetMgr<DHCP_SPACE_V4>;
using MySqlSubnet6Mgr = MySqlSubnetMgr<DHCP_SPACE_V6>;

}  // namespace dhcp
}  // namespace isc

#endif  // MYSQL_SUBNET_MGR_H

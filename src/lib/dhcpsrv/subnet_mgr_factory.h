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

#ifndef SUBNET_MGR_FACTORY_H
#define SUBNET_MGR_FACTORY_H

#include <database/database_connection.h>
#include <dhcpsrv/cfgmgr.h>
#include <dhcpsrv/dhcpsrv_log.h>
#include <dhcpsrv/memfile_subnet_mgr.h>
#include <dhcpsrv/subnet_mgr.h>
#include <exceptions/exceptions.h>

#include <string>

#ifdef HAVE_MYSQL
#include <dhcpsrv/mysql_subnet_mgr.h>
#endif
#ifdef HAVE_PGSQL
#include <dhcpsrv/pgsql_subnet_mgr.h>
#endif
#ifdef HAVE_CQL
#include <cql/cql_connection.h>
#include <dhcpsrv/cql_subnet_mgr.h>
#endif

namespace isc {
namespace dhcp {

/// @brief No subnet manager exception
///
/// Thrown if an attempt is made to get a reference to the current subnet
/// manager and none is currently available.
class NoSubnetManager : public Exception {
public:
    NoSubnetManager(const char* file, size_t line, const char* what)
        : isc::Exception(file, line, what) {
    }
};

/// @brief Subnet Manager Factory
///
/// This class comprises nothing but static methods used to create a subnet
/// manager.  It analyzes the database information passed to the creation
/// function and instantiates an appropriate subnet manager based on the type
/// requested.
///
/// Strictly speaking these functions could be stand-alone functions.  However,
/// it is convenient to encapsulate them in a class for naming purposes.
///
/// @todo: Will need to develop some form of registration mechanism for
///        user-supplied backends (so that there is no need to modify the code).
template <DhcpSpaceType D>
class SubnetMgrFactory {
public:
    /// @brief Create an instance of a subnet manager.
    ///
    /// Each database backend has its own subnet manager type.  This static
    /// method sets the "current" subnet manager to be a manager of the
    /// appropriate type.  The actual subnet manager is returned by the
    /// "instance" method.
    ///
    /// @note When called, the current subnet manager is <b>always</b> destroyed
    ///       and a new one created - even if the parameters are the same.
    ///
    /// dbaccess is a generic way of passing parameters. Parameters are passed
    /// in the "name=value" format, separated by spaces.  The data MUST include
    /// a keyword/value pair of the form "type=dbtype" giving the database
    /// type, e.q. "mysql" or "sqlite3".
    ///
    /// @param dbaccess Database access parameters.  These are in the form of
    ///        "keyword=value" pairs, separated by spaces. They are backend-
    ///        -end specific, although must include the "type" keyword which
    ///        gives the backend in use.
    ///
    /// @throw isc::InvalidParameter dbaccess std::string does not contain the "type"
    ///        keyword.
    /// @throw isc::dhcp::InvalidType The "type" keyword in dbaccess does not
    ///        identify a supported backend.
    static void create(const std::string& dbaccess) {
        const std::string type = "type";

        // Parse the access std::string and create a redacted string for logging.
        db::DatabaseConnection::ParameterMap parameters = db::DatabaseConnection::parse(dbaccess);
        std::string redacted = db::DatabaseConnection::redactedAccessString(parameters);

        // Is "type" present?
        if (parameters.find(type) == parameters.end()) {
            LOG_ERROR(dhcpsrv_logger, DHCPSRV_NOTYPE_DB).arg(dbaccess);
            isc_throw(InvalidParameter, "Database configuration parameters do not "
                                        "contain the 'type' keyword");
        }

        // Yes, check what it is.
#ifdef HAVE_MYSQL
        if (parameters[type] == std::string("mysql")) {
            getSubnetMgrPtr().reset(new MySqlSubnetMgr<D>(parameters));
            return;
        }
#endif
#ifdef HAVE_PGSQL
        if (parameters[type] == std::string("postgresql")) {
            LOG_INFO(dhcpsrv_logger, DHCPSRV_PGSQL_SUBNET_DB).arg(redacted);
            getSubnetMgrPtr().reset(new PgSqlSubnetMgr<D>(parameters));
            return;
        }
#endif
#ifdef HAVE_CQL
        if (parameters[type] == std::string("cql")) {
            LOG_INFO(dhcpsrv_logger, DHCPSRV_CQL_SUBNET_DB).arg(redacted);
            getSubnetMgrPtr().reset(new CqlSubnetMgr<D>(parameters));
            return;
        }
#endif
        if (parameters[type] == std::string("memfile")) {
            LOG_INFO(dhcpsrv_logger, DHCPSRV_MEMFILE_SUBNET_DB).arg(redacted);
            getSubnetMgrPtr().reset(new MemfileSubnetMgr<D>(parameters));
            return;
        }

        // Get here on no match
        LOG_ERROR(dhcpsrv_logger, DHCPSRV_UNKNOWN_DB).arg(parameters[type]);
        isc_throw(db::InvalidType, "Database access parameter 'type' does "
                                   "not specify a supported database backend: "
                                       << parameters[type]);
    }

    static void create(isc::db::DatabaseConnection const& connection) {
#ifdef HAVE_CQL
        if (isc::db::CqlConnection const* c = dynamic_cast<isc::db::CqlConnection const*>(&connection)) {
            getSubnetMgrPtr().reset(new CqlSubnetMgr<D>(*c));
            return;
        }
#endif
#ifdef HAVE_MYSQL
        if (isc::db::MySqlConnection const* c =
                       dynamic_cast<isc::db::MySqlConnection const*>(&connection)) {
            getSubnetMgrPtr().reset(new MySqlSubnetMgr<D>(*c));
            return;
        }
#endif
#ifdef HAVE_PGSQL
        if (isc::db::PgSqlConnection const* c =
                       dynamic_cast<isc::db::PgSqlConnection const*>(&connection)) {
            getSubnetMgrPtr().reset(new PgSqlSubnetMgr<D>(*c));
            return;
        }
#endif
        isc_throw(isc::Unexpected, PRETTY_FUNCTION_NAME() << ": connection is not Cql nor MySql nor PgSql");
    }

    /// @brief Destroy subnet manager
    ///
    /// Destroys the current subnet manager object.  This should have the effect
    /// of closing the database connection.  The method is a no-op if no
    /// subnet manager is available.
    static void destroy() {
        // Destroy current subnet manager. This is a no-op if no subnet manager is
        // available.
        if (getSubnetMgrPtr()) {
            getSubnetMgrPtr().reset();
        }
    }

    /// @brief Return current subnet manager
    ///
    /// Returns an instance of the "current" subnet manager.  An exception
    /// will be thrown if none is available.
    ///
    /// @throw isc::dhcp::NoSubnetManager No subnet manager is available: use
    ///        create() to create one before calling this method.
    static SubnetMgr<D>& instance() {
        SubnetMgr<D>* lmptr = getSubnetMgrPtr().get();
        if (lmptr == NULL) {
            isc_throw(NoSubnetManager, "no current subnet manager is available");
        }
        return *lmptr;
    }

    /// @brief Indicates if the subnet manager has been instantiated.
    ///
    /// @return True if the subnet manager instance exists, false otherwise.
    static bool haveInstance() {
        return getSubnetMgrPtr().get();
    }

private:
    /// @brief Hold pointer to subnet manager
    ///
    /// Holds a pointer to the singleton subnet manager.  The singleton
    /// is encapsulated in this method to avoid a "static initialization
    /// fiasco" if defined in an external static variable.
    static std::unique_ptr<SubnetMgr<D>>& getSubnetMgrPtr() {
        static std::unique_ptr<SubnetMgr<D>> subnetMgrPtr;
        return subnetMgrPtr;
    }
};

}  // namespace dhcp
}  // namespace isc

#endif  // SUBNET_MGR_FACTORY_H

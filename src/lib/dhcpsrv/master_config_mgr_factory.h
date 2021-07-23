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

#ifndef MASTER_CONFIG_MGR_FACTORY_H
#define MASTER_CONFIG_MGR_FACTORY_H

#include <database/database_connection.h>
#ifdef HAVE_CQL
#include <dhcpsrv/cql_master_config_mgr.h>
#endif
#include <dhcpsrv/dhcpsrv_log.h>
#include <dhcpsrv/master_config_mgr.h>
#ifdef HAVE_MYSQL
#include <dhcpsrv/mysql_master_config_mgr.h>
#endif
#ifdef HAVE_PGSQL
#include <dhcpsrv/pgsql_master_config_mgr.h>
#endif
#include <exceptions/exceptions.h>
#include <util/dhcp.h>

#include <string>

namespace isc {
namespace dhcp {

/// @brief No configuration manager exception
///
/// Thrown if an attempt is made to get a reference to the current configuration
/// manager and none is currently available.
struct NoServerConfigMasterManager : public Exception {
    NoServerConfigMasterManager(const char* file, size_t line, const char* what)
        : isc::Exception(file, line, what) {
    }
};

/// @brief master configuration manager factory
///
/// This class comprises nothing but static methods used to create a server
/// configuration manager. It analyzes the database information passed to the
/// creation function and instantiates an appropriate server configuration
/// manager based on the type requested.
///
/// Strictly speaking these functions could be stand-alone functions.  However,
/// it is convenient to encapsulate them in a class for naming purposes.
///
/// @todo: Will need to develop some form of registration mechanism for
///        user-supplied backends (so that there is no need to modify the code).
template <DhcpSpaceType D>
struct MasterConfigMgrFactory {
    /// @brief Create an instance of a server configuration manager.
    ///
    /// Each database backend has its own server configuration manager type.
    /// This static method sets the "current" server configuration manager
    /// to be a manager of the appropriate type.  The actual server
    /// configuration is returned by the "instance" method.
    ///
    /// @note When called, the current server configuration manager is
    ///       <b>always</b> destroyed and a new one created - even if
    ///       the parameters are the same.
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
    /// @throw isc::InvalidParameter dbaccess string does not contain the "type"
    ///        keyword.
    /// @throw isc::dhcp::InvalidType The "type" keyword in dbaccess does not
    ///        identify a supported backend.
    static void create(std::string const& dbaccess) {
        std::string const type("type");

        // Parse the access string and create a redacted string for logging.
        isc::db::DatabaseConnection::ParameterMap parameters(
            isc::db::DatabaseConnection::parse(dbaccess));
        std::string const& redacted(isc::db::DatabaseConnection::redactedAccessString(parameters));

        // Is "type" present?
        if (parameters.find(type) == parameters.end()) {
            LOG_ERROR(dhcpsrv_logger, DHCPSRV_NOTYPE_DB).arg(dbaccess);
            isc_throw(InvalidParameter,
                      "Database configuration parameters do not contain the 'type' keyword");
        }

        // Yes, check what it is.
#ifdef HAVE_CQL
        if (parameters[type] == "cql") {
            LOG_INFO(dhcpsrv_logger, DHCPSRV_CQL_DB).arg(redacted);
            getConfigurationMgrPtr() = std::make_shared<CqlMasterConfigMgr<D>>(parameters);
            return;
        }
#endif
#ifdef HAVE_MYSQL
        if (parameters[type] == "mysql") {
            LOG_INFO(dhcpsrv_logger, DHCPSRV_MYSQL_DB).arg(redacted);
            getConfigurationMgrPtr() = std::make_shared<MySqlMasterConfigMgr<D>>(parameters);
            return;
        }
#endif
#ifdef HAVE_PGSQL
        if (parameters[type] == "postgresql") {
            LOG_INFO(dhcpsrv_logger, DHCPSRV_PGSQL_DB).arg(redacted);
            getConfigurationMgrPtr() = std::make_shared<PgSqlMasterConfigMgr<D>>(parameters);
            return;
        }
#endif

        // Get here on no match
        LOG_ERROR(dhcpsrv_logger, DHCPSRV_UNKNOWN_DB).arg(parameters[type]);
        isc_throw(isc::db::InvalidType, "Database access parameter 'type' does "
                                        "not specify a supported database backend: "
                                            << parameters[type]);
    }

    /// @brief Destroy server configuration manager
    ///
    /// Destroys the current server configuration manager object.  This should
    /// have the effect of closing the database connection.  The method is a
    /// no-op if no server configuration manager is available.
    static void destroy() {
        if (getConfigurationMgrPtr()) {
            LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE, DHCPSRV_CLOSE_DB)
                .arg(getConfigurationMgrPtr()->getType());
        }
        getConfigurationMgrPtr().reset();
    }

    static bool initialized() {
        return getConfigurationMgrPtr().get() != nullptr;
    }

    /// @brief Return current server configuration manager
    ///
    /// Returns an instance of the "current" server configuration manager.
    /// An exception will be thrown if none is available.
    ///
    /// @throw isc::dhcp::NoLeaseManager No lease manager is available: use
    ///        create() to create one before calling this method.
    static MasterConfigMgr& instance() {
        std::shared_ptr<MasterConfigMgr> cfg_mgr_ptr(getConfigurationMgrPtr());
        if (cfg_mgr_ptr.get() == nullptr) {
            isc_throw(NoServerConfigMasterManager,
                      "no current server configuration manager is available");
        }
        return *cfg_mgr_ptr;
    }

private:
    /// @brief Hold pointer to server configuration manager
    ///
    /// Holds a pointer to the singleton server configuration manager.
    /// The singleton is encapsulated in this method to avoid a "static
    /// initialization fiasco" if defined in an external static variable.
    static std::shared_ptr<MasterConfigMgr>& getConfigurationMgrPtr() {
        static std::shared_ptr<MasterConfigMgr> configuration_mgr_ptr;
        return configuration_mgr_ptr;
    }
};

}  // namespace dhcp
}  // namespace isc

#endif  // MASTER_CONFIG_MGR_FACTORY_H

// Copyright (C) 2016-2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef CFG_DBACCESS_H
#define CFG_DBACCESS_H

#include <cc/cfg_to_element.h>
#include <database/database_connection.h>
#include <util/dhcp.h>
#include <list>
#include <string>

namespace isc {
namespace dhcp {

/// @brief Holds access parameters and the configuration of the
/// lease, hosts and configuration database connection.
///
/// The database access strings use the same format as the strings
/// passed to the @ref isc::dhcp::LeaseMgrFactory::create function.
class CfgDbAccess {
public:
    /// @brief Constructor.
    CfgDbAccess();

    /// @brief Sets parameters which will be appended to the database
    /// access strings.
    ///
    /// @param appended_parameters String holding collection of parameters
    /// in the following format: "parameter0=value0 parameter1=value1 ...".
    void setAppendedParameters(const std::string& appended_parameters) {
        appended_parameters_ = appended_parameters;
    }

    /// @brief Retrieves lease database access string.
    ///
    /// @return Lease database access string with additional parameters
    /// specified with @ref CfgDbAccess::setAppendedParameters.
    std::string getLeaseDbAccessString() const;

    /// @brief Sets lease database access string.
    ///
    /// @param lease_db_access New lease database access string.
    void setLeaseDbAccessString(const std::string& lease_db_access) {
        lease_db_access_ = lease_db_access;
    }

    /// @brief Retrieves host database access string.
    ///
    /// @return Host database access string with additional parameters
    /// specified with @ref CfgDbAccess::setAppendedParameters.
    std::string getHostDbAccessString() const;

    /// @brief Sets host database access string.
    ///
    /// @param host_db_access New host database access string.
    /// @param front Add at front if true, at back if false (default).
    void setHostDbAccessString(const std::string& host_db_access, bool front = false) {
        if (front) {
            host_db_access_.push_front(host_db_access);
        } else {
            host_db_access_.push_back(host_db_access);
        }
    }

    /// @brief Retrieves configuration database access string.
    ///
    /// @return Configuration database access string with additional parameters
    /// specified with @ref CfgDbAccess::setAppendedParameters.
    std::string getMasterCfgDbAccessString() const;

    /// @brief Sets the master database access string.
    ///
    /// @param master_cfg_db_access new master config database access
    /// string.
    void setMasterCfgDbAccessString(const std::string& master_cfg_db_access) {
        master_cfg_db_access_ = master_cfg_db_access;
    }

    /// @brief Retrieves configuration database access string.
    ///
    /// @return Configuration database access string with additional parameters
    /// specified with @ref CfgDbAccess::setAppendedParameters.
    std::string getShardCfgDbAccessString() const;

    /// @brief Sets config database access string.
    ///
    /// @param shard_cfg_db_access new config database access string
    void setShardCfgDbAccessString(const std::string& shard_cfg_db_access) {
        shard_cfg_db_access_ = shard_cfg_db_access;
    }

    template <DhcpSpaceType D>
    void createMasterCfgMgrs(std::optional<std::string> const& instance_ID = std::nullopt);

    template <DhcpSpaceType D>
    void createShardCfgMgrs(
        std::vector<std::optional<isc::data::ElementPtr>> const& credential_configurations = {});

    /// @brief Retrieves host database access string.
    ///
    /// @return Database access strings with additional parameters
    /// specified with @ref CfgDbAccess::setAppendedParameters
    std::list<std::string> getHostDbAccessStringList() const;

    /// @brief Creates instance of lease manager and host data sources
    /// according to the configuration specified.
    void createManagers() const;

    /// @brief Unparse an access string
    ///
    /// @param dbaccess the database access string
    /// @return a pointer to configuration
    static isc::data::ElementPtr toElementDbAccessString(const std::string& dbaccess);

protected:
    /// @brief Returns lease or host database access string.
    ///
    /// @param access_string without additional (appended) parameters.
    std::string getAccessString(const std::string& access_string) const;

    /// @brief Parameters to be appended to the database access
    /// strings.
    std::string appended_parameters_;

    /// @brief Holds lease database access string.
    std::string lease_db_access_;

    /// @brief Holds host database access strings.
    std::list<std::string> host_db_access_;

    /// @brief Holds configuration database access string.
    std::string shard_cfg_db_access_;

    /// @brief Holds configuration master database access string.
    std::string master_cfg_db_access_;
};

/// @brief A pointer to the @c CfgDbAccess.
typedef std::shared_ptr<CfgDbAccess> CfgDbAccessPtr;

/// @brief A pointer to the const @c CfgDbAccess.
typedef std::shared_ptr<const CfgDbAccess> ConstCfgDbAccessPtr;

/// @brief utility class for unparsing
struct CfgLeaseDbAccess : public CfgDbAccess, public isc::data::CfgToElement {
    /// @brief Constructor
    CfgLeaseDbAccess(const CfgDbAccess& super) : CfgDbAccess(super) {
    }

    /// @brief Unparse
    ///
    /// @ref isc::data::CfgToElement::toElement
    ///
    /// @result a pointer to a configuration
    virtual isc::data::ElementPtr toElement() const {
        return (db::DatabaseConnection::toElementDbAccessString(lease_db_access_));
    }
};

struct CfgHostDbAccess : public CfgDbAccess, public isc::data::CfgToElement {
    /// @brief Constructor
    CfgHostDbAccess(const CfgDbAccess& super) : CfgDbAccess(super) {
    }

    /// @brief Unparse
    ///
    /// @ref isc::data::CfgToElement::toElement
    ///
    /// @result a pointer to a configuration
    virtual isc::data::ElementPtr toElement() const {
        isc::data::ElementPtr result = isc::data::Element::createList();
        for (const std::string& dbaccess : host_db_access_) {
            isc::data::ElementPtr entry = db::DatabaseConnection::toElementDbAccessString(dbaccess);
            if (entry->size() > 0) {
                result->add(entry);
            }
        }
        return (result);
    }
};

}  // namespace dhcp
}  // namespace isc

#endif  // CFG_DBACCESS_H

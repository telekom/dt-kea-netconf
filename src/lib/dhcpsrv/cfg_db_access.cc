// Copyright (C) 2016-2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <database/dbaccess_parser.h>
#include <dhcpsrv/cfg_db_access.h>
#include <dhcpsrv/cfgmgr.h>
#include <dhcpsrv/db_type.h>
#include <dhcpsrv/host_data_source_factory.h>
#include <dhcpsrv/host_mgr.h>
#include <dhcpsrv/lease_mgr_factory.h>
#include <dhcpsrv/master_config_mgr_factory.h>
#include <dhcpsrv/shard_config_mgr_factory.h>
#include <dhcpsrv/subnet_mgr_factory.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <optional>
#include <sstream>
#include <vector>

using namespace isc::data;

namespace isc {
namespace dhcp {

CfgDbAccess::CfgDbAccess()
    : appended_parameters_(), lease_db_access_("type=memfile"), host_db_access_() {
}

std::string CfgDbAccess::getLeaseDbAccessString() const {
    return (getAccessString(lease_db_access_));
}

std::string CfgDbAccess::getHostDbAccessString() const {
    if (host_db_access_.empty()) {
        return ("");
    } else {
        return (getAccessString(host_db_access_.front()));
    }
}

std::string CfgDbAccess::getMasterCfgDbAccessString() const {
    return (getAccessString(master_cfg_db_access_));
}

std::string CfgDbAccess::getShardCfgDbAccessString() const {
    return (getAccessString(shard_cfg_db_access_));
}

std::list<std::string> CfgDbAccess::getHostDbAccessStringList() const {
    std::list<std::string> ret;
    for (const std::string& dbaccess : host_db_access_) {
        if (!dbaccess.empty()) {
            ret.push_back(getAccessString(dbaccess));
        }
    }
    return (ret);
}

void CfgDbAccess::createManagers() const {
    // Recreate lease manager.
    LeaseMgrFactory::destroy();
    LeaseMgrFactory::create(getLeaseDbAccessString());

    // Recreate host data source.
    HostMgr::create();

    // Restore the host cache.
    if (HostDataSourceFactory::registeredFactory("cache")) {
        HostMgr::addBackend("type=cache");
    }

    // Add database backends.
    std::list<std::string> host_db_access_list = getHostDbAccessStringList();
    for (std::string& hds : host_db_access_list) {
        HostMgr::addBackend(hds);
    }

    // Check for a host cache.
    HostMgr::checkCacheBackend(true);
}

std::string CfgDbAccess::getAccessString(const std::string& access_string) const {
    std::ostringstream s;
    s << access_string;
    // Only append additional parameters if any parameters are specified
    // in a configuration. For host database, no parameters mean that
    // database access is disabled and thus we don't want to append any
    // parameters.
    if ((s.tellp() != std::streampos(0)) && (!appended_parameters_.empty())) {
        s << " " << appended_parameters_;
    }

    return (s.str());
}

template <DhcpSpaceType D>
void masterCfgMgrs(CfgDbAccess& cfg_db_access, std::optional<std::string> const& instance_ID) {
    if (instance_ID.has_value()) {
        CfgMgr::instance().getStagingCfg()->getInstanceID() = instance_ID.value();
    } else {
        CfgMgr::instance().getStagingCfg()->getInstanceID() = "";
    }
    cfg_db_access.setAppendedParameters("universe=" + dhcpSpaceToString<D>());

    // Recreate server configuration manager for the master database.
    MasterConfigMgrFactory<D>::destroy();
    MasterConfigMgrFactory<D>::create(cfg_db_access.getMasterCfgDbAccessString());
}

template <>
void CfgDbAccess::createMasterCfgMgrs<DHCP_SPACE_V4>(
    std::optional<std::string> const& instance_ID) {
    masterCfgMgrs<DHCP_SPACE_V4>(*this, instance_ID);
}

template <>
void CfgDbAccess::createMasterCfgMgrs<DHCP_SPACE_V6>(
    std::optional<std::string> const& instance_ID) {
    masterCfgMgrs<DHCP_SPACE_V6>(*this, instance_ID);
}

template <DhcpSpaceType D>
void shardCfgMgrs(
    CfgDbAccess& cfg_db_access,
    std::vector<std::optional<isc::data::ElementPtr>> const& credential_configurations) {
    bool valid(true);
    for (std::optional<isc::data::ElementPtr> const& credentials_config : credential_configurations) {
        if (credentials_config.has_value()) {
            if (!credentials_config.value()) {
                valid = false;
                continue;
            }

            isc::dhcp::CfgMgr::instance().getStagingCfg()->getConfigurationType() =
                isc::dhcp::SrvConfigType::CONFIG_DATABASE;
            isc::db::DbAccessParser parser;
            std::string access_string;
            parser.parse(access_string, credentials_config.value());
            isc::dhcp::CfgMgr::instance()
                .getStagingCfg()
                ->getCfgDbAccess()
                ->setShardCfgDbAccessString(access_string);

            valid = true;
            break;
        }
    }

    if (!valid) {
        isc_throw(isc::BadValue, PRETTY_FUNCTION_NAME() << ": no credentials configuration");
    }

    cfg_db_access.setAppendedParameters("universe=" + dhcpSpaceToString<D>());
    CfgMgr::instance().setFamily(dhcpSpaceToInetFamily<D>());

    // Recreate server configuration manager.
    ShardConfigMgrFactory<D>::destroy();
    ShardConfigMgrFactory<D>::create(cfg_db_access.getShardCfgDbAccessString());
    SubnetMgrFactory<D>::destroy();
    SubnetMgrFactory<D>::create(cfg_db_access.getShardCfgDbAccessString());

    HostMgr::instance();
    HostMgr::delAllBackends();
    HostMgr::addBackend(cfg_db_access.getShardCfgDbAccessString());
}

template <>
void CfgDbAccess::createShardCfgMgrs<DHCP_SPACE_V4>(
    std::vector<std::optional<isc::data::ElementPtr>> const& credential_configurations) {
    shardCfgMgrs<DHCP_SPACE_V4>(*this, credential_configurations);
}

template <>
void CfgDbAccess::createShardCfgMgrs<DHCP_SPACE_V6>(
    std::vector<std::optional<isc::data::ElementPtr>> const& credential_configurations) {
    shardCfgMgrs<DHCP_SPACE_V6>(*this, credential_configurations);
}

}  // namespace dhcp
}  // namespace isc

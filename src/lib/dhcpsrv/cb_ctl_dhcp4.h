// Copyright (C) 2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef CB_CTL_DHCP4_H
#define CB_CTL_DHCP4_H

#include <dhcpsrv/cb_ctl_dhcp.h>
#include <dhcpsrv/config_backend_dhcp4_mgr.h>
#include <dhcpsrv/srv_config.h>

namespace isc {
namespace dhcp {

/// @brief Implementation of the mechanisms to control the use of
/// the Configuration Backends by the DHCPv4 server.
///
/// It implements fetching and merging DHCPv4 server configuration from
/// the database into the staging or current configuration.
///
/// @tparam ConfigBackendMgrType Type of the Config Backend Manager used
/// by the server implementing this class. For example, for the DHCPv4
/// server it will be @c ConfigBackendDHCPv4Mgr.
class CBControlDHCPv4 : public CBControlDHCP<ConfigBackendDHCPv4Mgr> {
protected:

    /// @brief DHCPv4 server specific method to fetch and apply back end
    /// configuration into the local configuration.
    ///
    /// @param backend_selector Backend selector.
    /// @param server_selector Server selector.
    /// @param lb_modification_time Lower bound modification time for the
    /// configuration elements to be fetched.
    /// @param audit_entries Audit entries fetched from the database since
    /// the last configuration update. This collection is empty if there
    /// were no updates.
    virtual void databaseConfigApply(const db::BackendSelector& backend_selector,
                                     const db::ServerSelector& server_selector,
                                     const boost::posix_time::ptime& lb_modification_time,
                                     const db::AuditEntryCollection& audit_entries);
};

typedef std::shared_ptr<CBControlDHCPv4> CBControlDHCPv4Ptr;

} // end of namespace isc::dhcp
} // end of namespace isc

#endif // CB_CTL_DHCP4_H

// Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef ISC_YANG_REVISIONS_H
#define ISC_YANG_REVISIONS_H 1

#include <string>
#include <unordered_map>

namespace isc {
namespace yang {

// Table of module name / revision.
static const std::unordered_map<std::string, std::string> YANG_REVISIONS = {
    { "dt-kea-dhcp4-distributed", "2020-01-30" },
    { "dt-kea-dhcp6-distributed", "2020-01-30" },
    { "ietf-dhcpv6-client", "2020-05-26" },
    { "ietf-dhcpv6-common", "2020-05-26" },
    { "ietf-dhcpv6-relay", "2020-05-26" },
    { "ietf-dhcpv6-server", "2020-05-26" },
    { "ietf-inet-types", "2013-07-15" },
    { "ietf-interfaces", "2018-02-20" },
    { "ietf-yang-types", "2013-07-15" },
#ifdef KEATEST_MODULE
    { "keatest-module", "2018-11-20" },
#endif  // KEATEST_MODULE
    { "kea-ctrl-agent", "2019-08-12" },
    { "kea-dhcp4-server", "2019-08-12" },
    { "kea-dhcp6-server", "2019-08-12" },
    { "kea-dhcp-ddns", "2019-08-12" },
    { "kea-dhcp-types", "2019-08-12" },
    { "kea-types", "2019-08-12" }};

}  // namespace yang
}  // namespace isc

#endif  // ISC_YANG_REVISIONS_H

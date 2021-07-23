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

#include <config.h>

#include <dhcpsrv/cql_subnet_mgr.h>

using namespace isc::db;
using namespace isc::dhcp;

namespace isc {
namespace dhcp {

template <>
void CqlSubnet4Exchange::bindDelta(AnyArray& data, Subnet4Info& subnet) {
    data.add(&subnet.strings_.boot_file_name_);
    data.add(&subnet.match_client_id_);
    data.add(&subnet.strings_.next_server_);
    data.add(&subnet.strings_.server_hostname_);
    data.add(&subnet.strings_.subnet_4o6_interface_);
    data.add(&subnet.strings_.subnet_4o6_interface_id_);
    data.add(&subnet.strings_.subnet_4o6_subnet_);
    data.add(&subnet.subnet_v4_psid_offset_);
    data.add(&subnet.subnet_v4_psid_len_);
    data.add(&subnet.strings_.subnet_v4_excluded_psids_);
}

template <>
void CqlSubnet6Exchange::bindDelta(AnyArray& data, Subnet6Info& subnet) {
    data.add(&subnet.strings_.pd_pools_list_);
    data.add(&subnet.preferred_lifetime_);
}

}  // namespace dhcp
}  // namespace isc

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

#include <dhcpsrv/tests/generic_subnet_mgr_unittest.h>

namespace isc {
namespace dhcp {
namespace test {

template <>
std::array<std::string, 8> const GenericSubnet4MgrTest::generateSubnets() {
    return {"192.0.0.0/24", "192.0.1.0/24", "192.0.2.0/24", "192.0.3.0/24",
            "192.0.4.0/24", "192.0.5.0/24", "192.0.6.0/24", "192.0.7.0/24"};
}

template <>
std::array<std::string, 8> const GenericSubnet6MgrTest::generateSubnets() {
    return {"2001:db8:0::/48", "2001:db8:1::/48", "2001:db8:2::/48", "2001:db8:3::/48",
            "2001:db8:4::/48", "2001:db8:5::/48", "2001:db8:6::/48", "2001:db8:7::/48"};
}

}  // namespace test
}  // namespace dhcp
}  // namespace isc

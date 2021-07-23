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

#ifndef MEMFILE_SUBNET_MGR_H
#define MEMFILE_SUBNET_MGR_H

#include <database/database_connection.h>
#include <dhcpsrv/subnet_mgr.h>

namespace isc {
namespace dhcp {

template <DhcpSpaceType D>
struct MemfileSubnetMgr : public SubnetMgr<D> {
    explicit MemfileSubnetMgr(db::DatabaseConnection::ParameterMap const& /* parameters */) {
    }

    void del(SubnetInfoPtr<D>& /* subnet */,
             std::string const& /* config_timestamp_path */) override final {
    }

    void insert(SubnetInfoPtr<D>& /* subnet */,
                std::string const& /* config_timestamp_path */) override final {
    }

    SubnetInfoCollection<D> select(std::string const& /* config_timestamp_path */) override final {
        SubnetInfoCollection<D> todo;
        return todo;
    }

    SubnetInfoPtr<D> selectBySubnet(std::string& /* subnet */,
                                    std::string const& /* config_timestamp_path */) override final {
        SubnetInfoPtr<D> todo;
        return todo;
    }

    SubnetInfoPtr<D> selectBySubnetID(SubnetID) override final {
        SubnetInfoPtr<D> todo;
        return todo;
    };

    void update(SubnetInfoPtr<D>& /* subnet */,
                std::string const& /* config_timestamp_path */) override final {
    }

    std::string getName() const override final {
        return "memory";
    }

    std::string getType() const override final {
        return "memfile";
    }

    SubnetID maxSubnetID() override final {
        return 0;
    }

    void commit() override final {
    }

    void rollback() override final {
    }

private:
    static int constexpr MAJOR_VERSION = 1;
    static int constexpr MINOR_VERSION = 0;

    /// @brief subnet information
    SubnetInfo<D> subnet_;
};
using MemfileSubnet4Mgr = MemfileSubnetMgr<DHCP_SPACE_V4>;
using MemfileSubnet6Mgr = MemfileSubnetMgr<DHCP_SPACE_V6>;

}  // namespace dhcp
}  // namespace isc

#endif  // MEMFILE_SUBNET_MGR_H

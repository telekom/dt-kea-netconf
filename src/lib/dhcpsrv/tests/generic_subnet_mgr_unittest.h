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

#ifndef GENERIC_SUBNET_MGR_UNITTEST_H
#define GENERIC_SUBNET_MGR_UNITTEST_H

#include <database/database_connection.h>
#include <database/db_exceptions.h>
#include <dhcpsrv/subnet_id.h>
#include <dhcpsrv/subnet_mgr.h>
#include <util/filesystem.h>

#include <gtest/gtest.h>

#include <boost/lexical_cast.hpp>

#include <iterator>
#include <string>
#include <type_traits>
#include <vector>

namespace isc {
namespace dhcp {
namespace test {

/// @brief Test Fixture class with utility functions for SubnetMgr backends
///
/// It contains utility functions, like dummy subnet creation.
/// All concrete SubnetMgr test classes should be derived from it.
template <DhcpSpaceType D>
struct GenericSubnetMgrTest : public ::testing::Test {
    /// @brief Default constructor
    GenericSubnetMgrTest()
        : subnets_(generateSubnets()),
          config_timestamp_("/tmp/test-config.timestamp", false, std::string()), smptr_(NULL) {
    }

    /// @brief Virtual destructor
    ///
    /// Does nothing. The derived classes are expected to clean up, i.e.
    /// remove the smptr_ pointer.
    virtual ~GenericSubnetMgrTest() = default;

    /// @brief Reopen the database
    ///
    /// Closes the database and re-opens it. It must be implemented
    /// in derived classes.
    virtual void reopen() = 0;

    /// @brief Initialize Subnet Fields
    ///
    /// Returns a pointer to a Subnet structure.  Different values are put into
    /// the subnet according to the address passed.
    ///
    /// This is just a convenience function for the test methods.
    ///
    /// @param subnet Address to use for the initialization
    ///
    /// @return SubnetInfoPtr<D>.  This will not point to anything if the
    ///         initialization failed (e.g. unknown address).
    SubnetInfoPtr<D> initialize(std::string subnet) {
        SubnetInfoPtr<D> s(std::make_shared<SubnetInfo<D>>());

        for (size_t i = 0; i < subnets_.size(); ++i) {
            if (subnet == subnets_[i]) {
                size_t const value = i ? (i <= 2 ? 1 : i) : 0;
                std::string const& value_str(boost::lexical_cast<std::string>(value));
                s->valid_lifetime_ = 3 * value;
                // -> ignore timestamp
                s->renew_timer_ = value;
                s->rebind_timer_ = 2 * value;
                s->id_ = value;
                s->rapid_commit_ = (value % 2 != 0);
                s->strings_.interface_ = "interface" + value_str;
                s->strings_.interface_id_ = "interface_id" + value_str;
                s->strings_.require_client_classes_ = "class" + value_str;
                break;
            }
        }

        s->strings_.subnet_ = subnet;
        return s;
    }

    /// @brief Check that subnets are present and return comparison result.
    ///
    /// Checks a vector of subnet pointers and ensures that all the subnets
    /// they point to are present and different.  If not, a GTest assertion
    /// will fail.
    ///
    /// @param subnets Vector of pointers to subnets
    void compare(std::vector<SubnetInfoPtr<D>> const& subnets) const {
        // Check they were created and are all the same.
        SubnetInfoPtr<D> reference_subnet;
        for (SubnetInfoPtr<D> const& subnet : subnets) {
            ASSERT_TRUE(subnet);
            if (reference_subnet) {
                EXPECT_TRUE(*subnet == *reference_subnet);
            } else {
                reference_subnet = subnet;
            }
        }
    }

    std::vector<SubnetInfoPtr<D>> order(std::vector<SubnetInfoPtr<D>> const& subnets) const {
        struct {
            bool operator()(SubnetInfoPtr<D> a, SubnetInfoPtr<D> b) const {
                return (strcmp(a->strings_.subnet_.c_str(), b->strings_.subnet_.c_str()) < 0);
            }
        } less;
        std::vector<SubnetInfoPtr<D>> sorted = subnets;
        std::sort(sorted.begin(), sorted.end(), less);
        return sorted;
    }

    void testDel() {
        SubnetInfoCollection<D> subnets;

        // Insert the first valid subnet.
        subnets.push_back(initialize(subnets_[2]));
        SubnetInfoPtr<D> subnet = subnets.back();
        assert_no_throw(smptr_->insert(subnet, config_timestamp_()));
        SubnetInfoCollection<D> returned = order(smptr_->select(config_timestamp_()));
        ASSERT_EQ(1, returned.size());
        compare({subnet, returned.back()});

        // Insert the second valid subnet.
        subnets.push_back(initialize(subnets_[3]));
        subnet = subnets.back();
        assert_no_throw(smptr_->insert(subnet, config_timestamp_()));
        returned = order(smptr_->select(config_timestamp_()));
        ASSERT_EQ(2, returned.size());
        compare({subnet, returned.back()});

        // Delete the second valid subnet.
        expect_no_throw(smptr_->del(subnet, config_timestamp_()));
        returned = order(smptr_->select(config_timestamp_()));
        ASSERT_EQ(1, returned.size());
        compare({subnets.front(), returned.front()});

        // Delete the second valid subnet again.
        EXPECT_THROW(smptr_->del(subnet, config_timestamp_()), db::DbOperationError);
        returned = order(smptr_->select(config_timestamp_()));
        ASSERT_EQ(1, returned.size());
        compare({subnets.front(), returned.front()});
    }

    void testInsert() {
        SubnetInfoCollection<D> subnets;

        // Insert the first valid subnet.
        subnets.push_back(initialize(subnets_[2]));
        SubnetInfoPtr<D> subnet = subnets.back();
        expect_no_throw(smptr_->insert(subnet, config_timestamp_()));
        SubnetInfoCollection<D> returned = order(smptr_->select(config_timestamp_()));
        ASSERT_EQ(1, returned.size());
        compare({subnet, returned.back()});

        // Insert the second valid subnet.
        subnets.push_back(initialize(subnets_[3]));
        subnet = subnets.back();
        expect_no_throw(smptr_->insert(subnet, config_timestamp_()));
        returned = order(smptr_->select(config_timestamp_()));
        ASSERT_EQ(2, returned.size());
        compare({subnet, returned.back()});

        // Insert the second valid subnet again.
        subnets.push_back(initialize(subnets_[3]));
        subnet = subnets.back();
        EXPECT_THROW(smptr_->insert(subnet, config_timestamp_()), db::DuplicateEntry);
        returned = order(smptr_->select(config_timestamp_()));
        ASSERT_EQ(2, returned.size());
        compare({subnet, returned.back()});
    }

    void testSelect() {
        SubnetInfoCollection<D> subnets;

        // Insert the first valid subnet.
        subnets.push_back(initialize(subnets_[2]));
        SubnetInfoPtr<D> subnet = subnets.back();
        assert_no_throw(smptr_->insert(subnet, config_timestamp_()));
        SubnetInfoCollection<D> returned = order(smptr_->select(config_timestamp_()));
        ASSERT_EQ(1, returned.size());
        compare({subnet, returned.back()});

        // Insert the second valid subnet.
        subnets.push_back(initialize(subnets_[3]));
        subnet = subnets.back();
        assert_no_throw(smptr_->insert(subnet, config_timestamp_()));
        returned = order(smptr_->select(config_timestamp_()));
        ASSERT_EQ(2, returned.size());
        compare({subnet, returned.back()});

        // Select the second valid subnet.
        SubnetInfoPtr<D> returned_one(smptr_->selectBySubnet(subnet, config_timestamp_()));
        compare({subnet, returned_one});
    }

    void testUpdate() {
        SubnetInfoCollection<D> subnets;

        // Insert the first valid subnet.
        subnets.push_back(initialize(subnets_[2]));
        SubnetInfoPtr<D> subnet = subnets.back();
        assert_no_throw(smptr_->insert(subnet, config_timestamp_()));
        SubnetInfoCollection<D> returned = order(smptr_->select(config_timestamp_()));
        ASSERT_EQ(1, returned.size());
        compare({subnet, returned.back()});

        // Update the first valid subnet.
        subnet->valid_lifetime_ += 1;
        assert_no_throw(smptr_->update(subnet, config_timestamp_()));
        returned = order(smptr_->select(config_timestamp_()));
        ASSERT_EQ(1, returned.size());
        compare({subnet, returned.back()});
    }

    std::array<std::string, 8> const generateSubnets();

    std::array<std::string, 8> const subnets_;

    isc::util::File config_timestamp_;

    /// @brief Pointer to the subnet manager
    SubnetMgr<D>* smptr_;
};
using GenericSubnet4MgrTest = GenericSubnetMgrTest<DHCP_SPACE_V4>;
using GenericSubnet6MgrTest = GenericSubnetMgrTest<DHCP_SPACE_V6>;

}  // namespace test
}  // namespace dhcp
}  // namespace isc

#endif

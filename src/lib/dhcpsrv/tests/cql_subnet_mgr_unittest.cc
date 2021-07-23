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

#include <cql/testutils/cql_schema.h>

#include <database/database_connection.h>
#include <database/testutils/schema.h>

#include <dhcpsrv/cql_subnet_mgr.h>
#include <dhcpsrv/subnet_mgr_factory.h>
#include <dhcpsrv/tests/generic_subnet_mgr_unittest.h>
#include <dhcpsrv/tests/test_utils.h>

#include <gtest/gtest.h>

using namespace isc::db;
using namespace isc::db::test;
using namespace isc::dhcp;
using namespace isc::dhcp::test;

using isc::InvalidParameter;
using std::string;

namespace {

/// @brief Test fixture class for testing Cassandra Subnet Manager
///
/// Opens the database prior to each test and closes it afterwards.
/// All pending transactions are deleted prior to closure.
template <DhcpSpaceType D>
struct CqlSubnetMgrTest : GenericSubnetMgrTest<D> {
    /// @brief Clears the database and opens connection to it.
    void initializeTest() {
        // Ensure schema is the correct one.
        createCqlSchema();

        // Connect to the database.
        try {
            SubnetMgrFactory<D>::create(validCqlConnectionString());
        } catch (...) {
            std::cerr << "*** ERROR: unable to open database. The test\n"
                         "*** environment is broken and must be fixed before\n"
                         "*** the CQL tests will run correctly.\n"
                         "*** The reason for the problem is described in the\n"
                         "*** accompanying exception output.\n";
            throw;
        }

        this->smptr_ = &SubnetMgrFactory<D>::instance();
    }

    /// @brief Destroys the LM and the schema.
    void destroyTest() {
        try {
            this->smptr_->rollback();
        } catch (...) {
            // Rollback may fail if backend is in read only mode. That's ok.
        }
        SubnetMgrFactory<D>::destroy();
        destroyCqlSchema();
    }

    /// @brief Constructor
    ///
    /// Deletes everything from the database and opens it.
    CqlSubnetMgrTest() {
        initializeTest();
    }

    /// @brief Destructor
    ///
    /// Rolls back all pending transactions.  The deletion of GenericSubnetMgrTest<D>::smptr_ will
    /// close the database.  Then reopen it and delete everything created by the test.
    virtual ~CqlSubnetMgrTest() {
        destroyTest();
    }

    /// @brief Reopen the database
    ///
    /// Closes the database and re-open it.  Anything committed should be
    /// visible.
    ///
    /// Parameter is ignored for CQL backend as the v4 and v6 subnets share
    /// the same database.
    void reopen() {
        SubnetMgrFactory<D>::destroy();
        SubnetMgrFactory<D>::create(validCqlConnectionString());
        this->smptr_ = &SubnetMgrFactory<D>::instance();
    }
};
using CqlSubnet4MgrTest = CqlSubnetMgrTest<DHCP_SPACE_V4>;
using CqlSubnet6MgrTest = CqlSubnetMgrTest<DHCP_SPACE_V6>;

/// @brief Check that database can be opened
///
/// This test checks if the CqlSubnetMgr can be instantiated.  This happens
/// only if the database can be opened.  Note that this is not part of the
/// CqlSubnetMgr test fixure set.  This test checks that the database can be
/// opened: the fixtures assume that and check basic operations.
template <DhcpSpaceType D>
void testCqlOpen() {
    // Schema needs to be created for the test to work.
    createCqlSchema();

    // Check that subnet manager open the database opens correctly and tidy up.
    // If it fails, print the error message.
    try {
        SubnetMgrFactory<D>::create(validCqlConnectionString());
        EXPECT_NO_THROW((void)SubnetMgrFactory<D>::instance());
        SubnetMgrFactory<D>::destroy();
    } catch (const isc::Exception& ex) {
        FAIL() << "*** ERROR: unable to open database, reason:\n"
               << "    " << ex.what() << "\n"
               << "*** The test environment is broken and must be fixed\n"
               << "*** before the CQL tests will run correctly.\n";
    }

    // Check that subnet manager open the database opens correctly with a longer
    // timeout.  If it fails, print the error message.
    try {
        // CQL specifies the timeout values in ms, not seconds. Therefore
        // we need to add extra 000 to the "connect-timeout=10" string.
        string connection_string =
            validCqlConnectionString() + string(" ") + string(VALID_TIMEOUT) + "000";
        SubnetMgrFactory<D>::create(connection_string);
        EXPECT_NO_THROW((void)SubnetMgrFactory<D>::instance());
        SubnetMgrFactory<D>::destroy();
    } catch (const isc::Exception& ex) {
        FAIL() << "*** ERROR: unable to open database, reason:\n"
               << "    " << ex.what() << "\n"
               << "*** The test environment is broken and must be fixed\n"
               << "*** before the CQL tests will run correctly.\n";
    }

    // Check that attempting to get an instance of the subnet manager when
    // none is set throws an exception.
    EXPECT_THROW(SubnetMgrFactory<D>::instance(), NoSubnetManager);

    // Check that wrong specification of backend throws an exception.
    // (This is really a check on SubnetMgrFactory<D>, but is convenient to
    // perform here.)
    EXPECT_THROW(SubnetMgrFactory<D>::create(
                     connectionString(NULL, VALID_NAME, VALID_HOST, INVALID_USER, VALID_PASSWORD)),
                 InvalidParameter);

    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(INVALID_TYPE, VALID_NAME, VALID_HOST,
                                                              VALID_USER, VALID_PASSWORD)),
                 InvalidType);

    // Check that invalid login data does not cause an exception, CQL should use
    // default values.
    EXPECT_NO_THROW(SubnetMgrFactory<D>::create(
        connectionString(CQL_VALID_TYPE, INVALID_NAME, VALID_HOST, VALID_USER, VALID_PASSWORD)));

    EXPECT_NO_THROW(SubnetMgrFactory<D>::create(
        connectionString(CQL_VALID_TYPE, VALID_NAME, INVALID_HOST, VALID_USER, VALID_PASSWORD)));

    EXPECT_NO_THROW(SubnetMgrFactory<D>::create(
        connectionString(CQL_VALID_TYPE, VALID_NAME, VALID_HOST, INVALID_USER, VALID_PASSWORD)));

    EXPECT_NO_THROW(SubnetMgrFactory<D>::create(
        connectionString(CQL_VALID_TYPE, VALID_NAME, VALID_HOST, VALID_USER, INVALID_PASSWORD)));

    // Check for invalid timeouts
    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(CQL_VALID_TYPE, VALID_NAME,
                                                              VALID_HOST, VALID_USER,
                                                              VALID_PASSWORD, INVALID_TIMEOUT_1)),
                 DbOperationError);

    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(CQL_VALID_TYPE, VALID_NAME,
                                                              VALID_HOST, VALID_USER,
                                                              VALID_PASSWORD, INVALID_TIMEOUT_2)),
                 DbOperationError);

    // Check for missing parameters
    EXPECT_NO_THROW(SubnetMgrFactory<D>::create(
        connectionString(CQL_VALID_TYPE, NULL, VALID_HOST, INVALID_USER, VALID_PASSWORD)));

    // Check that invalid login data does not cause an exception, CQL should use
    // default values.
    EXPECT_NO_THROW(SubnetMgrFactory<D>::create(
        connectionString(CQL_VALID_TYPE, INVALID_NAME, VALID_HOST, VALID_USER, VALID_PASSWORD)));

    EXPECT_NO_THROW(SubnetMgrFactory<D>::create(
        connectionString(CQL_VALID_TYPE, VALID_NAME, INVALID_HOST, VALID_USER, VALID_PASSWORD)));

    EXPECT_NO_THROW(SubnetMgrFactory<D>::create(
        connectionString(CQL_VALID_TYPE, VALID_NAME, VALID_HOST, INVALID_USER, VALID_PASSWORD)));

    EXPECT_NO_THROW(SubnetMgrFactory<D>::create(
        connectionString(CQL_VALID_TYPE, VALID_NAME, VALID_HOST, VALID_USER, INVALID_PASSWORD)));

    // Check that invalid timeouts throw DbOperationError.
    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(CQL_VALID_TYPE, VALID_NAME,
                                                              VALID_HOST, VALID_USER,
                                                              VALID_PASSWORD, INVALID_TIMEOUT_1)),
                 DbOperationError);
    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(CQL_VALID_TYPE, VALID_NAME,
                                                              VALID_HOST, VALID_USER,
                                                              VALID_PASSWORD, INVALID_TIMEOUT_2)),
                 DbOperationError);

    // Check that CQL allows the hostname to not be specified.
    EXPECT_NO_THROW(SubnetMgrFactory<D>::create(
        connectionString(CQL_VALID_TYPE, NULL, VALID_HOST, INVALID_USER, VALID_PASSWORD)));

    // Tidy up after the test
    destroyCqlSchema();
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// SUBNET4 ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Check the getType() method
///
/// getType() returns a string giving the type of the backend, which should
/// always be "cql".
TEST_F(CqlSubnet4MgrTest, getType) {
    EXPECT_EQ(string("cql"), smptr_->getType());
}

/// @brief Check getName() returns correct database name
TEST_F(CqlSubnet4MgrTest, getName) {
    EXPECT_EQ(string("keatest"), smptr_->getName());
}

TEST(CqlOpenTest, OpenDatabase4) {
    testCqlOpen<DHCP_SPACE_V4>();
}

TEST_F(CqlSubnet4MgrTest, testDel) {
    testDel();
}

TEST_F(CqlSubnet4MgrTest, testInsert) {
    testInsert();
}

TEST_F(CqlSubnet4MgrTest, testSelect) {
    testSelect();
}

TEST_F(CqlSubnet4MgrTest, testUpdate) {
    testUpdate();
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// SUBNET6 ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Check the getType() method
///
/// getType() returns a string giving the type of the backend, which should
/// always be "cql".
TEST_F(CqlSubnet6MgrTest, getType) {
    EXPECT_EQ(string("cql"), smptr_->getType());
}

/// @brief Check getName() returns correct database name
TEST_F(CqlSubnet6MgrTest, getName) {
    EXPECT_EQ(string("keatest"), smptr_->getName());
}

TEST(CqlOpenTest, OpenDatabase6) {
    testCqlOpen<DHCP_SPACE_V6>();
}

TEST_F(CqlSubnet6MgrTest, testDel) {
    testDel();
}

TEST_F(CqlSubnet6MgrTest, testInsert) {
    testInsert();
}

TEST_F(CqlSubnet6MgrTest, testSelect) {
    testSelect();
}

TEST_F(CqlSubnet6MgrTest, testUpdate) {
    testUpdate();
}

}  // namespace

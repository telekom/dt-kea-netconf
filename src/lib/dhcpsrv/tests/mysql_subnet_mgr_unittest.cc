// Copyright (C) 2012-2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <database/testutils/schema.h>

#include <dhcpsrv/mysql_subnet_mgr.h>
#include <dhcpsrv/subnet_mgr_factory.h>
#include <dhcpsrv/tests/generic_subnet_mgr_unittest.h>
#include <dhcpsrv/tests/test_utils.h>

#include <mysql/testutils/mysql_schema.h>

#include <gtest/gtest.h>

using namespace isc::db;
using namespace isc::db::test;
using namespace isc::dhcp;
using namespace isc::dhcp::test;

using isc::InvalidParameter;
using std::string;

namespace {

/// @brief Test fixture class for testing MySQL Subnet Manager
///
/// Opens the database prior to each test and closes it afterwards.
/// All pending transactions are deleted prior to closure.
template <DhcpSpaceType D>
class MySqlSubnetMgrTest : public GenericSubnetMgrTest<D> {
public:
    /// @brief Clears the database and opens connection to it.
    void initializeTest() {
        // Ensure schema is the correct one.
        destroyMySQLSchema();
        createMySQLSchema();

        // Connect to the database.
        try {
            SubnetMgrFactory<D>::create(validMySQLConnectionString());
        } catch (...) {
            std::cerr << "*** ERROR: unable to open database. The test\n"
                         "*** environment is broken and must be fixed before\n"
                         "*** the MySQL tests will run correctly.\n"
                         "*** The reason for the problem is described in the\n"
                         "*** accompanying exception output.\n";
            throw;
        }

        this->smptr_ = &(SubnetMgrFactory<D>::instance());
    }

    /// @brief Destroys the LM and the schema.
    void destroyTest() {
        try {
            this->smptr_->rollback();
        } catch (...) {
            // Rollback may fail if backend is in read only mode. That's ok.
        }
        SubnetMgrFactory<D>::destroy();
        destroyMySQLSchema();
    }

    /// @brief Constructor
    ///
    /// Deletes everything from the database and opens it.
    MySqlSubnetMgrTest() {
        initializeTest();
    }

    /// @brief Destructor
    ///
    /// Rolls back all pending transactions.  The deletion of smptr_ will close
    /// the database.  Then reopen it and delete everything created by the test.
    virtual ~MySqlSubnetMgrTest() {
        destroyTest();
    }

    /// @brief Reopen the database
    ///
    /// Closes the database and re-open it.  Anything committed should be
    /// visible.
    ///
    /// Parameter is ignored for MySQL backend as the v4 and v6 subnets share
    /// the same database.
    void reopen() {
        SubnetMgrFactory<D>::destroy();
        SubnetMgrFactory<D>::create(validMySQLConnectionString());
        this->smptr_ = &SubnetMgrFactory<D>::instance();
    }
};
using MySqlSubnet4MgrTest = MySqlSubnetMgrTest<DHCP_SPACE_V4>;
using MySqlSubnet6MgrTest = MySqlSubnetMgrTest<DHCP_SPACE_V6>;

/// @brief Check that database can be opened
///
/// This test checks if the MySqlSubnetMgr can be instantiated.  This happens
/// only if the database can be opened.  Note that this is not part of the
/// MySqlSubnetMgr test fixure set.  This test checks that the database can be
/// opened: the fixtures assume that and check basic operations.
template <DhcpSpaceType D>
void testMySqlOpen() {
    // Schema needs to be created for the test to work.
    destroyMySQLSchema();
    createMySQLSchema();

    // Check that subnet manager open the database opens correctly and tidy up.
    // If it fails, print the error message.
    try {
        SubnetMgrFactory<D>::create(validMySQLConnectionString());
        EXPECT_NO_THROW((void)SubnetMgrFactory<D>::instance());
        SubnetMgrFactory<D>::destroy();
    } catch (const isc::Exception& ex) {
        FAIL() << "*** ERROR: unable to open database, reason:\n"
               << "    " << ex.what() << "\n"
               << "*** The test environment is broken and must be fixed\n"
               << "*** before the MySQL tests will run correctly.\n";
    }

    // Check that subnet manager open the database opens correctly with a longer
    // timeout.  If it fails, print the error message.
    try {
        string connection_string =
            validMySQLConnectionString() + string(" ") + string(VALID_TIMEOUT);
        SubnetMgrFactory<D>::create(connection_string);
        EXPECT_NO_THROW((void)SubnetMgrFactory<D>::instance());
        SubnetMgrFactory<D>::destroy();
    } catch (const isc::Exception& ex) {
        FAIL() << "*** ERROR: unable to open database, reason:\n"
               << "    " << ex.what() << "\n"
               << "*** The test environment is broken and must be fixed\n"
               << "*** before the MySQL tests will run correctly.\n";
    }

    // Check that attempting to get an instance of the subnet manager when
    // none is set throws an exception.
    EXPECT_THROW(SubnetMgrFactory<D>::instance(), NoSubnetManager);

    // Check that wrong specification of backend throws an exception.
    // (This is really a check on SubnetMgrFactory, but is convenient to
    // perform here.)
    EXPECT_THROW(SubnetMgrFactory<D>::create(
                     connectionString(NULL, VALID_NAME, VALID_HOST, INVALID_USER, VALID_PASSWORD)),
                 InvalidParameter);

    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(INVALID_TYPE, VALID_NAME, VALID_HOST,
                                                              VALID_USER, VALID_PASSWORD)),
                 InvalidType);

    // Check that invalid login data causes an exception.
    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(
                     MYSQL_VALID_TYPE, INVALID_NAME, VALID_HOST, VALID_USER, VALID_PASSWORD)),
                 DbOpenError);

#ifndef OS_OSX
    // Under MacOS, connecting with an invalid host can cause a TCP/IP socket
    // to be orphaned and never closed.  This can interfere with subsequent tests
    // which attempt to locate and manipulate MySQL client socket descriptor.
    // In the interests of progress, we'll just avoid this test.
    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(
                     MYSQL_VALID_TYPE, VALID_NAME, INVALID_HOST, VALID_USER, VALID_PASSWORD)),
                 DbOpenError);
#endif

    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(
                     MYSQL_VALID_TYPE, VALID_NAME, VALID_HOST, INVALID_USER, VALID_PASSWORD)),
                 DbOpenError);

    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(
                     MYSQL_VALID_TYPE, VALID_NAME, VALID_HOST, VALID_USER, INVALID_PASSWORD)),
                 DbOpenError);

    // Check for invalid timeouts
    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(MYSQL_VALID_TYPE, VALID_NAME,
                                                              VALID_HOST, VALID_USER,
                                                              VALID_PASSWORD, INVALID_TIMEOUT_1)),
                 DbInvalidTimeout);

    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(MYSQL_VALID_TYPE, VALID_NAME,
                                                              VALID_HOST, VALID_USER,
                                                              VALID_PASSWORD, INVALID_TIMEOUT_2)),
                 DbInvalidTimeout);

    // Check for missing parameters
    EXPECT_THROW(SubnetMgrFactory<D>::create(connectionString(MYSQL_VALID_TYPE, NULL, VALID_HOST,
                                                              INVALID_USER, VALID_PASSWORD)),
                 NoDatabaseName);

    // Tidy up after the test
    destroyMySQLSchema();
    SubnetMgrFactory<D>::destroy();
}

/// @brief Check the getType() method
///
/// getType() returns a string giving the type of the backend, which should
/// always be "mysql".
TEST_F(MySqlSubnet4MgrTest, getType) {
    EXPECT_EQ(string("mysql"), smptr_->getType());
}

/// @brief Check getName() returns correct database name
TEST_F(MySqlSubnet4MgrTest, getName) {
    EXPECT_EQ(string("keatest"), smptr_->getName());
}

TEST(MySqlOpenTest, OpenDatabase4) {
    testMySqlOpen<DHCP_SPACE_V4>();
}

TEST_F(MySqlSubnet4MgrTest, testDel) {
    testDel();
}

TEST_F(MySqlSubnet4MgrTest, testInsert) {
    testInsert();
}

TEST_F(MySqlSubnet4MgrTest, testSelect) {
    testSelect();
}

TEST_F(MySqlSubnet4MgrTest, testUpdate) {
    testUpdate();
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// SUBNET6 ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/// @brief Check the getType() method
///
/// getType() returns a string giving the type of the backend, which should
/// always be "mysql".
TEST_F(MySqlSubnet6MgrTest, getType) {
    EXPECT_EQ(string("mysql"), smptr_->getType());
}

/// @brief Check getName() returns correct database name
TEST_F(MySqlSubnet6MgrTest, getName) {
    EXPECT_EQ(string("keatest"), smptr_->getName());
}

TEST(MySqlOpenTest, OpenDatabase6) {
    testMySqlOpen<DHCP_SPACE_V6>();
}

TEST_F(MySqlSubnet6MgrTest, testDel) {
    testDel();
}

TEST_F(MySqlSubnet6MgrTest, testInsert) {
    testInsert();
}

TEST_F(MySqlSubnet6MgrTest, testSelect) {
    testSelect();
}

TEST_F(MySqlSubnet6MgrTest, testUpdate) {
    testUpdate();
}

}  // namespace

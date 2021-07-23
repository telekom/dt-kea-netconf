// Copyright (C) 2019-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>
#include <mysql_cb_dhcp6.h>
#include <asiolink/addr_utilities.h>
#include <database/db_exceptions.h>
#include <database/testutils/schema.h>
#include <dhcp/dhcp6.h>
#include <dhcp/libdhcp++.h>
#include <dhcp/option6_addrlst.h>
#include <dhcp/option_int.h>
#include <dhcp/option_space.h>
#include <dhcp/option_string.h>
#include <dhcpsrv/pool.h>
#include <dhcpsrv/subnet.h>
#include <dhcpsrv/testutils/mysql_generic_backend_unittest.h>
#include <mysql/testutils/mysql_schema.h>
#include <gtest/gtest.h>
#include <mysql.h>
#include <map>
#include <sstream>

using namespace isc::asiolink;
using namespace isc::db;
using namespace isc::db::test;
using namespace isc::data;
using namespace isc::dhcp;
using namespace isc::dhcp::test;

namespace {

/// @brief Test implementation of the MySQL configuration backend.
///
/// It exposes protected members of the @c MySqlConfigBackendDHCPv6.
class TestMySqlConfigBackendDHCPv6 : public MySqlConfigBackendDHCPv6 {
public:

    /// @brief Constructor.
    ///
    /// @param parameters A data structure relating keywords and values
    /// concerned with the database.
    explicit TestMySqlConfigBackendDHCPv6(const DatabaseConnection::ParameterMap& parameters)
        : MySqlConfigBackendDHCPv6(parameters) {
    }

    using MySqlConfigBackendDHCPv6::base_impl_;

};

/// @brief Test fixture class for @c MySqlConfigBackendDHCPv6.
///
/// @todo The tests we're providing here only test cases when the
/// server selector is set to 'ALL' (configuration elements belong to
/// all servers). Currently we have no API to insert servers into
/// the database, and therefore we can't test the case when
/// configuration elements are assigned to particular servers by
/// server tags. We will have to expand existing tests when
/// the API is extended allowing for inserting servers to the
/// database.
class MySqlConfigBackendDHCPv6Test : public MySqlGenericBackendTest {
public:

    /// @brief Constructor.
    MySqlConfigBackendDHCPv6Test()
        : test_subnets_(), test_networks_(), test_option_defs_(),
          test_options_(), test_servers_(), timestamps_(), cbptr_(),
          audit_entries_() {
        // Ensure we have the proper schema with no transient data.
        createMySQLSchema();

        try {
            // Create MySQL connection and use it to start the backend.
            DatabaseConnection::ParameterMap params =
                DatabaseConnection::parse(validMySQLConnectionString());
            cbptr_.reset(new TestMySqlConfigBackendDHCPv6(params));

        } catch (...) {
            std::cerr << "*** ERROR: unable to open database. The test\n"
                         "*** environment is broken and must be fixed before\n"
                         "*** the MySQL tests will run correctly.\n"
                         "*** The reason for the problem is described in the\n"
                         "*** accompanying exception output.\n";
            throw;
        }

        // Create test data.
        initTestServers();
        initTestOptions();
        initTestSubnets();
        initTestSharedNetworks();
        initTestOptionDefs();
        initTimestamps();
    }

    /// @brief Destructor.
    virtual ~MySqlConfigBackendDHCPv6Test() {
        cbptr_.reset();
        // If data wipe enabled, delete transient data otherwise destroy the schema.
        destroyMySQLSchema();
    }

    /// @brief Counts rows in a selected table in MySQL database.
    ///
    /// This method can be used to verify that some configuration elements were
    /// deleted from a selected table as a result of cascade delete or a trigger.
    /// For example, deleting a subnet should trigger deletion of its address
    /// pools and options. By counting the rows on each table we can determine
    /// whether the deletion took place on all tables for which it was expected.
    ///
    /// @param table Table name.
    /// @return Number of rows in the specified table.
    size_t countRows(const std::string& table) const {
        auto p = std::dynamic_pointer_cast<TestMySqlConfigBackendDHCPv6>(cbptr_);
        if (!p) {
            ADD_FAILURE() << "cbptr_ does not cast to TestMySqlConfigBackendDHCPv6";
            return (0);
        }

        // Reuse the existing connection of the backend.
        auto impl = std::dynamic_pointer_cast<MySqlConfigBackendImpl>(p->base_impl_);
        auto& conn = impl->conn_;

        return (MySqlGenericBackendTest::countRows(conn, table));
    }

    /// @brief Creates several servers used in tests.
    void initTestServers() {
        test_servers_.push_back(Server::create(ServerTag("server1"), "this is server 1"));
        test_servers_.push_back(Server::create(ServerTag("server1"), "this is server 1 bis"));
        test_servers_.push_back(Server::create(ServerTag("server2"), "this is server 2"));
        test_servers_.push_back(Server::create(ServerTag("server3"), "this is server 3"));
    }

    /// @brief Creates several subnets used in tests.
    void initTestSubnets() {
        // First subnet includes all parameters.
        std::string interface_id_text = "whale";
        OptionBuffer interface_id(interface_id_text.begin(), interface_id_text.end());
        OptionPtr opt_interface_id(new Option(Option::V6, D6O_INTERFACE_ID,
                                              interface_id));
        ElementPtr user_context = Element::createMap();
        user_context->set("foo", Element::create("bar"));

        Subnet6Ptr subnet(new Subnet6(IOAddress("2001:db8::"),
                                      64, 30, 40, 50, 60, 1024));
        subnet->allowClientClass("home");
        subnet->setIface("eth1");
        subnet->setInterfaceId(opt_interface_id);
        subnet->setT2(323212);
        subnet->addRelayAddress(IOAddress("2001:db8:1::2"));
        subnet->addRelayAddress(IOAddress("2001:db8:3::4"));
        subnet->setT1(1234);
        subnet->requireClientClass("required-class1");
        subnet->requireClientClass("required-class2");
        subnet->setHostReservationMode(Subnet4::HR_DISABLED);
        subnet->setContext(user_context);
        subnet->setValid(555555);
        subnet->setPreferred(4444444);
        subnet->setCalculateTeeTimes(true);
        subnet->setT1Percent(0.345);
        subnet->setT2Percent(0.444);
        subnet->setDdnsSendUpdates(false);

        Pool6Ptr pool1(new Pool6(Lease::TYPE_NA,
                                 IOAddress("2001:db8::10"),
                                 IOAddress("2001:db8::20")));
        subnet->addPool(pool1);

        Pool6Ptr pool2(new Pool6(Lease::TYPE_NA,
                                 IOAddress("2001:db8::50"),
                                 IOAddress("2001:db8::60")));
        subnet->addPool(pool2);

        Pool6Ptr pdpool1(new Pool6(Lease::TYPE_PD,
                                   IOAddress("2001:db8:a::"), 48, 64));
        subnet->addPool(pdpool1);

        Pool6Ptr pdpool2(new Pool6(Lease::TYPE_PD,
                                   IOAddress("2001:db8:b::"), 48, 64));
        subnet->addPool(pdpool2);

        // Add several options to the subnet.
        subnet->getCfgOption()->add(test_options_[0]->option_,
                                    test_options_[0]->persistent_,
                                    test_options_[0]->space_name_);

        subnet->getCfgOption()->add(test_options_[1]->option_,
                                    test_options_[1]->persistent_,
                                    test_options_[1]->space_name_);

        subnet->getCfgOption()->add(test_options_[2]->option_,
                                    test_options_[2]->persistent_,
                                    test_options_[2]->space_name_);

        test_subnets_.push_back(subnet);

        // Adding another subnet with the same subnet id to test
        // cases that this second instance can override existing
        // subnet instance.
        subnet.reset(new Subnet6(IOAddress("2001:db8:1::"),
                                 48, 20, 30, 40, 50, 1024));

        pool1.reset(new Pool6(Lease::TYPE_NA,
                              IOAddress("2001:db8:1::10"),
                              IOAddress("2001:db8:1::20")));
        subnet->addPool(pool1);

        pool1->getCfgOption()->add(test_options_[3]->option_,
                                   test_options_[3]->persistent_,
                                   test_options_[3]->space_name_);

        pool1->getCfgOption()->add(test_options_[4]->option_,
                                   test_options_[4]->persistent_,
                                   test_options_[4]->space_name_);

        pool2.reset(new Pool6(Lease::TYPE_NA,
                              IOAddress("2001:db8:1::50"),
                              IOAddress("2001:db8:1::60")));
        subnet->addPool(pool2);

        pool2->allowClientClass("work");
        pool2->requireClientClass("required-class3");
        pool2->requireClientClass("required-class4");
        user_context = Element::createMap();
        user_context->set("bar", Element::create("foo"));
        pool2->setContext(user_context);

        pdpool1.reset(new Pool6(IOAddress("2001:db8:c::"), 48, 64,
                                IOAddress("2001:db8:c::1"), 96));
        subnet->addPool(pdpool1);

        pdpool1->getCfgOption()->add(test_options_[3]->option_,
                                     test_options_[3]->persistent_,
                                     test_options_[3]->space_name_);

        pdpool1->getCfgOption()->add(test_options_[4]->option_,
                                     test_options_[4]->persistent_,
                                     test_options_[4]->space_name_);

        // Create the prefix delegation pool with an excluded prefix.
        pdpool2.reset(new Pool6(IOAddress("2001:db8:d::"), 48, 64,
                                IOAddress("2001:db8:d::2000"), 120));

        subnet->addPool(pdpool2);

        pdpool2->allowClientClass("work");
        pdpool2->requireClientClass("required-class3");
        pdpool2->requireClientClass("required-class4");
        user_context = Element::createMap();
        user_context->set("bar", Element::create("foo"));
        pdpool2->setContext(user_context);

        test_subnets_.push_back(subnet);

        subnet.reset(new Subnet6(IOAddress("2001:db8:3::"),
                                 64, 20, 30, 40, 50, 2048));
        Triplet<uint32_t> null_timer;
        subnet->setPreferred(null_timer);
        subnet->setT1(null_timer);
        subnet->setT2(null_timer);
        subnet->setValid(null_timer);
        subnet->setPreferred(null_timer);
        subnet->setDdnsSendUpdates(true);
        subnet->setDdnsOverrideNoUpdate(true);
        subnet->setDdnsOverrideClientUpdate(false);
        subnet->setDdnsReplaceClientNameMode(D2ClientConfig::ReplaceClientNameMode::RCM_WHEN_PRESENT);
        subnet->setDdnsGeneratedPrefix("myhost");
        subnet->setDdnsQualifyingSuffix("example.org");

        subnet->getCfgOption()->add(test_options_[0]->option_,
                                    test_options_[0]->persistent_,
                                    test_options_[0]->space_name_);

        test_subnets_.push_back(subnet);

        // Add a subnet with all defaults.
        subnet.reset(new Subnet6(IOAddress("2001:db8:4::"), 64,
                                 Triplet<uint32_t>(), Triplet<uint32_t>(),
                                 Triplet<uint32_t>(), Triplet<uint32_t>(),
                                 4096));
        test_subnets_.push_back(subnet);
    }

    /// @brief Creates several subnets used in tests.
    void initTestSharedNetworks() {
        ElementPtr user_context = Element::createMap();
        user_context->set("foo", Element::create("bar"));

        std::string interface_id_text = "fish";
        OptionBuffer interface_id(interface_id_text.begin(),
                                  interface_id_text.end());
        OptionPtr opt_interface_id(new Option(Option::V6, D6O_INTERFACE_ID,
                                              interface_id));

        SharedNetwork6Ptr shared_network(new SharedNetwork6("level1"));
        shared_network->allowClientClass("foo");
        shared_network->setIface("eth1");
        shared_network->setInterfaceId(opt_interface_id);
        shared_network->setT2(323212);
        shared_network->addRelayAddress(IOAddress("2001:db8:1::2"));
        shared_network->addRelayAddress(IOAddress("2001:db8:3::4"));
        shared_network->setT1(1234);
        shared_network->requireClientClass("required-class1");
        shared_network->requireClientClass("required-class2");
        shared_network->setHostReservationMode(Subnet6::HR_DISABLED);
        shared_network->setContext(user_context);
        shared_network->setValid(5555);
        shared_network->setPreferred(4444);
        shared_network->setCalculateTeeTimes(true);
        shared_network->setT1Percent(0.345);
        shared_network->setT2Percent(0.444);
        shared_network->setDdnsSendUpdates(false);

        // Add several options to the shared network.
        shared_network->getCfgOption()->add(test_options_[2]->option_,
                                            test_options_[2]->persistent_,
                                            test_options_[2]->space_name_);

        shared_network->getCfgOption()->add(test_options_[3]->option_,
                                            test_options_[3]->persistent_,
                                            test_options_[3]->space_name_);

        shared_network->getCfgOption()->add(test_options_[4]->option_,
                                            test_options_[4]->persistent_,
                                            test_options_[4]->space_name_);

        test_networks_.push_back(shared_network);

        // Adding another shared network called "level1" to test
        // cases that this second instance can override existing
        // "level1" instance.
        shared_network.reset(new SharedNetwork6("level1"));
        test_networks_.push_back(shared_network);

        // Add more shared networks.
        shared_network.reset(new SharedNetwork6("level2"));
        Triplet<uint32_t> null_timer;
        shared_network->setPreferred(null_timer);
        shared_network->setT1(null_timer);
        shared_network->setT2(null_timer);
        shared_network->setValid(null_timer);
        shared_network->setPreferred(null_timer);
        shared_network->setDdnsSendUpdates(true);
        shared_network->setDdnsOverrideNoUpdate(true);
        shared_network->setDdnsOverrideClientUpdate(false);
        shared_network->setDdnsReplaceClientNameMode(D2ClientConfig::ReplaceClientNameMode::RCM_WHEN_PRESENT);
        shared_network->setDdnsGeneratedPrefix("myhost");
        shared_network->setDdnsQualifyingSuffix("example.org");

        shared_network->getCfgOption()->add(test_options_[0]->option_,
                                            test_options_[0]->persistent_,
                                            test_options_[0]->space_name_);
        test_networks_.push_back(shared_network);

        shared_network.reset(new SharedNetwork6("level3"));
        test_networks_.push_back(shared_network);
    }

    /// @brief Creates several option definitions used in tests.
    void initTestOptionDefs() {
        ElementPtr user_context = Element::createMap();
        user_context->set("foo", Element::create("bar"));

        OptionDefinitionPtr option_def(new OptionDefinition("foo", 1234, "string",
                                                            "espace"));
        option_def->setOptionSpaceName(DHCP6_OPTION_SPACE);
        test_option_defs_.push_back(option_def);

        option_def.reset(new OptionDefinition("bar", 1234, "uint32", true));
        option_def->setOptionSpaceName(DHCP6_OPTION_SPACE);
        test_option_defs_.push_back(option_def);

        option_def.reset(new OptionDefinition("fish", 5235, "record", true));
        option_def->setOptionSpaceName(DHCP6_OPTION_SPACE);
        option_def->addRecordField("uint32");
        option_def->addRecordField("string");
        test_option_defs_.push_back(option_def);

        option_def.reset(new OptionDefinition("whale", 20236, "string"));
        option_def->setOptionSpaceName("xyz");
        test_option_defs_.push_back(option_def);

        option_def.reset(new OptionDefinition("bar", 1234, "uint64", true));
        option_def->setOptionSpaceName(DHCP6_OPTION_SPACE);
        test_option_defs_.push_back(option_def);
    }

    /// @brief Creates several DHCP options used in tests.
    void initTestOptions() {
        ElementPtr user_context = Element::createMap();
        user_context->set("foo", Element::create("bar"));

        OptionDefSpaceContainer defs;

        OptionDescriptor desc =
            createOption<OptionString>(Option::V6, D6O_NEW_POSIX_TIMEZONE,
                                       true, false, "my-timezone");
        desc.space_name_ = DHCP6_OPTION_SPACE;
        desc.setContext(user_context);
        test_options_.push_back(OptionDescriptorPtr(new OptionDescriptor(desc)));

        desc = createOption<OptionUint8>(Option::V6, D6O_PREFERENCE,
                                         false, true, 64);
        desc.space_name_ = DHCP6_OPTION_SPACE;
        test_options_.push_back(OptionDescriptorPtr(new OptionDescriptor(desc)));

        desc = createOption<OptionUint32>(Option::V6, 1, false, false, 312131),
        desc.space_name_ = "vendor-encapsulated-options";
        test_options_.push_back(OptionDescriptorPtr(new OptionDescriptor(desc)));

        desc = createAddressOption<Option6AddrLst>(1254, true, true,
                                                   "2001:db8::3");
        desc.space_name_ = DHCP6_OPTION_SPACE;
        test_options_.push_back(OptionDescriptorPtr(new OptionDescriptor(desc)));

        desc = createEmptyOption(Option::V6, 1, true);
        desc.space_name_ = "isc";
        test_options_.push_back(OptionDescriptorPtr(new OptionDescriptor(desc)));

        desc = createAddressOption<Option6AddrLst>(2, false, true,
                                                   "2001:db8:1::5",
                                                   "2001:db8:1::3",
                                                   "2001:db8:3::4");
        desc.space_name_ = "isc";
        test_options_.push_back(OptionDescriptorPtr(new OptionDescriptor(desc)));

        desc = createOption<OptionString>(Option::V6, D6O_NEW_POSIX_TIMEZONE,
                                          true, false, "my-timezone-2");
        desc.space_name_ = DHCP6_OPTION_SPACE;
        desc.setContext(user_context);
        test_options_.push_back(OptionDescriptorPtr(new OptionDescriptor(desc)));

        desc = createOption<OptionString>(Option::V6, D6O_NEW_POSIX_TIMEZONE,
                                          true, false, "my-timezone-3");
        desc.space_name_ = DHCP6_OPTION_SPACE;
        desc.setContext(user_context);
        test_options_.push_back(OptionDescriptorPtr(new OptionDescriptor(desc)));

        // Add definitions for DHCPv6 non-standard options in case we need to
        // compare subnets, networks and pools in JSON format. In that case,
        // the @c toElement functions require option definitions to generate the
        // proper output.
        defs.addItem(OptionDefinitionPtr(new OptionDefinition(
                         "vendor-encapsulated-1", 1, "uint32")),
                     "vendor-encapsulated-options");
        defs.addItem(OptionDefinitionPtr(new OptionDefinition(
                         "option-1254", 1254, "ipv6-address", true)),
                     DHCP6_OPTION_SPACE);
        defs.addItem(OptionDefinitionPtr(new OptionDefinition("isc-1", 1, "empty")), "isc");
        defs.addItem(OptionDefinitionPtr(new OptionDefinition("isc-2", 2, "ipv6-address", true)),
                     "isc");

        // Register option definitions.
        LibDHCP::setRuntimeOptionDefs(defs);
    }

    /// @brief Initialize posix time values used in tests.
    void initTimestamps() {
        // Current time minus 1 hour to make sure it is in the past.
        timestamps_["today"] = boost::posix_time::second_clock::local_time()
            - boost::posix_time::hours(1);
        // One second after today.
        timestamps_["after today"] = timestamps_["today"] + boost::posix_time::seconds(1);
        // Yesterday.
        timestamps_["yesterday"] = timestamps_["today"] - boost::posix_time::hours(24);
        // One second after yesterday.
        timestamps_["after yesterday"] = timestamps_["yesterday"] + boost::posix_time::seconds(1);
        // Two days ago.
        timestamps_["two days ago"] = timestamps_["today"] - boost::posix_time::hours(48);
        // Tomorrow.
        timestamps_["tomorrow"] = timestamps_["today"] + boost::posix_time::hours(24);
        // One second after tomorrow.
        timestamps_["after tomorrow"] = timestamps_["tomorrow"] + boost::posix_time::seconds(1);
    }

    /// @brief Logs audit entries in the @c audit_entries_ member.
    ///
    /// This function is called in case of an error.
    ///
    /// @param server_tag Server tag for which the audit entries should be logged.
    std::string logExistingAuditEntries(const std::string& server_tag) {
        std::ostringstream s;

        auto& mod_time_idx = audit_entries_[server_tag].get<AuditEntryModificationTimeIdTag>();

        for (auto audit_entry_it = mod_time_idx.begin();
             audit_entry_it != mod_time_idx.end();
             ++audit_entry_it) {
            auto audit_entry = *audit_entry_it;
            s << audit_entry->getObjectType() << ", "
              << audit_entry->getObjectId() << ", "
              << static_cast<int>(audit_entry->getModificationType()) << ", "
              << audit_entry->getModificationTime() << ", "
              << audit_entry->getRevisionId() << ", "
              << audit_entry->getLogMessage()
              << std::endl;
        }

        return (s.str());
    }

    /// @brief Tests that the new audit entry is added.
    ///
    /// This method retrieves a collection of the existing audit entries and
    /// checks that the new one has been added at the end of this collection.
    /// It then verifies the values of the audit entry against the values
    /// specified by the caller.
    ///
    /// @param exp_object_type Expected object type.
    /// @param exp_modification_type Expected modification type.
    /// @param exp_log_message Expected log message.
    /// @param server_selector Server selector to be used for next query.
    /// @param new_entries_num Number of the new entries expected to be inserted.
    /// @param max_tested_entries Maximum number of entries tested.
    void testNewAuditEntry(const std::string& exp_object_type,
                           const AuditEntry::ModificationType& exp_modification_type,
                           const std::string& exp_log_message,
                           const ServerSelector& server_selector = ServerSelector::ALL(),
                           const size_t new_entries_num = 1,
                           const size_t max_tested_entries = 65535) {
        // Get the server tag for which the entries are fetched.
        std::string tag;
        if (server_selector.getType() == ServerSelector::Type::ALL) {
            // Server tag is 'all'.
            tag = "all";

        } else {
            auto tags = server_selector.getTags();
            // This test is not meant to handle multiple server tags all at once.
            if (tags.size() > 1) {
                ADD_FAILURE() << "Test error: do not use multiple server tags";

            } else if (tags.size() == 1) {
                // Get the server tag for which we run the current test.
                tag = tags.begin()->get();
            }
        }

        auto audit_entries_size_save = audit_entries_[tag].size();

        // Audit entries for different server tags are stored in separate
        // containers.
        audit_entries_[tag] = cbptr_->getRecentAuditEntries(server_selector,
                                                            timestamps_["two days ago"], 0);
        ASSERT_EQ(audit_entries_size_save + new_entries_num, audit_entries_[tag].size())
            << logExistingAuditEntries(tag);

        auto& mod_time_idx = audit_entries_[tag].get<AuditEntryModificationTimeIdTag>();

        // Iterate over specified number of entries starting from the most recent
        // one and check they have correct values.
        for (auto audit_entry_it = mod_time_idx.rbegin();
             ((std::distance(mod_time_idx.rbegin(), audit_entry_it) < new_entries_num) &&
              (std::distance(mod_time_idx.rbegin(), audit_entry_it) < max_tested_entries));
             ++audit_entry_it) {
            auto audit_entry = *audit_entry_it;
            EXPECT_EQ(exp_object_type, audit_entry->getObjectType())
                << logExistingAuditEntries(tag);
            EXPECT_EQ(exp_modification_type, audit_entry->getModificationType())
                << logExistingAuditEntries(tag);
            EXPECT_EQ(exp_log_message, audit_entry->getLogMessage())
                << logExistingAuditEntries(tag);
        }
    }

    /// @brief Holds pointers to subnets used in tests.
    std::vector<Subnet6Ptr> test_subnets_;

    /// @brief Holds pointers to shared networks used in tests.
    std::vector<SharedNetwork6Ptr> test_networks_;

    /// @brief Holds pointers to option definitions used in tests.
    std::vector<OptionDefinitionPtr> test_option_defs_;

    /// @brief Holds pointers to options used in tests.
    std::vector<OptionDescriptorPtr> test_options_;

    /// @brief Holds pointers to the servers used in tests.
    std::vector<ServerPtr> test_servers_;

    /// @brief Holds timestamp values used in tests.
    std::map<std::string, boost::posix_time::ptime> timestamps_;

    /// @brief Holds pointer to the backend.
    std::shared_ptr<ConfigBackendDHCPv6> cbptr_;

    /// @brief Holds the most recent audit entries.
    std::map<std::string, AuditEntryCollection> audit_entries_;
};

// This test verifies that the expected backend type is returned.
TEST_F(MySqlConfigBackendDHCPv6Test, getType) {
    DatabaseConnection::ParameterMap params;
    params["name"] = "keatest";
    params["password"] = "keatest";
    params["user"] = "keatest";
    ASSERT_NO_THROW(cbptr_.reset(new MySqlConfigBackendDHCPv6(params)));
    EXPECT_EQ("mysql", cbptr_->getType());
}

// This test verifies that by default localhost is returned as MySQL connection
// host.
TEST_F(MySqlConfigBackendDHCPv6Test, getHost) {
    DatabaseConnection::ParameterMap params;
    params["name"] = "keatest";
    params["password"] = "keatest";
    params["user"] = "keatest";
    ASSERT_NO_THROW(cbptr_.reset(new MySqlConfigBackendDHCPv6(params)));
    EXPECT_EQ("localhost", cbptr_->getHost());
}

// This test verifies that by default port of 0 is returned as MySQL connection
// port.
TEST_F(MySqlConfigBackendDHCPv6Test, getPort) {
    DatabaseConnection::ParameterMap params;
    params["name"] = "keatest";
    params["password"] = "keatest";
    params["user"] = "keatest";
    ASSERT_NO_THROW(cbptr_.reset(new MySqlConfigBackendDHCPv6(params)));
    EXPECT_EQ(0, cbptr_->getPort());
}

// This test verifies that the server can be added, updated and deleted.
TEST_F(MySqlConfigBackendDHCPv6Test, createUpdateDeleteServer) {
    // Explicitly set modification time to make sure that the time
    // returned from the database is correct.
    test_servers_[0]->setModificationTime(timestamps_["yesterday"]);
    test_servers_[1]->setModificationTime(timestamps_["today"]);

    // Insert the server1 into the database.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[0]));

    {
        SCOPED_TRACE("CREATE audit entry for server");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    // It should not be possible to create a duplicate of the logical
    // server 'all'.
    auto all_server = Server::create(ServerTag("all"), "this is logical server all");
    EXPECT_THROW(cbptr_->createUpdateServer6(all_server), isc::InvalidOperation);

    ServerPtr returned_server;

    // An attempt to fetch the server that hasn't been inserted should return
    // a null pointer.
    EXPECT_NO_THROW(returned_server = cbptr_->getServer6(ServerTag("server2")));
    EXPECT_FALSE(returned_server);

    // Try to fetch the server which we expect to exist.
    EXPECT_NO_THROW(returned_server = cbptr_->getServer6(ServerTag("server1")));
    ASSERT_TRUE(returned_server);
    EXPECT_EQ("server1", returned_server->getServerTagAsText());
    EXPECT_EQ("this is server 1", returned_server->getDescription());
    EXPECT_EQ(timestamps_["yesterday"], returned_server->getModificationTime());

    // This call is expected to update the existing server.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[1]));

    {
        SCOPED_TRACE("UPDATE audit entry for server");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::UPDATE,
                          "server set");
    }

    // Verify that the server has been updated.
    EXPECT_NO_THROW(returned_server = cbptr_->getServer6(ServerTag("server1")));
    ASSERT_TRUE(returned_server);
    EXPECT_EQ("server1", returned_server->getServerTag().get());
    EXPECT_EQ("this is server 1 bis", returned_server->getDescription());
    EXPECT_EQ(timestamps_["today"], returned_server->getModificationTime());


    uint64_t servers_deleted = 0;

    // Try to delete non-existing server.
    EXPECT_NO_THROW(servers_deleted = cbptr_->deleteServer6(ServerTag("server2")));
    EXPECT_EQ(0, servers_deleted);

    // Make sure that the server1 wasn't deleted.
    EXPECT_NO_THROW(returned_server = cbptr_->getServer6(ServerTag("server1")));
    EXPECT_TRUE(returned_server);

    // Deleting logical server 'all' is not allowed.
    EXPECT_THROW(cbptr_->deleteServer6(ServerTag()), isc::InvalidOperation);

    // Delete the existing server.
    EXPECT_NO_THROW(servers_deleted = cbptr_->deleteServer6(ServerTag("server1")));
    EXPECT_EQ(1, servers_deleted);

    {
        SCOPED_TRACE("DELETE audit entry for server");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::DELETE,
                          "deleting a server");
    }

    // Make sure that the server is gone.
    EXPECT_NO_THROW(returned_server = cbptr_->getServer6(ServerTag("server1")));
    EXPECT_FALSE(returned_server);
}

// This test verifies that it is possible to retrieve all servers from the
// database and then delete all of them.
TEST_F(MySqlConfigBackendDHCPv6Test, getAndDeleteAllServers) {
    for (auto i = 1; i < test_servers_.size(); ++i) {
        EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[i]));
    }

    ServerCollection servers;
    EXPECT_NO_THROW(servers = cbptr_->getAllServers6());
    ASSERT_EQ(test_servers_.size() - 1, servers.size());

    // All servers should have been returned.
    EXPECT_TRUE(ServerFetcher::get(servers, ServerTag("server1")));
    EXPECT_TRUE(ServerFetcher::get(servers, ServerTag("server2")));
    EXPECT_TRUE(ServerFetcher::get(servers, ServerTag("server3")));

    // The logical server all should not be returned. We merely return the
    // user configured servers.
    EXPECT_FALSE(ServerFetcher::get(servers, ServerTag()));

    // Delete all servers and make sure they are gone.
    uint64_t deleted_servers = 0;
    EXPECT_NO_THROW(deleted_servers = cbptr_->deleteAllServers6());

    EXPECT_NO_THROW(servers = cbptr_->getAllServers6());
    EXPECT_TRUE(servers.empty());

    // All servers should be gone.
    EXPECT_FALSE(ServerFetcher::get(servers, ServerTag("server1")));
    EXPECT_FALSE(ServerFetcher::get(servers, ServerTag("server2")));
    EXPECT_FALSE(ServerFetcher::get(servers, ServerTag("server3")));

    // The number of deleted server should be equal to the number of
    // inserted servers. The logical 'all' server should be excluded.
    EXPECT_EQ(test_servers_.size() - 1, deleted_servers);
}

// This test verifies that the global parameter can be added, updated and
// deleted.
TEST_F(MySqlConfigBackendDHCPv6Test, createUpdateDeleteGlobalParameter6) {
    StampedValuePtr global_parameter = StampedValue::create("global", "whale");

    // Explicitly set modification time to make sure that the time
    // returned from the database is correct.
    global_parameter->setModificationTime(timestamps_["yesterday"]);
    cbptr_->createUpdateGlobalParameter6(ServerSelector::ALL(),
                                         global_parameter);

    {
        SCOPED_TRACE("CREATE audit entry for global parameter");
        testNewAuditEntry("dhcp6_global_parameter",
                          AuditEntry::ModificationType::CREATE,
                          "global parameter set");
    }

    // Verify returned parameter and the modification time.
    StampedValuePtr returned_global_parameter =
        cbptr_->getGlobalParameter6(ServerSelector::ALL(), "global");
    ASSERT_TRUE(returned_global_parameter);
    EXPECT_EQ("global", returned_global_parameter->getName());
    EXPECT_EQ("whale", returned_global_parameter->getValue());
    EXPECT_TRUE(returned_global_parameter->getModificationTime() ==
                global_parameter->getModificationTime());
    ASSERT_EQ(1, returned_global_parameter->getServerTags().size());
    EXPECT_EQ("all", returned_global_parameter->getServerTags().begin()->get());

    // Because we have added the global parameter for all servers, it
    // should be also returned for the explicitly specified server.
    returned_global_parameter = cbptr_->getGlobalParameter6(ServerSelector::ONE("server1"),
                                                            "global");
    ASSERT_TRUE(returned_global_parameter);
    EXPECT_EQ("global", returned_global_parameter->getName());
    EXPECT_EQ("whale", returned_global_parameter->getValue());
    EXPECT_TRUE(returned_global_parameter->getModificationTime() ==
                global_parameter->getModificationTime());
    ASSERT_EQ(1, returned_global_parameter->getServerTags().size());
    EXPECT_EQ("all", returned_global_parameter->getServerTags().begin()->get());

    // Check that the parameter is updated when selector is specified correctly.
    global_parameter = StampedValue::create("global", "fish");
    cbptr_->createUpdateGlobalParameter6(ServerSelector::ALL(),
                                         global_parameter);
    returned_global_parameter = cbptr_->getGlobalParameter6(ServerSelector::ALL(),
                                                            "global");
    ASSERT_TRUE(returned_global_parameter);
    EXPECT_EQ("global", returned_global_parameter->getName());
    EXPECT_EQ("fish", returned_global_parameter->getValue());
    EXPECT_TRUE(returned_global_parameter->getModificationTime() ==
                global_parameter->getModificationTime());

    {
        SCOPED_TRACE("UPDATE audit entry for the global parameter");
        testNewAuditEntry("dhcp6_global_parameter",
                          AuditEntry::ModificationType::UPDATE,
                          "global parameter set");
    }

    // Should not delete parameter specified for all servers if explicit
    // server name is provided.
    EXPECT_EQ(0, cbptr_->deleteGlobalParameter6(ServerSelector::ONE("server1"),
                                                "global"));

    // Delete parameter and make sure it is gone.
    cbptr_->deleteGlobalParameter6(ServerSelector::ALL(), "global");
    returned_global_parameter = cbptr_->getGlobalParameter6(ServerSelector::ALL(),
                                                            "global");
    EXPECT_FALSE(returned_global_parameter);

    {
        SCOPED_TRACE("DELETE audit entry for the global parameter");
        testNewAuditEntry("dhcp6_global_parameter",
                          AuditEntry::ModificationType::DELETE,
                          "global parameter deleted");
    }
}

// This test verifies that it is possible to differentiate between the
// global parameters by server tag and that the value specified for the
// particular server overrides the value specified for all servers.
TEST_F(MySqlConfigBackendDHCPv6Test, globalParameters6WithServerTags) {
    // Create three global parameters having the same name.
    StampedValuePtr global_parameter1 = StampedValue::create("global", "value1");
    StampedValuePtr global_parameter2 = StampedValue::create("global", "value2");
    StampedValuePtr global_parameter3 = StampedValue::create("global", "value3");

    // Try to insert one of them and associate with non-existing server.
    // This should fail because the server must be inserted first.
    EXPECT_THROW(cbptr_->createUpdateGlobalParameter6(ServerSelector::ONE("server1"),
                                                      global_parameter1),
                 NullKeyError);

    // Create two servers.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[1]));
    {
        SCOPED_TRACE("server1 is created");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[2]));
    {
        SCOPED_TRACE("server2 is created");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    // This time inserting the global parameters for the server1 and server2 should
    // be successful.
    EXPECT_NO_THROW(cbptr_->createUpdateGlobalParameter6(ServerSelector::ONE("server1"),
                                                         global_parameter1));
    {
        SCOPED_TRACE("Global parameter for server1 is set");
        // The value of 3 means there should be 3 audit entries available for the
        // server1, two that indicate creation of the servers and one that we
        // validate, which sets the global value.
        testNewAuditEntry("dhcp6_global_parameter",
                          AuditEntry::ModificationType::CREATE,
                          "global parameter set",
                          ServerSelector::ONE("server1"),
                          3, 1);
    }


    EXPECT_NO_THROW(cbptr_->createUpdateGlobalParameter6(ServerSelector::ONE("server2"),
                                                         global_parameter2));
    {
        SCOPED_TRACE("Global parameter for server2 is set");
        // Same as in case of the server2, there should be 3 audit entries of
        // which one we validate.
        testNewAuditEntry("dhcp6_global_parameter",
                          AuditEntry::ModificationType::CREATE,
                          "global parameter set",
                          ServerSelector::ONE("server2"),
                          3, 1);
    }

    // The last parameter is associated with all servers.
    EXPECT_NO_THROW(cbptr_->createUpdateGlobalParameter6(ServerSelector::ALL(),
                                                         global_parameter3));
    {
        SCOPED_TRACE("Global parameter for all servers is set");
        // There should be one new audit entry for all servers. It indicates
        // the insertion of the global value.
        testNewAuditEntry("dhcp6_global_parameter",
                          AuditEntry::ModificationType::CREATE,
                          "global parameter set",
                          ServerSelector::ALL(),
                          1, 1);
    }

    StampedValuePtr returned_global;

    // Try to fetch the value specified for all servers.
    EXPECT_NO_THROW(
        returned_global = cbptr_->getGlobalParameter6(ServerSelector::ALL(),
                                                      "global")
    );
    ASSERT_TRUE(returned_global);
    EXPECT_EQ(global_parameter3->getValue(), returned_global->getValue());
    ASSERT_EQ(1, returned_global->getServerTags().size());
    EXPECT_EQ("all", returned_global->getServerTags().begin()->get());

    // Try to fetch the value specified for the server1. This should override the
    // value specified for all servers.
    EXPECT_NO_THROW(
        returned_global = cbptr_->getGlobalParameter6(ServerSelector::ONE("server1"),
                                                      "global")
    );
    ASSERT_TRUE(returned_global);
    EXPECT_EQ(global_parameter1->getValue(), returned_global->getValue());

    ASSERT_EQ(1, returned_global->getServerTags().size());
    EXPECT_EQ("server1", returned_global->getServerTags().begin()->get());

    // The same in case of the server2.
    EXPECT_NO_THROW(
        returned_global = cbptr_->getGlobalParameter6(ServerSelector::ONE("server2"),
                                                      "global")
    );
    ASSERT_TRUE(returned_global);
    EXPECT_EQ(global_parameter2->getValue(), returned_global->getValue());
    ASSERT_EQ(1, returned_global->getServerTags().size());
    EXPECT_EQ("server2", returned_global->getServerTags().begin()->get());

    StampedValueCollection returned_globals;

    // Try to fetch the collection of globals for the server1, server2 and server3.
    // The server3 does not have an explicit value so for this server we should get
    /// the value for 'all'.
    EXPECT_NO_THROW(
        returned_globals = cbptr_->getAllGlobalParameters6(ServerSelector::
                                                           MULTIPLE({ "server1", "server2",
                                                                      "server3" }));
    );
    ASSERT_EQ(3, returned_globals.size());

    // Capture the returned values into the map so as we can check the
    // values against the servers.
    std::map<std::string, std::string> values;
    for (auto g = returned_globals.begin(); g != returned_globals.end(); ++g) {
        ASSERT_EQ(1, (*g)->getServerTags().size());
        values[(*g)->getServerTags().begin()->get()] = ((*g)->getValue());
    }

    ASSERT_EQ(3, values.size());
    EXPECT_EQ(global_parameter1->getValue(), values["server1"]);
    EXPECT_EQ(global_parameter2->getValue(), values["server2"]);
    EXPECT_EQ(global_parameter3->getValue(), values["all"]);

    // Try to fetch the collection of global parameters specified for all servers.
    // This excludes the values specific to server1 and server2. It returns only the
    // common ones.
    EXPECT_NO_THROW(
        returned_globals = cbptr_->getAllGlobalParameters6(ServerSelector::ALL())
    );
    ASSERT_EQ(1, returned_globals.size());
    returned_global = *returned_globals.begin();
    EXPECT_EQ(global_parameter3->getValue(), returned_global->getValue());
    ASSERT_EQ(1, returned_global->getServerTags().size());
    EXPECT_EQ("all", returned_global->getServerTags().begin()->get());

    // Delete the server1. It should remove associations of this server with the
    // global parameter and the global parameter itself.
    EXPECT_NO_THROW(cbptr_->deleteServer6(ServerTag("server1")));
    EXPECT_NO_THROW(
        returned_globals = cbptr_->getAllGlobalParameters6(ServerSelector::ONE("server1"))
    );
    ASSERT_EQ(1, returned_globals.size());
    returned_global = *returned_globals.begin();
    // As a result, the value fetched for the server1 should be the one available for
    // all servers, rather than the one dedicated for server1. The association of
    // the server1 specific value with the server1 should be gone.
    EXPECT_EQ(global_parameter3->getValue(), returned_global->getValue());
    ASSERT_EQ(1, returned_global->getServerTags().size());
    EXPECT_EQ("all", returned_global->getServerTags().begin()->get());

    {
        SCOPED_TRACE("DELETE audit entry for the global parameter after server deletion");
        // We expect two new audit entries for the server1, one indicating that the
        // server has been deleted and another one indicating that the corresponding
        // global value has been deleted. We check the latter entry.
        testNewAuditEntry("dhcp6_global_parameter",
                          AuditEntry::ModificationType::DELETE,
                          "deleting a server", ServerSelector::ONE("server1"),
                          2, 1);
    }

    // Attempt to delete global parameter for server1.
    uint64_t deleted_num = 0;
    EXPECT_NO_THROW(deleted_num = cbptr_->deleteGlobalParameter6(ServerSelector::ONE("server1"),
                                                                 "global"));
    // No parameters should be deleted. In particular, the parameter for the logical
    // server 'all' should not be deleted.
    EXPECT_EQ(0, deleted_num);

    // Deleting the existing value for server2 should succeed.
    EXPECT_NO_THROW(deleted_num = cbptr_->deleteGlobalParameter6(ServerSelector::ONE("server2"),
                                                                 "global"));
    EXPECT_EQ(1, deleted_num);

    // Create it again to test that deletion of all server removes this too.
    EXPECT_NO_THROW(cbptr_->createUpdateGlobalParameter6(ServerSelector::ONE("server2"),
                                                         global_parameter2));

    // Delete all servers, except 'all'.
    EXPECT_NO_THROW(deleted_num = cbptr_->deleteAllServers6());
    EXPECT_NO_THROW(
        returned_globals = cbptr_->getAllGlobalParameters6(ServerSelector::ALL())
    );
    EXPECT_EQ(1, deleted_num);
    ASSERT_EQ(1, returned_globals.size());
    returned_global = *returned_globals.begin();
    // The common value for all servers should still be available because 'all'
    // logical server should not be deleted.
    EXPECT_EQ(global_parameter3->getValue(), returned_global->getValue());
    ASSERT_EQ(1, returned_global->getServerTags().size());
    EXPECT_EQ("all", returned_global->getServerTags().begin()->get());

    {
        SCOPED_TRACE("DELETE audit entry for the global parameter after deletion of"
                     " all servers");
        // There should be 4 new audit entries. One for deleting the global, one for
        // re-creating it, one for deleting the server2 and one for deleting the
        // global again as a result of deleting the server2.
        testNewAuditEntry("dhcp6_global_parameter",
                          AuditEntry::ModificationType::DELETE,
                          "deleting all servers", ServerSelector::ONE("server2"),
                          4, 1);
    }
}

// This test verifies that all global parameters can be retrieved and deleted.
TEST_F(MySqlConfigBackendDHCPv6Test, getAllGlobalParameters6) {
    // Create 3 parameters and put them into the database.
    cbptr_->createUpdateGlobalParameter6(ServerSelector::ALL(),
        StampedValue::create("name1", "value1"));
    cbptr_->createUpdateGlobalParameter6(ServerSelector::ALL(),
        StampedValue::create("name2", Element::create(static_cast<int64_t>(65))));
    cbptr_->createUpdateGlobalParameter6(ServerSelector::ALL(),
        StampedValue::create("name3", "value3"));
    cbptr_->createUpdateGlobalParameter6(ServerSelector::ALL(),
        StampedValue::create("name4", Element::create(static_cast<bool>(true))));
    cbptr_->createUpdateGlobalParameter6(ServerSelector::ALL(),
        StampedValue::create("name5", Element::create(static_cast<double>(1.65))));

    // Fetch all parameters.
    auto parameters = cbptr_->getAllGlobalParameters6(ServerSelector::ALL());
    ASSERT_EQ(5, parameters.size());

    const auto& parameters_index = parameters.get<StampedValueNameIndexTag>();

    for (auto param = parameters_index.begin(); param != parameters_index.end();
         ++param) {
        ASSERT_EQ(1, (*param)->getServerTags().size());
        EXPECT_EQ("all", (*param)->getServerTags().begin()->get());
    }

    // Verify their values.
    EXPECT_EQ("value1", (*parameters_index.find("name1"))->getValue());
    EXPECT_EQ(65, (*parameters_index.find("name2"))->getIntegerValue());
    EXPECT_EQ("value3", (*parameters_index.find("name3"))->getValue());
    EXPECT_TRUE((*parameters_index.find("name4"))->getBoolValue());
    EXPECT_EQ(1.65, (*parameters_index.find("name5"))->getDoubleValue());

    // Should be able to fetch these parameters when explicitly providing
    // the server tag.
    parameters = cbptr_->getAllGlobalParameters6(ServerSelector::ONE("server1"));
    EXPECT_EQ(5, parameters.size());

    // Deleting global parameters with non-matching server selector
    // should fail.
    EXPECT_EQ(0, cbptr_->deleteAllGlobalParameters6(ServerSelector::ONE("server1")));

    // Delete all parameters and make sure they are gone.
    EXPECT_EQ(5, cbptr_->deleteAllGlobalParameters6(ServerSelector::ALL()));
    parameters = cbptr_->getAllGlobalParameters6(ServerSelector::ALL());
    EXPECT_TRUE(parameters.empty());
}

// This test verifies that modified global parameters can be retrieved.
TEST_F(MySqlConfigBackendDHCPv6Test, getModifiedGlobalParameters6) {
    // Create 3 global parameters and assign modification times:
    // "yesterday", "today" and "tomorrow" respectively.
    StampedValuePtr value = StampedValue::create("name1", "value1");
    value->setModificationTime(timestamps_["yesterday"]);
    cbptr_->createUpdateGlobalParameter6(ServerSelector::ALL(),
                                         value);

    value = StampedValue::create("name2", Element::create(static_cast<int64_t>(65)));
    value->setModificationTime(timestamps_["today"]);
    cbptr_->createUpdateGlobalParameter6(ServerSelector::ALL(),
                                         value);

    value = StampedValue::create("name3", "value3");
    value->setModificationTime(timestamps_["tomorrow"]);
    cbptr_->createUpdateGlobalParameter6(ServerSelector::ALL(),
                                         value);

    // Get parameters modified after "today".
    auto parameters = cbptr_->getModifiedGlobalParameters6(ServerSelector::ALL(),
                                                           timestamps_["after today"]);

    const auto& parameters_index = parameters.get<StampedValueNameIndexTag>();

    // It should be the one modified "tomorrow".
    ASSERT_EQ(1, parameters_index.size());

    auto parameter = parameters_index.find("name3");
    ASSERT_FALSE(parameter == parameters_index.end());

    ASSERT_TRUE(*parameter);
    EXPECT_EQ("value3", (*parameter)->getValue());

    // Should be able to fetct these parameters when explicitly providing
    // the server tag.
    parameters = cbptr_->getModifiedGlobalParameters6(ServerSelector::ONE("server1"),
                                                      timestamps_["after today"]);
    EXPECT_EQ(1, parameters.size());
}

// Test that the NullKeyError message is correctly updated.
TEST_F(MySqlConfigBackendDHCPv6Test, nullKeyError) {
    // Create a global parameter (it should work with any object type).
    StampedValuePtr global_parameter = StampedValue::create("global", "value");

    // Try to insert it and associate with non-existing server.
    std::string msg;
    try {
        cbptr_->createUpdateGlobalParameter6(ServerSelector::ONE("server1"),
                                             global_parameter);
        msg = "got no exception";
    } catch (const NullKeyError& ex) {
        msg = ex.what();
    } catch (const std::exception&) {
        msg = "got another exception";
    }
    EXPECT_EQ("server 'server1' does not exist", msg);
}

// Test that ceateUpdateSubnet6 throws appropriate exceptions for various
// server selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, createUpdateSubnet6Selectors) {
    ASSERT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[0]));
    ASSERT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[2]));

    // Supported selectors.
    Subnet6Ptr subnet = test_subnets_[0];
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(),
                                                subnet));
    subnet = test_subnets_[2];
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ONE("server1"),
                                                subnet));
    subnet = test_subnets_[3];
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                                subnet));

    // Not supported server selectors.
    EXPECT_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ANY(), subnet),
                 isc::InvalidOperation);

    // Not implemented server selectors.
    EXPECT_THROW(cbptr_->createUpdateSubnet6(ServerSelector::UNASSIGNED(),
                                             subnet),
                 isc::NotImplemented);
}

// Test that subnet can be inserted, fetched, updated and then fetched again.
TEST_F(MySqlConfigBackendDHCPv6Test, getSubnet6) {
    // Insert the server2 into the database.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[2]));
    {
        SCOPED_TRACE("CREATE audit entry for server");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    auto subnet = test_subnets_[0];
    auto subnet2 = test_subnets_[2];

    // An attempt to add a subnet to a non-existing server (server1) should fail.
    EXPECT_THROW(cbptr_->createUpdateSubnet6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                             subnet2),
                 NullKeyError);

    // The subnet shouldn't have been added, even though one of the servers exists.
    Subnet6Ptr returned_subnet;
    ASSERT_NO_THROW(returned_subnet = cbptr_->getSubnet6(ServerSelector::ONE("server2"),
                                                                             subnet2->getID()));
    EXPECT_FALSE(returned_subnet);

    // Insert two subnets, one for all servers and one for server2.
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet));
    {
        SCOPED_TRACE("CREATE audit entry for the subnet");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::CREATE,
                          "subnet set");
    }


    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ONE("server2"), subnet2));
    {
        SCOPED_TRACE("CREATE audit entry for the subnet");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::CREATE,
                          "subnet set", ServerSelector::ONE("subnet2"),
                          2, 1);
    }

    // We are not going to support selection of a single entry for multiple servers.
    EXPECT_THROW(cbptr_->getSubnet6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                    subnet->getID()),
                 isc::InvalidOperation);

    EXPECT_THROW(cbptr_->getSubnet6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                    subnet->toText()),
                 isc::InvalidOperation);

    // Test that this subnet will be fetched for various server selectors.
    auto test_get_subnet = [this, &subnet] (const std::string& test_case_name,
                                            const ServerSelector& server_selector,
                                            const std::string& expected_tag = ServerTag::ALL) {
        SCOPED_TRACE(test_case_name);

        // Test fetching subnet by id.
        Subnet6Ptr returned_subnet;
        ASSERT_NO_THROW(returned_subnet = cbptr_->getSubnet6(server_selector, subnet->getID()));
        ASSERT_TRUE(returned_subnet);

        ASSERT_EQ(1, returned_subnet->getServerTags().size());
        EXPECT_TRUE(returned_subnet->hasServerTag(ServerTag(expected_tag)));

        EXPECT_EQ(subnet->toElement()->str(), returned_subnet->toElement()->str());

        // Test fetching subnet by prefix.
        ASSERT_NO_THROW(returned_subnet = cbptr_->getSubnet6(server_selector,
                                                             subnet->toText()));
        ASSERT_TRUE(returned_subnet);

        ASSERT_EQ(1, returned_subnet->getServerTags().size());
        EXPECT_TRUE(returned_subnet->hasServerTag(ServerTag(expected_tag)));

        EXPECT_EQ(subnet->toElement()->str(), returned_subnet->toElement()->str());
    };

    {
        SCOPED_TRACE("testing various server selectors before update");
        test_get_subnet("all servers", ServerSelector::ALL());
        test_get_subnet("one server", ServerSelector::ONE("server1"));
        test_get_subnet("any server", ServerSelector::ANY());
    }

    subnet = subnet2;
    {
        SCOPED_TRACE("testing server selectors for another server");
        test_get_subnet("one server", ServerSelector::ONE("server2"), "server2");
        test_get_subnet("any server", ServerSelector::ANY(), "server2");
    }

    // Update the subnet in the database (both use the same ID).
    subnet = test_subnets_[1];
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet));
    {
        SCOPED_TRACE("CREATE audit entry for the subnet");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::UPDATE,
                          "subnet set");
    }

    {
        SCOPED_TRACE("testing various server selectors after update");
        test_get_subnet("all servers", ServerSelector::ALL());
        test_get_subnet("one server", ServerSelector::ONE("server1"));
        test_get_subnet("any server", ServerSelector::ANY());
    }

    // The server2 specific subnet should not be returned if the server selector
    // is not matching.
    EXPECT_FALSE(cbptr_->getSubnet6(ServerSelector::ALL(), subnet2->getID()));
    EXPECT_FALSE(cbptr_->getSubnet6(ServerSelector::ALL(), subnet2->toText()));
    EXPECT_FALSE(cbptr_->getSubnet6(ServerSelector::ONE("server1"), subnet2->getID()));
    EXPECT_FALSE(cbptr_->getSubnet6(ServerSelector::ONE("server1"), subnet2->toText()));

    // Update the subnet in the database (both use the same prefix).
    subnet2.reset(new Subnet6(IOAddress("2001:db8:3::"),
                              64, 30, 40, 50, 80, 8192));
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ONE("server2"),  subnet2));

    // Fetch again and verify.
    returned_subnet = cbptr_->getSubnet6(ServerSelector::ONE("server2"), subnet2->toText());
    ASSERT_TRUE(returned_subnet);
    EXPECT_EQ(subnet2->toElement()->str(), returned_subnet->toElement()->str());

    // Update the subnet when it conflicts same id and same prefix both
    // with different subnets. This should throw.
    // Subnets are 2001:db8:1::/48 id 1024 and 2001:db8:3::/64 id 8192
    subnet2.reset(new Subnet6(IOAddress("2001:db8:1::"),
                              48, 30, 40, 50, 80, 8192));
    EXPECT_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ONE("server2"),  subnet2),
                 DuplicateEntry);
}

// Test that getSubnet6 by ID throws appropriate exceptions for various server
// selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, getSubnet6byIdSelectors) {
    // Supported selectors.
    EXPECT_NO_THROW(cbptr_->getSubnet6(ServerSelector::ANY(), SubnetID(1)));
    EXPECT_NO_THROW(cbptr_->getSubnet6(ServerSelector::UNASSIGNED(), SubnetID(1)));
    EXPECT_NO_THROW(cbptr_->getSubnet6(ServerSelector::ALL(), SubnetID(1)));
    EXPECT_NO_THROW(cbptr_->getSubnet6(ServerSelector::ONE("server1"), SubnetID(1)));

    // Not supported selectors.
    EXPECT_THROW(cbptr_->getSubnet6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                    SubnetID(1)),
                 isc::InvalidOperation);
}

// Test that the information about unspecified optional parameters gets
// propagated to the database.
TEST_F(MySqlConfigBackendDHCPv6Test, getSubnet6WithOptionalUnspecified) {
    // Create a subnet and wrap it within a shared network. It is important
    // to have the shared network to verify that the subnet doesn't inherit
    // the values of the shared network but stores the NULL values in the
    // for those parameters that are unspecified on the subnet level.
    Subnet6Ptr subnet = test_subnets_[2];
    SharedNetwork6Ptr shared_network = test_networks_[0];
    shared_network->add(subnet);

    // Need to add the shared network to the database because otherwise
    // the subnet foreign key would fail.
    cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(), shared_network);
    cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet);

    // Fetch this subnet by subnet identifier.
    Subnet6Ptr returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                                    subnet->getID());
    ASSERT_TRUE(returned_subnet);

    EXPECT_TRUE(returned_subnet->getIface().unspecified());
    EXPECT_TRUE(returned_subnet->getIface().empty());

    EXPECT_TRUE(returned_subnet->getClientClass().unspecified());
    EXPECT_TRUE(returned_subnet->getClientClass().empty());

    EXPECT_TRUE(returned_subnet->getValid().unspecified());
    EXPECT_EQ(0, returned_subnet->getValid().get());

    EXPECT_TRUE(returned_subnet->getPreferred().unspecified());
    EXPECT_EQ(0, returned_subnet->getPreferred().get());

    EXPECT_TRUE(returned_subnet->getT1().unspecified());
    EXPECT_EQ(0, returned_subnet->getT1().get());

    EXPECT_TRUE(returned_subnet->getT2().unspecified());
    EXPECT_EQ(0, returned_subnet->getT2().get());

    EXPECT_TRUE(returned_subnet->getHostReservationMode().unspecified());
    EXPECT_EQ(Network::HR_ALL, returned_subnet->getHostReservationMode().get());

    EXPECT_TRUE(returned_subnet->getCalculateTeeTimes().unspecified());
    EXPECT_FALSE(returned_subnet->getCalculateTeeTimes().get());

    EXPECT_TRUE(returned_subnet->getT1Percent().unspecified());
    EXPECT_EQ(0.0, returned_subnet->getT1Percent().get());

    EXPECT_TRUE(returned_subnet->getT2Percent().unspecified());
    EXPECT_EQ(0.0, returned_subnet->getT2Percent().get());

    EXPECT_TRUE(returned_subnet->getRapidCommit().unspecified());
    EXPECT_FALSE(returned_subnet->getRapidCommit().get());

    EXPECT_FALSE(returned_subnet->getDdnsSendUpdates().unspecified());
    EXPECT_TRUE(returned_subnet->getDdnsSendUpdates().get());

    EXPECT_FALSE(returned_subnet->getDdnsOverrideNoUpdate().unspecified());
    EXPECT_TRUE(returned_subnet->getDdnsOverrideNoUpdate().get());

    EXPECT_FALSE(returned_subnet->getDdnsOverrideClientUpdate().unspecified());
    EXPECT_FALSE(returned_subnet->getDdnsOverrideClientUpdate().get());

    EXPECT_FALSE(returned_subnet->getDdnsReplaceClientNameMode().unspecified());
    EXPECT_EQ(D2ClientConfig::ReplaceClientNameMode::RCM_WHEN_PRESENT,
              returned_subnet->getDdnsReplaceClientNameMode().get());

    EXPECT_FALSE(returned_subnet->getDdnsGeneratedPrefix().unspecified());
    EXPECT_EQ("myhost", returned_subnet->getDdnsGeneratedPrefix().get());

    EXPECT_FALSE(returned_subnet->getDdnsQualifyingSuffix().unspecified());
    EXPECT_EQ("example.org", returned_subnet->getDdnsQualifyingSuffix().get());

    // The easiest way to verify whether the returned subnet matches the inserted
    // subnet is to convert both to text.
    EXPECT_EQ(subnet->toElement()->str(), returned_subnet->toElement()->str());

}

// Test that subnet can be associated with a shared network.
TEST_F(MySqlConfigBackendDHCPv6Test, getSubnet6SharedNetwork) {
    Subnet6Ptr subnet = test_subnets_[0];
    SharedNetwork6Ptr shared_network = test_networks_[0];

    // Add subnet to a shared network.
    shared_network->add(subnet);

    // Store shared network in the database.
    cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(),
                                       shared_network);

    // Store subnet associated with the shared network in the database.
    cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet);

    // Fetch this subnet by subnet identifier.
    Subnet6Ptr returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                                    test_subnets_[0]->getID());
    ASSERT_TRUE(returned_subnet);
    ASSERT_EQ(1, returned_subnet->getServerTags().size());
    EXPECT_EQ("all", returned_subnet->getServerTags().begin()->get());

    // The easiest way to verify whether the returned subnet matches the inserted
    // subnet is to convert both to text.
    EXPECT_EQ(subnet->toElement()->str(), returned_subnet->toElement()->str());

    // However, the check above doesn't verify whether shared network name was
    // correctly returned from the database.
    EXPECT_EQ(shared_network->getName(), returned_subnet->getSharedNetworkName());
}

// Test that subnet can be fetched by prefix.
TEST_F(MySqlConfigBackendDHCPv6Test, getSubnet6ByPrefix) {
    // Insert subnet to the database.
    Subnet6Ptr subnet = test_subnets_[0];
    cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet);

    // Fetch the subnet by prefix.
    Subnet6Ptr returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                                    "2001:db8::/64");
    ASSERT_TRUE(returned_subnet);
    ASSERT_EQ(1, returned_subnet->getServerTags().size());
    EXPECT_EQ("all", returned_subnet->getServerTags().begin()->get());

    // Verify subnet contents.
    EXPECT_EQ(subnet->toElement()->str(), returned_subnet->toElement()->str());

    // Fetching the subnet for an explicitly specified server tag should
    // succeeed too.
    returned_subnet = cbptr_->getSubnet6(ServerSelector::ONE("server1"),
                                         "2001:db8::/64");
    EXPECT_EQ(subnet->toElement()->str(), returned_subnet->toElement()->str());
}

// Test that getSubnet6 by prefix throws appropriate exceptions for various server
// selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, getSubnet6byPrefixSelectors) {
    // Supported selectors.
    EXPECT_NO_THROW(cbptr_->getSubnet6(ServerSelector::ANY(), "192.0.2.0/26"));
    EXPECT_NO_THROW(cbptr_->getSubnet6(ServerSelector::UNASSIGNED(), "192.0.2.0/26"));
    EXPECT_NO_THROW(cbptr_->getSubnet6(ServerSelector::ALL(), "192.0.2.0/26"));
    EXPECT_NO_THROW(cbptr_->getSubnet6(ServerSelector::ONE("server1"), "192.0.2.0/26"));

    // Not supported selectors.
    EXPECT_THROW(cbptr_->getSubnet6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                    "192.0.2.0/26"),
                 isc::InvalidOperation);
}

// Test that all subnets can be fetched and then deleted.
TEST_F(MySqlConfigBackendDHCPv6Test, getAllSubnets6) {
    // Insert test subnets into the database. Note that the second subnet will
    // overwrite the first subnet as they use the same ID.
    for (auto subnet : test_subnets_) {
        cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet);

        // That subnet overrides the first subnet so the audit entry should
        // indicate an update.
        if (subnet->toText() == "2001:db8:1::/48") {
            SCOPED_TRACE("UPDATE audit entry for the subnet " + subnet->toText());
            testNewAuditEntry("dhcp6_subnet",
                              AuditEntry::ModificationType::UPDATE,
                              "subnet set");

        } else {
            SCOPED_TRACE("CREATE audit entry for the subnet " + subnet->toText());
            testNewAuditEntry("dhcp6_subnet",
                              AuditEntry::ModificationType::CREATE,
                              "subnet set");
        }
    }

    // Fetch all subnets.
    Subnet6Collection subnets = cbptr_->getAllSubnets6(ServerSelector::ALL());
    ASSERT_EQ(test_subnets_.size() - 1, subnets.size());

    // All subnets should also be returned for explicitly specified server tag.
    subnets = cbptr_->getAllSubnets6(ServerSelector::ONE("server1"));
    ASSERT_EQ(test_subnets_.size() - 1, subnets.size());

    // See if the subnets are returned ok.
    auto subnet_it = subnets.begin();
    for (auto i = 0; i < subnets.size(); ++i, ++subnet_it) {
        EXPECT_EQ(test_subnets_[i + 1]->toElement()->str(),
                  (*subnet_it)->toElement()->str());
        ASSERT_EQ(1, (*subnet_it)->getServerTags().size());
        EXPECT_EQ("all", (*subnet_it)->getServerTags().begin()->get());
    }

    // Attempt to remove the non existing subnet should  return 0.
    EXPECT_EQ(0, cbptr_->deleteSubnet6(ServerSelector::ALL(), 22));
    EXPECT_EQ(0, cbptr_->deleteSubnet6(ServerSelector::ALL(),
                                       "2001:db8:555::/64"));
    // All subnets should be still there.
    ASSERT_EQ(test_subnets_.size() - 1, subnets.size());

    // Should not delete the subnet for explicit server tag because
    // our subnet is for all servers.
    EXPECT_EQ(0, cbptr_->deleteSubnet6(ServerSelector::ONE("server1"),
                                       test_subnets_[1]->getID()));

    // Also, verify that behavior when deleting by prefix.
    EXPECT_EQ(0, cbptr_->deleteSubnet6(ServerSelector::ONE("server1"),
                                       test_subnets_[2]->toText()));

    // Same for all subnets.
    EXPECT_EQ(0, cbptr_->deleteAllSubnets6(ServerSelector::ONE("server1")));

    // Delete first subnet by id and verify that it is gone.
    EXPECT_EQ(1, cbptr_->deleteSubnet6(ServerSelector::ALL(),
                                       test_subnets_[1]->getID()));

    {
        SCOPED_TRACE("DELETE first subnet audit entry");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::DELETE,
                          "subnet deleted");
    }

    subnets = cbptr_->getAllSubnets6(ServerSelector::ALL());
    ASSERT_EQ(test_subnets_.size() - 2, subnets.size());

    // Delete second subnet by prefix and verify it is gone.
    EXPECT_EQ(1, cbptr_->deleteSubnet6(ServerSelector::ALL(),
                                       test_subnets_[2]->toText()));
    subnets = cbptr_->getAllSubnets6(ServerSelector::ALL());
    ASSERT_EQ(test_subnets_.size() - 3, subnets.size());

    {
        SCOPED_TRACE("DELETE second subnet audit entry");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::DELETE,
                          "subnet deleted");
    }

    // Delete all.
    EXPECT_EQ(1, cbptr_->deleteAllSubnets6(ServerSelector::ALL()));
    subnets = cbptr_->getAllSubnets6(ServerSelector::ALL());
    ASSERT_TRUE(subnets.empty());

    {
        SCOPED_TRACE("DELETE all subnets audit entry");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::DELETE,
                          "deleted all subnets");
    }
}

// Test that getAllSubnets6 throws appropriate exceptions for various
// server selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, getAllSubnets6Selectors) {
    // Supported selectors.
    EXPECT_NO_THROW(cbptr_->getAllSubnets6(ServerSelector::UNASSIGNED()));
    EXPECT_NO_THROW(cbptr_->getAllSubnets6(ServerSelector::ALL()));
    EXPECT_NO_THROW(cbptr_->getAllSubnets6(ServerSelector::ONE("server1")));
    EXPECT_NO_THROW(cbptr_->getAllSubnets6(ServerSelector::MULTIPLE({ "server1", "server2" })));

    // Not supported selectors.
    EXPECT_THROW(cbptr_->getAllSubnets6(ServerSelector::ANY()), isc::InvalidOperation);
}

// Test that subnets with different server associations are returned.
TEST_F(MySqlConfigBackendDHCPv6Test, getAllSubnets6WithServerTags) {
    auto subnet1 = test_subnets_[0];
    auto subnet2 = test_subnets_[2];
    auto subnet3 = test_subnets_[3];

    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[0]));
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[2]));

    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(),
                                                subnet1));
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ONE("server1"),
                                                subnet2));
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                                subnet3));

    Subnet6Collection subnets;

    // All three subnets are associated with the server1.
    EXPECT_NO_THROW(subnets = cbptr_->getAllSubnets6(ServerSelector::ONE("server1")));
    EXPECT_EQ(3, subnets.size());

    // First subnet is associated with all servers.
    auto returned_subnet = SubnetFetcher6::get(subnets, SubnetID(1024));
    ASSERT_TRUE(returned_subnet);
    EXPECT_TRUE(returned_subnet->hasAllServerTag());
    EXPECT_FALSE(returned_subnet->hasServerTag(ServerTag("server1")));
    EXPECT_FALSE(returned_subnet->hasServerTag(ServerTag("server2")));

    // Second subnet is only associated with the server1.
    returned_subnet = SubnetFetcher6::get(subnets, SubnetID(2048));
    ASSERT_TRUE(returned_subnet);
    EXPECT_FALSE(returned_subnet->hasAllServerTag());
    EXPECT_TRUE(returned_subnet->hasServerTag(ServerTag("server1")));
    EXPECT_FALSE(returned_subnet->hasServerTag(ServerTag("server2")));

    // Third subnet is associated with both server1 and server2.
    returned_subnet = SubnetFetcher6::get(subnets, SubnetID(4096));
    ASSERT_TRUE(returned_subnet);
    EXPECT_FALSE(returned_subnet->hasAllServerTag());
    EXPECT_TRUE(returned_subnet->hasServerTag(ServerTag("server1")));
    EXPECT_TRUE(returned_subnet->hasServerTag(ServerTag("server2")));

    // For server2 we should only get two subnets, i.e. first and last.
    EXPECT_NO_THROW(subnets = cbptr_->getAllSubnets6(ServerSelector::ONE("server2")));
    EXPECT_EQ(2, subnets.size());

    // First subnet is associated with all servers.
    returned_subnet = SubnetFetcher6::get(subnets, SubnetID(1024));
    ASSERT_TRUE(returned_subnet);
    EXPECT_TRUE(returned_subnet->hasAllServerTag());
    EXPECT_FALSE(returned_subnet->hasServerTag(ServerTag("server1")));
    EXPECT_FALSE(returned_subnet->hasServerTag(ServerTag("server2")));

    // Last subnet is associated with server1 and server2.
    returned_subnet = SubnetFetcher6::get(subnets, SubnetID(4096));
    ASSERT_TRUE(returned_subnet);
    EXPECT_FALSE(returned_subnet->hasAllServerTag());
    EXPECT_TRUE(returned_subnet->hasServerTag(ServerTag("server1")));
    EXPECT_TRUE(returned_subnet->hasServerTag(ServerTag("server2")));

    // Only the first subnet is associated with all servers.
    EXPECT_NO_THROW(subnets = cbptr_->getAllSubnets6(ServerSelector::ALL()));
    EXPECT_EQ(1, subnets.size());

    returned_subnet = SubnetFetcher6::get(subnets, SubnetID(1024));
    ASSERT_TRUE(returned_subnet);
    EXPECT_TRUE(returned_subnet->hasAllServerTag());
    EXPECT_FALSE(returned_subnet->hasServerTag(ServerTag("server1")));
    EXPECT_FALSE(returned_subnet->hasServerTag(ServerTag("server2")));
}

// Test that selected subnet can be deleted.
TEST_F(MySqlConfigBackendDHCPv6Test, deleteSubnet6) {
    // Create two servers in the database.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[0]));
    {
        SCOPED_TRACE("CREATE audit entry for server");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[2]));
    {
        SCOPED_TRACE("CREATE audit entry for server");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    auto subnet1 = test_subnets_[0];
    auto subnet2 = test_subnets_[2];
    auto subnet3 = test_subnets_[3];

    auto create_test_subnets = [&] () {
        // Insert three subnets, one for all servers, one for server2 and one for two
        // servers: server1 and server2.
        EXPECT_NO_THROW(
            cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet1)
        );
        EXPECT_NO_THROW(
            cbptr_->createUpdateSubnet6(ServerSelector::ONE("server2"), subnet2)
        );
        EXPECT_NO_THROW(
            cbptr_->createUpdateSubnet6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                        subnet3)
        );
    };

    create_test_subnets();

    // Test that subnet is not deleted for a specified server selector.
    auto test_no_delete = [this] (const std::string& test_case_name,
                                  const ServerSelector& server_selector,
                                  const Subnet6Ptr& subnet) {
        SCOPED_TRACE(test_case_name);
        uint64_t deleted_count = 0;
        EXPECT_NO_THROW(
            deleted_count = cbptr_->deleteSubnet6(server_selector, subnet->getID())
        );
        EXPECT_EQ(0, deleted_count);

        deleted_count = 0;
        EXPECT_NO_THROW(
            deleted_count = cbptr_->deleteSubnet6(server_selector, subnet->toText())
        );
        EXPECT_EQ(0, deleted_count);
    };

    {
        SCOPED_TRACE("Test valid but non matching server selectors");
        test_no_delete("selector: one, actual: all", ServerSelector::ONE("server2"),
                       subnet1);
        test_no_delete("selector: all, actual: one", ServerSelector::ALL(),
                       subnet2);
        test_no_delete("selector: all, actual: multiple", ServerSelector::ALL(),
                       subnet3);
    }

    // Test successful deletion of a subnet by ID.
    auto test_delete_by_id = [this] (const std::string& test_case_name,
                                     const ServerSelector& server_selector,
                                     const Subnet6Ptr& subnet) {
        SCOPED_TRACE(test_case_name);
        uint64_t deleted_count = 0;
        EXPECT_NO_THROW(
            deleted_count = cbptr_->deleteSubnet6(server_selector, subnet->getID())
        );
        EXPECT_EQ(1, deleted_count);

        EXPECT_FALSE(cbptr_->getSubnet6(server_selector, subnet->getID()));
    };

    test_delete_by_id("all servers", ServerSelector::ALL(), subnet1);
    test_delete_by_id("any server", ServerSelector::ANY(), subnet2);
    test_delete_by_id("one server", ServerSelector::ONE("server1"), subnet3);

    // Re-create deleted subnets.
    create_test_subnets();

    // Test successful deletion of a subnet by prefix.
    auto test_delete_by_prefix = [this] (const std::string& test_case_name,
                                         const ServerSelector& server_selector,
                                         const Subnet6Ptr& subnet) {
        SCOPED_TRACE(test_case_name);
        uint64_t deleted_count = 0;
        EXPECT_NO_THROW(
            deleted_count = cbptr_->deleteSubnet6(server_selector, subnet->toText())
        );
        EXPECT_EQ(1, deleted_count);

        EXPECT_FALSE(cbptr_->getSubnet6(server_selector, subnet->toText()));
    };

    test_delete_by_prefix("all servers", ServerSelector::ALL(), subnet1);
    test_delete_by_prefix("any server", ServerSelector::ANY(), subnet2);
    test_delete_by_prefix("one server", ServerSelector::ONE("server1"), subnet3);
}

// Test that deleteSubnet6 by ID throws appropriate exceptions for various
// server selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, deleteSubnet6ByIdSelectors) {
    // Supported selectors.
    EXPECT_NO_THROW(cbptr_->deleteSubnet6(ServerSelector::ANY(), SubnetID(1)));
    EXPECT_NO_THROW(cbptr_->deleteSubnet6(ServerSelector::ALL(), SubnetID(1)));
    EXPECT_NO_THROW(cbptr_->deleteSubnet6(ServerSelector::ONE("server1"), SubnetID(1)));

    // Not supported selectors.
    EXPECT_THROW(cbptr_->deleteSubnet6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                           SubnetID(1)),
                 isc::InvalidOperation);

    // Not implemented selectors.
    EXPECT_THROW(cbptr_->deleteSubnet6(ServerSelector::UNASSIGNED(), SubnetID(1)),
                 isc::NotImplemented);
}

// Test that deleteSubnet6 by prefix throws appropriate exceptions for various
// server selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, deleteSubnet6ByPrefixSelectors) {
    // Supported selectors.
    EXPECT_NO_THROW(cbptr_->deleteSubnet6(ServerSelector::ANY(), "192.0.2.0/26"));
    EXPECT_NO_THROW(cbptr_->deleteSubnet6(ServerSelector::ALL(), "192.0.2.0/26"));
    EXPECT_NO_THROW(cbptr_->deleteSubnet6(ServerSelector::ONE("server1"), "192.0.2.0/26"));

    // Not supported selectors.
    EXPECT_THROW(cbptr_->deleteSubnet6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                           "192.0.2.0/26"),
                 isc::InvalidOperation);

    // Not implemented selectors.
    EXPECT_THROW(cbptr_->deleteSubnet6(ServerSelector::UNASSIGNED(), "192.0.2.0/26"),
                 isc::NotImplemented);
}

// Test that deleteAllSubnets6 throws appropriate exceptions for various
// server selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, deleteAllSubnets6Selectors) {
    // Supported selectors.
    EXPECT_NO_THROW(cbptr_->deleteAllSubnets6(ServerSelector::UNASSIGNED()));
    EXPECT_NO_THROW(cbptr_->deleteAllSubnets6(ServerSelector::ALL()));
    EXPECT_NO_THROW(cbptr_->deleteAllSubnets6(ServerSelector::ONE("server1")));

    // Not supported selectors.
    EXPECT_THROW(cbptr_->deleteAllSubnets6(ServerSelector::ANY()),
                 isc::InvalidOperation);
    EXPECT_THROW(cbptr_->deleteAllSubnets6(ServerSelector::MULTIPLE({ "server1", "server2" })),
                 isc::InvalidOperation);
}

// Test that it is possible to retrieve and delete orphaned subnet.
TEST_F(MySqlConfigBackendDHCPv6Test, unassignedSubnet6) {
    // Create the server.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[0]));

    // Create the subnets and associate them with the server1.
    auto subnet = test_subnets_[0];
    auto subnet2 = test_subnets_[2];
    EXPECT_NO_THROW(
        cbptr_->createUpdateSubnet6(ServerSelector::ONE("server1"), subnet)
    );
    EXPECT_NO_THROW(
        cbptr_->createUpdateSubnet6(ServerSelector::ONE("server1"), subnet2)
    );

    // Delete the server. The subnets should be preserved but are considered orphaned,
    // i.e. do not belong to any server.
    uint64_t deleted_count = 0;
    EXPECT_NO_THROW(deleted_count = cbptr_->deleteServer6(ServerTag("server1")));
    EXPECT_EQ(1, deleted_count);

    // Trying to fetch the subnet by server tag should return no result.
    Subnet6Ptr returned_subnet;
    EXPECT_NO_THROW(returned_subnet = cbptr_->getSubnet6(ServerSelector::ONE("server1"),
                                                         subnet->getID()));
    EXPECT_FALSE(returned_subnet);

    // The same if we use other calls.
    EXPECT_NO_THROW(returned_subnet = cbptr_->getSubnet6(ServerSelector::ONE("server1"),
                                                         subnet->toText()));
    EXPECT_FALSE(returned_subnet);

    Subnet6Collection returned_subnets;
    EXPECT_NO_THROW(returned_subnets = cbptr_->getAllSubnets6(ServerSelector::ONE("server1")));
    EXPECT_TRUE(returned_subnets.empty());

    EXPECT_NO_THROW(
        returned_subnets = cbptr_->getModifiedSubnets6(ServerSelector::ONE("server1"),
                                                       timestamps_["two days ago"])
    );
    EXPECT_TRUE(returned_subnets.empty());

    // We should get the subnet if we ask for unassigned.
    EXPECT_NO_THROW(returned_subnet = cbptr_->getSubnet6(ServerSelector::UNASSIGNED(),
                                                         subnet->getID()));
    ASSERT_TRUE(returned_subnet);

    EXPECT_NO_THROW(returned_subnet = cbptr_->getSubnet6(ServerSelector::UNASSIGNED(),
                                                         subnet->toText()));
    ASSERT_TRUE(returned_subnet);

    // Also if we ask for all unassigned subnets it should be returned.
    EXPECT_NO_THROW(returned_subnets = cbptr_->getAllSubnets6(ServerSelector::UNASSIGNED()));
    ASSERT_EQ(2, returned_subnets.size());

    // Same for modified subnets.
    EXPECT_NO_THROW(
        returned_subnets = cbptr_->getModifiedSubnets6(ServerSelector::UNASSIGNED(),
                                                       timestamps_["two days ago"])
    );
    ASSERT_EQ(2, returned_subnets.size());

    // If we ask for any subnet by subnet id, it should be returned too.
    EXPECT_NO_THROW(returned_subnet = cbptr_->getSubnet6(ServerSelector::ANY(),
                                                         subnet->getID()));
    ASSERT_TRUE(returned_subnet);

    EXPECT_NO_THROW(returned_subnet = cbptr_->getSubnet6(ServerSelector::ANY(),
                                                         subnet->toText()));
    ASSERT_TRUE(returned_subnet);

    // Deleting the subnet with the mismatched server tag should not affect our
    // subnet.
    EXPECT_NO_THROW(
        deleted_count = cbptr_->deleteSubnet6(ServerSelector::ONE("server1"),
                                              subnet->getID())
    );
    EXPECT_EQ(0, deleted_count);

    // Also, if we delete all subnets for server1.
    EXPECT_NO_THROW(
        deleted_count = cbptr_->deleteAllSubnets6(ServerSelector::ONE("server1"))
    );
    EXPECT_EQ(0, deleted_count);

    // We can delete this subnet when we specify ANY and the matching id.
    EXPECT_NO_THROW(
        deleted_count = cbptr_->deleteSubnet6(ServerSelector::ANY(), subnet->getID())
    );
    EXPECT_EQ(1, deleted_count);

    // We can delete all subnets using UNASSIGNED selector.
    EXPECT_NO_THROW(
        deleted_count = cbptr_->deleteAllSubnets6(ServerSelector::UNASSIGNED());
    );
    EXPECT_EQ(1, deleted_count);
}

// Test that subnets modified after given time can be fetched.
TEST_F(MySqlConfigBackendDHCPv6Test, getModifiedSubnets6) {
    // Explicitly set timestamps of subnets. First subnet has a timestamp
    // pointing to the future. Second subnet has timestamp pointing to the
    // past (yesterday). Third subnet has a timestamp pointing to the
    // past (an hour ago).
    test_subnets_[1]->setModificationTime(timestamps_["tomorrow"]);
    test_subnets_[2]->setModificationTime(timestamps_["yesterday"]);
    test_subnets_[3]->setModificationTime(timestamps_["today"]);

    // Insert subnets into the database.
    for (int i = 1; i < test_subnets_.size(); ++i) {
        cbptr_->createUpdateSubnet6(ServerSelector::ALL(),
                                    test_subnets_[i]);
    }

    // Fetch subnets with timestamp later than today. Only one subnet
    // should be returned.
    Subnet6Collection
        subnets = cbptr_->getModifiedSubnets6(ServerSelector::ALL(),
                                              timestamps_["after today"]);
    ASSERT_EQ(1, subnets.size());

    // All subnets should also be returned for explicitly specified server tag.
    subnets = cbptr_->getModifiedSubnets6(ServerSelector::ONE("server1"),
                                          timestamps_["after today"]);
    ASSERT_EQ(1, subnets.size());

    // Fetch subnets with timestamp later than yesterday. We should get
    // two subnets.
    subnets = cbptr_->getModifiedSubnets6(ServerSelector::ALL(),
                                          timestamps_["after yesterday"]);
    ASSERT_EQ(2, subnets.size());

    // Fetch subnets with timestamp later than tomorrow. Nothing should
    // be returned.
    subnets = cbptr_->getModifiedSubnets6(ServerSelector::ALL(),
                                          timestamps_["after tomorrow"]);
    ASSERT_TRUE(subnets.empty());
}

// Test that getModifiedSubnets6 throws appropriate exceptions for various
// server selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, getModifiedSubnets6Selectors) {
    // Supported selectors.
    EXPECT_NO_THROW(cbptr_->getModifiedSubnets6(ServerSelector::UNASSIGNED(),
                                                timestamps_["yesterday"]));
    EXPECT_NO_THROW(cbptr_->getModifiedSubnets6(ServerSelector::ALL(),
                                                timestamps_["yesterday"]));
    EXPECT_NO_THROW(cbptr_->getModifiedSubnets6(ServerSelector::ONE("server1"),
                                                timestamps_["yesterday"]));
    EXPECT_NO_THROW(cbptr_->getModifiedSubnets6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                                timestamps_["yesterday"]));

    // Not supported selectors.
    EXPECT_THROW(cbptr_->getModifiedSubnets6(ServerSelector::ANY(),
                                             timestamps_["yesterday"]),
                 isc::InvalidOperation);
}

// Test that lifetimes in subnets are handled as expected.
TEST_F(MySqlConfigBackendDHCPv6Test, subnetLifetime) {
    // Insert new subnet with unspecified valid lifetime
    Triplet<uint32_t> unspecified;
    Subnet6Ptr subnet(new Subnet6(IOAddress("2001:db8::"), 64, 30, 40,
                                  unspecified, unspecified, 1111));
    subnet->setIface("eth1");
    cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet);

    // Fetch this subnet by subnet identifier
    Subnet6Ptr returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                                    subnet->getID());
    ASSERT_TRUE(returned_subnet);

    // Verified returned and original subnets match.
    EXPECT_EQ(subnet->toElement()->str(), returned_subnet->toElement()->str());

    // Update the preferred and valid lifetime.
    subnet->setPreferred( Triplet<uint32_t>(100, 200, 300));
    subnet->setValid( Triplet<uint32_t>(200, 300, 400));
    cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet);

    // Fetch and verify again.
    returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(), subnet->getID());
    ASSERT_TRUE(returned_subnet);
    EXPECT_EQ(subnet->toElement()->str(), returned_subnet->toElement()->str());
}

// Test that subnets belonging to a shared network can be retrieved.
TEST_F(MySqlConfigBackendDHCPv6Test, getSharedNetworkSubnets6) {
    // Assign test subnets to shared networks level1 and level2.
    test_subnets_[1]->setSharedNetworkName("level1");
    test_subnets_[2]->setSharedNetworkName("level2");
    test_subnets_[3]->setSharedNetworkName("level2");

    // Store shared networks in the database.
    for (auto network : test_networks_) {
        cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(), network);
    }

    // Store subnets in the database.
    for (auto subnet : test_subnets_) {
        cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet);
    }

    // Fetch all subnets belonging to shared network level1.
    Subnet6Collection subnets = cbptr_->getSharedNetworkSubnets6(ServerSelector::ALL(),
                                                                 "level1");
    ASSERT_EQ(1, subnets.size());

    // Returned subnet should match test subnet #1.
    EXPECT_TRUE(isEquivalent(test_subnets_[1]->toElement(),
                             (*subnets.begin())->toElement()));

    // All subnets should also be returned for ANY server.
    subnets = cbptr_->getSharedNetworkSubnets6(ServerSelector::ANY(), "level1");
    ASSERT_EQ(1, subnets.size());

    // Returned subnet should match test subnet #1.
    EXPECT_TRUE(isEquivalent(test_subnets_[1]->toElement(),
                             (*subnets.begin())->toElement()));

    // Fetch all subnets belonging to shared network level2.
    subnets = cbptr_->getSharedNetworkSubnets6(ServerSelector::ALL(), "level2");
    ASSERT_EQ(2, subnets.size());

    ElementPtr test_list = Element::createList();
    test_list->add(test_subnets_[2]->toElement());
    test_list->add(test_subnets_[3]->toElement());

    ElementPtr returned_list = Element::createList();
    auto subnet = subnets.begin();
    returned_list->add((*subnet)->toElement());
    returned_list->add((*++subnet)->toElement());

    EXPECT_TRUE(isEquivalent(returned_list, test_list));

    // All subnets should also be returned for explicitly specified server tag.
    subnets = cbptr_->getSharedNetworkSubnets6(ServerSelector::ONE("server1"), "level2");
    ASSERT_EQ(2, subnets.size());

    returned_list = Element::createList();
    subnet = subnets.begin();
    returned_list->add((*subnet)->toElement());
    returned_list->add((*++subnet)->toElement());

    EXPECT_TRUE(isEquivalent(returned_list, test_list));
}

// Test that pools are properly updated as a result a subnet update.
TEST_F(MySqlConfigBackendDHCPv6Test, subnetUpdatePools) {

    auto test_subnet_update = [this](const std::string& subnet_prefix,
                                     const SubnetID& subnet_id) {
        // Add the subnet with two address pools and two prefix delegation
        // pools.
        EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(),
                                                    test_subnets_[0]));
        // Make sure that the pools have been added to the database.
        EXPECT_EQ(2, countRows("dhcp6_pool"));
        EXPECT_EQ(2, countRows("dhcp6_pd_pool"));

        // Create the subnet without options which updates the existing
        // subnet.
        Subnet6Ptr subnet(new Subnet6(IOAddress(subnet_prefix), 64, 30, 60, 50, 60,
                                      subnet_id));
        EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet));
        // Check that options are gone.
        EXPECT_EQ(0, countRows("dhcp6_pool"));
        EXPECT_EQ(0, countRows("dhcp6_pd_pool"));
    };

    {
        SCOPED_TRACE("update subnet, modify subnet id");
        // Create another subnet with the same prefix as the original subnet but
        // different id. This is legal to update the subnet id if the prefix is
        // stable. However, the new subnet has no address pools, so we need to
        // check of the pools associated with the existing subnet instance are
        // gone after the update.
        test_subnet_update("2001:db8::", 2048);
    }

    {
        SCOPED_TRACE("update subnet, modify prefix");
        // Create a subnet with the same subnet id but different prefix.
        // The prefix should be updated.
        test_subnet_update("2001:db9::", 1024);
    }
}

// Test that deleting a subnet triggers deletion of the options associated
// with the subnet and pools.
TEST_F(MySqlConfigBackendDHCPv6Test, subnetOptions) {
    // Add the subnet with two pools and three options.
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(), test_subnets_[0]));
    EXPECT_EQ(2, countRows("dhcp6_pool"));
    EXPECT_EQ(2, countRows("dhcp6_pd_pool"));
    EXPECT_EQ(3, countRows("dhcp6_options"));

    // The second subnet uses the same subnet id, so this operation should replace
    // the existing subnet and its options. The new instance has four pools, each
    // including one option, so we should end up with four options.
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(), test_subnets_[1]));
    EXPECT_EQ(2, countRows("dhcp6_pool"));
    EXPECT_EQ(2, countRows("dhcp6_pd_pool"));
    EXPECT_EQ(4, countRows("dhcp6_options"));

    // Add third subnet with a single option. The number of options in the database
    // should now be 5.
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(), test_subnets_[2]));
    EXPECT_EQ(2, countRows("dhcp6_pool"));
    EXPECT_EQ(2, countRows("dhcp6_pd_pool"));
    EXPECT_EQ(5, countRows("dhcp6_options"));

    // Delete the subnet. All options and pools it contains should also be removed, leaving
    // the last added subnet and its sole option.
    EXPECT_NO_THROW(cbptr_->deleteSubnet6(ServerSelector::ALL(), test_subnets_[1]->getID()));
    EXPECT_EQ(1, countRows("dhcp6_subnet"));
    EXPECT_EQ(0, countRows("dhcp6_pool"));
    EXPECT_EQ(0, countRows("dhcp6_pd_pool"));
    EXPECT_EQ(1, countRows("dhcp6_options"));

    // Add the first subnet again. We should now have 4 options: 3 options from the
    // newly added subnet and one option from the existing subnet.
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(), test_subnets_[0]));
    EXPECT_EQ(2, countRows("dhcp6_pool"));
    EXPECT_EQ(2, countRows("dhcp6_pd_pool"));
    EXPECT_EQ(4, countRows("dhcp6_options"));

    // Delete the subnet including 3 options. The option from the other subnet should not
    // be affected.
    EXPECT_NO_THROW(cbptr_->deleteSubnet6(ServerSelector::ALL(), test_subnets_[0]->getID()));
    EXPECT_EQ(1, countRows("dhcp6_subnet"));
    EXPECT_EQ(0, countRows("dhcp6_pool"));
    EXPECT_EQ(0, countRows("dhcp6_pd_pool"));
    EXPECT_EQ(1, countRows("dhcp6_options"));
}

// Test that shared network can be inserted, fetched, updated and then
// fetched again.
TEST_F(MySqlConfigBackendDHCPv6Test, getSharedNetwork6) {
    // Insert the server2 into the database.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[2]));
    {
        SCOPED_TRACE("CREATE audit entry for server");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    auto shared_network = test_networks_[0];
    auto shared_network2 = test_networks_[2];

    // Insert two shared networks, one for all servers, and one for server2.
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(),
                                                       shared_network));
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ONE("server2"),
                                                       shared_network2));

    // We are not going to support selection of a single entry for multiple servers.
    EXPECT_THROW(cbptr_->getSharedNetwork6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                           test_networks_[0]->getName()),
                 isc::InvalidOperation);

    // Test that this shared network will be fetched for various server selectors.
    auto test_get_network = [this, &shared_network] (const std::string& test_case_name,
                                                     const ServerSelector& server_selector,
                                                     const std::string& expected_tag = ServerTag::ALL) {
        SCOPED_TRACE(test_case_name);
        SharedNetwork6Ptr network;
        ASSERT_NO_THROW(network = cbptr_->getSharedNetwork6(server_selector,
                                                            shared_network->getName()));
        ASSERT_TRUE(network);

        EXPECT_GT(network->getId(), 0);
        ASSERT_EQ(1, network->getServerTags().size());
        EXPECT_EQ(expected_tag, network->getServerTags().begin()->get());

        // The easiest way to verify whether the returned shared network matches the
        // inserted shared network is to convert both to text.
        EXPECT_EQ(shared_network->toElement()->str(), network->toElement()->str());
    };

    {
        SCOPED_TRACE("testing various server selectors before update");
        test_get_network("all servers", ServerSelector::ALL());
        test_get_network("one server", ServerSelector::ONE("server1"));
        test_get_network("any server", ServerSelector::ANY());
    }

    {
        SCOPED_TRACE("CREATE audit entry for a shared network");
        testNewAuditEntry("dhcp6_shared_network",
                          AuditEntry::ModificationType::CREATE,
                          "shared network set");
    }

    // Update shared network in the database.
    shared_network = test_networks_[1];
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(),
                                                       shared_network));

    {
        SCOPED_TRACE("testing various server selectors after update");
        test_get_network("all servers after update", ServerSelector::ALL());
        test_get_network("one server after update", ServerSelector::ONE("server1"));
        test_get_network("any server after update", ServerSelector::ANY());
    }

    {
        SCOPED_TRACE("UPDATE audit entry for a shared network");
        testNewAuditEntry("dhcp6_shared_network",
                          AuditEntry::ModificationType::UPDATE,
                          "shared network set");
    }

    // The server2 specific shared network should not be returned if the
    // server selector is not matching.
    EXPECT_FALSE(cbptr_->getSharedNetwork6(ServerSelector::ALL(),
                                           shared_network2->getName()));
    EXPECT_FALSE(cbptr_->getSharedNetwork6(ServerSelector::ONE("server1"),
                                           shared_network2->getName()));

    {
        SCOPED_TRACE("testing selectors for server2 specific shared network");
        shared_network = shared_network2;
        test_get_network("one server", ServerSelector::ONE("server2"), "server2");
        test_get_network("any server", ServerSelector::ANY(), "server2");
    }
}

// Test that getSharedNetwork6 throws appropriate exceptions for various
// server selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, getSharedNetwork6Selectors) {
    // Supported selectors.
    EXPECT_NO_THROW(cbptr_->getSharedNetwork6(ServerSelector::ANY(), "level1"));
    EXPECT_NO_THROW(cbptr_->getSharedNetwork6(ServerSelector::UNASSIGNED(), "level1"));
    EXPECT_NO_THROW(cbptr_->getSharedNetwork6(ServerSelector::ALL(), "level1"));
    EXPECT_NO_THROW(cbptr_->getSharedNetwork6(ServerSelector::ONE("server1"), "level1"));

    // Not supported selectors.
    EXPECT_THROW(cbptr_->getSharedNetwork6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                           "level1"),
                 isc::InvalidOperation);
}

// Test that shared network may be created and updated and the server tags
// are properly assigned to it.
TEST_F(MySqlConfigBackendDHCPv6Test, createUpdateSharedNetwork6) {
    auto shared_network = test_networks_[0];

    // An attempto insert the shared network for non-existing server should fail.
    EXPECT_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ONE("server1"),
                                                    shared_network),
                 NullKeyError);

    // Insert the server1 into the database.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[0]));
    {
        SCOPED_TRACE("CREATE audit entry for server");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    // Insert the server2 into the database.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[2]));
    {
        SCOPED_TRACE("CREATE audit entry for server");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(),
                                                       shared_network));
    {
        SCOPED_TRACE("CREATE audit entry for shared network and ALL servers");
        testNewAuditEntry("dhcp6_shared_network",
                          AuditEntry::ModificationType::CREATE,
                          "shared network set");
    }

    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                                       shared_network));
    {
        SCOPED_TRACE("UPDATE audit entry for shared network and MULTIPLE servers");
        testNewAuditEntry("dhcp6_shared_network",
                          AuditEntry::ModificationType::UPDATE,
                          "shared network set");
    }

    SharedNetwork6Ptr network;
    ASSERT_NO_THROW(network = cbptr_->getSharedNetwork6(ServerSelector::ANY(),
                                                        shared_network->getName()));
    ASSERT_TRUE(network);
    EXPECT_TRUE(network->hasServerTag(ServerTag("server1")));
    EXPECT_TRUE(network->hasServerTag(ServerTag("server2")));
    EXPECT_FALSE(network->hasServerTag(ServerTag()));
}

// Test that craeteUpdateSharedNetwork6 throws appropriate exceptions for various
// server selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, createUpdateSharedNetwork6Selectors) {
    ASSERT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[0]));
    ASSERT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[2]));

    // Supported selectors.
    SharedNetwork6Ptr shared_network(new SharedNetwork6("all"));
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(),
                                                       shared_network));
    shared_network.reset(new SharedNetwork6("one"));
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ONE("server1"),
                                                       shared_network));
    shared_network.reset(new SharedNetwork6("multiple"));
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                                       shared_network));

    // Not supported server selectors.
    EXPECT_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ANY(), shared_network),
                 isc::InvalidOperation);

    // Not implemented server selectors.
    EXPECT_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::UNASSIGNED(),
                                                    shared_network),
                 isc::NotImplemented);
}

// Test that the information about unspecified optional parameters gets
// propagated to the database.
TEST_F(MySqlConfigBackendDHCPv6Test, getSharedNetwork6WithOptionalUnspecified) {
    // Insert new shared network.
    SharedNetwork6Ptr shared_network = test_networks_[2];
    cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(), shared_network);

    // Fetch this shared network by name.
    SharedNetwork6Ptr
        returned_network = cbptr_->getSharedNetwork6(ServerSelector::ALL(),
                                                     test_networks_[2]->getName());
    ASSERT_TRUE(returned_network);

    EXPECT_TRUE(returned_network->getIface().unspecified());
    EXPECT_TRUE(returned_network->getIface().empty());

    EXPECT_TRUE(returned_network->getClientClass().unspecified());
    EXPECT_TRUE(returned_network->getClientClass().empty());

    EXPECT_TRUE(returned_network->getValid().unspecified());
    EXPECT_EQ(0, returned_network->getValid().get());

    EXPECT_TRUE(returned_network->getPreferred().unspecified());
    EXPECT_EQ(0, returned_network->getPreferred().get());

    EXPECT_TRUE(returned_network->getT1().unspecified());
    EXPECT_EQ(0, returned_network->getT1().get());

    EXPECT_TRUE(returned_network->getT2().unspecified());
    EXPECT_EQ(0, returned_network->getT2().get());

    EXPECT_TRUE(returned_network->getHostReservationMode().unspecified());
    EXPECT_EQ(Network::HR_ALL, returned_network->getHostReservationMode().get());

    EXPECT_TRUE(returned_network->getCalculateTeeTimes().unspecified());
    EXPECT_FALSE(returned_network->getCalculateTeeTimes().get());

    EXPECT_TRUE(returned_network->getT1Percent().unspecified());
    EXPECT_EQ(0.0, returned_network->getT1Percent().get());

    EXPECT_TRUE(returned_network->getT2Percent().unspecified());
    EXPECT_EQ(0.0, returned_network->getT2Percent().get());

    EXPECT_TRUE(returned_network->getRapidCommit().unspecified());
    EXPECT_FALSE(returned_network->getRapidCommit().get());

    EXPECT_FALSE(returned_network->getDdnsSendUpdates().unspecified());
    EXPECT_TRUE(returned_network->getDdnsSendUpdates().get());

    EXPECT_FALSE(returned_network->getDdnsOverrideNoUpdate().unspecified());
    EXPECT_TRUE(returned_network->getDdnsOverrideNoUpdate().get());

    EXPECT_FALSE(returned_network->getDdnsOverrideClientUpdate().unspecified());
    EXPECT_FALSE(returned_network->getDdnsOverrideClientUpdate().get());

    EXPECT_FALSE(returned_network->getDdnsReplaceClientNameMode().unspecified());
    EXPECT_EQ(D2ClientConfig::ReplaceClientNameMode::RCM_WHEN_PRESENT,
              returned_network->getDdnsReplaceClientNameMode().get());

    EXPECT_FALSE(returned_network->getDdnsGeneratedPrefix().unspecified());
    EXPECT_EQ("myhost", returned_network->getDdnsGeneratedPrefix().get());

    EXPECT_FALSE(returned_network->getDdnsQualifyingSuffix().unspecified());
    EXPECT_EQ("example.org", returned_network->getDdnsQualifyingSuffix().get());
}

// Test that deleteSharedNetworkSubnets6 with not ANY selector throw.
TEST_F(MySqlConfigBackendDHCPv6Test, deleteSharedNetworkSubnets6) {
    EXPECT_THROW(cbptr_->deleteSharedNetworkSubnets6(ServerSelector::UNASSIGNED(),
                                                     test_networks_[1]->getName()),
                 isc::InvalidOperation);
    EXPECT_THROW(cbptr_->deleteSharedNetworkSubnets6(ServerSelector::ALL(),
                                                     test_networks_[1]->getName()),
                 isc::InvalidOperation);
    EXPECT_THROW(cbptr_->deleteSharedNetworkSubnets6(ServerSelector::ONE("server1"),
                                                     test_networks_[1]->getName()),
                 isc::InvalidOperation);
    EXPECT_THROW(cbptr_->deleteSharedNetworkSubnets6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                                     test_networks_[1]->getName()),
                 isc::InvalidOperation);
}

// Test that all shared networks can be fetched.
TEST_F(MySqlConfigBackendDHCPv6Test, getAllSharedNetworks6) {
    // Insert test shared networks into the database. Note that the second shared
    // network will overwrite the first shared network as they use the same name.
    for (auto network : test_networks_) {
        cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(), network);

        // That shared network overrides the first one so the audit entry should
        // indicate an update.
        if ((network->getName() == "level1") && (!audit_entries_["all"].empty())) {
            SCOPED_TRACE("UPDATE audit entry for the shared network " +
                         network->getName());
            testNewAuditEntry("dhcp6_shared_network",
                              AuditEntry::ModificationType::UPDATE,
                              "shared network set");

        } else {
            SCOPED_TRACE("CREATE audit entry for the shared network " +
                         network->getName());
            testNewAuditEntry("dhcp6_shared_network",
                              AuditEntry::ModificationType::CREATE,
                              "shared network set");
        }
    }

    // Fetch all shared networks.
    SharedNetwork6Collection networks =
        cbptr_->getAllSharedNetworks6(ServerSelector::ALL());
    ASSERT_EQ(test_networks_.size() - 1, networks.size());

    // All shared networks should also be returned for explicitly specified
    // server tag.
    networks = cbptr_->getAllSharedNetworks6(ServerSelector::ONE("server1"));
    ASSERT_EQ(test_networks_.size() - 1, networks.size());

    // See if shared networks are returned ok.
    for (auto i = 0; i < networks.size(); ++i) {
        EXPECT_EQ(test_networks_[i + 1]->toElement()->str(),
                  networks[i]->toElement()->str());
        ASSERT_EQ(1, networks[i]->getServerTags().size());
        EXPECT_EQ("all", networks[i]->getServerTags().begin()->get());
    }

    // Add some subnets.
    test_networks_[1]->add(test_subnets_[0]);
    test_subnets_[2]->setSharedNetworkName("level2");
    test_networks_[2]->add(test_subnets_[3]);
    cbptr_->createUpdateSubnet6(ServerSelector::ALL(), test_subnets_[0]);
    cbptr_->createUpdateSubnet6(ServerSelector::ALL(), test_subnets_[2]);
    cbptr_->createUpdateSubnet6(ServerSelector::ALL(), test_subnets_[3]);

    // Both ways to attach a subnet are equivalent.
    Subnet6Ptr subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                           test_subnets_[0]->getID());
    ASSERT_TRUE(subnet);
    EXPECT_EQ("level1", subnet->getSharedNetworkName());

    {
        SCOPED_TRACE("CREATE audit entry for subnets");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::CREATE,
                          "subnet set", ServerSelector::ALL(), 3);
    }

    // Deleting non-existing shared network should return 0.
    EXPECT_EQ(0, cbptr_->deleteSharedNetwork6(ServerSelector::ALL(),
                                              "big-fish"));
    // All shared networks should be still there.
    ASSERT_EQ(test_networks_.size() - 1, networks.size());

    // Should not delete the shared network for explicit server tag
    // because our shared network is for all servers.
    EXPECT_EQ(0, cbptr_->deleteSharedNetwork6(ServerSelector::ONE("server1"),
                                              test_networks_[1]->getName()));

    // Same for all shared networks.
    EXPECT_EQ(0, cbptr_->deleteAllSharedNetworks6(ServerSelector::ONE("server1")));

    // Delete first shared network with it subnets and verify it is gone.
    // Begin by its subnet.
    EXPECT_EQ(1, cbptr_->deleteSharedNetworkSubnets6(ServerSelector::ANY(),
                                                     test_networks_[1]->getName()));

    {
        SCOPED_TRACE("DELETE audit entry for subnets of the first shared network");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::DELETE,
                          "deleted all subnets for a shared network");
    }

    // Check that the subnet is gone..
    subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                test_subnets_[0]->getID());
    EXPECT_FALSE(subnet);

    // And after the shared network itself.
    EXPECT_EQ(1, cbptr_->deleteSharedNetwork6(ServerSelector::ALL(),
                                              test_networks_[1]->getName()));
    networks = cbptr_->getAllSharedNetworks6(ServerSelector::ALL());
    ASSERT_EQ(test_networks_.size() - 2, networks.size());

    {
        SCOPED_TRACE("DELETE audit entry for the first shared network");
        testNewAuditEntry("dhcp6_shared_network",
                          AuditEntry::ModificationType::DELETE,
                          "shared network deleted");
    }

    // Delete all.
    EXPECT_EQ(2, cbptr_->deleteAllSharedNetworks6(ServerSelector::ALL()));
    networks = cbptr_->getAllSharedNetworks6(ServerSelector::ALL());
    ASSERT_TRUE(networks.empty());

    {
        SCOPED_TRACE("DELETE audit entry for the remaining two shared networks");
        // The last parameter indicates that we expect two new audit entries.
        testNewAuditEntry("dhcp6_shared_network",
                          AuditEntry::ModificationType::DELETE,
                          "deleted all shared networks",
                          ServerSelector::ALL(), 2);
    }

    // Check that subnets are still there but detached.
    subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                test_subnets_[2]->getID());
    ASSERT_TRUE(subnet);
    EXPECT_TRUE(subnet->getSharedNetworkName().empty());
    subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                test_subnets_[3]->getID());
    ASSERT_TRUE(subnet);
    EXPECT_TRUE(subnet->getSharedNetworkName().empty());
}

// Test that getAllSharedNetworks6 throws appropriate exceptions for various
// server selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, getAllSharedNetworks6Selectors) {
    // Supported selectors.
    EXPECT_NO_THROW(cbptr_->getAllSharedNetworks6(ServerSelector::UNASSIGNED()));
    EXPECT_NO_THROW(cbptr_->getAllSharedNetworks6(ServerSelector::ALL()));
    EXPECT_NO_THROW(cbptr_->getAllSharedNetworks6(ServerSelector::ONE("server1")));
    EXPECT_NO_THROW(cbptr_->getAllSharedNetworks6(ServerSelector::MULTIPLE({ "server1", "server2" })));

    // Not supported selectors.
    EXPECT_THROW(cbptr_->getAllSharedNetworks6(ServerSelector::ANY()),
                 isc::InvalidOperation);
}

// Test that shared networks with different server associations are returned.
TEST_F(MySqlConfigBackendDHCPv6Test, getAllSharedNetworks6WithServerTags) {
    auto shared_network1 = test_networks_[0];
    auto shared_network2 = test_networks_[2];
    auto shared_network3 = test_networks_[3];

    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[0]));
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[2]));

    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(),
                                                       shared_network1));
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ONE("server1"),
                                                       shared_network2));
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                                       shared_network3));

    SharedNetwork6Collection networks;

    // All three networks are associated with the server1.
    EXPECT_NO_THROW(networks = cbptr_->getAllSharedNetworks6(ServerSelector::ONE("server1")));
    EXPECT_EQ(3, networks.size());

    // First network is associated with all servers.
    auto returned_network = SharedNetworkFetcher6::get(networks, "level1");
    ASSERT_TRUE(returned_network);
    EXPECT_TRUE(returned_network->hasAllServerTag());
    EXPECT_FALSE(returned_network->hasServerTag(ServerTag("server1")));
    EXPECT_FALSE(returned_network->hasServerTag(ServerTag("server2")));

    // Second network is only associated with the server1.
    returned_network = SharedNetworkFetcher6::get(networks, "level2");
    ASSERT_TRUE(returned_network);
    EXPECT_FALSE(returned_network->hasAllServerTag());
    EXPECT_TRUE(returned_network->hasServerTag(ServerTag("server1")));
    EXPECT_FALSE(returned_network->hasServerTag(ServerTag("server2")));

    // Third network is associated with both server1 and server2.
    returned_network = SharedNetworkFetcher6::get(networks, "level3");
    ASSERT_TRUE(returned_network);
    EXPECT_FALSE(returned_network->hasAllServerTag());
    EXPECT_TRUE(returned_network->hasServerTag(ServerTag("server1")));
    EXPECT_TRUE(returned_network->hasServerTag(ServerTag("server2")));

    // For server2 we should only get two shared networks, i.e. first and last.
    EXPECT_NO_THROW(networks = cbptr_->getAllSharedNetworks6(ServerSelector::ONE("server2")));
    EXPECT_EQ(2, networks.size());

    // First shared network is associated with all servers.
    returned_network = SharedNetworkFetcher6::get(networks, "level1");
    ASSERT_TRUE(returned_network);
    EXPECT_TRUE(returned_network->hasAllServerTag());
    EXPECT_FALSE(returned_network->hasServerTag(ServerTag("server1")));
    EXPECT_FALSE(returned_network->hasServerTag(ServerTag("server2")));

    // Last shared network is associated with server1 and server2.
    returned_network = SharedNetworkFetcher6::get(networks, "level3");
    ASSERT_TRUE(returned_network);
    EXPECT_FALSE(returned_network->hasAllServerTag());
    EXPECT_TRUE(returned_network->hasServerTag(ServerTag("server1")));
    EXPECT_TRUE(returned_network->hasServerTag(ServerTag("server2")));

    // Only the first shared network is associated with all servers.
    EXPECT_NO_THROW(networks = cbptr_->getAllSharedNetworks6(ServerSelector::ALL()));
    EXPECT_EQ(1, networks.size());

    returned_network = SharedNetworkFetcher6::get(networks, "level1");
    ASSERT_TRUE(returned_network);
    EXPECT_TRUE(returned_network->hasAllServerTag());
    EXPECT_FALSE(returned_network->hasServerTag(ServerTag("server1")));
    EXPECT_FALSE(returned_network->hasServerTag(ServerTag("server2")));
}

// Test that shared networks modified after given time can be fetched.
TEST_F(MySqlConfigBackendDHCPv6Test, getModifiedSharedNetworks6) {
    // Explicitly set timestamps of shared networks. First shared
    // network has a timestamp pointing to the future. Second shared
    // network has timestamp pointing to the past (yesterday).
    // Third shared network has a timestamp pointing to the
    // past (an hour ago).
    test_networks_[1]->setModificationTime(timestamps_["tomorrow"]);
    test_networks_[2]->setModificationTime(timestamps_["yesterday"]);
    test_networks_[3]->setModificationTime(timestamps_["today"]);

    // Insert shared networks into the database.
    for (int i = 1; i < test_networks_.size(); ++i) {
        cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(),
                                           test_networks_[i]);
    }

    // Fetch shared networks with timestamp later than today. Only one
    // shared network  should be returned.
    SharedNetwork6Collection
        networks = cbptr_->getModifiedSharedNetworks6(ServerSelector::ALL(),
                                                      timestamps_["after today"]);
    ASSERT_EQ(1, networks.size());

    // Fetch shared networks with timestamp later than yesterday. We
    // should get two shared networks.
    networks = cbptr_->getModifiedSharedNetworks6(ServerSelector::ALL(),
                                                 timestamps_["after yesterday"]);
    ASSERT_EQ(2, networks.size());

    // Fetch shared networks with timestamp later than tomorrow. Nothing
    // should be returned.
    networks = cbptr_->getModifiedSharedNetworks6(ServerSelector::ALL(),
                                                  timestamps_["after tomorrow"]);
    ASSERT_TRUE(networks.empty());
}

// Test that getModifiedSharedNetworks6 throws appropriate exceptions for various
// server selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, getModifiedSharedNetworks6Selectors) {
    // Supported selectors.
    EXPECT_NO_THROW(cbptr_->getModifiedSharedNetworks6(ServerSelector::UNASSIGNED(),
                                                       timestamps_["yesterday"]));
    EXPECT_NO_THROW(cbptr_->getModifiedSharedNetworks6(ServerSelector::ALL(),
                                                       timestamps_["yesterday"]));
    EXPECT_NO_THROW(cbptr_->getModifiedSharedNetworks6(ServerSelector::ONE("server1"),
                                                       timestamps_["yesterday"]));
    EXPECT_NO_THROW(cbptr_->getModifiedSharedNetworks6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                                       timestamps_["yesterday"]));

    // Not supported selectors.
    EXPECT_THROW(cbptr_->getModifiedSharedNetworks6(ServerSelector::ANY(),
                                                    timestamps_["yesterday"]),
                 isc::InvalidOperation);
}

// Test that selected shared network can be deleted.
TEST_F(MySqlConfigBackendDHCPv6Test, deleteSharedNetwork6) {
    // Create two servers in the database.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[0]));
    {
        SCOPED_TRACE("CREATE audit entry for server");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[2]));
    {
        SCOPED_TRACE("CREATE audit entry for server");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    auto shared_network1 = test_networks_[0];
    auto shared_network2 = test_networks_[2];
    auto shared_network3 = test_networks_[3];

    // Insert two shared networks, one for all servers, and one for server2.
    EXPECT_NO_THROW(
        cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(), shared_network1)
    );
    EXPECT_NO_THROW(
        cbptr_->createUpdateSharedNetwork6(ServerSelector::ONE("server2"), shared_network2)
    );
    EXPECT_NO_THROW(
        cbptr_->createUpdateSharedNetwork6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                           shared_network3)
    );

    auto test_no_delete = [this] (const std::string& test_case_name,
                                  const ServerSelector& server_selector,
                                  const SharedNetwork6Ptr& shared_network) {
        SCOPED_TRACE(test_case_name);
        uint64_t deleted_count = 0;
        EXPECT_NO_THROW(
            deleted_count = cbptr_->deleteSharedNetwork6(server_selector,
                                                         shared_network->getName())
        );
        EXPECT_EQ(0, deleted_count);
    };

    {
        SCOPED_TRACE("Test valid but non matching server selectors");
        test_no_delete("selector: one, actual: all", ServerSelector::ONE("server2"),
                       shared_network1);
        test_no_delete("selector: all, actual: one", ServerSelector::ALL(),
                       shared_network2);
        test_no_delete("selector: all, actual: multiple", ServerSelector::ALL(),
                       shared_network3);
    }

    // We are not going to support deletion of a single entry for multiple servers.
    EXPECT_THROW(cbptr_->deleteSharedNetwork6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                              shared_network3->getName()),
                 isc::InvalidOperation);

    // We currently don't support deleting a shared network with specifying
    // an unassigned server tag. Use ANY to delete any subnet instead.
    EXPECT_THROW(cbptr_->deleteSharedNetwork6(ServerSelector::UNASSIGNED(),
                                              shared_network1->getName()),
                 isc::NotImplemented);

    // Test successful deletion of a shared network.
    auto test_delete = [this] (const std::string& test_case_name,
                               const ServerSelector& server_selector,
                               const SharedNetwork6Ptr& shared_network) {
        SCOPED_TRACE(test_case_name);
        uint64_t deleted_count = 0;
        EXPECT_NO_THROW(
            deleted_count = cbptr_->deleteSharedNetwork6(server_selector,
                                                         shared_network->getName())
        );
        EXPECT_EQ(1, deleted_count);

        EXPECT_FALSE(cbptr_->getSharedNetwork6(server_selector,
                                               shared_network->getName()));
    };

    test_delete("all servers", ServerSelector::ALL(), shared_network1);
    test_delete("any server", ServerSelector::ANY(), shared_network2);
    test_delete("one server", ServerSelector::ONE("server1"), shared_network3);
}

// Test that deleteSharedNetwork6 throws appropriate exceptions for various
// server selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, deleteSharedNetwork6Selectors) {
    // Supported selectors.
    EXPECT_NO_THROW(cbptr_->deleteSharedNetwork6(ServerSelector::ANY(), "level1"));
    EXPECT_NO_THROW(cbptr_->deleteSharedNetwork6(ServerSelector::ALL(), "level1"));
    EXPECT_NO_THROW(cbptr_->deleteSharedNetwork6(ServerSelector::ONE("server1"), "level1"));

    // Not supported selectors.
    EXPECT_THROW(cbptr_->deleteSharedNetwork6(ServerSelector::MULTIPLE({ "server1", "server2" }),
                                           "level1"),
                 isc::InvalidOperation);

    // Not implemented selectors.
    EXPECT_THROW(cbptr_->deleteSharedNetwork6(ServerSelector::UNASSIGNED(), "level1"),
                 isc::NotImplemented);
}

// Test that deleteAllSharedNetworks6 throws appropriate exceptions for various
// server selectors.
TEST_F(MySqlConfigBackendDHCPv6Test, deleteAllSharedNetworks6Selectors) {
    // Supported selectors.
    EXPECT_NO_THROW(cbptr_->deleteAllSharedNetworks6(ServerSelector::UNASSIGNED()));
    EXPECT_NO_THROW(cbptr_->deleteAllSharedNetworks6(ServerSelector::ALL()));
    EXPECT_NO_THROW(cbptr_->deleteAllSharedNetworks6(ServerSelector::ONE("server1")));

    // Not supported selectors.
    EXPECT_THROW(cbptr_->deleteAllSharedNetworks6(ServerSelector::ANY()),
                 isc::InvalidOperation);
    EXPECT_THROW(cbptr_->deleteAllSharedNetworks6(ServerSelector::MULTIPLE({ "server1", "server2" })),
                 isc::InvalidOperation);
}

// Test that it is possible to retrieve and delete orphaned shared network.
TEST_F(MySqlConfigBackendDHCPv6Test, unassignedSharedNetwork) {
    // Create the server.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[0]));

    // Create the shared networks and associate them with the server1.
    auto shared_network = test_networks_[0];
    auto shared_network2 = test_networks_[2];
    EXPECT_NO_THROW(
        cbptr_->createUpdateSharedNetwork6(ServerSelector::ONE("server1"), shared_network)
    );
    EXPECT_NO_THROW(
        cbptr_->createUpdateSharedNetwork6(ServerSelector::ONE("server1"), shared_network2)
    );

    // Delete the server. The shared networks should be preserved but are
    // considered orphaned, i.e. do not belong to any server.
    uint64_t deleted_count = 0;
    EXPECT_NO_THROW(deleted_count = cbptr_->deleteServer6(ServerTag("server1")));
    EXPECT_EQ(1, deleted_count);

    // Trying to fetch this shared network by server tag should return no result.
    SharedNetwork6Ptr returned_network;
    EXPECT_NO_THROW(returned_network = cbptr_->getSharedNetwork6(ServerSelector::ONE("server1"),
                                                                 "level1"));
    EXPECT_FALSE(returned_network);

    // The same if we use other calls.
    SharedNetwork6Collection returned_networks;
    EXPECT_NO_THROW(
        returned_networks = cbptr_->getAllSharedNetworks6(ServerSelector::ONE("server1"))
    );
    EXPECT_TRUE(returned_networks.empty());

    EXPECT_NO_THROW(
        returned_networks = cbptr_->getModifiedSharedNetworks6(ServerSelector::ONE("server1"),
                                                               timestamps_["two days ago"])
    );
    EXPECT_TRUE(returned_networks.empty());

    // We should get the shared network if we ask for unassigned.
    EXPECT_NO_THROW(returned_network = cbptr_->getSharedNetwork6(ServerSelector::UNASSIGNED(),
                                                                 "level1"));
    ASSERT_TRUE(returned_network);

    // Also if we ask for all unassigned networks it should be returned.
    EXPECT_NO_THROW(returned_networks = cbptr_->getAllSharedNetworks6(ServerSelector::UNASSIGNED()));
    ASSERT_EQ(2, returned_networks.size());

    // And all modified.
    EXPECT_NO_THROW(
        returned_networks = cbptr_->getModifiedSharedNetworks6(ServerSelector::UNASSIGNED(),
                                                               timestamps_["two days ago"])
    );
    ASSERT_EQ(2, returned_networks.size());

    // If we ask for any network by name, it should be returned too.
    EXPECT_NO_THROW(returned_network = cbptr_->getSharedNetwork6(ServerSelector::ANY(),
                                                                 "level1"));
    ASSERT_TRUE(returned_network);

    // Deleting a shared network with the mismatched server tag should not affect
    // our shared network.
    EXPECT_NO_THROW(
        deleted_count = cbptr_->deleteSharedNetwork6(ServerSelector::ONE("server1"),
                                                     "level1")
    );
    EXPECT_EQ(0, deleted_count);

    // Also, if we delete all shared networks for server1.
    EXPECT_NO_THROW(
        deleted_count = cbptr_->deleteAllSharedNetworks6(ServerSelector::ONE("server1"))
    );
    EXPECT_EQ(0, deleted_count);

    // We can delete this shared network when we specify ANY and the matching name.
    EXPECT_NO_THROW(
        deleted_count = cbptr_->deleteSharedNetwork6(ServerSelector::ANY(), "level1")
    );
    EXPECT_EQ(1, deleted_count);

    // We can delete all second networks using UNASSIGNED selector.
    EXPECT_NO_THROW(
        deleted_count = cbptr_->deleteAllSharedNetworks6(ServerSelector::UNASSIGNED());
    );
    EXPECT_EQ(1, deleted_count);
}

// Test that lifetimes in shared networks are handled as expected.
TEST_F(MySqlConfigBackendDHCPv6Test, sharedNetworkLifetime) {
    // Insert new shared network with unspecified valid lifetime
    SharedNetwork6Ptr network(new SharedNetwork6("foo"));
    Triplet<uint32_t> unspecified;
    network->setPreferred(unspecified);
    network->setValid(unspecified);
    network->setIface("eth1");
    cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(), network);

    // Fetch this shared network.
    SharedNetwork6Ptr returned_network =
        cbptr_->getSharedNetwork6(ServerSelector::ALL(), "foo");
    ASSERT_TRUE(returned_network);

    // Verified returned and original shared networks match.
    EXPECT_EQ(network->toElement()->str(),
              returned_network->toElement()->str());

    // Update the preferred and valid lifetime.
    network->setPreferred( Triplet<uint32_t>(100, 200, 300));
    network->setValid( Triplet<uint32_t>(200, 300, 400));
    cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(), network);

    // Fetch and verify again.
    returned_network = cbptr_->getSharedNetwork6(ServerSelector::ALL(), "foo");
    ASSERT_TRUE(returned_network);
    EXPECT_EQ(network->toElement()->str(),
              returned_network->toElement()->str());
}

// Test that deleting a shared network triggers deletion of the options
// associated with the shared network.
TEST_F(MySqlConfigBackendDHCPv6Test, sharedNetworkOptions) {
    // Add shared network with three options.
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(), test_networks_[0]));
    EXPECT_EQ(3, countRows("dhcp6_options"));

    // Add another shared network with a single option. The numnber of options in the
    // database should now be 4.
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(), test_networks_[2]));
    EXPECT_EQ(4, countRows("dhcp6_options"));

    // The second shared network uses the same name as the first shared network, so
    // this operation should replace the existing shared network and its options.
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(), test_networks_[1]));
    EXPECT_EQ(1, countRows("dhcp6_options"));

    // Remove the shared network. This should not affect options assigned to the
    // other shared network.
    EXPECT_NO_THROW(cbptr_->deleteSharedNetwork6(ServerSelector::ALL(),
                                                 test_networks_[1]->getName()));
    EXPECT_EQ(1, countRows("dhcp6_shared_network"));
    EXPECT_EQ(1, countRows("dhcp6_options"));

    // Create the first option again. The number of options should be equal to the
    // sum of options associated with both shared networks.
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(), test_networks_[0]));
    EXPECT_EQ(4, countRows("dhcp6_options"));

    // Delete this shared netwiork. This should not affect the option associated
    // with the remaining shared network.
    EXPECT_NO_THROW(cbptr_->deleteSharedNetwork6(ServerSelector::ALL(),
                                                 test_networks_[0]->getName()));
    EXPECT_EQ(1, countRows("dhcp6_shared_network"));
    EXPECT_EQ(1, countRows("dhcp6_options"));
}

// Test that option definition can be inserted, fetched, updated and then
// fetched again.
TEST_F(MySqlConfigBackendDHCPv6Test, getOptionDef6) {
    // Insert new option definition.
    OptionDefinitionPtr option_def = test_option_defs_[0];
    cbptr_->createUpdateOptionDef6(ServerSelector::ALL(), option_def);

    // Fetch this option_definition by subnet identifier.
    OptionDefinitionPtr returned_option_def =
        cbptr_->getOptionDef6(ServerSelector::ALL(),
                              test_option_defs_[0]->getCode(),
                              test_option_defs_[0]->getOptionSpaceName());
    ASSERT_TRUE(returned_option_def);
    EXPECT_GT(returned_option_def->getId(), 0);
    ASSERT_EQ(1, returned_option_def->getServerTags().size());
    EXPECT_EQ("all", returned_option_def->getServerTags().begin()->get());

    EXPECT_TRUE(returned_option_def->equals(*option_def));

    {
        SCOPED_TRACE("CREATE audit entry for an option definition");
        testNewAuditEntry("dhcp6_option_def",
                          AuditEntry::ModificationType::CREATE,
                          "option definition set");
    }

    // Update the option definition in the database.
    OptionDefinitionPtr option_def2 = test_option_defs_[1];
    cbptr_->createUpdateOptionDef6(ServerSelector::ALL(), option_def2);

    // Fetch updated option definition and see if it matches.
    returned_option_def = cbptr_->getOptionDef6(ServerSelector::ALL(),
                                                test_option_defs_[1]->getCode(),
                                                test_option_defs_[1]->getOptionSpaceName());
    EXPECT_TRUE(returned_option_def->equals(*option_def2));

    // Fetching option definition for an explicitly specified server tag
    // should succeed too.
    returned_option_def = cbptr_->getOptionDef6(ServerSelector::ONE("server1"),
                                                test_option_defs_[1]->getCode(),
                                                test_option_defs_[1]->getOptionSpaceName());
    EXPECT_TRUE(returned_option_def->equals(*option_def2));

    {
        SCOPED_TRACE("UPDATE audit entry for an option definition");
        testNewAuditEntry("dhcp6_option_def",
                          AuditEntry::ModificationType::UPDATE,
                          "option definition set");
    }
}

// This test verifies that it is possible to differentiate between the
// option definitions by server tag and that the option definition
// specified for the particular server overrides the definition for
// all servers.
TEST_F(MySqlConfigBackendDHCPv6Test, optionDefs6WithServerTags) {
    OptionDefinitionPtr option1 = test_option_defs_[0];
    OptionDefinitionPtr option2 = test_option_defs_[1];
    OptionDefinitionPtr option3 = test_option_defs_[4];

    // An attempt to create option definition for non-existing server should
    // fail.
    EXPECT_THROW(cbptr_->createUpdateOptionDef6(ServerSelector::ONE("server1"),
                                                option1),
                 NullKeyError);

    // Create two servers.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[1]));
    {
        SCOPED_TRACE("server1 is created");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[2]));
    {
        SCOPED_TRACE("server2 is created");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    // This time creation of the option definition for the server1 should pass.
    EXPECT_NO_THROW(cbptr_->createUpdateOptionDef6(ServerSelector::ONE("server1"),
                                                   option1));
    {
        SCOPED_TRACE("option definition for server1 is set");
        // The value of 3 means there should be 3 audit entries available for the
        // server1, two that indicate creation of the servers and one that we
        // validate, which sets the option definition.
        testNewAuditEntry("dhcp6_option_def",
                          AuditEntry::ModificationType::CREATE,
                          "option definition set",
                          ServerSelector::ONE("server1"),
                          3, 1);
    }

    // Creation of the option definition for the server2 should also pass.
    EXPECT_NO_THROW(cbptr_->createUpdateOptionDef6(ServerSelector::ONE("server2"),
                                                   option2));
    {
        SCOPED_TRACE("option definition for server2 is set");
        // Same as in case of the server1, there should be 3 audit entries and
        // we validate one of them.
        testNewAuditEntry("dhcp6_option_def",
                          AuditEntry::ModificationType::CREATE,
                          "option definition set",
                          ServerSelector::ONE("server2"),
                          3, 1);
    }

    // Finally, creation of the option definition for all servers should
    // also pass.
    EXPECT_NO_THROW(cbptr_->createUpdateOptionDef6(ServerSelector::ALL(),
                                                   option3));
    {
        SCOPED_TRACE("option definition for server2 is set");
        // There should be one new audit entry for all servers. It logs
        // the insertion of the option definition.
        testNewAuditEntry("dhcp6_option_def",
                          AuditEntry::ModificationType::CREATE,
                          "option definition set",
                          ServerSelector::ALL(),
                          1, 1);
    }

    OptionDefinitionPtr returned_option_def;

    // Try to fetch the option definition specified for all servers. It should
    // return the third one.
    EXPECT_NO_THROW(
        returned_option_def = cbptr_->getOptionDef6(ServerSelector::ALL(),
                                                    option3->getCode(),
                                                    option3->getOptionSpaceName())
    );
    ASSERT_TRUE(returned_option_def);
    EXPECT_TRUE(returned_option_def->equals(*option3));

    // Try to fetch the option definition specified for server1. It should
    // override the definition for all servers.
    EXPECT_NO_THROW(
        returned_option_def = cbptr_->getOptionDef6(ServerSelector::ONE("server1"),
                                                    option1->getCode(),
                                                    option1->getOptionSpaceName())
    );
    ASSERT_TRUE(returned_option_def);
    EXPECT_TRUE(returned_option_def->equals(*option1));

    // The same in case of the server2.
    EXPECT_NO_THROW(
        returned_option_def = cbptr_->getOptionDef6(ServerSelector::ONE("server2"),
                                                    option2->getCode(),
                                                    option2->getOptionSpaceName())
    );
    ASSERT_TRUE(returned_option_def);
    EXPECT_TRUE(returned_option_def->equals(*option2));

    OptionDefContainer returned_option_defs;

    // Try to fetch the collection of the option definitions for server1, server2
    // and server3. The server3 does not have an explicit option definition, so
    // for this server we should get the definition associated with "all" servers.
    EXPECT_NO_THROW(
        returned_option_defs = cbptr_->getAllOptionDefs6(ServerSelector::
                                                         MULTIPLE({ "server1", "server2",
                                                                    "server3" }));
    );
    ASSERT_EQ(3, returned_option_defs.size());

    // Check that expected option definitions have been returned.
    auto current_option = returned_option_defs.begin();
    EXPECT_TRUE((*current_option)->equals(*option1));
    EXPECT_TRUE((*(++current_option))->equals(*option2));
    EXPECT_TRUE((*(++current_option))->equals(*option3));

    // Try to fetch the collection of options specified for all servers.
    // This excludes the options specific to server1 and server2. It returns
    // only the common ones.
    EXPECT_NO_THROW(
        returned_option_defs = cbptr_->getAllOptionDefs6(ServerSelector::ALL());

    );
    ASSERT_EQ(1, returned_option_defs.size());
    EXPECT_TRUE((*returned_option_defs.begin())->equals(*option3));

    // Delete the server1. It should remove associations of this server with the
    // option definitions and the option definition itself.
    EXPECT_NO_THROW(cbptr_->deleteServer6(ServerTag("server1")));
    EXPECT_NO_THROW(
        returned_option_defs = cbptr_->getAllOptionDefs6(ServerSelector::ONE("server1"));

    );
    ASSERT_EQ(1, returned_option_defs.size());
    EXPECT_TRUE((*returned_option_defs.begin())->equals(*option3));

    {
        SCOPED_TRACE("DELETE audit entry for the option definition after server deletion");
        testNewAuditEntry("dhcp6_option_def",
                          AuditEntry::ModificationType::DELETE,
                          "deleting a server", ServerSelector::ONE("server1"),
                          2, 1);
    }

    // Attempt to delete option definition for server1.
    uint64_t deleted_num = 0;
    EXPECT_NO_THROW(deleted_num = cbptr_->deleteOptionDef6(ServerSelector::ONE("server1"),
                                                           option1->getCode(),
                                                           option1->getOptionSpaceName()));
    EXPECT_EQ(0, deleted_num);

    // Deleting the existing option definition for server2 should succeed.
    EXPECT_NO_THROW(deleted_num = cbptr_->deleteOptionDef6(ServerSelector::ONE("server2"),
                                                           option2->getCode(),
                                                           option2->getOptionSpaceName()));
    EXPECT_EQ(1, deleted_num);

    // Create this option definition again to test that deletion of all servers
    // removes it too.
    EXPECT_NO_THROW(cbptr_->createUpdateOptionDef6(ServerSelector::ONE("server2"),
                                                   option2));

    // Delete all servers, except 'all'.
    EXPECT_NO_THROW(deleted_num = cbptr_->deleteAllServers6());
    EXPECT_NO_THROW(
        returned_option_defs = cbptr_->getAllOptionDefs6(ServerSelector::ALL());
    );
    EXPECT_EQ(1, deleted_num);
    EXPECT_EQ(1, returned_option_defs.size());
    EXPECT_TRUE((*returned_option_defs.begin())->equals(*option3));

    {
        SCOPED_TRACE("DELETE audit entry for the option definition after deletion of"
                     " all servers");
        testNewAuditEntry("dhcp6_option_def",
                          AuditEntry::ModificationType::DELETE,
                          "deleting all servers", ServerSelector::ONE("server2"),
                          4, 1);
    }
}

// Test that all option definitions can be fetched.
TEST_F(MySqlConfigBackendDHCPv6Test, getAllOptionDefs6) {
    // Insert test option definitions into the database. Note that the second
    // option definition will overwrite the first option definition as they use
    // the same code and space.
    size_t updates_num = 0;
    for (auto option_def : test_option_defs_) {
        cbptr_->createUpdateOptionDef6(ServerSelector::ALL(), option_def);

        // That option definition overrides the first one so the audit entry should
        // indicate an update.
        if (option_def->getName() == "bar") {
            SCOPED_TRACE("UPDATE audit entry for the option definition " +
                         option_def->getName());
            testNewAuditEntry("dhcp6_option_def",
                              AuditEntry::ModificationType::UPDATE,
                              "option definition set");
            ++updates_num;

        } else {
            SCOPED_TRACE("CREATE audit entry for the option definition " +
                         option_def->getName());
            testNewAuditEntry("dhcp6_option_def",
                              AuditEntry::ModificationType::CREATE,
                              "option definition set");
        }
    }

    // Fetch all option_definitions.
    OptionDefContainer option_defs = cbptr_->getAllOptionDefs6(ServerSelector::ALL());
    ASSERT_EQ(test_option_defs_.size() - updates_num, option_defs.size());

    // All option definitions should also be returned for explicitly specified
    // server tag.
    option_defs = cbptr_->getAllOptionDefs6(ServerSelector::ONE("server1"));
    ASSERT_EQ(test_option_defs_.size() - updates_num, option_defs.size());

    // See if option definitions are returned ok.
    for (auto def = option_defs.begin(); def != option_defs.end(); ++def) {
        ASSERT_EQ(1, (*def)->getServerTags().size());
        EXPECT_EQ("all", (*def)->getServerTags().begin()->get());
        bool success = false;
        for (auto i = 1; i < test_option_defs_.size(); ++i) {
            if ((*def)->equals(*test_option_defs_[i])) {
                success = true;
            }
        }
        ASSERT_TRUE(success) << "failed for option definition " << (*def)->getCode()
            << ", option space " << (*def)->getOptionSpaceName();
    }

    // Deleting non-existing option definition should return 0.
    EXPECT_EQ(0, cbptr_->deleteOptionDef6(ServerSelector::ALL(),
                                          99, "non-exiting-space"));
    // All option definitions should be still there.
    ASSERT_EQ(test_option_defs_.size() - updates_num, option_defs.size());

    // Should not delete option definition for explicit server tag
    // because our option definition is for all servers.
    EXPECT_EQ(0, cbptr_->deleteOptionDef6(ServerSelector::ONE("server1"),
                                          test_option_defs_[1]->getCode(),
                                          test_option_defs_[1]->getOptionSpaceName()));

    // Same for all option definitions.
    EXPECT_EQ(0, cbptr_->deleteAllOptionDefs6(ServerSelector::ONE("server1")));

    // Delete one of the option definitions and see if it is gone.
    EXPECT_EQ(1, cbptr_->deleteOptionDef6(ServerSelector::ALL(),
                                          test_option_defs_[2]->getCode(),
                                          test_option_defs_[2]->getOptionSpaceName()));
    ASSERT_FALSE(cbptr_->getOptionDef6(ServerSelector::ALL(),
                                       test_option_defs_[2]->getCode(),
                                       test_option_defs_[2]->getOptionSpaceName()));

    {
        SCOPED_TRACE("DELETE audit entry for the first option definition");
        testNewAuditEntry("dhcp6_option_def",
                          AuditEntry::ModificationType::DELETE,
                          "option definition deleted");
    }

    // Delete all remaining option definitions.
    EXPECT_EQ(2, cbptr_->deleteAllOptionDefs6(ServerSelector::ALL()));
    option_defs = cbptr_->getAllOptionDefs6(ServerSelector::ALL());
    ASSERT_TRUE(option_defs.empty());

    {
        SCOPED_TRACE("DELETE audit entries for the remaining option definitions");
        // The last parameter indicates that we expect two new audit entries.
        testNewAuditEntry("dhcp6_option_def",
                          AuditEntry::ModificationType::DELETE,
                          "deleted all option definitions",
                          ServerSelector::ALL(), 2);
    }
}

// Test that option definitions modified after given time can be fetched.
TEST_F(MySqlConfigBackendDHCPv6Test, getModifiedOptionDefs6) {
    // Explicitly set timestamps of option definitions. First option
    // definition has a timestamp pointing to the future. Second option
    // definition has timestamp pointing to the past (yesterday).
    // Third option definitions has a timestamp pointing to the
    // past (an hour ago).
    test_option_defs_[1]->setModificationTime(timestamps_["tomorrow"]);
    test_option_defs_[2]->setModificationTime(timestamps_["yesterday"]);
    test_option_defs_[3]->setModificationTime(timestamps_["today"]);

    // Insert option definitions into the database.
    for (int i = 1; i < test_networks_.size(); ++i) {
        cbptr_->createUpdateOptionDef6(ServerSelector::ALL(),
                                       test_option_defs_[i]);
    }

    // Fetch option definitions with timestamp later than today. Only one
    // option definition should be returned.
    OptionDefContainer
        option_defs = cbptr_->getModifiedOptionDefs6(ServerSelector::ALL(),
                                                     timestamps_["after today"]);
    ASSERT_EQ(1, option_defs.size());

    // Fetch option definitions with timestamp later than yesterday. We
    // should get two option definitions.
    option_defs = cbptr_->getModifiedOptionDefs6(ServerSelector::ALL(),
                                                 timestamps_["after yesterday"]);
    ASSERT_EQ(2, option_defs.size());

    // Fetch option definitions with timestamp later than tomorrow. Nothing
    // should be returned.
    option_defs = cbptr_->getModifiedOptionDefs6(ServerSelector::ALL(),
                                              timestamps_["after tomorrow"]);
    ASSERT_TRUE(option_defs.empty());
}

// This test verifies that global option can be added, updated and deleted.
TEST_F(MySqlConfigBackendDHCPv6Test, createUpdateDeleteOption6) {
    // Add option to the database.
    OptionDescriptorPtr opt_posix_timezone = test_options_[0];
    cbptr_->createUpdateOption6(ServerSelector::ALL(),
                                opt_posix_timezone);

    // Make sure we can retrieve this option and that it is equal to the
    // option we have inserted into the database.
    OptionDescriptorPtr returned_opt_posix_timezone =
        cbptr_->getOption6(ServerSelector::ALL(),
                           opt_posix_timezone->option_->getType(),
                           opt_posix_timezone->space_name_);
    ASSERT_TRUE(returned_opt_posix_timezone);

    {
        SCOPED_TRACE("verify created option");
        testOptionsEquivalent(*opt_posix_timezone,
                              *returned_opt_posix_timezone);
    }

    {
        SCOPED_TRACE("CREATE audit entry for an option");
        testNewAuditEntry("dhcp6_options",
                          AuditEntry::ModificationType::CREATE,
                          "global option set");
    }

    // Modify option and update it in the database.
    opt_posix_timezone->persistent_ = !opt_posix_timezone->persistent_;
    cbptr_->createUpdateOption6(ServerSelector::ALL(),
                                opt_posix_timezone);

    // Retrieve the option again and make sure that updates were
    // properly propagated to the database. Use explicit server selector
    // which should also return this option.
    returned_opt_posix_timezone = cbptr_->getOption6(ServerSelector::ONE("server1"),
                                                     opt_posix_timezone->option_->getType(),
                                                     opt_posix_timezone->space_name_);
    ASSERT_TRUE(returned_opt_posix_timezone);

    {
        SCOPED_TRACE("verify updated option");
        testOptionsEquivalent(*opt_posix_timezone,
                              *returned_opt_posix_timezone);
    }

    {
        SCOPED_TRACE("UPDATE audit entry for an option");
        testNewAuditEntry("dhcp6_options",
                          AuditEntry::ModificationType::UPDATE,
                          "global option set");
    }

    // Deleting an option with explicitly specified server tag should fail.
    EXPECT_EQ(0, cbptr_->deleteOption6(ServerSelector::ONE("server1"),
                                       opt_posix_timezone->option_->getType(),
                                       opt_posix_timezone->space_name_));

    // Deleting option for all servers should succeed.
    EXPECT_EQ(1, cbptr_->deleteOption6(ServerSelector::ALL(),
                                       opt_posix_timezone->option_->getType(),
                                       opt_posix_timezone->space_name_));

    EXPECT_FALSE(cbptr_->getOption6(ServerSelector::ALL(),
                                    opt_posix_timezone->option_->getType(),
                                    opt_posix_timezone->space_name_));

    {
        SCOPED_TRACE("DELETE audit entry for an option");
        testNewAuditEntry("dhcp6_options",
                          AuditEntry::ModificationType::DELETE,
                          "global option deleted");
    }
}

// This test verifies that it is possible to differentiate between the
// global options by server tag and that the option specified for the
// particular server overrides the value specified for all servers.
TEST_F(MySqlConfigBackendDHCPv6Test, globalOptions6WithServerTags) {
    OptionDescriptorPtr opt_timezone1 = test_options_[0];
    OptionDescriptorPtr opt_timezone2 = test_options_[6];
    OptionDescriptorPtr opt_timezone3 = test_options_[7];

    EXPECT_THROW(cbptr_->createUpdateOption6(ServerSelector::ONE("server1"),
                                             opt_timezone1),
                 NullKeyError);

    // Create two servers.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[1]));
    {
        SCOPED_TRACE("server1 is created");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[2]));
    {
        SCOPED_TRACE("server2 is created");
        testNewAuditEntry("dhcp6_server",
                          AuditEntry::ModificationType::CREATE,
                          "server set");
    }

    EXPECT_NO_THROW(cbptr_->createUpdateOption6(ServerSelector::ONE("server1"),
                                                opt_timezone1));
    {
        SCOPED_TRACE("global option for server1 is set");
        // The value of 3 means there should be 3 audit entries available for the
        // server1, two that indicate creation of the servers and one that we
        // validate, which sets the global option.
        testNewAuditEntry("dhcp6_options",
                          AuditEntry::ModificationType::CREATE,
                          "global option set",
                          ServerSelector::ONE("server1"),
                          3, 1);

    }

    EXPECT_NO_THROW(cbptr_->createUpdateOption6(ServerSelector::ONE("server2"),
                                                opt_timezone2));
    {
        SCOPED_TRACE("global option for server2 is set");
        // Same as in case of the server1, there should be 3 audit entries and
        // we validate one of them.
        testNewAuditEntry("dhcp6_options",
                          AuditEntry::ModificationType::CREATE,
                          "global option set",
                          ServerSelector::ONE("server2"),
                          3, 1);

    }

    EXPECT_NO_THROW(cbptr_->createUpdateOption6(ServerSelector::ALL(),
                                                opt_timezone3));
    {
        SCOPED_TRACE("global option for all servers is set");
        // There should be one new audit entry for all servers. It logs
        // the insertion of the global option.
        testNewAuditEntry("dhcp6_options",
                          AuditEntry::ModificationType::CREATE,
                          "global option set",
                          ServerSelector::ALL(),
                          1, 1);

    }

    OptionDescriptorPtr returned_option;

    // Try to fetch the option specified for all servers. It should return
    // the third option.
    EXPECT_NO_THROW(
        returned_option = cbptr_->getOption6(ServerSelector::ALL(),
                                             opt_timezone3->option_->getType(),
                                             opt_timezone3->space_name_);
    );
    ASSERT_TRUE(returned_option);
    testOptionsEquivalent(*opt_timezone3, *returned_option);

    // Try to fetch the option specified for the server1. It should override the
    // option specified for all servers.
    EXPECT_NO_THROW(
        returned_option = cbptr_->getOption6(ServerSelector::ONE("server1"),
                                             opt_timezone1->option_->getType(),
                                             opt_timezone1->space_name_);
    );
    ASSERT_TRUE(returned_option);
    testOptionsEquivalent(*opt_timezone1, *returned_option);

    // The same in case of the server2.
    EXPECT_NO_THROW(
        returned_option = cbptr_->getOption6(ServerSelector::ONE("server2"),
                                             opt_timezone2->option_->getType(),
                                             opt_timezone2->space_name_);
    );
    ASSERT_TRUE(returned_option);
    testOptionsEquivalent(*opt_timezone2, *returned_option);

    OptionContainer returned_options;

    // Try to fetch the collection of global options for the server1, server2
    // and server3. The server3 does not have an explicit value so for this server
    // we should get the option associated with "all" servers.
    EXPECT_NO_THROW(
        returned_options = cbptr_->getAllOptions6(ServerSelector::
                                                  MULTIPLE({ "server1", "server2",
                                                             "server3" }));
    );
    ASSERT_EQ(3, returned_options.size());

    // Check that expected options have been returned.
    auto current_option = returned_options.begin();
    testOptionsEquivalent(*opt_timezone1, *current_option);
    testOptionsEquivalent(*opt_timezone2, *(++current_option));
    testOptionsEquivalent(*opt_timezone3, *(++current_option));

    // Try to fetch the collection of options specified for all servers.
    // This excludes the options specific to server1 and server2. It returns
    // only the common ones.
    EXPECT_NO_THROW(
        returned_options = cbptr_->getAllOptions6(ServerSelector::ALL());
    );
    ASSERT_EQ(1, returned_options.size());
    testOptionsEquivalent(*opt_timezone3, *returned_options.begin());

    // Delete the server1. It should remove associations of this server with the
    // option and the option itself.
    EXPECT_NO_THROW(cbptr_->deleteServer6(ServerTag("server1")));
    EXPECT_NO_THROW(
        returned_options = cbptr_->getAllOptions6(ServerSelector::ONE("server1"));
    );
    ASSERT_EQ(1, returned_options.size());
    testOptionsEquivalent(*opt_timezone3, *returned_options.begin());

    {
        SCOPED_TRACE("DELETE audit entry for the global option after server deletion");
        testNewAuditEntry("dhcp6_options",
                          AuditEntry::ModificationType::DELETE,
                          "deleting a server", ServerSelector::ONE("server1"),
                          2, 1);
    }

    // Attempt to delete global option for server1.
    uint64_t deleted_num = 0;
    EXPECT_NO_THROW(deleted_num = cbptr_->deleteOption6(ServerSelector::ONE("server1"),
                                                        opt_timezone1->option_->getType(),
                                                        opt_timezone1->space_name_));
    EXPECT_EQ(0, deleted_num);

    // Deleting the existing option for server2 should succeed.
    EXPECT_NO_THROW(deleted_num = cbptr_->deleteOption6(ServerSelector::ONE("server2"),
                                                        opt_timezone2->option_->getType(),
                                                        opt_timezone2->space_name_));
    EXPECT_EQ(1, deleted_num);

    // Create this option again to test that deletion of all servers removes it too.
    EXPECT_NO_THROW(cbptr_->createUpdateOption6(ServerSelector::ONE("server2"),
                                                opt_timezone2));

    // Delete all servers, except 'all'.
    EXPECT_NO_THROW(deleted_num = cbptr_->deleteAllServers6());
    EXPECT_NO_THROW(
        returned_options = cbptr_->getAllOptions6(ServerSelector::ALL());
    );
    EXPECT_EQ(1, deleted_num);
    ASSERT_EQ(1, returned_options.size());
    testOptionsEquivalent(*opt_timezone3, *returned_options.begin());

    {
        SCOPED_TRACE("DELETE audit entry for the global option after deletion of"
                     " all servers");
        testNewAuditEntry("dhcp6_options",
                          AuditEntry::ModificationType::DELETE,
                          "deleting all servers", ServerSelector::ONE("server2"),
                          4, 1);
    }
}

// This test verifies that all global options can be retrieved.
TEST_F(MySqlConfigBackendDHCPv6Test, getAllOptions6) {
    // Add three global options to the database.
    cbptr_->createUpdateOption6(ServerSelector::ALL(),
                                test_options_[0]);
    cbptr_->createUpdateOption6(ServerSelector::ALL(),
                                test_options_[1]);
    cbptr_->createUpdateOption6(ServerSelector::ALL(),
                                test_options_[5]);

    // Retrieve all these options.
    OptionContainer returned_options = cbptr_->getAllOptions6(ServerSelector::ALL());
    ASSERT_EQ(3, returned_options.size());

    // Fetching global options with explicitly specified server tag should return
    // the same result.
    returned_options = cbptr_->getAllOptions6(ServerSelector::ONE("server1"));
    ASSERT_EQ(3, returned_options.size());

    // Get the container index used to search options by option code.
    const OptionContainerTypeIndex& index = returned_options.get<1>();

    // Verify that all options we put into the database were
    // returned.
    {
        SCOPED_TRACE("verify test_options_[0]");
        auto option0 = index.find(test_options_[0]->option_->getType());
        ASSERT_FALSE(option0 == index.end());
        testOptionsEquivalent(*test_options_[0], *option0);
        EXPECT_GT(option0->getId(), 0);
        ASSERT_EQ(1, option0->getServerTags().size());
        EXPECT_EQ("all", option0->getServerTags().begin()->get());
    }

    {
        SCOPED_TRACE("verify test_options_[1]");
        auto option1 = index.find(test_options_[1]->option_->getType());
        ASSERT_FALSE(option1 == index.end());
        testOptionsEquivalent(*test_options_[1], *option1);
        EXPECT_GT(option1->getId(), 0);
        ASSERT_EQ(1, option1->getServerTags().size());
        EXPECT_EQ("all", option1->getServerTags().begin()->get());
    }

    {
        SCOPED_TRACE("verify test_options_[5]");
        auto option5 = index.find(test_options_[5]->option_->getType());
        ASSERT_FALSE(option5 == index.end());
        testOptionsEquivalent(*test_options_[5], *option5);
        EXPECT_GT(option5->getId(), 0);
        ASSERT_EQ(1, option5->getServerTags().size());
        EXPECT_EQ("all", option5->getServerTags().begin()->get());
    }
}

// This test verifies that modified global options can be retrieved.
TEST_F(MySqlConfigBackendDHCPv6Test, getModifiedOptions6) {
    // Assign timestamps to the options we're going to store in the
    // database.
    test_options_[0]->setModificationTime(timestamps_["tomorrow"]);
    test_options_[1]->setModificationTime(timestamps_["yesterday"]);
    test_options_[5]->setModificationTime(timestamps_["today"]);

    // Put options into the database.
    cbptr_->createUpdateOption6(ServerSelector::ALL(),
                                test_options_[0]);
    cbptr_->createUpdateOption6(ServerSelector::ALL(),
                                test_options_[1]);
    cbptr_->createUpdateOption6(ServerSelector::ALL(),
                                test_options_[5]);

    // Get options with the timestamp later than today. Only
    // one option should be returned.
    OptionContainer returned_options =
        cbptr_->getModifiedOptions6(ServerSelector::ALL(),
                                    timestamps_["after today"]);
    ASSERT_EQ(1, returned_options.size());

    // Fetching modified options with explicitly specified server selector
    // should return the same result.
    returned_options = cbptr_->getModifiedOptions6(ServerSelector::ONE("server1"),
                                                   timestamps_["after today"]);
    ASSERT_EQ(1, returned_options.size());

    // The returned option should be the one with the timestamp
    // set to tomorrow.
    const OptionContainerTypeIndex& index = returned_options.get<1>();
    auto option0 = index.find(test_options_[0]->option_->getType());
    ASSERT_FALSE(option0 == index.end());
    {
        SCOPED_TRACE("verify returned option");
        testOptionsEquivalent(*test_options_[0], *option0);
    }
}

// This test verifies that subnet level option can be added, updated and
// deleted.
TEST_F(MySqlConfigBackendDHCPv6Test, createUpdateDeleteSubnetOption6) {
    // Insert new subnet.
    Subnet6Ptr subnet = test_subnets_[1];
    cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet);

    // Fetch this subnet by subnet identifier.
    Subnet6Ptr returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                                    subnet->getID());
    ASSERT_TRUE(returned_subnet);

    {
        SCOPED_TRACE("CREATE audit entry for a new subnet");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::CREATE,
                          "subnet set");
    }

    // The inserted subnet contains four options.
    ASSERT_EQ(4, countRows("dhcp6_options"));

    OptionDescriptorPtr opt_posix_timezone = test_options_[0];
    cbptr_->createUpdateOption6(ServerSelector::ANY(), subnet->getID(),
                                opt_posix_timezone);

    returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                         subnet->getID());
    ASSERT_TRUE(returned_subnet);

    OptionDescriptor returned_opt_posix_timezone =
        returned_subnet->getCfgOption()->get(DHCP6_OPTION_SPACE, D6O_NEW_POSIX_TIMEZONE);
    ASSERT_TRUE(returned_opt_posix_timezone.option_);

    {
        SCOPED_TRACE("verify returned option");
        testOptionsEquivalent(*opt_posix_timezone, returned_opt_posix_timezone);
        EXPECT_GT(returned_opt_posix_timezone.getId(), 0);
    }

    {
        SCOPED_TRACE("UPDATE audit entry for an added subnet option");
        // Instead of adding an audit entry for an option we add an audit
        // entry for the entire subnet so as the server refreshes the
        // subnet with the new option. Note that the server doesn't
        // have means to retrieve only the newly added option.
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::UPDATE,
                          "subnet specific option set");
    }

    // We have added one option to the existing subnet. We should now have
    // five options.
    ASSERT_EQ(5, countRows("dhcp6_options"));

    opt_posix_timezone->persistent_ = !opt_posix_timezone->persistent_;
    cbptr_->createUpdateOption6(ServerSelector::ANY(), subnet->getID(),
                                opt_posix_timezone);

    returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                         subnet->getID());
    ASSERT_TRUE(returned_subnet);
    returned_opt_posix_timezone =
        returned_subnet->getCfgOption()->get(DHCP6_OPTION_SPACE, D6O_NEW_POSIX_TIMEZONE);
    ASSERT_TRUE(returned_opt_posix_timezone.option_);

    {
        SCOPED_TRACE("verify returned option with modified persistence");
        testOptionsEquivalent(*opt_posix_timezone, returned_opt_posix_timezone);
    }

    {
        SCOPED_TRACE("UPDATE audit entry for an updated subnet option");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::UPDATE,
                          "subnet specific option set");
    }

    // Updating the option should replace the existing instance with the new
    // instance. Therefore, we should still have five options.
    ASSERT_EQ(5, countRows("dhcp6_options"));

    // It should succeed for any server.
    EXPECT_EQ(1, cbptr_->deleteOption6(ServerSelector::ANY(), subnet->getID(),
                                       opt_posix_timezone->option_->getType(),
                                       opt_posix_timezone->space_name_));

    returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                         subnet->getID());
    ASSERT_TRUE(returned_subnet);

    EXPECT_FALSE(returned_subnet->getCfgOption()->get(DHCP6_OPTION_SPACE, D6O_NEW_POSIX_TIMEZONE).option_);

    {
        SCOPED_TRACE("UPDATE audit entry for a deleted subnet option");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::UPDATE,
                          "subnet specific option deleted");
    }

    // We should have only four options after deleting one of them.
    ASSERT_EQ(4, countRows("dhcp6_options"));
}

// This test verifies that option can be inserted, updated and deleted
// from the pool.
TEST_F(MySqlConfigBackendDHCPv6Test, createUpdateDeletePoolOption6) {
    // Insert new subnet.
    Subnet6Ptr subnet = test_subnets_[1];
    cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet);

    {
        SCOPED_TRACE("CREATE audit entry for a subnet");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::CREATE,
                          "subnet set");
    }

    // Inserted subnet has four options.
    ASSERT_EQ(4, countRows("dhcp6_options"));

    // Add an option into the pool.
    const PoolPtr pool = subnet->getPool(Lease::TYPE_NA,
                                         IOAddress("2001:db8::10"));
    ASSERT_TRUE(pool);
    OptionDescriptorPtr opt_posix_timezone = test_options_[0];
    cbptr_->createUpdateOption6(ServerSelector::ANY(),
                                pool->getFirstAddress(),
                                pool->getLastAddress(),
                                opt_posix_timezone);

    // Query for a subnet.
    Subnet6Ptr returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                                    subnet->getID());
    ASSERT_TRUE(returned_subnet);

    // The returned subnet should include our pool.
    const PoolPtr returned_pool = returned_subnet->getPool(Lease::TYPE_NA,
                                                           IOAddress("2001:db8::10"));
    ASSERT_TRUE(returned_pool);

    // The pool should contain option we added earlier.
    OptionDescriptor returned_opt_posix_timezone =
        returned_pool->getCfgOption()->get(DHCP6_OPTION_SPACE, D6O_NEW_POSIX_TIMEZONE);
    ASSERT_TRUE(returned_opt_posix_timezone.option_);

    {
        SCOPED_TRACE("verify returned pool option");
        testOptionsEquivalent(*opt_posix_timezone, returned_opt_posix_timezone);
        EXPECT_GT(returned_opt_posix_timezone.getId(), 0);
    }

    {
        SCOPED_TRACE("UPDATE audit entry for a subnet after adding an option "
                     "to the address pool");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::UPDATE,
                          "address pool specific option set");
    }

    // With the newly inserted option we should now have five options.
    ASSERT_EQ(5, countRows("dhcp6_options"));

    // Modify the option and update it in the database.
    opt_posix_timezone->persistent_ = !opt_posix_timezone->persistent_;
    cbptr_->createUpdateOption6(ServerSelector::ANY(),
                                pool->getFirstAddress(),
                                pool->getLastAddress(),
                                opt_posix_timezone);

    // Fetch the subnet and the corresponding pool.
    returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                         subnet->getID());
    ASSERT_TRUE(returned_subnet);
    const PoolPtr returned_pool1 = returned_subnet->getPool(Lease::TYPE_NA,
                                                            IOAddress("2001:db8::10"));
    ASSERT_TRUE(returned_pool1);

    // Test that the option has been correctly updated in the database.
    returned_opt_posix_timezone =
        returned_pool1->getCfgOption()->get(DHCP6_OPTION_SPACE, D6O_NEW_POSIX_TIMEZONE);
    ASSERT_TRUE(returned_opt_posix_timezone.option_);

    {
        SCOPED_TRACE("verify updated option with modified persistence");
        testOptionsEquivalent(*opt_posix_timezone, returned_opt_posix_timezone);
    }

    {
        SCOPED_TRACE("UPDATE audit entry for a subnet when updating "
                     "address  pool specific option");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::UPDATE,
                          "address pool specific option set");
    }

    // The new option instance should replace the existing one, so we should
    // still have five options.
    ASSERT_EQ(5, countRows("dhcp6_options"));

    // Delete option for any server should succeed.
    EXPECT_EQ(1, cbptr_->deleteOption6(ServerSelector::ANY(),
                                       pool->getFirstAddress(),
                                       pool->getLastAddress(),
                                       opt_posix_timezone->option_->getType(),
                                       opt_posix_timezone->space_name_));

    // Fetch the subnet and the pool from the database again to make sure
    // that the option is really gone.
    returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                         subnet->getID());
    ASSERT_TRUE(returned_subnet);
    const PoolPtr returned_pool2 = returned_subnet->getPool(Lease::TYPE_NA,
                                                            IOAddress("2001:db8::10"));
    ASSERT_TRUE(returned_pool2);

    // Option should be gone.
    EXPECT_FALSE(returned_pool2->getCfgOption()->get(DHCP6_OPTION_SPACE,
                                                     D6O_NEW_POSIX_TIMEZONE).option_);

    {
        SCOPED_TRACE("UPDATE audit entry for a subnet when deleting "
                     "address pool specific option");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::UPDATE,
                          "address pool specific option deleted");
    }

    // The option has been deleted so the number of options should now
    // be down to 4.
    EXPECT_EQ(4, countRows("dhcp6_options"));
}

// This test verifies that option can be inserted, updated and deleted
// from the pd pool.
TEST_F(MySqlConfigBackendDHCPv6Test, createUpdateDeletePdPoolOption6) {
    // Insert new subnet.
    Subnet6Ptr subnet = test_subnets_[1];
    cbptr_->createUpdateSubnet6(ServerSelector::ALL(), subnet);

    {
        SCOPED_TRACE("CREATE audit entry for a subnet");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::CREATE,
                          "subnet set");
    }

    // Inserted subnet has four options.
    ASSERT_EQ(4, countRows("dhcp6_options"));

    // Add an option into the pd pool.
    const PoolPtr pd_pool = subnet->getPool(Lease::TYPE_PD,
                                            IOAddress("2001:db8:a:10::"));
    ASSERT_TRUE(pd_pool);
    OptionDescriptorPtr opt_posix_timezone = test_options_[0];
    int pd_pool_len = prefixLengthFromRange(pd_pool->getFirstAddress(),
                                            pd_pool->getLastAddress());
    cbptr_->createUpdateOption6(ServerSelector::ANY(),
                                pd_pool->getFirstAddress(),
                                static_cast<uint8_t>(pd_pool_len),
                                opt_posix_timezone);

    // Query for a subnet.
    Subnet6Ptr returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                                    subnet->getID());
    ASSERT_TRUE(returned_subnet);

    // The returned subnet should include our pool.
    const PoolPtr returned_pd_pool =
        returned_subnet->getPool(Lease::TYPE_PD, IOAddress("2001:db8:a:10::"));
    ASSERT_TRUE(returned_pd_pool);

    // The pd pool should contain option we added earlier.
    OptionDescriptor returned_opt_posix_timezone =
        returned_pd_pool->getCfgOption()->get(DHCP6_OPTION_SPACE,
                                              D6O_NEW_POSIX_TIMEZONE);
    ASSERT_TRUE(returned_opt_posix_timezone.option_);

    {
        SCOPED_TRACE("verify returned pool option");
        testOptionsEquivalent(*opt_posix_timezone, returned_opt_posix_timezone);
        EXPECT_GT(returned_opt_posix_timezone.getId(), 0);
    }

    {
        SCOPED_TRACE("UPDATE audit entry for a subnet after adding an option "
                     "to the prefix delegation pool");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::UPDATE,
                          "prefix delegation pool specific option set");
    }

    // With the newly inserted option we should now have five options.
    ASSERT_EQ(5, countRows("dhcp6_options"));

    // Modify the option and update it in the database.
    opt_posix_timezone->persistent_ = !opt_posix_timezone->persistent_;
    cbptr_->createUpdateOption6(ServerSelector::ANY(),
                                pd_pool->getFirstAddress(),
                                static_cast<uint8_t>(pd_pool_len),
                                opt_posix_timezone);

    // Fetch the subnet and the corresponding pd pool.
    returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                         subnet->getID());
    ASSERT_TRUE(returned_subnet);
    const PoolPtr returned_pd_pool1 =
        returned_subnet->getPool(Lease::TYPE_PD, IOAddress("2001:db8:a:10::"));
    ASSERT_TRUE(returned_pd_pool1);

    // Test that the option has been correctly updated in the database.
    returned_opt_posix_timezone =
        returned_pd_pool1->getCfgOption()->get(DHCP6_OPTION_SPACE,
                                               D6O_NEW_POSIX_TIMEZONE);
    ASSERT_TRUE(returned_opt_posix_timezone.option_);

    {
        SCOPED_TRACE("verify updated option with modified persistence");
        testOptionsEquivalent(*opt_posix_timezone, returned_opt_posix_timezone);
    }

    {
        SCOPED_TRACE("UPDATE audit entry for a subnet when updating "
                     "prefix delegation pool specific option");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::UPDATE,
                          "prefix delegation pool specific option set");
    }

    // The new option instance should replace the existing one, so we should
    // still have five options.
    ASSERT_EQ(5, countRows("dhcp6_options"));

    // Delete option for any server should succeed.
    EXPECT_EQ(1, cbptr_->deleteOption6(ServerSelector::ANY(),
                                       pd_pool->getFirstAddress(),
                                       static_cast<uint8_t>(pd_pool_len),
                                       opt_posix_timezone->option_->getType(),
                                       opt_posix_timezone->space_name_));

    // Fetch the subnet and the pool from the database again to make sure
    // that the option is really gone.
    returned_subnet = cbptr_->getSubnet6(ServerSelector::ALL(),
                                         subnet->getID());
    ASSERT_TRUE(returned_subnet);
    const PoolPtr returned_pd_pool2 =
        returned_subnet->getPool(Lease::TYPE_PD, IOAddress("2001:db8:a:10::"));
    ASSERT_TRUE(returned_pd_pool2);

    // Option should be gone.
    EXPECT_FALSE(returned_pd_pool2->getCfgOption()->get(DHCP6_OPTION_SPACE,
                                                        D6O_NEW_POSIX_TIMEZONE).option_);

    {
        SCOPED_TRACE("UPDATE audit entry for a subnet when deleting "
                     "prefix delegation pool specific option");
        testNewAuditEntry("dhcp6_subnet",
                          AuditEntry::ModificationType::UPDATE,
                          "prefix delegation pool specific option deleted");
    }

    // The option has been deleted so the number of options should now
    // be down to 4.
    EXPECT_EQ(4, countRows("dhcp6_options"));
}

// This test verifies that shared network level option can be added,
// updated and deleted.
TEST_F(MySqlConfigBackendDHCPv6Test, createUpdateDeleteSharedNetworkOption6) {
    // Insert new shared network.
    SharedNetwork6Ptr shared_network = test_networks_[1];
    cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(),
                                       shared_network);

    // Fetch this shared network by name.
    SharedNetwork6Ptr returned_network =
        cbptr_->getSharedNetwork6(ServerSelector::ALL(),
                                  shared_network->getName());
    ASSERT_TRUE(returned_network);

    {
        SCOPED_TRACE("CREATE audit entry for the new shared network");
        testNewAuditEntry("dhcp6_shared_network",
                          AuditEntry::ModificationType::CREATE,
                          "shared network set");
    }

    // The inserted shared network has no options.
    ASSERT_EQ(0, countRows("dhcp6_options"));

    OptionDescriptorPtr opt_posix_timezone = test_options_[0];
    cbptr_->createUpdateOption6(ServerSelector::ANY(),
                                shared_network->getName(),
                                opt_posix_timezone);

    returned_network = cbptr_->getSharedNetwork6(ServerSelector::ALL(),
                                                shared_network->getName());
    ASSERT_TRUE(returned_network);

    OptionDescriptor returned_opt_posix_timezone =
        returned_network->getCfgOption()->get(DHCP6_OPTION_SPACE, D6O_NEW_POSIX_TIMEZONE);
    ASSERT_TRUE(returned_opt_posix_timezone.option_);

    {
        SCOPED_TRACE("verify returned option");
        testOptionsEquivalent(*opt_posix_timezone, returned_opt_posix_timezone);
        EXPECT_GT(returned_opt_posix_timezone.getId(), 0);
    }

    {
        SCOPED_TRACE("UPDATE audit entry for the added shared network option");
        // Instead of adding an audit entry for an option we add an audit
        // entry for the entire shared network so as the server refreshes the
        // shared network with the new option. Note that the server doesn't
        // have means to retrieve only the newly added option.
        testNewAuditEntry("dhcp6_shared_network",
                          AuditEntry::ModificationType::UPDATE,
                          "shared network specific option set");
    }

    // One option should now be stored in the database.
    ASSERT_EQ(1, countRows("dhcp6_options"));

    opt_posix_timezone->persistent_ = !opt_posix_timezone->persistent_;
    cbptr_->createUpdateOption6(ServerSelector::ANY(),
                                shared_network->getName(),
                                opt_posix_timezone);

    returned_network = cbptr_->getSharedNetwork6(ServerSelector::ALL(),
                                                 shared_network->getName());
    ASSERT_TRUE(returned_network);
    returned_opt_posix_timezone =
        returned_network->getCfgOption()->get(DHCP6_OPTION_SPACE, D6O_NEW_POSIX_TIMEZONE);
    ASSERT_TRUE(returned_opt_posix_timezone.option_);

    {
        SCOPED_TRACE("verify updated option with modified persistence");
        testOptionsEquivalent(*opt_posix_timezone, returned_opt_posix_timezone);
    }

    {
        SCOPED_TRACE("UPDATE audit entry for the updated shared network option");
        testNewAuditEntry("dhcp6_shared_network",
                          AuditEntry::ModificationType::UPDATE,
                          "shared network specific option set");
    }

    // The new option instance should replace the existing option instance,
    // so we should still have one option.
    ASSERT_EQ(1, countRows("dhcp6_options"));

    // Deleting an option for any server should succeed.
    EXPECT_EQ(1, cbptr_->deleteOption6(ServerSelector::ANY(),
                                       shared_network->getName(),
                                       opt_posix_timezone->option_->getType(),
                                       opt_posix_timezone->space_name_));
    returned_network = cbptr_->getSharedNetwork6(ServerSelector::ALL(),
                                                 shared_network->getName());
    ASSERT_TRUE(returned_network);
    EXPECT_FALSE(returned_network->getCfgOption()->get(DHCP6_OPTION_SPACE, D6O_NEW_POSIX_TIMEZONE).option_);

    {
        SCOPED_TRACE("UPDATE audit entry for the deleted shared network option");
        testNewAuditEntry("dhcp6_shared_network",
                          AuditEntry::ModificationType::UPDATE,
                          "shared network specific option deleted");
    }

    // After deleting the option we should be back to 0.
    EXPECT_EQ(0, countRows("dhcp6_options"));
}

// This test verifies that option id values in one subnet do
// not impact options returned in subsequent subnets when
// fetching subnets from the backend.
TEST_F(MySqlConfigBackendDHCPv6Test, subnetOptionIdOrder) {

    // Add a network with two pools with two options each.
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(), test_subnets_[1]));
    EXPECT_EQ(2, countRows("dhcp6_pool"));
    EXPECT_EQ(4, countRows("dhcp6_options"));

    // Add second subnet with a single option. The number of options in the database
    // should now be 3.
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(), test_subnets_[2]));
    EXPECT_EQ(2, countRows("dhcp6_pool"));
    EXPECT_EQ(5, countRows("dhcp6_options"));

    // Now replace the first subnet with three options and two pools.  This will cause
    // the option id values for this subnet to be larger than those in the second
    // subnet.
    EXPECT_NO_THROW(cbptr_->createUpdateSubnet6(ServerSelector::ALL(), test_subnets_[0]));
    EXPECT_EQ(2, countRows("dhcp6_pool"));
    EXPECT_EQ(4, countRows("dhcp6_options"));

    // Now fetch all subnets.
    Subnet6Collection subnets;
    EXPECT_NO_THROW(subnets = cbptr_->getAllSubnets6(ServerSelector::ALL()));
    ASSERT_EQ(2, subnets.size());

    // Verify that the subnets returned are as expected.
    for (auto subnet : subnets) {
        ASSERT_EQ(1, subnet->getServerTags().size());
        EXPECT_EQ("all", subnet->getServerTags().begin()->get());
        if (subnet->getID() == 1024) {
            EXPECT_EQ(test_subnets_[0]->toElement()->str(), subnet->toElement()->str());
        } else if (subnet->getID() == 2048) {
            EXPECT_EQ(test_subnets_[2]->toElement()->str(), subnet->toElement()->str());
        } else {
            ADD_FAILURE() << "unexpected subnet id:" << subnet->getID();
        }
    }
}

// This test verifies that option id values in one shared network do
// not impact options returned in subsequent shared networks when
// fetching shared networks from the backend.
TEST_F(MySqlConfigBackendDHCPv6Test, sharedNetworkOptionIdOrder) {
    auto level1_options = test_networks_[0];
    auto level1_no_options = test_networks_[1];
    auto level2 = test_networks_[2];

    // Insert two shared networks. We insert level1 without options first,
    // then level2.
    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(),
                                                       level1_no_options));

    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(),
                                                       level2));
    // Fetch all shared networks.
    SharedNetwork6Collection networks =
        cbptr_->getAllSharedNetworks6(ServerSelector::ALL());

    ASSERT_EQ(2, networks.size());

    // See if shared networks are returned ok.
    for (auto i = 0; i < networks.size(); ++i) {
        if (i == 0) {
            // level1_no_options
            EXPECT_EQ(level1_no_options->toElement()->str(),
                  networks[i]->toElement()->str());
        } else {
            // bar
            EXPECT_EQ(level2->toElement()->str(),
                  networks[i]->toElement()->str());
        }
    }

    EXPECT_NO_THROW(cbptr_->createUpdateSharedNetwork6(ServerSelector::ALL(),
                                                       level1_options));

    // Fetch all shared networks.
    networks = cbptr_->getAllSharedNetworks6(ServerSelector::ALL());
    ASSERT_EQ(2, networks.size());

    // See if shared networks are returned ok.
    for (auto i = 0; i < networks.size(); ++i) {
        if (i == 0) {
            // level1_no_options
            EXPECT_EQ(level1_options->toElement()->str(),
                  networks[i]->toElement()->str());
        } else {
            // bar
            EXPECT_EQ(level2->toElement()->str(),
                  networks[i]->toElement()->str());
        }
    }
}

/// This test verifies that audit entries can be retrieved from a given
/// timestamp and id including when two entries can get the same timestamp.
/// (either it is a common even and this should catch it, or it is a rare
/// event and it does not matter).
TEST_F(MySqlConfigBackendDHCPv6Test, multipleAuditEntries) {
    // Get current time.
    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

    // Create a server.
    EXPECT_NO_THROW(cbptr_->createUpdateServer6(test_servers_[1]));

    // Create a global parameter and update it many times.
    const ServerSelector& server_selector = ServerSelector::ALL();
    StampedValuePtr param;
    ElementPtr value;
    for (int i = 0; i < 100; ++i) {
        value = Element::create(i);
        param = StampedValue::create("my-parameter", value);
        cbptr_->createUpdateGlobalParameter6(server_selector, param);
    }

    // Get all audit entries from now.
    AuditEntryCollection audit_entries =
        cbptr_->getRecentAuditEntries(server_selector, now, 0);

    // Check that partial retrieves return the right count.
    auto& mod_time_idx = audit_entries.get<AuditEntryModificationTimeIdTag>();
    for (auto it = mod_time_idx.begin(); it != mod_time_idx.end(); ++it) {
        size_t partial_size =
            cbptr_->getRecentAuditEntries(server_selector,
                                          (*it)->getModificationTime(),
                                          (*it)->getRevisionId()).size();
        EXPECT_EQ(partial_size + 1,
                  std::distance(it, mod_time_idx.end()));
    }
}

}

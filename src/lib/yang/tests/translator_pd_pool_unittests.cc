// Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <yang/tests/sysrepo_setup.h>
#include <yang/translator_pd_pool.h>
#include <yang/yang_models.h>

#include <gtest/gtest.h>
#include <sstream>

using namespace std;
using namespace isc;
using namespace isc::data;
using namespace isc::yang;
using namespace isc::yang::test;
using namespace sysrepo;

namespace {

/// @brief Translator name.
extern char const pd_pool_list[] = "pd pool list";

/// @brief Test fixture class for @ref TranslatorPdPools.
struct TranslatorPdPoolsTest : GenericTranslatorTest<pd_pool_list, TranslatorPdPools> {
    /// Destructor (does nothing).
    virtual ~TranslatorPdPoolsTest() = default;
};

// This test verifies that an empty pd pool list can be properly
// translated from YANG to JSON using the IETF model.
TEST_F(TranslatorPdPoolsTest, getEmptyIetf) {
    useModel(IETF_DHCPV6_SERVER);

    // Get the pd-pool list and check if it is empty.
    const string& xpath = "/ietf-dhcpv6-server:server/server-config/network-ranges"
                          "/network-range[id='111']/pd-pools";
    ElementPtr pools;
    EXPECT_NO_THROW(pools = t_obj_->getPdPools(xpath));
    ASSERT_TRUE(pools);
    ASSERT_EQ(Element::list, pools->getType());
    EXPECT_EQ(0, pools->size());
}

// This test verifies that an empty pd pool list can be properly
// translated from YANG to JSON using the Kea ad hoc model.
TEST_F(TranslatorPdPoolsTest, getEmptyKea) {
    useModel(KEA_DHCP6_SERVER);

    // Get the pd-pool list and check if it is empty.
    const string& xpath = "/kea-dhcp6-server:config/subnet6[id='111']";
    ElementPtr pools;
    EXPECT_NO_THROW(pools = t_obj_->getPdPools(xpath));
    ASSERT_TRUE(pools);
    ASSERT_EQ(Element::list, pools->getType());
    EXPECT_EQ(0, pools->size());
}

// This test verifies that one pd pool can be properly
// translated from YANG to JSON using the IETF model.
TEST_F(TranslatorPdPoolsTest, getIetf) {
    useModel(IETF_DHCPV6_SERVER);

    // Create the subnet 2001:db8::/48 #111.
    const string& subnet = "/ietf-dhcpv6-server:server/server-config/network-ranges"
                           "/network-range[id='111']";
    S_Val v_subnet(new Val("2001:db8::/48", SR_STRING_T));
    const string& subnet_subnet = subnet + "/network-prefix";
    EXPECT_NO_THROW(sess_->set_item(subnet_subnet.c_str(), v_subnet));

    // Create the pd-pool 2001:db8:0:1000::/56 #222.
    const string& xpath = subnet + "/pd-pools/pd-pool[pool-id='222']";
    const string& prefix = xpath + "/prefix";
    S_Val s_prefix(new Val("2001:db8:0:1000::/56"));
    EXPECT_NO_THROW(sess_->set_item(prefix.c_str(), s_prefix));
    const string& length = xpath + "/prefix-length";
    uint8_t len = 56;
    S_Val s_length(new Val(len));
    EXPECT_NO_THROW(sess_->set_item(length.c_str(), s_length));

    // Get the pool.
    ElementPtr pool;
    EXPECT_NO_THROW(pool = t_obj_->getPdPool(xpath));
    ASSERT_TRUE(pool);
    ElementPtr expected = Element::createMap();
    expected->set("prefix", Element::create(string("2001:db8:0:1000::")));
    expected->set("prefix-len", Element::create(56));
    EXPECT_TRUE(expected->equals(*pool));

    // Get the pd-pool list and check if the pd-pool is in it.
    ElementPtr pools;
    EXPECT_NO_THROW(pools = t_obj_->getPdPools(subnet + "/pd-pools"));
    ASSERT_TRUE(pools);
    ASSERT_EQ(Element::list, pools->getType());
    ASSERT_EQ(1, pools->size());
    EXPECT_TRUE(pool->equals(*pools->get(0)));
}

// This test verifies that one pd pool can be properly
// translated from YANG to JSON using the Kea ad hoc model.
TEST_F(TranslatorPdPoolsTest, getKea) {
    useModel(KEA_DHCP6_SERVER);

    // Create the subnet 2001:db8::/48 #111.
    const string& xpath = "/kea-dhcp6-server:config/subnet6[id='111']";
    S_Val v_subnet(new Val("2001:db8::/48", SR_STRING_T));
    const string& subnet = xpath + "/subnet";
    EXPECT_NO_THROW(sess_->set_item(subnet.c_str(), v_subnet));

    // Create the pd-pool 2001:db8:0:1000::/56.
    const string& prefix = "2001:db8:0:1000::";
    ostringstream spool;
    spool << xpath + "/pd-pools[prefix='" << prefix << "']";

    const string& x_prefix_len = spool.str() + "/prefix-len";
    uint8_t prefix_len = 56;
    S_Val s_prefix_len(new Val(prefix_len));
    EXPECT_NO_THROW(sess_->set_item(x_prefix_len.c_str(), s_prefix_len));

    const string& x_delegated = spool.str() + "/delegated-len";
    uint8_t dl = 64;
    S_Val s_delegated(new Val(dl));
    EXPECT_NO_THROW(sess_->set_item(x_delegated.c_str(), s_delegated));

    // Get the pool.
    ElementPtr pool;
    EXPECT_NO_THROW(pool = t_obj_->getPdPool(spool.str()));
    ASSERT_TRUE(pool);
    ElementPtr expected = Element::createMap();
    expected->set("prefix", Element::create(string("2001:db8:0:1000::")));
    expected->set("prefix-len", Element::create(56));
    expected->set("delegated-len", Element::create(64));
    EXPECT_TRUE(expected->equals(*pool));

    // Get the pd-pool list and check if the pd-pool is in it.
    ElementPtr pools;
    EXPECT_NO_THROW(pools = t_obj_->getPdPools(xpath));
    ASSERT_TRUE(pools);
    ASSERT_EQ(Element::list, pools->getType());
    ASSERT_EQ(1, pools->size());
    EXPECT_TRUE(pool->equals(*pools->get(0)));
}

// This test verifies that an empty pd pool list can be properly
// translated from JSON to YANG using the IETF model.
TEST_F(TranslatorPdPoolsTest, setEmptyIetf) {
    useModel(IETF_DHCPV6_SERVER);

    // Create the subnet 2001:db8::/48 #111.
    const string& subnet = "/ietf-dhcpv6-server:server/server-config/network-ranges"
                           "/network-range[id='111']";
    S_Val v_subnet(new Val("2001:db8::/48", SR_STRING_T));
    const string& subnet_subnet = subnet + "/network-prefix";
    EXPECT_NO_THROW(sess_->set_item(subnet_subnet.c_str(), v_subnet));

    // Set empty list.
    const string& xpath = subnet + "/pd-pools";
    ElementPtr pools = Element::createList();
    EXPECT_NO_THROW(t_obj_->setPdPools(xpath, pools));

    // Get it back.
    pools.reset();
    EXPECT_NO_THROW(pools = t_obj_->getPdPools(xpath));
    ASSERT_TRUE(pools);
    ASSERT_EQ(Element::list, pools->getType());
    EXPECT_EQ(0, pools->size());
}

// This test verifies that an empty pd pool list can be properly
// translated from JSON to YANG using the Kea ad hoc model.
TEST_F(TranslatorPdPoolsTest, setEmptyKea) {
    useModel(KEA_DHCP6_SERVER);

    // Create the subnet 2001:db8::/48 #111.
    const string& xpath = "/kea-dhcp6-server:config/subnet6[id='111']";
    S_Val v_subnet(new Val("2001:db8::/48", SR_STRING_T));
    const string& subnet = xpath + "/subnet";
    EXPECT_NO_THROW(sess_->set_item(subnet.c_str(), v_subnet));

    // Set empty list.
    ElementPtr pools = Element::createList();
    EXPECT_NO_THROW(t_obj_->setPdPools(xpath, pools));

    // Get it back.
    pools.reset();
    EXPECT_NO_THROW(pools = t_obj_->getPdPools(xpath));
    ASSERT_TRUE(pools);
    ASSERT_EQ(Element::list, pools->getType());
    EXPECT_EQ(0, pools->size());
}

// This test verifies that one pd pool can be properly
// translated from JSON to YANG using the IETF model.
TEST_F(TranslatorPdPoolsTest, setIetf) {
    useModel(IETF_DHCPV6_SERVER);

    // Create the subnet 2001:db8::/48 #111.
    const string& subnet = "/ietf-dhcpv6-server:server/server-config/network-ranges"
                           "/network-range[id='111']";
    S_Val v_subnet(new Val("2001:db8::/48", SR_STRING_T));
    const string& subnet_subnet = subnet + "/network-prefix";
    EXPECT_NO_THROW(sess_->set_item(subnet_subnet.c_str(), v_subnet));

    // Set one pool.
    const string& xpath = subnet + "/pd-pools";
    ElementPtr pools = Element::createList();
    ElementPtr pool = Element::createMap();
    pool->set("prefix", Element::create(string("2001:db8:0:1000::")));
    pool->set("prefix-len", Element::create(56));
    pools->add(pool);
    EXPECT_NO_THROW(t_obj_->setPdPools(xpath, pools));

    // Get it back.
    pools.reset();
    EXPECT_NO_THROW(pools = t_obj_->getPdPools(xpath));
    ASSERT_TRUE(pools);
    ASSERT_EQ(Element::list, pools->getType());
    ASSERT_EQ(1, pools->size());
    EXPECT_TRUE(pool->equals(*pools->get(0)));

    /* TODO
    // Check the tree representation.
    S_Tree tree;
    EXPECT_NO_THROW(tree = sess_->get_subtree("/ietf-dhcpv6-server:server"));
    ASSERT_TRUE(tree);
    string expected = "ietf-dhcpv6-server:server (container)\n"
                      " |\n"
                      " -- server-config (container)\n"
                      "     |\n"
                      "     -- network-ranges (container)\n"
                      "         |\n"
                      "         -- network-range (list instance)\n"
                      "             |\n"
                      "             -- id = 111\n"
                      "             |\n"
                      "             -- network-prefix = 2001:db8::/48\n"
                      "             |\n"
                      "             -- pd-pools (container)\n"
                      "                 |\n"
                      "                 -- pd-pool (list instance)\n"
                      "                     |\n"
                      "                     -- pool-id = 0\n"
                      "                     |\n"
                      "                     -- prefix = 2001:db8:0:1000::/56\n"
                      "                     |\n"
                      "                     -- prefix-length = 56\n"
                      "                     |\n"
                      "                     -- max-pd-space-utilization = disabled\n";
    EXPECT_EQ(expected, tree->to_string(100));
    // TODO */
}

// This test verifies that one pd pool can be properly
// translated from JSON to YANG using the kea ad hoc model.
TEST_F(TranslatorPdPoolsTest, setKea) {
    useModel(KEA_DHCP6_SERVER);

    // Create the subnet 2001:db8::/48 #111.
    const string& xpath = "/kea-dhcp6-server:config/subnet6[id='111']";
    S_Val v_subnet(new Val("2001:db8::/48", SR_STRING_T));
    const string& subnet = xpath + "/subnet";
    EXPECT_NO_THROW(sess_->set_item(subnet.c_str(), v_subnet));

    // Set one pool.
    ElementPtr pools = Element::createList();
    ElementPtr pool = Element::createMap();
    pool->set("prefix", Element::create(string("2001:db8:0:1000::")));
    pool->set("prefix-len", Element::create(56));
    pool->set("delegated-len", Element::create(64));
    pools->add(pool);
    EXPECT_NO_THROW(t_obj_->setPdPools(xpath, pools));

    // Get it back.
    pools.reset();
    EXPECT_NO_THROW(pools = t_obj_->getPdPools(xpath));
    ASSERT_TRUE(pools);
    ASSERT_EQ(Element::list, pools->getType());
    ASSERT_EQ(1, pools->size());
    EXPECT_TRUE(pool->equals(*pools->get(0)));

    /* TODO
    // Check the tree representation.
    S_Tree tree;
    EXPECT_NO_THROW(tree = sess_->get_subtree("/kea-dhcp6-server:config"));
    ASSERT_TRUE(tree);
    string expected =
        R"(kea-dhcp6-server:config (container)
 |
 -- subnet6 (list instance)
     |
     -- id = 111
     |
     -- subnet = 2001:db8::/48
     |
     -- pd-pools (list instance)
         |
         -- prefix = 2001:db8:0:1000::
         |
         -- prefix-len = 56
         |
         -- delegated-len = 64
)";
    EXPECT_EQ(expected, tree->to_string(100));
    // TODO */

    // Check it validates.
    EXPECT_NO_THROW(sess_->validate());
}

// This test verifies that a non-empty list of pd pools can be properly
// translated from YANG to JSON using the Kea ad hoc model.
TEST_F(TranslatorPdPoolsTest, getListKea) {
    useModel(KEA_DHCP6_SERVER);

    // Create the subnet 2001:db8::/48 #111.
    const string& xpath = "/kea-dhcp6-server:config/subnet6[id='111']";
    S_Val v_subnet(new Val("2001:db8::/48", SR_STRING_T));
    const string& subnet = xpath + "/subnet";
    EXPECT_NO_THROW(sess_->set_item(subnet.c_str(), v_subnet));

    // Create the first pd-pool 2001:db8:0:1000::/56.
    const string& prefix = "2001:db8:0:1000::";
    ostringstream spool;
    spool << xpath + "/pd-pools[prefix='" << prefix << "']";

    const string& x_prefix_len = spool.str() + "/prefix-len";
    uint8_t prefix_len = 56;
    S_Val s_prefix_len(new Val(prefix_len));
    EXPECT_NO_THROW(sess_->set_item(x_prefix_len.c_str(), s_prefix_len));

    const string& x_delegated = spool.str() + "/delegated-len";
    uint8_t dl = 64;
    S_Val s_delegated(new Val(dl));
    EXPECT_NO_THROW(sess_->set_item(x_delegated.c_str(), s_delegated));

    // Create the second pd-pool 2001:db8:0:2000::/56
    const string& prefix2 = "2001:db8:0:2000::";
    ostringstream spool2;
    spool2 << xpath + "/pd-pools[prefix='" << prefix2 << "']";

    const string& x_prefix_len2 = spool2.str() + "/prefix-len";
    uint8_t prefix_len2 = 56;
    S_Val s_prefix_len2(new Val(prefix_len2));
    EXPECT_NO_THROW(sess_->set_item(x_prefix_len2.c_str(), s_prefix_len2));

    const string& x_delegated2 = spool2.str() + "/delegated-len";
    uint8_t dl2 = 60;
    S_Val s_delegated2(new Val(dl2));
    EXPECT_NO_THROW(sess_->set_item(x_delegated2.c_str(), s_delegated2));

    // Get the pools list.
    ElementPtr pools;
    EXPECT_NO_THROW(pools = t_obj_->getPdPools(xpath));
    ASSERT_TRUE(pools);

    // Check that both of them are returned properly.
    EXPECT_EQ(pools->str(), "[ { \"delegated-len\": 64, \"prefix\": \"2001:db8:0:1000::\", "
                            "\"prefix-len\": 56 }, { \"delegated-len\": 60, \"prefix\": "
                            "\"2001:db8:0:2000::\", \"prefix-len\": 56 } ]");
}

}  //anonymous namespace

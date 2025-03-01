// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <yang/tests/sysrepo_setup.h>
#include <yang/translator_option_def.h>
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
extern char const option_definition_list[] = "option definition list";

/// @brief Test fixture class for @ref TranslatorOptionDefList.
struct TranslatorOptionDefListTest : GenericTranslatorTest<option_definition_list, TranslatorOptionDefList> {
    /// Destructor (does nothing).
    virtual ~TranslatorOptionDefListTest() = default;
};

// This test verifies that an empty option definition list can be properly
// translated from YANG to JSON.
TEST_F(TranslatorOptionDefListTest, getEmpty) {
    useModel(KEA_DHCP4_SERVER);

    // Get the option definition list and check if it is empty.
    const string& xpath = "/kea-dhcp4-server:config";
    ElementPtr options;
    EXPECT_NO_THROW(options = t_obj_->getOptionDefList(xpath));
    ASSERT_TRUE(options);
    ASSERT_EQ(Element::list, options->getType());
    EXPECT_EQ(0, options->size());
}

// This test verifies that one option definition can be properly
// translated from YANG to JSON.
TEST_F(TranslatorOptionDefListTest, get) {
    useModel(KEA_DHCP6_SERVER);

    // Create the option code 100.
    const string& xpath = "/kea-dhcp6-server:config";
    const string& xdef = xpath + "/option-def[code='100'][space='isc']";
    const string& xname = xdef + "/name";
    const string& xtype = xdef + "/type";
    const string& xarray = xdef + "/array";
    S_Val s_name(new Val("foo"));
    ASSERT_NO_THROW(sess_->set_item(xname.c_str(), s_name));
    S_Val s_type(new Val("string"));
    ASSERT_NO_THROW(sess_->set_item(xtype.c_str(), s_type));
    S_Val s_array(new Val(false));
    ASSERT_NO_THROW(sess_->set_item(xarray.c_str(), s_array));

    // Get the option def.
    ElementPtr def;
    EXPECT_NO_THROW(def = t_obj_->getOptionDef(xdef));
    ASSERT_TRUE(def);
    EXPECT_EQ("{ "
              "\"array\": false, "
              "\"code\": 100, "
              "\"name\": \"foo\", "
              "\"space\": \"isc\", "
              "\"type\": \"string\" }",
              def->str());

    // Get the option definition list.
    ElementPtr defs;
    EXPECT_NO_THROW(defs = t_obj_->getOptionDefList(xpath));
    ASSERT_TRUE(defs);
    ASSERT_EQ(Element::list, defs->getType());
    EXPECT_EQ(1, defs->size());
    EXPECT_TRUE(def->equals(*defs->get(0)));
}

// This test verifies that an empty option definition list can be properly
// translated from JSON to YANG.
TEST_F(TranslatorOptionDefListTest, setEmpty) {
    useModel(KEA_DHCP4_SERVER);

    // Set empty list.
    const string& xpath = "/kea-dhcp4-server:config";
    ElementPtr defs = Element::createList();
    EXPECT_NO_THROW(t_obj_->setOptionDefList(xpath, defs));

    // Get it back.
    defs.reset();
    EXPECT_NO_THROW(defs = t_obj_->getOptionDefList(xpath));
    ASSERT_TRUE(defs);
    EXPECT_EQ(0, defs->size());

    /* TODO
    // Check that the tree representation is empty.
    S_Tree tree;
    EXPECT_NO_THROW(tree = sess_->get_subtree("/kea-dhcp4-server:config"));
    EXPECT_FALSE(tree);
    // TODO */
}

// This test verifies that one option definition can be properly
// translated from JSON to YANG.
TEST_F(TranslatorOptionDefListTest, set) {
    useModel(KEA_DHCP6_SERVER);

    // Set one option def.
    const string& xpath = "/kea-dhcp6-server:config";
    ElementPtr defs = Element::createList();
    ElementPtr def = Element::createMap();
    def->set("code", Element::create(100));
    def->set("name", Element::create(string("foo")));
    def->set("space", Element::create(string("isc")));
    def->set("type", Element::create(string("string")));
    def->set("array", Element::create(false));
    defs->add(def);
    EXPECT_NO_THROW(t_obj_->setOptionDefList(xpath, defs));

    // Get it back.
    ElementPtr got;
    EXPECT_NO_THROW(got = t_obj_->getOptionDefList(xpath));
    ASSERT_TRUE(got);
    ASSERT_EQ(1, got->size());
    EXPECT_TRUE(def->equals(*got->get(0)));

    // Check the tree representation.
    /* TODO
    S_Tree tree;
    EXPECT_NO_THROW(tree = sess_->get_subtree("/kea-dhcp6-server:config"));
    ASSERT_TRUE(tree);
    string expected = "kea-dhcp6-server:config (container)\n"
                      " |\n"
                      " -- option-def (list instance)\n"
                      "     |\n"
                      "     -- code = 100\n"
                      "     |\n"
                      "     -- space = isc\n"
                      "     |\n"
                      "     -- name = foo\n"
                      "     |\n"
                      "     -- type = string\n"
                      "     |\n"
                      "     -- array = false\n";
    EXPECT_EQ(expected, tree->to_string(100));
    // TODO */

    // Check it validates.
    EXPECT_NO_THROW(sess_->validate());
}

}  //anonymous namespace

// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <yang/tests/sysrepo_setup.h>
#include <yang/translator_class.h>
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
extern char const client_classes[] = "client classes";

/// @brief Test fixture class for @ref TranslatorClasses.
struct TranslatorClassesTest : GenericTranslatorTest<client_classes, TranslatorClasses> {
    /// Destructor (does nothing).
    virtual ~TranslatorClassesTest() = default;
};

// This test verifies that an empty client class list can be properly
// translated from YANG to JSON.
TEST_F(TranslatorClassesTest, getEmpty) {
    useModel(KEA_DHCP4_SERVER);

    // Get the client class list and check if it is empty.
    const string& xpath = "/kea-dhcp4-server:config";
    ElementPtr classes;
    EXPECT_NO_THROW(classes = t_obj_->getClasses(xpath));
    EXPECT_FALSE(classes);
}

// This test verifies that one client class can be properly translated
// from YANG to JSON.
TEST_F(TranslatorClassesTest, get) {
    useModel(KEA_DHCP6_SERVER);

    // Create the client class.
    const string& xpath = "/kea-dhcp6-server:config";
    const string& xclass = xpath + "/client-classes[name='foo']";
    const string& xtest = xclass + "/test";
    S_Val v_test(new Val("not member('ALL')", SR_STRING_T));
    EXPECT_NO_THROW(sess_->set_item(xtest.c_str(), v_test));

    // Get the client class.
    ElementPtr cclass;
    EXPECT_NO_THROW(cclass = t_obj_->getClass(xclass));
    ASSERT_TRUE(cclass);
    ElementPtr expected = Element::createMap();
    expected->set("name", Element::create(string("foo")));
    expected->set("test", Element::create(string("not member('ALL')")));
    EXPECT_TRUE(expected->equals(*cclass));

    // Get the client class list and check if the client class is in it.
    ElementPtr classes;
    EXPECT_NO_THROW(classes = t_obj_->getClasses(xpath));
    ASSERT_TRUE(classes);
    ASSERT_EQ(Element::list, classes->getType());
    ASSERT_EQ(1, classes->size());
    EXPECT_TRUE(cclass->equals(*classes->get(0)));
}

// This test verifies that an empty client class list can be properly
// translated from JSON to YANG.
TEST_F(TranslatorClassesTest, setEmpty) {
    useModel(KEA_DHCP4_SERVER);

    // Set empty list.
    const string& xpath = "/kea-dhcp4-server:config";
    ElementPtr classes = Element::createList();
    EXPECT_NO_THROW(t_obj_->setClasses(xpath, classes));

    // Get it back.
    classes.reset();
    EXPECT_NO_THROW(classes = t_obj_->getClasses(xpath));
    EXPECT_FALSE(classes);

    // Check that the tree representation is empty.
    /* TODO
    S_Tree tree;
    EXPECT_NO_THROW(tree = sess_->get_subtree("/kea-dhcp4-server:config"));
    EXPECT_FALSE(tree);
    // TODO */
}

// This test verifies that one client class can be properly translated
// from JSON to YANG.
TEST_F(TranslatorClassesTest, set) {
    useModel(KEA_DHCP6_SERVER);

    // Set one client class.
    const string& xpath = "/kea-dhcp6-server:config";
    ElementPtr classes = Element::createList();
    ElementPtr cclass = Element::createMap();
    cclass->set("name", Element::create(string("foo")));
    cclass->set("test", Element::create(string("''==''")));
    cclass->set("only-if-required", Element::create(false));
    classes->add(cclass);
    EXPECT_NO_THROW(t_obj_->setClasses(xpath, classes));

    // Get it back.
    ElementPtr got;
    EXPECT_NO_THROW(got = t_obj_->getClasses(xpath));
    ASSERT_TRUE(got);
    ASSERT_EQ(Element::list, got->getType());
    ASSERT_EQ(1, got->size());
    EXPECT_TRUE(cclass->equals(*got->get(0)));

    /* TODO
    // Check the tree representation.
    S_Tree tree;
    EXPECT_NO_THROW(tree = sess_->get_subtree("/kea-dhcp6-server:config"));
    ASSERT_TRUE(tree);
    string expected = "kea-dhcp6-server:config (container)\n"
                      " |\n"
                      " -- client-classes (list instance)\n"
                      "     |\n"
                      "     -- name = foo\n"
                      "     |\n"
                      "     -- test = ''==''\n"
                      "     |\n"
                      "     -- only-if-required = false\n";
    EXPECT_EQ(expected, tree->to_string(100));
    // TODO */

    // Check it validates.
    EXPECT_NO_THROW(sess_->validate());
}

}  //anonymous namespace

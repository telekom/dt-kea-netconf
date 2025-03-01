// Copyright (C) 2015-2017 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>
#include <util/staged_value.h>
#include <gtest/gtest.h>

namespace {

using namespace isc::util;

// This test verifies that the value can be assigned and committed.
TEST(StagedValueTest, assignAndCommit) {
    // Initially the value should be set to a default
    StagedValue<int> value;
    ASSERT_EQ(0, value.getValue());

    // Set the new value without committing it and make sure it
    // can be retrieved.
    value.setValue(4);
    ASSERT_EQ(4, value.getValue());

    // Commit the value and make sure it still can be retrieved.
    value.commit();
    ASSERT_EQ(4, value.getValue());

    // Set new value and retrieve it.
    value.setValue(10);
    ASSERT_EQ(10, value.getValue());

    // Do it again and commit it.
    value.setValue(20);
    ASSERT_EQ(20, value.getValue());
    value.commit();
    EXPECT_EQ(20, value.getValue());
}

// This test verifies that the value can be reverted if it hasn't been
// committed.
TEST(StagedValueTest, revert) {
    // Set the value and commit.
    StagedValue<int> value;
    value.setValue(123);
    value.commit();

    // Set new value and do not commit.
    value.setValue(500);
    // The new value should be the one returned.
    ASSERT_EQ(500, value.getValue());
    // But, reverting gets us back to original value.
    value.revert();
    EXPECT_EQ(123, value.getValue());
    // Reverting again doesn't have any effect.
    value.revert();
    EXPECT_EQ(123, value.getValue());
}

// This test verifies that the value can be restored to an original one.
TEST(StagedValueTest, reset) {
    // Set the new value and commit.
    StagedValue<int> value;
    value.setValue(123);
    value.commit();

    // Override the value but do not commit.
    value.setValue(500);

    // Resetting should take us back to default value.
    value.reset();
    EXPECT_EQ(0, value.getValue());
    value.revert();
    EXPECT_EQ(0, value.getValue());
}

// This test verifies that second commit doesn't modify a value.
TEST(StagedValueTest, commit) {
    // Set the value and commit.
    StagedValue<int> value;
    value.setValue(123);
    value.commit();

    // Second commit should have no effect.
    value.commit();
    EXPECT_EQ(123, value.getValue());
}

// This test checks that type conversion operator works correctly.
TEST(StagedValueTest, conversionOperator) {
    StagedValue<int> value;
    value.setValue(244);
    EXPECT_EQ(244, value);
}

// This test checks that the assignment operator works correctly.
TEST(StagedValueTest, assignmentOperator) {
    StagedValue<int> value;
    value = 111;
    EXPECT_EQ(111, value);
}


} // end of anonymous namespace

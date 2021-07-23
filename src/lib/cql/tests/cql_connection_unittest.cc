// Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
// Copyright (C) 2017 Deutsche Telekom AG.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <cql/cql_connection.h>

#include <gtest/gtest.h>

namespace {

using isc::db::CqlConnection;
using isc::db::CqlTaggedStatement;
using isc::db::StatementMap;
using isc::db::StatementTag;
using isc::db::StatementText;
using std::string_view;

/// @brief Check that the key is properly hashed for StatementMap.
TEST(CqlConnectionTest, statementMapHash) {
    // Build strings to prevent optimizations on underlying C string.
    StatementTag tag1_s("same");
    StatementTag tag2_s("same");
    StatementTag tag1(tag1_s);
    StatementTag tag2(tag2_s);

    // Make sure addresses are different.
    EXPECT_EQ(tag1, tag2);

    // Insert two entries with the same key value.
    StatementMap map;
    map.insert({tag1, {tag1, "<your fancy select here>"}});
    map.insert({tag2, {tag2, "DELETE FROM world.evil}"}});

    // Make sure the first one was overwritten.
    StatementText const tag1_text(map.at(tag1).text_);
    StatementText const tag2_text(map.at(tag2).text_);
    EXPECT_NE(tag1_text.size(), 0);
    EXPECT_NE(tag2_text.size(), 0);
    ASSERT_EQ(tag1_text, tag2_text);
    ASSERT_EQ(map.size(), 1u);
}

/// @brief Check that the parseConsistency function return proper values.
TEST(CqlConnection, parseConsistency) {
    ASSERT_EQ(CqlConnection::parseConsistency("quorum"), CASS_CONSISTENCY_QUORUM);
    ASSERT_EQ(CqlConnection::parseConsistency("serial"), CASS_CONSISTENCY_SERIAL);
    ASSERT_EQ(CqlConnection::parseConsistency(""), CASS_CONSISTENCY_UNKNOWN);
    ASSERT_EQ(CqlConnection::parseConsistency("unknown"), CASS_CONSISTENCY_UNKNOWN);
}

}  // namespace

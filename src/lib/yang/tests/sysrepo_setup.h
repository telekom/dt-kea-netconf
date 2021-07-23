// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef SYSREPO_SETUP_H
#define SYSREPO_SETUP_H

#include <config.h>

#include <yang/translator.h>
#include <gtest/gtest.h>

namespace isc {
namespace yang {
namespace test {

/// @brief Test Fixture template for translator tests.
///
/// @tparam Name The name of the translator to test.
/// @tparam Type The type of the translator to test.
template<char const* Name, typename Type>
struct GenericTranslatorTest : public ::testing::Test {
    using Connection = sysrepo::Connection;
    using Session = sysrepo::Session;
    using S_Connection = sysrepo::S_Connection;
    using S_Session = sysrepo::S_Session;

    /// @brief Constructor.
    GenericTranslatorTest() : conn_(), sess_(), t_obj_() { }

    /// @brief useModel
    ///
    /// Open a sysrepo session and create a translator object using
    /// the given model.
    ///
    /// @param model The model to use.
    void useModel(std::string model) {
        conn_.reset(new Connection());
        sess_.reset(new Session(conn_, SR_DS_CANDIDATE));
        EXPECT_NO_THROW(t_obj_.reset(new Type(sess_, model)));
    }

    /// @brief Destructor.
    ///
    /// Destroy all objects.
    virtual ~GenericTranslatorTest() {
        t_obj_.reset();
        sess_.reset();
        conn_.reset();
    }

    /// @brief Sysrepo connection.
    S_Connection conn_;

    /// @brief Sysrepo session.
    S_Session sess_;

    /// @brief Shared pointer to the transaction object.
    std::shared_ptr<Type> t_obj_;
};

} // namespace test
} // namespace yang
} // namespace isc

#endif // SYSREPO_SETUP_H

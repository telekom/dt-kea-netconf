// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef ISC_TRANSLATOR_TEST_H
#define ISC_TRANSLATOR_TEST_H 1

#include <iostream>
#include <vector>

#include <yang/translator.h>

namespace isc {
namespace yang {
namespace test {

/// @brief Yang/sysrepo datastore textual representation class.
///
/// This class allows to store a configuration tree in YANG format.
/// It is used in tests to conduct operations on whole configurations.
struct YangRepr {
    using S_Session = sysrepo::S_Session;

    /// @brief Constructor.
    ///
    /// @param model The model name.
    YangRepr(const std::string& model) : model_(model) {
    }

    /// @brief A struct representing a single entry.
    struct YangReprItem {
        /// @brief Constructor with content.
        ///
        /// @param xpath The xpath.
        /// @param value The textual value.
        /// @param type The type of the value.
        /// @param settable The settable flag.
        YangReprItem(std::string xpath, std::string value, sr_type_t type, bool settable)
            : xpath_(xpath), value_(value), type_(type), settable_(settable) {
        }

        /// @brief Retrieves configuration parameter from sysrepo.
        ///
        /// @param xpath The xpath of an element to be retrieved.
        /// @param session Sysrepo session.
        /// @return YangReprItem instance representing configuration parameter.
        static YangReprItem get(std::string const& xpath, S_Session session);

        /// @brief The xpath.
        std::string xpath_;

        /// @brief The textual value.
        std::string value_;

        /// @brief The type of the value.
        sr_type_t type_;

        /// @brief The settable flag.
        bool settable_;

        /// @brief The equal operator ignoring settable.
        ///
        /// @param other the other object to compare with.
        /// @return true if equal.
        bool operator==(const YangReprItem& other) const {
            return ((xpath_ == other.xpath_) && (value_ == other.value_) && (type_ == other.type_));
        }

        /// @brief The unequal operator ignoring settable.
        ///
        /// @param other the other object to compare with.
        /// @return false if equal.
        bool operator!=(const YangReprItem& other) const {
            return (!(*this == other));
        }
    };

    /// @brief Tree type.
    typedef std::vector<YangReprItem> Tree;

    /// @brief Get tree from session.
    ///
    /// @param session Sysrepo session.
    Tree get(S_Session session) const;

    /// @brief Verifies a tree.
    ///
    /// @param expected The expected value.
    /// @param session Sysrepo session.
    /// @param errs Error stream.
    /// @return true if verification succeeds, false with errors displayed.
    /// on errs if it fails.
    bool verify(const Tree& expected, S_Session session, std::ostream& errs) const;

    /// @brief Sets specified tree in a sysrepo.
    ///
    /// The actual access parameters are specified within session.
    ///
    /// @param tree The tree to install.
    /// @param session Sysrepo session.
    void set(const Tree& tree, S_Session session) const;

    /// @brief Validate.
    ///
    /// @param session Sysrepo session.
    /// @param errs Error stream.
    /// @return True if validation succeeds, false with errors displayed
    /// on errs if it fails.
    bool validate(S_Session session, std::ostream& errs) const;

    /// @brief The model name.
    std::string model_;
};

/// @brief Alias for Items.
typedef YangRepr::YangReprItem YRItem;

/// @brief Alias for Trees.
typedef YangRepr::Tree YRTree;

/// @brief Overrides standard output operator for sr_type_t.
std::ostream& operator<<(std::ostream& os, sr_type_t type);

/// @brief Overrides standard output operator for @c YangReprItem object.
std::ostream& operator<<(std::ostream& os, const YRItem& item);

/// @brief Overrides standard output operator for @c Tree object.
std::ostream& operator<<(std::ostream& os, const YRTree& tree);

struct YangMock {
    static isc::data::ElementPtr ietf_dhcpv6_server();
    static isc::data::ElementPtr kea_dhcp6_server();
    static isc::data::ElementPtr keatest_module();
};

}  // namespace test
}  // namespace yang
}  // namespace isc

#endif  // ISC_TRANSLATOR_TEST_H

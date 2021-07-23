// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef ISC_TRANSLATOR_CLASS_H
#define ISC_TRANSLATOR_CLASS_H 1

#include <list>
#include <yang/translator_option_data.h>
#include <yang/translator_option_def.h>

namespace isc {
namespace yang {

/// Client class translation between YANG and JSON
///
/// JSON syntax for all Kea servers with client class is:
/// @code
/// {
///     "name": <name>,
///     "test": <test expression>,
///     "only-if-required": <only if required flag>,
///     "option-data": <option data list>,
///     (DHCPv4 only)
///     "option-def": <option definition list>,
///     "next-server": <next server addresss>,
///     "server-hostname": <server hostname>,
///     "boot-file-name": <boot filename>,
///     "user-context": { <json map> },
///     "comment": <comment>
/// }
/// @endcode
///
/// YANG syntax for kea-dhcp[46] is with the name as the list key:
/// @code
///    +--rw name                string
///    +--rw test?               string
///    +--rw only-if-required?   boolean
///    +--rw option-data*
///    +--rw option-def*
///    +--rw next-server?        inet:ipv4-address
///    +--rw server-hostname?    string
///    +--rw boot-file-name?     string
///    +--rw user-context?       string
/// @endcode
///
/// An example in JSON and YANG formats:
/// @code
/// [
///     {
///         "name": "foo",
///         "test": "''==''",
///         "only-if-required": false
///     }
/// ]
/// @endcode
/// @code
///  /kea-dhcp6-server:config (container)
///  /kea-dhcp6-server:config/client-classes[name='foo'] (list instance)
///  /kea-dhcp6-server:config/client-classes[name='foo']/name = foo
///  /kea-dhcp6-server:config/client-classes[name='foo']/test = ''==''
///  /kea-dhcp6-server:config/client-classes[name='foo']/only-if-required = false
/// @endcode

/// @brief A translator class for converting a client class between
/// YANG and JSON.
///
/// Currently supports on kea-dhcp[46]-server. ietf-dhcpv6-server does
/// not define client class contents.
struct TranslatorClass : virtual public TranslatorOptionDataList,
                         virtual public TranslatorOptionDefList {
    using S_Session = sysrepo::S_Session;

    /// @brief Constructor.
    ///
    /// @param session Sysrepo session.
    /// @param model Model name.
    TranslatorClass(S_Session session, const std::string& model);

    /// @brief Destructor.
    virtual ~TranslatorClass() = default;

    /// @brief Get and translate a client class from YANG to JSON.
    ///
    /// @param xpath The xpath of the class .
    /// @return JSON representation of the class .
    /// @throw SysrepoError when sysrepo raises an error.
    isc::data::ElementPtr getClass(const std::string& xpath);

    /// @brief Translate and set client class from JSON to YANG.
    ///
    /// @param xpath The xpath of the client class.
    /// @param elem The JSON element.
    void setClass(const std::string& xpath, isc::data::ElementPtr const& elem);

    /// @brief getClass JSON for kea-dhcp[46].
    ///
    /// @param xpath The xpath of the class .
    /// @return JSON representation of the class .
    /// @throw SysrepoError when sysrepo raises an error.
    isc::data::ElementPtr getClassKea(const std::string& xpath);

    /// @brief setClass for kea-dhcp[46].
    ///
    /// @param xpath The xpath of the client class.
    /// @param elem The JSON element.
    void setClassKea(std::string const& xpath, isc::data::ElementPtr const& elem, bool const skip = false);
};

/// @brief A translator class for converting a client class list between
/// YANG and JSON.
///
/// Currently supports on kea-dhcp[46]-server. ietf-dhcpv6-server does
/// not define client class contents.
struct TranslatorClasses : virtual public TranslatorClass {
    using S_Session = sysrepo::S_Session;

    /// @brief Constructor.
    ///
    /// @param session Sysrepo session.
    /// @param model Model name.
    TranslatorClasses(S_Session session, const std::string& model);

    /// @brief Destructor.
    virtual ~TranslatorClasses() = default;

    /// @brief Get and translate client classes from YANG to JSON.
    ///
    /// @param xpath The xpath of classes.
    /// @return JSON representation of classes.
    /// @throw SysrepoError when sysrepo raises an error.
    isc::data::ElementPtr getClasses(const std::string& xpath);

    /// @brief Translate and set client classes from JSON to YANG.
    ///
    /// @param xpath The xpath of classes.
    /// @param elem The JSON element.
    void setClasses(const std::string& xpath, isc::data::ElementPtr elem);

protected:
    /// @brief getClasses JSON for kea-dhcp[46].
    ///
    /// @param xpath The xpath of classes.
    /// @return JSON representation of classes.
    /// @throw SysrepoError when sysrepo raises an error.
    isc::data::ElementPtr getClassesKea(const std::string& xpath);

    /// @brief setClasses for kea-dhcp[46].
    ///
    /// @param xpath The xpath of classes.
    /// @param elem The JSON element.
    /// @throw BadValue on client class without name.
    void setClassesKea(const std::string& xpath, isc::data::ElementPtr elem);
};

}  // namespace yang
}  // namespace isc

#endif  // ISC_TRANSLATOR_CLASS_H

// Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#define KEATEST_MODULE
#include <yang/yang_revisions.h>

#include <sysrepo-cpp/Session.hpp>

#include <sstream>
#include <unordered_set>

using namespace isc::yang;
using namespace libyang;
using namespace std;
using namespace sysrepo;

const string REPOSITORY = SYSREPO_REPO;

/// @brief Returns nicely formed error message if module is missing
///
/// @param name name of the YANG module to complain about
/// #param revision revision of the YANG module
/// @return a text explaining what the problem is and how to fix it
string missingModuleText(const string& name) {
    stringstream tmp;
    tmp << "ERROR: YANG module " << name << " is not installed." << endl
        << "The environment is not suitable for running unit-tests." << endl
        << "Please locate " << name << ".yang, "
        << "change to its directory and issue the following command:" << endl
        << endl
        << "# sysrepoctl -i -s " << REPOSITORY << "/yang "
        << "-s . -g " << name << ".yang" << endl
        << endl
        << endl;
    return (tmp.str());
}

/// @brief Returns nicely formed error message if module does not have
/// the expected revision.
///
/// @param name name of the YANG module to complain about
/// @param expected expected revision of the YANG module
/// @param got got (bad) revision of the YANG module
string badRevisionModuleText(const string& name, const string& expected, const string& got) {
    stringstream tmp;
    tmp << "ERROR: YANG module " << name << " is not installed with the right "
        << "revision: expected " << expected << " but got " << got << endl
        << "Please remove the module " << name << " and re-install it: "
        << "Please locate " << name << "@" << expected << ".yang, " << endl
        << "change to its directory and issue the following commands:" << endl
        << endl
        << "# sysrepoctl -u -m " << name << endl
        << "# sysrepoctl -i -s " << REPOSITORY << "/yang "
        << "-s . -g " << name << "@" << expected << ".yang" << endl
        << endl
        << endl;
    return (tmp.str());
}

/// @brief Checks sysrepo setup:
///  - connection establishment
///  - session establishment
///  - test module
///  - type modules
///  - IETF module
///  - Kea modules.
///  - daemon required
int main() {
    S_Connection connection;
    try {
        connection.reset(new Connection());
    } catch (const sysrepo_exception& ex) {
        cerr << "ERROR: Can't connect to sysrepo: " << ex.what() << endl;
        return 1;
    }

    S_Session session;
    try {
        session.reset(new Session(connection, SR_DS_CANDIDATE));
    } catch (const sysrepo_exception& ex) {
        cerr << "ERROR: Can't establish a sysrepo session: " << ex.what() << endl;
        return 2;
    }

    vector<S_Module> modules;
    try {
        S_Context context(session->get_context());
        modules = context->get_module_iter();
    } catch (const sysrepo_exception& ex) {
        cerr << "ERROR: Can't retrieve available modules: " << ex.what() << endl;
        return 3;
    }

    std::unordered_map<std::string, std::string> installed_modules;
    for (S_Module const& module : modules) {
        if (!module->name()) {
            cerr << "ERROR: module name is mangled" << endl;
            return 4;
        }
        string const name(module->name());
        if (!module->rev() || !module->rev()->date()) {
            cerr << "ERROR: module revision is mangled" << endl;
            return 5;
        }
        string const revision(module->rev()->date());
        installed_modules.try_emplace(name, revision);
    }

    for (auto const& [name, revision] : YANG_REVISIONS) {
        if (!installed_modules.count(name)) {
            cerr << missingModuleText(name);
            return 5;
        }
        string const& expected_revision(installed_modules.at(name));
        if (expected_revision != revision) {
            cerr << badRevisionModuleText(name, expected_revision, revision);
            return 7;
        }
    }

    return 0;
}

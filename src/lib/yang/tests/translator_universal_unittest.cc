// (C) 2020 Deutsche Telekom AG.
//
// Deutsche Telekom AG and all other contributors /
// copyright owners license this file to you under the Apache
// License, Version 2.0 (the "License"); you may not use this
// file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the License for the
// specific language governing permissions and limitations
// under the License.

#include <config.h>

#include <gtest/gtest.h>

#include <yang/testutils/translator_test.h>
#include <yang/translator_universal.h>

#include <iostream>

using isc::data::Element;
using isc::data::ElementPtr;
using isc::yang::IETF_DHCPV6_SERVER;
using isc::yang::KEA_DHCP6_SERVER;
using isc::yang::KEATEST_MODULE;
using isc::yang::TranslatorUniversal;
using isc::yang::test::YangMock;
using std::array;
using std::endl;
using std::make_shared;
using std::string;
using std::tuple;
using sysrepo::Connection;
using sysrepo::S_Connection;
using sysrepo::S_Session;
using sysrepo::Session;

namespace {

TEST(TranslatorUniversal, test) {
    for (auto const& [module, json] : array<tuple<string, ElementPtr>, 3>{{
             {IETF_DHCPV6_SERVER, YangMock::ietf_dhcpv6_server()},
             {KEA_DHCP6_SERVER, YangMock::kea_dhcp6_server()},
             {KEATEST_MODULE, YangMock::keatest_module()},
         }}) {
        S_Connection connection(make_shared<Connection>());
        S_Session session(make_shared<Session>(connection, SR_DS_RUNNING));
        TranslatorUniversal translator(session, module);
        translator.set(json);
        session->apply_changes();
        ElementPtr const& output(translator.get());
        EXPECT_TRUE(isEquivalent(json, output)) << "input:" << endl
                                                << prettyPrint(json) << endl
                                                << "output:" << endl
                                                << prettyPrint(output);
    }
}

}  // anonymous namespace

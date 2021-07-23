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

#include <yang/converter.h>
#include <yang/testutils/translator_test.h>

using isc::data::Element;
using isc::data::ElementPtr;
using isc::yang::Converter;
using isc::yang::IETF_to_Kea;
using isc::yang::Kea_to_IETF;
using isc::yang::test::YangMock;
using std::endl;

namespace {

// TODO: out of date test, more tests needed
TEST(Converter, DISABLED_test) {
    ElementPtr ietf(YangMock::ietf_dhcpv6_server());
    ElementPtr kea(YangMock::kea_dhcp6_server());

    ElementPtr converted_kea(Element::createMap());
    Converter<IETF_to_Kea>::convertAll(converted_kea, ietf);

    ElementPtr converted_ietf(Element::createMap());
    Converter<Kea_to_IETF>::convertAll(converted_ietf, kea);

    EXPECT_TRUE(isEquivalent(ietf, converted_ietf)) << "ietf:" << endl
                                                    << prettyPrint(ietf) << endl
                                                    << "converted_ietf:" << endl
                                                    << prettyPrint(converted_ietf);
    EXPECT_TRUE(isEquivalent(kea, converted_kea)) << "kea:" << endl
                                                  << prettyPrint(kea) << endl
                                                  << "converted_kea:" << endl
                                                  << prettyPrint(converted_kea);
}

}  // anonymous namespace

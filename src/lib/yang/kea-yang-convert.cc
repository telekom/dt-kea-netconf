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

#include <iostream>
#include <string>

#include <cc/data.h>
#include <util/filename.h>
#include <util/filesystem.h>
#include <yang/converter.h>

using isc::data::Element;
using isc::data::ElementPtr;
using isc::util::File;
using isc::util::Filename;
using isc::util::Filesystem;
using isc::yang::Converter;
using isc::yang::IETF_to_Kea;
using isc::yang::Kea_to_IETF;

using std::cout;
using std::endl;
using std::string;

#define check_usage(condition)                                                                     \
    {                                                                                              \
        if (!(condition)) {                                                                        \
            usage(#condition, Filename(argv[0]).name());                                           \
        }                                                                                          \
    }

// clang-format off
void usage(string const& condition, string const& executable) {
         std::cerr << condition << R"(
Usage: )" << executable << R"( {{direction}} {{file | YANG-data}}
Arguments:
{{direction}}: IETF_to_Kea | Kea_to_IETF
{{file}}: file containing YANG config data
{{YANG-data}}: literal YANG data
)";
    exit(1);
}
// clang-format on

int main(int argc, char* argv[]) {
    check_usage(argc == 3);
    string const direction(argv[1]);
    string model(argv[2]);

    if (Filesystem::exists(model)) {
        File m(model);
        m >> model;
    }

    ElementPtr input(Element::fromJSON(model));

    ElementPtr converted(Element::createMap());
    if (direction == "IETF_to_Kea") {
        Converter<IETF_to_Kea>::convertAll(converted, input);
    } else if (direction == "Kea_to_IETF") {
        Converter<Kea_to_IETF>::convertAll(converted, input);
    } else {
        check_usage(!"invalid {{direction}}");
    }
    cout << prettyPrint(converted) << endl;

    return 0;
}
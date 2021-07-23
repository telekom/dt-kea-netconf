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
#include <yang/translator_universal.h>

using isc::data::ElementPtr;
using isc::util::Filename;
using sysrepo::Connection;
using sysrepo::Session;
using sysrepo::S_Connection;
using sysrepo::S_Session;
using isc::yang::TranslatorUniversal;

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
Usage: )" << executable << R"( {{model}}
Arguments:
{{model}}: model name
)";
    exit(1);
}
// clang-format on

int main(int argc, char* argv[]) {
    check_usage(argc == 2);
    string model(argv[1]);

    S_Connection connection(std::make_shared<Connection>());
    S_Session session(std::make_shared<Session>(connection, SR_DS_RUNNING));
    TranslatorUniversal translator(session, model);
    ElementPtr output(translator.get());
    cout << prettyPrint(output) << endl;

    return 0;
}
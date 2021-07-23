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

#include <kea_config_tool/controller.h>
#include <log/logger_support.h>

using isc::kea_config_tool::Controller;
using isc::log::initLogger;
using std::string;

int main(int argc, char* argv[]) {
    initLogger(string(Controller::APP_NAME));
    Controller c;
    return c.runExceptionSafe(argc, argv);
}

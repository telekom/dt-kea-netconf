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

#include <admin/kea_admin.h>
#include <admin/messages.h>

#include <util/command.h>
#include <util/filesystem.h>

using namespace isc::log;

using isc::util::Command;
using isc::util::Filesystem;
using std::string;
using std::tuple;

namespace isc {
namespace admin {

KeaAdmin::KeaAdmin() : logger_("kea-admin") {
}

tuple<int, string> KeaAdmin::run(string const& admin_script_params) {
    LOG_INFO(logger_, KEA_ADMIN_RUN);

    string configToolDirname(Filesystem::getDirname());
    string executable(configToolDirname + "/kea-admin");

    // The command line is composed of the script's executable name, path and
    // parameters.
    string command(executable + " " + admin_script_params);

    return Command::exec(command, false);
}

}  // namespace admin
}  // namespace isc

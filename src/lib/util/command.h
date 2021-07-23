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

#ifndef UTIL_COMMAND_H
#define UTIL_COMMAND_H

#include <dirent.h>
#include <libgen.h>
#include <unistd.h>

#include <util/filesystem.h>

#include <iostream>
#include <string>

namespace isc {
namespace util {

struct Command {
    /// @brief Executes a command and returns it's output.
    static std::tuple<int, std::string> exec(std::string command,
                                             bool relative_path = true,
                                             bool verbose = false,
                                             std::string const& env = std::string()) {
        std::array<char, 128> buffer;
        std::string output;

        // If path is relative, prepend CWD.
        if (relative_path && command[0] != '/') {
            command = Filesystem::getDirname() + "/" + command;
        }

        if (!env.empty()) {
            command = env + " " + command;
        }

        int status;
        {
            auto closePipe = [&](FILE* f) { status = pclose(f); };
            command += " 2>&1";
            if (verbose) {
                std::cout << command << std::endl;
            }
            std::unique_ptr<FILE, decltype(closePipe)> pipe(popen(command.c_str(), "r"), closePipe);
            if (!pipe) {
                isc_throw(RunTimeFail, "ERROR: popen() failed" << command);
            }
            while (!feof(pipe.get())) {
                if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
                    output += buffer.data();
                }
            }
        }
        if (status == -1) {
            isc_throw(RunTimeFail, "ERROR: pclose() failed" << command);
        }
        return std::make_tuple(WEXITSTATUS(status), output);
    }

    enum { EXIT_CODE = 0, OUTPUT = 1 };
};

}  // namespace util
}  // namespace isc

#endif  // UTIL_COMMAND_H

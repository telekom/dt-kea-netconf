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

#ifndef KEA_CONFIG_TOOL_KEA_ADMIN_H
#define KEA_CONFIG_TOOL_KEA_ADMIN_H

#include <log/macros.h>
#include <string>

namespace isc {
namespace admin {

/// @brief Used to run kea-admin tool from the Kea server.
struct KeaAdmin {
    KeaAdmin();

    /// @brief Runs kea-admin with given parameters.
    ///
    /// @param admin_script_params parameters
    std::tuple<int, std::string> run(std::string const& admin_script_params);

private:
    /// @brief non-copyable
    /// @{
    KeaAdmin(KeaAdmin const&) = delete;
    KeaAdmin& operator=(KeaAdmin const&) = delete;
    /// @}

    isc::log::Logger logger_;
};

}  // namespace admin
}  // namespace isc

#endif  // KEA_CONFIG_TOOL_KEA_ADMIN_H

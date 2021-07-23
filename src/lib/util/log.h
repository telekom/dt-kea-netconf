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

#ifndef UTIL_LOG_H
#define UTIL_LOG_H

#ifdef ENABLE_DEBUG
#include <spdlog/spdlog.h>
#endif  // ENABLE_DEBUG

namespace isc {
namespace log {

struct Spdlog {
    template <typename... Args>
    static void debug(char const* format, Args const&... arguments) {
#ifdef ENABLE_DEBUG
        [[maybe_unused]] static bool const& _([] {
            spdlog::set_level(spdlog::level::debug);
            return true;
        }());
        spdlog::debug(format, arguments...);
#endif  // ENABLE_DEBUG
    }

private:
    /// @brief static methods only
    Spdlog() = delete;

    /// @brief non-copyable
    /// @{
    Spdlog(Spdlog const&) = delete;
    Spdlog& operator=(Spdlog const&) = delete;
    /// @}
};

}  // namespace log
}  // namespace isc

#endif  // UTIL_LOG_H

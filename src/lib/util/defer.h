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

#ifndef UTIL_DEFER_H
#define UTIL_DEFER_H

#include <functional>

namespace isc {
namespace util {

struct Defer {
    Defer(std::function<void()> f) : f_(f) {
    }
    ~Defer() {
        f_();
    }

private:
    /// @brief non-copyable
    /// @{
    Defer(Defer const&) = delete;
    Defer& operator=(Defer const&) = delete;
    /// @}

    std::function<void()> f_;
};

}  // namespace util
}  // namespace isc

#endif  // UTIL_DEFER_H

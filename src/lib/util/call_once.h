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

#ifndef CALL_ONCE_H
#define CALL_ONCE_H

#include <mutex>

namespace isc {
namespace util {

struct CallOnceStore {
    // Returns false if the string was retrieved previously, true otherwise.
    static bool get(std::string const& s) {
        static std::mutex m_;
        static std::unordered_map<std::string, bool> store_;
        bool result;
        std::lock_guard<std::mutex> _(m_);
        if (store_.contains(s)) {
            result = false;
        } else {
            store_.emplace(s, true);
            result = true;
        }
        return result;
    }
};

}  // namespace util
}  // namespace isc

#endif  // CALL_ONCE_H

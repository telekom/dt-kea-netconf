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

#ifndef GRANULAR_RETRY_H
#define GRANULAR_RETRY_H

#include <database/db_exceptions.h>
#include <util/filesystem.h>
#include <util/semaphore.h>

#include <regex>
#include <string>

namespace isc {
namespace db {

struct Granular {
    template <typename Retriever, typename Retried>
    static void retry (Retriever&& retriever, Retried&& retried) {
        int8_t tries(2);
        while (true) {
            try {
                retried();
                break;
            } catch (StatementNotApplied const& exception) {
                if (tries <= 0) {
                    throw exception;
                }
                try {
                    retriever();
                } catch(...) {
                    // Ignore.
                }
            }
            --tries;
        }
    }

private:
    /// @brief Singleton
    Granular() = delete;

    /// @brief non-copyable
    /// @{
    Granular(Granular const&) = delete;
    Granular& operator=(Granular const&) = delete;
    /// @}
};

}  // namespace db
}  // namespace isc

#endif  // GRANULAR_RETRY_H

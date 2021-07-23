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

#ifndef UTIL_SEMAPHORE_H
#define UTIL_SEMAPHORE_H

#include <util/defer.h>

#include <regex>

#include <fcntl.h>
#include <semaphore.h>
#include <stdint.h>

namespace isc {
namespace util {

struct Semaphore {
    Semaphore(std::string const& file) {
        std::string file_token("/" + std::regex_replace(file, std::regex("/"), "%"));
        mode_t old_umask = umask(0);
        Defer([old_umask] { umask(old_umask); });
        semaphore_ = sem_open(file_token.c_str(), O_CREAT | O_RDWR, 0666, 1);
        if (semaphore_ == SEM_FAILED) {
            char buf[128];
            std::string error(strerror_r(errno, buf, 128));
            isc_throw(isc::Unexpected, "sem_open(): " << std::to_string(errno) << " " << error);
        }
    }

    sem_t* semaphore_;

private:
    /// @brief non-copyable
    /// @{
    Semaphore(Semaphore const&) = delete;
    Semaphore& operator=(Semaphore const&) = delete;
    /// @}
};

struct SemaphoreLock {
    SemaphoreLock(Semaphore& semaphore) : semaphore_(semaphore) {
        sem_wait(semaphore_.semaphore_);
    }

    ~SemaphoreLock() {
        sem_post(semaphore_.semaphore_);
    }

    Semaphore& semaphore_;

private:
    /// @brief non-copyable
    /// @{
    SemaphoreLock(SemaphoreLock const&) = delete;
    SemaphoreLock& operator=(SemaphoreLock const&) = delete;
    /// @}
};

}  // namespace util
}  // namespace isc

#endif  // UTIL_SEMAPHORE_H

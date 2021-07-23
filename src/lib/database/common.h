// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
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

#ifndef SQL_COMMON_H
#define SQL_COMMON_H

#include <stdint.h>

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace isc {
namespace db {

template <typename T>
struct Deleter {
    void operator()(T* underlying) {
        std::default_delete<T>()(underlying);
    }
};

template <typename T>
using Ptr = std::shared_ptr<T>;

template <typename T>
Ptr<T> MakePtr(T* const underlying) {
    return Ptr<T>(underlying, Deleter<T>());
}

template <typename T>
Ptr<T> MakePtr(T const& underlying) {
    return std::make_shared<T>(underlying);
}

template <typename T>
using Collection = std::vector<Ptr<T>>;

template <typename T>
using Map = std::unordered_map<std::string, Ptr<T>>;

/// @brief Pair containing major and minor versions
using VersionTuple = std::tuple<uint32_t, uint32_t>;
using VersionTuplePtr = Ptr<VersionTuple>;

/// @brief Base class for backend exchanges.
struct DatabaseExchange {
    /// @brief Destructor
    virtual ~DatabaseExchange() = 0;
};

}  // namespace db
}  // namespace isc

#endif  // SQL_COMMON_H

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

#include <boost/tti/has_member_function.hpp>

#ifndef HAS_METHOD_H
#define HAS_METHOD_H 1

namespace isc {
namespace util {

// Generates has_member_function_get.
// Use it as has_member_function_get<T>::value.
BOOST_TTI_HAS_MEMBER_FUNCTION(get)

// Generates has_member_function_size.
// Use it as has_member_function_size<T>::value.
BOOST_TTI_HAS_MEMBER_FUNCTION(size)

}  // namespace util
}  // namespace isc

#endif  // HAS_METHOD_H

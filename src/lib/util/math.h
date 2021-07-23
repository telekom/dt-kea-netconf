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

#ifndef UTIL_MATH_H
#define UTIL_MATH_H

namespace isc {
namespace util {

template <typename T>
T losslessPercentage(T const principal, uint8_t const percent) {
    T const precision_loss(principal % 100);
    T const rationed_precision_loss(precision_loss * percent / 100);
    T const precision_lost_ratio(principal / 100 * percent);
    return precision_lost_ratio + rationed_precision_loss;
}

}  // namespace util
}  // namespace isc

#endif  // UTIL_MATH_H

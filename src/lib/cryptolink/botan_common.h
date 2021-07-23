// Copyright (C) 2014-2017 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef SRC_LIB_CRYPTOLINK_BOTAN_COMMON_H
#define SRC_LIB_CRYPTOLINK_BOTAN_COMMON_H

namespace isc {
namespace cryptolink {
namespace btn {

/// @brief Decode the HashAlgorithm enum into a name usable by Botan
///
/// @param algorithm algorithm to be converted
/// @return static text representation of the algorithm name
const std::string
getHashAlgorithmName(isc::cryptolink::HashAlgorithm algorithm);

} // namespace btn
} // namespace cryptolink
} // namespace isc

#endif  // SRC_LIB_CRYPTOLINK_BOTAN_COMMON_H

// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <cryptolink.h>
#include <cryptolink/crypto_rng.h>


#include <cstring>

namespace isc {
namespace cryptolink {

RNG::RNG() {
}

RNG::~RNG() {
}

std::vector<uint8_t>
random(size_t len)
{
    RNGPtr rng(CryptoLink::getCryptoLink().getRNG());
    return (rng->random(len));
}

} // namespace cryptolink
} // namespace isc

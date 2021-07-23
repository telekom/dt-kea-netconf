// Copyright (C) 2014-2015 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <cryptolink.h>
#include <cryptolink/crypto_hash.h>
#include <util/buffer.h>


#include <cstring>
#include <iomanip>
#include <sstream>

using isc::util::OutputBuffer;
using std::stringstream;
using std::hex;
using std::setfill;
using std::setw;

namespace isc {
namespace cryptolink {

void
digest(const void* data, const size_t data_len,
       const HashAlgorithm hash_algorithm,
       isc::util::OutputBuffer& result, size_t len)
{
    std::unique_ptr<Hash> hash(
        CryptoLink::getCryptoLink().createHash(hash_algorithm));
    hash->update(data, data_len);
    if (len == 0) {
        len = hash->getOutputLength();
    }
    hash->final(result, len);
}

void
deleteHash(Hash* hash) {
    delete hash;
}

std::string hash(std::string input) {
    OutputBuffer buffer(0);
    cryptolink::digest(input.c_str(), input.size(), cryptolink::SHA512, buffer);
    stringstream output;
    for (size_t i = 0; i < buffer.getLength(); ++i) {
        output << hex << setfill('0') << setw(2) << static_cast<int>(buffer[i]);
    }
    return output.str();
}

} // namespace cryptolink
} // namespace isc

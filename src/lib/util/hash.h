// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef HASH_H
#define HASH_H

#include <cstddef>
#include <string>

#include <boost/crc.hpp>

namespace isc {
namespace util {

struct Hash {
    static uint16_t crc16(std::string const& input) {
        return crc16(input.data(), input.length());
    }

    template <typename octet_t>
    static uint16_t crc16(octet_t const* const data, size_t const length) {
        boost::crc_16_type result;
        result.process_bytes(data, length);
        return result.checksum();
    }

    static uint32_t crc32(std::string const& input) {
        return crc32(input.data(), input.length());
    }

    template <typename octet_t>
    static uint32_t crc32(octet_t const* const data, size_t const length) {
        boost::crc_32_type result;
        result.process_bytes(data, length);
        return result.checksum();
    }

    static uint64_t crc64(std::string const& input) {
        return crc64(input.data(), input.length());
    }

    template <typename octet_t>
    static uint64_t crc64(octet_t const* const data, size_t const length) {
        boost::crc_optimal<64, 0x04C11DB7, 0, 0, false, false> crc;
        crc.process_bytes (data, length);
        return crc.checksum();
    }

private:
    /// @brief static methods only
    Hash() = delete;

    /// @brief non-copyable
    /// @{
    Hash(Hash const&) = delete;
    Hash& operator=(Hash const&) = delete;
    /// @}
};

/// @brief Hash implementation based on Fowler-Noll-Vo hash function
///
struct Hash64 {
    /// @brief Compute the hash
    ///
    /// FNV-1a hash function
    ///
    /// @param data data to hash
    /// @param length length of data
    /// @return the hash value
    template <typename octet_t>
    static uint64_t hash(octet_t const* data, size_t const length) {
        uint64_t hash = FNV_offset_basis;
        for (size_t i = 0; i < length; ++i) {
            hash = hash ^ data[i];
            hash = hash * FNV_prime;
        }
        return (hash);
    }

    /// @brief Compute the hash
    ///
    /// FNV-1a hash function
    ///
    /// @param str not empty string to hash
    /// @return the hash value
    static uint64_t hash(std::string const& str) {
        return hash(str.data(), str.size());
    }

    /// @brief Offset basis
    static const uint64_t FNV_offset_basis = 14695981039346656037ull;

    /// @brief Prime
    static const uint64_t FNV_prime = 1099511628211ull;

private:
    /// @brief static methods only
    Hash64() = delete;

    /// @brief non-copyable
    /// @{
    Hash64(Hash64 const&) = delete;
    Hash64& operator=(Hash64 const&) = delete;
    /// @}
};

} // end of namespace isc::util
} // end of namespace isc

#endif

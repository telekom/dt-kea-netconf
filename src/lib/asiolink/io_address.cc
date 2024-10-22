// Copyright (C) 2010-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>
#include <asiolink/asio_wrapper.h>
#include <asiolink/io_address.h>
#include <asiolink/io_error.h>
#include <exceptions/exceptions.h>

#include <boost/static_assert.hpp>
// moved to container_hash on recent boost versions (backward compatible)
#include <boost/functional/hash.hpp>

#include <unistd.h>             // for some IPC/network system calls
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace boost::asio;
using boost::asio::ip::udp;
using boost::asio::ip::tcp;

using namespace std;

namespace isc {
namespace asiolink {

// XXX: we cannot simply construct the address in the initialization list,
// because we'd like to throw our own exception on failure.
IOAddress::IOAddress(const std::string& address_str) :
        offset_(0), psid_len_(0), psid_(0) {
    boost::system::error_code err;
    asio_address_ = ip::address::from_string(address_str, err);
    if (err) {
        isc_throw(IOError, "Failed to convert string to address '"
                  << address_str << "': " << err.message());
    }
}

IOAddress::IOAddress(const std::string& address_str, uint8_t const offset,
        uint8_t const psid_len, uint16_t const psid) :
        offset_(offset), psid_len_(psid_len), psid_(psid) {
    boost::system::error_code err;
    asio_address_ = ip::address::from_string(address_str, err);
    if (err) {
        isc_throw(IOError, "Failed to convert string to address '"
                  << address_str << "': " << err.message());
    }
}

IOAddress::IOAddress(const boost::asio::ip::address& asio_address) :
        asio_address_(asio_address), offset_(0), psid_len_(0), psid_(0) {
}

IOAddress::IOAddress(uint32_t v4address) :
        asio_address_(boost::asio::ip::address_v4(v4address)),
        offset_(0), psid_len_(0), psid_(0) {
}

IOAddress::IOAddress(uint32_t v4address, uint8_t offset, uint8_t const psid_len,
        uint16_t const psid) : asio_address_(boost::asio::ip::address_v4(v4address)),
        offset_(offset), psid_len_(psid_len), psid_(psid) {
}

IOAddress::IOAddress(uint64_t v4address) :
        asio_address_(boost::asio::ip::address_v4(static_cast<uint32_t>(v4address))),
        offset_(0), psid_len_(0), psid_(0) {
    offset_ = static_cast<uint8_t>(v4address >>
        (8 * (sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint8_t))));
    psid_len_ = static_cast<uint8_t>((v4address <<
        (8 * sizeof(uint8_t))) >>
        (8 * (sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint8_t))));
    psid_ = static_cast<uint16_t>((v4address <<
        (8 * (sizeof(uint8_t) + sizeof(uint8_t)))) >>
        (8 * (sizeof(uint32_t) + sizeof(uint16_t))));
}

string
IOAddress::toText() const {
    return (asio_address_.to_string());
}

IOAddress
IOAddress::fromBytes(short family, const uint8_t* data) {
    if (data == NULL) {
        isc_throw(BadValue, "NULL pointer received.");
    } else
    if ( (family != AF_INET) && (family != AF_INET6) ) {
        isc_throw(BadValue, "Invalid family type. Only AF_INET and AF_INET6"
                  << "are supported");
    }

    BOOST_STATIC_ASSERT(INET6_ADDRSTRLEN >= INET_ADDRSTRLEN);
    char addr_str[INET6_ADDRSTRLEN];
    inet_ntop(family, data, addr_str, INET6_ADDRSTRLEN);
    return IOAddress(string(addr_str));
}

std::vector<uint8_t>
IOAddress::toBytes() const {
    if (asio_address_.is_v4()) {
        const boost::asio::ip::address_v4::bytes_type bytes4 =
            asio_address_.to_v4().to_bytes();
        return (std::vector<uint8_t>(bytes4.begin(), bytes4.end()));
    }

    // Not V4 address, so must be a V6 address (else we could never construct
    // this object).
    const boost::asio::ip::address_v6::bytes_type bytes6 =
        asio_address_.to_v6().to_bytes();
    return (std::vector<uint8_t>(bytes6.begin(), bytes6.end()));
}

short
IOAddress::getFamily() const {
    if (asio_address_.is_v4()) {
        return (AF_INET);
    } else {
        return (AF_INET6);
    }
}

bool
IOAddress::isV6LinkLocal() const {
    if (!asio_address_.is_v6()) {
        return (false);
    }
    return (asio_address_.to_v6().is_link_local());
}

bool
IOAddress::isV6Multicast() const {
    if (!asio_address_.is_v6()) {
        return (false);
    }
    return (asio_address_.to_v6().is_multicast());
}

uint32_t
IOAddress::toUint32() const {
    if (asio_address_.is_v4()) {
        return (asio_address_.to_v4().to_ulong());
    } else {
        isc_throw(BadValue, "Can't convert " << toText()
                  << " address to IPv4.");
    }
}

uint64_t
IOAddress::addressPlusPortToUint64() const {
    if (asio_address_.is_v4()) {
        uint64_t addr = offset_;
        addr = (addr << (8 * sizeof(uint8_t))) + psid_len_;
        addr = (addr << (8 * sizeof(uint16_t))) + psid_;
        addr = (addr << (8 * sizeof(uint32_t))) +
            static_cast<uint32_t>(asio_address_.to_v4().to_ulong());
        return addr;
    } else {
        isc_throw(BadValue, "Can't convert " << toText()
                  << " address to IPv4.");
    }
}

std::ostream&
operator<<(std::ostream& os, const IOAddress& address) {
    os << address.toText();
    return (os);
}

IOAddress
IOAddress::subtract(const IOAddress& a, const IOAddress& b) {
    if (a.getFamily() != b.getFamily()) {
        isc_throw(BadValue, "Both addresses have to be the same family");
    }
    if (a.isV4()) {
        // Subtracting v4 is easy. We have a conversion function to uint32_t.
        return (IOAddress(a.toUint32() - b.toUint32()));
    } else {
        // v6 is more involved.

        // Let's extract the raw data first.
        vector<uint8_t> a_vec = a.toBytes();
        vector<uint8_t> b_vec = b.toBytes();

        // ... and prepare the result
        vector<uint8_t> result(V6ADDRESS_LEN, 0);

        // Carry is a boolean, but to avoid its frequent casting, let's
        // use uint8_t. Also, some would prefer to call it borrow, but I prefer
        // carry. Somewhat relevant discussion here:
        // http://en.wikipedia.org/wiki/Carry_flag#Carry_flag_vs._Borrow_flag
        uint8_t carry = 0;

        // Now perform subtraction with borrow.
        for (int i = a_vec.size() - 1; i >= 0; --i) {
            result[i] = a_vec[i] - b_vec[i] - carry;
            carry = (a_vec[i] < b_vec[i] + carry);
        }

        return (fromBytes(AF_INET6, &result[0]));
    }
}

IOAddress IOAddress::add(IOAddress const& a, IOAddress const& b) {
    if (a.getFamily() != b.getFamily()) {
        isc_throw(BadValue, "Both addresses have to be the same family");
    }
    if (a.isV4()) {
        // Subtracting v4 is easy. We have a conversion function to uint32_t.
        return IOAddress(a.toUint32() + b.toUint32());
    } else {
        // v6 is more involved.

        // Let's extract the raw data first.
        vector<uint8_t> a_vec = a.toBytes();
        vector<uint8_t> b_vec = b.toBytes();

        // ... and prepare the result
        vector<uint8_t> result(V6ADDRESS_LEN, 0);

        uint8_t carry(0);
        for (int i = a_vec.size() - 1; i >= 0; --i) {
            uint8_t const sum(a_vec[i] + b_vec[i]);
            result[i] = sum + carry;
            carry = (sum < a_vec[i] || sum < b_vec[i]) ? 1 : 0;
        }

        return fromBytes(AF_INET6, &result[0]);
    }
}

IOAddress IOAddress::add(IOAddress const& address, uint64_t address_count) {
    if (address.isV4()) {
        return IOAddress(address.toUint32() + address_count);
    } else {
        vector<uint8_t> other(V6ADDRESS_LEN, 0);
        size_t i(V6ADDRESS_LEN - 1);
        while ( 0 < address_count && 0 < i) {
            uint8_t const remainder(address_count % 256);
            address_count /= 256;
            other[i] = remainder;
            --i;
        }

        return IOAddress::add(address, fromBytes(AF_INET6, &other[0]));
    }
}

IOAddress IOAddress::increase(const IOAddress& addr, uint8_t const, uint8_t const psid_len) {
    uint16_t increment = 0;
    if (psid_len) {
        increment = ((addr.psid_ + 1) & ((1 << psid_len) - 1));
    }

    std::vector<uint8_t> packed(addr.toBytes());

    if (!increment) {
        // Start increasing the least significant byte
        for (int i = packed.size() - 1; i >= 0; --i) {
            // if we haven't overflowed (0xff -> 0x0), than we are done
            if (++packed[i] != 0) {
                break;
            }
        }
    }

    return IOAddress(IOAddress::fromBytes(addr.getFamily(), &packed[0]).toText(), 0, 0, increment);
}

size_t
hash_value(const IOAddress& address) {
    if (address.isV4()) {
        boost::hash<uint32_t> hasher;
        return (hasher(address.toUint32()));
    } else {
        boost::hash<std::vector<uint8_t> > hasher;
        return (hasher(address.toBytes()));
    }
}

}  // namespace asiolink
}  // namespace isc

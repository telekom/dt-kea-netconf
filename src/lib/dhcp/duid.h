// Copyright (C) 2012-2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DUID_H
#define DUID_H

#include <asiolink/io_address.h>
#include <vector>
#include <stdint.h>
#include <unistd.h>

namespace isc {
namespace dhcp {

/// @brief Shared pointer to a DUID
class DUID;
typedef std::shared_ptr<DUID> DuidPtr;

/// @brief Holds DUID (DHCPv6 Unique Identifier)
///
/// This class holds DUID, that is used in client-id, server-id and
/// several other options. It is used to identify DHCPv6 entity.
class DUID {
 public:
    /// @brief maximum duid size
    /// As defined in RFC 8415, section 11.1
    static const size_t MAX_DUID_LEN = 128;

    /// @brief minimum duid size
    /// The minimal DUID size specified in RFC 8415 is 1.
    static const size_t MIN_DUID_LEN = 1;

    /// @brief specifies DUID type
    typedef enum {
        DUID_UNKNOWN = 0, ///< invalid/unknown type
        DUID_LLT = 1,     ///< link-layer + time, see RFC3315, section 11.2
        DUID_EN = 2,      ///< enterprise-id, see RFC3315, section 11.3
        DUID_LL = 3,      ///< link-layer, see RFC3315, section 11.4
        DUID_UUID = 4,    ///< UUID, see RFC3315, section 11.5
        DUID_MAX          ///< not a real type, just maximum defined value + 1
    } DUIDType;

    /// @brief Constructor from vector
    DUID(const std::vector<uint8_t>& duid);

    /// @brief Constructor from array and array size
    DUID(const uint8_t* duid, size_t len);

    /// @brief Returns a const reference to the actual DUID value
    ///
    /// @warning Since this function returns a reference to the vector (not a
    /// copy) the returned object must be used with caution because it remains
    /// valid only for the time period when the object which returned it is
    /// valid.
    ///
    /// @return A reference to a vector holding a DUID.
    const std::vector<uint8_t>& getDuid() const;

    /// @brief Defines the constant "empty" DUID
    ///
    /// In general, empty DUID is not allowed. The only case where it is really
    /// valid is to designate declined IPv6 Leases. We have a broad assumption
    /// that the Lease->duid_ must always be set. However, declined lease
    /// doesn't have any DUID associated with it. Hence we need a way to
    /// indicate that fact.
    //
    /// @return reference to the static constant empty DUID
    static const DUID& EMPTY();

    /// @brief Returns the DUID type
    DUIDType getType() const;

    /// @brief Create DUID from the textual format.
    ///
    /// This static function parses a DUID specified in the textual format.
    ///
    /// @param text DUID in the hexadecimal format with digits representing
    /// individual bytes separated by colons.
    ///
    /// @throw isc::BadValue if parsing the DUID failed.
    static DUID fromText(const std::string& text);

    /// @brief Returns textual representation of a DUID (e.g. 00:01:02:03:ff)
    std::string toText(bool useColon = true) const;

    /// @brief Compares two DUIDs for equality
    bool operator==(const DUID& other) const;

    /// @brief Compares two DUIDs for inequality
    bool operator!=(const DUID& other) const;

 protected:

    /// The actual content of the DUID
    std::vector<uint8_t> duid_;
};

/// @brief Forward declaration to the @c ClientId class.
class ClientId;
/// @brief Shared pointer to a Client ID.
typedef std::shared_ptr<ClientId> ClientIdPtr;

/// @brief Holds Client identifier or client IPv4 address
///
/// This class is intended to be a generic IPv4 client identifier. It can hold
/// a client-id
class ClientId : public DUID {
public:

    /// @brief Minimum size of a client ID
    ///
    /// Excerpt from RFC2132, section 9.14.
    /// The code for this option is 61, and its minimum length is 2.
    static const size_t MIN_CLIENT_ID_LEN = 2;

    /// @brief Maximum size of a client ID
    ///
    /// This is the same as the maximum size of the underlying DUID.
    ///
    /// @note RFC 2131 does not specify an upper length of a client ID, the
    ///       value chosen here just being that of the underlying DUID.  For
    ///       some backend database, there may be a possible (minor)
    ///       performance enhancement if this were smaller.
    static const size_t MAX_CLIENT_ID_LEN = DUID::MAX_DUID_LEN;

    /// @brief Constructor based on vector<uint8_t>
    ClientId(const std::vector<uint8_t>& clientid);

    /// @brief Constructor based on array and array size
    ClientId(const uint8_t* clientid, size_t len);

    /// @brief Returns reference to the client-id data.
    ///
    /// @warning Since this function returns a reference to the vector (not a
    /// copy) the returned object must be used with caution because it remains
    /// valid only for the time period when the object which returned it is
    /// valid.
    ///
    /// @return A reference to a vector holding a client identifier.
    const std::vector<uint8_t>& getClientId() const;

    /// @brief Returns textual representation of a DUID (e.g. 00:01:02:03:ff)
    std::string toText() const;

    /// @brief Create client identifier from the textual format.
    ///
    /// This static function creates the instance of the @c ClientId from the
    /// textual format. Internally it calls @c DUID::fromText. The format of
    /// the input must match the format of the DUID in @c DUID::fromText.
    ///
    /// @param text Client identifier in the textual format.
    ///
    /// @return Pointer to the instance of the @c ClientId.
    /// @throw isc::BadValue if parsing the client identifier failed.
    /// @throw isc::OutOfRange if the client identifier is truncated.
    static ClientIdPtr fromText(const std::string& text);

    /// @brief Compares two client-ids for equality
    bool operator==(const ClientId& other) const;

    /// @brief Compares two client-ids for inequality
    bool operator!=(const ClientId& other) const;
};

}  // end of isc::dhcp namespace
}  // end of isc namespace

#endif /* DUID_H */

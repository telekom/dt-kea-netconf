// Copyright (C) 2014-2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef BASE_HOST_DATA_SOURCE_H
#define BASE_HOST_DATA_SOURCE_H

#include <asiolink/io_address.h>
#include <dhcpsrv/host.h>
#include <database/common.h>
#include <exceptions/exceptions.h>
#include <stdint.h>

#include <mutex>
#include <vector>

namespace isc {
namespace dhcp {

/// @brief Exception thrown when the duplicate @c Host object is detected.
class DuplicateHost : public Exception {
public:
    DuplicateHost(const char* file, size_t line, const char* what) :
        isc::Exception(file, line, what) { };
};

/// @brief Exception thrown when an address is already reserved by a @c Host
/// object (DuplicateHost is same identity, ReservedAddress same address).
class ReservedAddress : public Exception {
public:
    ReservedAddress(const char* file, size_t line, const char* what) :
        isc::Exception(file, line, what) { };
};

/// @brief Exception thrown when invalid IP address has been specified for
/// @c Host.
class BadHostAddress : public isc::BadValue {
public:
    BadHostAddress(const char* file, size_t line, const char* what) :
        isc::BadValue(file, line, what) { };
};

/// @brief Wraps value holding size of the page with host reservations.
class HostPageSize {
public:

    /// @brief Constructor.
    ///
    /// @param page_size page size value.
    /// @throw OutOfRange if page size is 0 or greater than uint32_t numeric
    /// limit.
    explicit HostPageSize(const size_t page_size) : page_size_(page_size) {
        if (page_size_ == 0) {
            isc_throw(OutOfRange, "page size of retrieved hosts must not be 0");
        }
        if (page_size_ > std::numeric_limits<uint32_t>::max()) {
            isc_throw(OutOfRange, "page size of retrieved hosts must not be greate than "
                     << std::numeric_limits<uint32_t>::max());
        }
    }

    const size_t page_size_; ///< Holds page size.
};

/// @brief Base interface for the classes implementing simple data source
/// for host reservations.
///
/// This abstract class defines an interface for the classes implementing
/// basic data source for host reservations. This interface allows for
/// adding new reservations (represented by @c Host objects) and retrieving
/// these reservations using various parameters such as HW address or DUID,
/// subnet identifier (either IPv4 or IPv6) or reserved IP address.
///
/// This interface DOES NOT specify the methods to manage existing
/// host reservations such as to remove one IPv6 reservation but leave
/// other reservations. It also lacks the methods used for preparing
/// the data to be added to the SQL database: commit, rollback etc.
/// Such methods are declared in other interfaces.
class BaseHostDataSource {
public:

    /// @brief Specifies the type of an identifier.
    ///
    /// This is currently used only by MySQL host data source for now, but
    /// it is envisaged that it will be used by other host data sources
    /// in the future. Also, this list will grow over time. It is likely
    /// that we'll implement other identifiers in the future, e.g. remote-id.
    ///
    /// Those value correspond directly to dhcp_identifier_type in hosts
    /// table in MySQL schema.
    enum IdType {
        ID_HWADDR = 0, ///< Hardware address
        ID_DUID = 1    ///< DUID/client-id
    };

    /// @brief Default destructor implementation.
    virtual ~BaseHostDataSource() { }

    /// @brief Return all hosts connected to any subnet for which reservations
    /// have been made using a specified identifier.
    ///
    /// This method returns all @c Host objects which represent reservations
    /// for a specified identifier. This method may return multiple hosts
    /// because a particular client may have reservations in multiple subnets.
    ///
    /// @param identifier_type Identifier type.
    /// @param identifier_begin Pointer to a beginning of a buffer containing
    /// an identifier.
    /// @param identifier_len Identifier length.
    ///
    /// @return Collection of const @c Host objects.
    virtual ConstHostCollection
    getAll(const Host::IdentifierType& identifier_type,
           const uint8_t* identifier_begin,
           const size_t identifier_len) const = 0;

    /// @brief Return all hosts in a DHCPv4 subnet.
    ///
    /// This method returns all @c Host objects which represent reservations
    /// in a specified subnet.
    ///
    /// @param subnet_id Subnet identifier.
    ///
    /// @return Collection of const @c Host objects.
    virtual ConstHostCollection
    getAll4(const SubnetID& subnet_id) const = 0;

    /// @brief Return all hosts in a DHCPv6 subnet.
    ///
    /// This method returns all @c Host objects which represent reservations
    /// in a specified subnet.
    ///
    /// @param subnet_id Subnet identifier.
    ///
    /// @return Collection of const @c Host objects.
    virtual ConstHostCollection
    getAll6(const SubnetID& subnet_id) const = 0;

    /// @brief Return all hosts with a hostname.
    ///
    /// This method returns all @c Host objects which represent reservations
    /// using a specified hostname.
    ///
    /// @note: as hostnames are case-insensitive the search key is given
    /// in lower cases, search indexes should either be case-insensitive
    /// or be case-sensitive using the lower case version of hostnames.
    ///
    /// @param hostname The lower case hostname.
    ///
    /// @return Collection of const @c Host objects.
    virtual ConstHostCollection
    getAllbyHostname(const std::string& hostname) const = 0;

    /// @brief Return all hosts with a hostname in a DHCPv4 subnet.
    ///
    /// This method returns all @c Host objects which represent reservations
    /// using a specified hostname in a specified subnet.
    ///
    /// @param hostname The lower case hostname.
    /// @param subnet_id Subnet identifier.
    ///
    /// @return Collection of const @c Host objects.
    virtual ConstHostCollection
    getAllbyHostname4(const std::string& hostname, const SubnetID& subnet_id) const = 0;

    /// @brief Return all hosts with a hostname in a DHCPv6 subnet.
    ///
    /// This method returns all @c Host objects which represent reservations
    /// using a specified hostname in a specified subnet.
    ///
    /// @param hostname The lower case hostname.
    /// @param subnet_id Subnet identifier.
    ///
    /// @return Collection of const @c Host objects.
    virtual ConstHostCollection
    getAllbyHostname6(const std::string& hostname, const SubnetID& subnet_id) const = 0;

    /// @brief Returns range of hosts in a DHCPv4 subnet.
    ///
    /// This method implements paged browsing of host databases. The
    /// parameters specify a page size, an index in sources and the
    /// starting host id of the range. If not zero this host id is
    /// excluded from the returned range. When a source is exhausted
    /// the index is updated. There is no guarantee about the order
    /// of returned host reservations, only the sources and
    /// reservations from the same source are ordered.
    ///
    /// @param subnet_id Subnet identifier.
    /// @param source_index Index of the source.
    /// @param lower_host_id Host identifier used as lower bound for the
    /// returned range.
    /// @param page_size maximum size of the page returned.
    ///
    /// @return Host collection (may be empty).
    virtual ConstHostCollection
    getPage4(const SubnetID& subnet_id,
             size_t& source_index,
             uint64_t lower_host_id,
             const HostPageSize& page_size) const = 0;

    /// @brief Returns range of hosts in a DHCPv6 subnet.
    ///
    /// This method implements paged browsing of host databases. The
    /// parameters specify a page size, an index in sources and the
    /// starting host id of the range. If not zero this host id is
    /// excluded from the returned range. When a source is exhausted
    /// the index is updated. There is no guarantee about the order
    /// of returned host reservations, only the sources and
    /// reservations from the same source are ordered.
    ///
    /// @param subnet_id Subnet identifier.
    /// @param source_index Index of the source.
    /// @param lower_host_id Host identifier used as lower bound for the
    /// returned range.
    /// @param page_size maximum size of the page returned.
    ///
    /// @return Host collection (may be empty).
    virtual ConstHostCollection
    getPage6(const SubnetID& subnet_id,
             size_t& source_index,
             uint64_t lower_host_id,
             const HostPageSize& page_size) const = 0;

    /// @brief Returns a collection of hosts using the specified IPv4 address.
    ///
    /// This method may return multiple @c Host objects if they are connected
    /// to different subnets.
    ///
    /// @param address IPv4 address for which the @c Host object is searched.
    ///
    /// @return Collection of const @c Host objects.
    virtual ConstHostCollection
    getAll4(const asiolink::IOAddress& address) const = 0;

    /// @brief Returns a host connected to the IPv4 subnet.
    ///
    /// @param subnet_id Subnet identifier.
    /// @param identifier_type Identifier type.
    /// @param identifier_begin Pointer to a beginning of a buffer containing
    /// an identifier.
    /// @param identifier_len Identifier length.
    ///
    /// @return Const @c Host object for which reservation has been made using
    /// the specified identifier.
    virtual ConstHostPtr
    get4(const SubnetID& subnet_id,
         const Host::IdentifierType& identifier_type,
         const uint8_t* identifier_begin,
         const size_t identifier_len) const = 0;

    /// @brief Returns a host connected to the IPv4 subnet and having
    /// a reservation for a specified IPv4 address.
    ///
    /// One of the use cases for this method is to detect collisions between
    /// dynamically allocated addresses and reserved addresses. When the new
    /// address is assigned to a client, the allocation mechanism should check
    /// if this address is not reserved for some other host and do not allocate
    /// this address if reservation is present.
    ///
    /// Implementations of this method should guard against invalid addresses,
    /// such as IPv6 address.
    ///
    /// @param subnet_id Subnet identifier.
    /// @param address reserved IPv4 address.
    ///
    /// @return Const @c Host object using a specified IPv4 address.
    virtual ConstHostPtr
    get4(const SubnetID& subnet_id,
         const asiolink::IOAddress& address) const = 0;

    /// @brief Returns a host connected to the IPv6 subnet.
    ///
    /// @param subnet_id Subnet identifier.
    /// @param identifier_type Identifier type.
    /// @param identifier_begin Pointer to a beginning of a buffer containing
    /// an identifier.
    /// @param identifier_len Identifier length.
    ///
    /// @return Const @c Host object for which reservation has been made using
    /// the specified identifier.
    virtual ConstHostPtr
    get6(const SubnetID& subnet_id,
         const Host::IdentifierType& identifier_type,
         const uint8_t* identifier_begin,
         const size_t identifier_len) const = 0;

    /// @brief Returns a host using the specified IPv6 prefix.
    ///
    /// @param prefix IPv6 prefix for which the @c Host object is searched.
    /// @param prefix_len IPv6 prefix length.
    ///
    /// @return Const @c Host object using a specified IPv6 prefix.
    virtual ConstHostPtr
    get6(const asiolink::IOAddress& prefix, const uint8_t prefix_len) const = 0;

    /// @brief Returns a host connected to the IPv6 subnet and having
    /// a reservation for a specified IPv6 address or prefix.
    ///
    /// @param subnet_id Subnet identifier.
    /// @param address reserved IPv6 address/prefix.
    ///
    /// @return Const @c Host object using a specified IPv6 address/prefix.
    virtual ConstHostPtr
    get6(const SubnetID& subnet_id, const asiolink::IOAddress& address) const = 0;

    /// @brief Adds a new host to the collection.
    ///
    /// The implementations of this method should guard against duplicate
    /// reservations for the same host, where possible. For example, when the
    /// reservation for the same HW address and subnet id is added twice, the
    /// implementation should throw an exception. Note, that usually it is
    /// impossible to guard against adding duplicated host, where one instance
    /// is identified by HW address, another one by DUID.
    ///
    /// @param host Pointer to the new @c Host object being added.
    virtual void add(const HostPtr& host) = 0;

    /// @brief Attempts to delete a host by (subnet-id, address)
    ///
    /// This method supports both v4 and v6.
    ///
    /// @param subnet_id subnet identifier.
    /// @param addr specified address.
    /// @return true if deletion was successful, false if the host was not there.
    /// @throw various exceptions in case of errors
    virtual bool del(const SubnetID& subnet_id, const asiolink::IOAddress& addr) = 0;

    /// @brief Attempts to delete a host by (subnet-id4, identifier, identifier-type)
    ///
    /// This method supports v4 hosts only.
    ///
    /// @param subnet_id IPv4 Subnet identifier.
    /// @param identifier_type Identifier type.
    /// @param identifier_begin Pointer to a beginning of a buffer containing
    /// an identifier.
    /// @param identifier_len Identifier length.
    /// @return true if deletion was successful, false if the host was not there.
    /// @throw various exceptions in case of errors
    virtual bool del4(const SubnetID& subnet_id,
                      const Host::IdentifierType& identifier_type,
                      const uint8_t* identifier_begin, const size_t identifier_len) = 0;

    /// @brief Attempts to delete a host by (subnet-id6, identifier, identifier-type)
    ///
    /// This method supports v6 hosts only.
    ///
    /// @param subnet_id IPv6 Subnet identifier.
    /// @param identifier_type Identifier type.
    /// @param identifier_begin Pointer to a beginning of a buffer containing
    /// an identifier.
    /// @param identifier_len Identifier length.
    /// @return true if deletion was successful, false if the host was not there.
    /// @throw various exceptions in case of errors
    virtual bool del6(const SubnetID& subnet_id,
                      const Host::IdentifierType& identifier_type,
                      const uint8_t* identifier_begin, const size_t identifier_len) = 0;

    /// @brief Return backend type
    ///
    /// Returns the type of the backend (e.g. "mysql", "memfile" etc.)
    ///
    /// @return Type of the backend.
    virtual std::string getType() const = 0;

    /// @brief Commit Transactions
    ///
    /// Commits all pending database operations.  On databases that don't
    /// support transactions, this is a no-op.
    virtual void commit() {};

    /// @brief Rollback Transactions
    ///
    /// Rolls back all pending database operations.  On databases that don't
    /// support transactions, this is a no-op.
    virtual void rollback() {};

    /// @brief Sync database tables with kea.conf
    ///
    /// Truncates database tables, retrieves reservations read from kea.conf and
    /// inserts corresponding hosts, reservations and options in the database.
    virtual void syncReservations() {};
};

/// @brief HostDataSource pointer
typedef std::shared_ptr<BaseHostDataSource> HostDataSourcePtr;

/// @brief HostDataSource list
typedef std::vector<HostDataSourcePtr> HostDataSourceList;

}  // namespace dhcp
}  // namespace isc

#endif // BASE_HOST_DATA_SOURCE_H

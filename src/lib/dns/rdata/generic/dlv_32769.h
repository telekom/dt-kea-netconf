// Copyright (C) 2011-2015 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// BEGIN_HEADER_GUARD

#include <stdint.h>

#include <string>

#include <dns/name.h>
#include <dns/rrtype.h>
#include <dns/rrttl.h>
#include <dns/rdata.h>

// BEGIN_ISC_NAMESPACE

// BEGIN_COMMON_DECLARATIONS
// END_COMMON_DECLARATIONS

// BEGIN_RDATA_NAMESPACE

namespace detail {
template <class Type, uint16_t typeCode> class DSLikeImpl;
}

/// \brief \c rdata::generic::DLV class represents the DLV RDATA as defined in
/// RFC4431.
///
/// This class implements the basic interfaces inherited from the abstract
/// \c rdata::Rdata class, and provides trivial accessors specific to the
/// DLV RDATA.
class DLV : public Rdata {
public:
    // BEGIN_COMMON_MEMBERS
    // END_COMMON_MEMBERS

    /// \brief Assignment operator.
    ///
    /// It internally allocates a resource, and if it fails a corresponding
    /// standard exception will be thrown.
    /// This operator never throws an exception otherwise.
    ///
    /// This operator provides the strong exception guarantee: When an
    /// exception is thrown the content of the assignment target will be
    /// intact.
    DLV& operator=(const DLV& source);

    /// \brief The destructor.
    ~DLV();

    /// \brief Return the value of the Tag field.
    ///
    /// This method never throws an exception.
    uint16_t getTag() const;
private:
    typedef detail::DSLikeImpl<DLV, 32769> DLVImpl;
    DLVImpl* impl_;
};

// END_RDATA_NAMESPACE
// END_ISC_NAMESPACE
// END_HEADER_GUARD

// Local Variables:
// mode: c++
// End:

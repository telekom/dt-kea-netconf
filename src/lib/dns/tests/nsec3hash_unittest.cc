// Copyright (C) 2012-2015 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <string>

#include <gtest/gtest.h>


#include <dns/nsec3hash.h>
#include <dns/labelsequence.h>
#include <dns/rdataclass.h>
#include <util/encode/hex.h>

using std::unique_ptr;
using namespace std;
using namespace isc::dns;
using namespace isc::dns::rdata;
using namespace isc::util;
using namespace isc::util::encode;

namespace {
typedef unique_ptr<NSEC3Hash> NSEC3HashPtr;

// Commonly used NSEC3 suffix, defined to reduce the amount of typing
const char* const nsec3_common = "2T7B4G4VSA5SMI47K61MV5BV1A22BOJR A RRSIG";

class NSEC3HashTest : public ::testing::Test {
protected:
    NSEC3HashTest() :
        test_hash(NSEC3Hash::create(generic::NSEC3PARAM("1 0 12 aabbccdd"))),
        test_hash_nsec3(NSEC3Hash::create(generic::NSEC3
                                          ("1 0 12 aabbccdd " +
                                           string(nsec3_common))))
    {
        const uint8_t salt[] = {0xaa, 0xbb, 0xcc, 0xdd};
        test_hash_args.reset(NSEC3Hash::create(1, 12, salt, sizeof(salt)));
    }

    ~NSEC3HashTest() {
        // Make sure we reset the hash creator to the default
        setNSEC3HashCreator(NULL);
    }

    // An NSEC3Hash object commonly used in tests.  Parameters are borrowed
    // from the RFC5155 example.  Construction of this object implicitly
    // checks a successful case of the creation.
    NSEC3HashPtr test_hash;

    // Similar to test_hash, but created from NSEC3 RR.
    NSEC3HashPtr test_hash_nsec3;

    // Similar to test_hash, but created from passed args.
    NSEC3HashPtr test_hash_args;
};

TEST_F(NSEC3HashTest, unknownAlgorithm) {
    EXPECT_THROW(NSEC3HashPtr(
                     NSEC3Hash::create(
                         generic::NSEC3PARAM("2 0 12 aabbccdd"))),
                     UnknownNSEC3HashAlgorithm);
    EXPECT_THROW(NSEC3HashPtr(
                     NSEC3Hash::create(
                         generic::NSEC3("2 0 12 aabbccdd " +
                                        string(nsec3_common)))),
                     UnknownNSEC3HashAlgorithm);

    const uint8_t salt[] = {0xaa, 0xbb, 0xcc, 0xdd};
    EXPECT_THROW(NSEC3HashPtr(NSEC3Hash::create(2, 12, salt, sizeof(salt))),
                 UnknownNSEC3HashAlgorithm);
}

// Common checks for NSEC3 hash calculation
void
calculateCheck(NSEC3Hash& hash) {
    // A couple of normal cases from the RFC5155 example.
    EXPECT_EQ("0P9MHAVEQVM6T7VBL5LOP2U3T2RP3TOM",
              hash.calculate(Name("example")));
    EXPECT_EQ("35MTHGPGCU1QG68FAB165KLNSNK3DPVL",
              hash.calculate(Name("a.example")));

    // Check case-insensitiveness
    EXPECT_EQ("0P9MHAVEQVM6T7VBL5LOP2U3T2RP3TOM",
              hash.calculate(Name("EXAMPLE")));

    // Repeat for the LabelSequence variant.

    // A couple of normal cases from the RFC5155 example.
    EXPECT_EQ("0P9MHAVEQVM6T7VBL5LOP2U3T2RP3TOM",
              hash.calculate(LabelSequence(Name("example"))));
    EXPECT_EQ("35MTHGPGCU1QG68FAB165KLNSNK3DPVL",
              hash.calculate(LabelSequence(Name("a.example"))));

    // Check case-insensitiveness
    EXPECT_EQ("0P9MHAVEQVM6T7VBL5LOP2U3T2RP3TOM",
              hash.calculate(LabelSequence(Name("EXAMPLE"))));
}

TEST_F(NSEC3HashTest, calculate) {
    {
        SCOPED_TRACE("calculate check with NSEC3PARAM based hash");
        calculateCheck(*test_hash);
    }
    {
        SCOPED_TRACE("calculate check with NSEC3 based hash");
        calculateCheck(*test_hash_nsec3);
    }
    {
        SCOPED_TRACE("calculate check with args based hash");
        calculateCheck(*test_hash_args);
    }

    // Some boundary cases: 0-iteration and empty salt.  Borrowed from the
    // .com zone data.
    EXPECT_EQ("CK0POJMG874LJREF7EFN8430QVIT8BSM",
              NSEC3HashPtr(NSEC3Hash::create(generic::NSEC3PARAM("1 0 0 -")))
              ->calculate(Name("com")));
    EXPECT_EQ("CK0POJMG874LJREF7EFN8430QVIT8BSM",
              NSEC3HashPtr(NSEC3Hash::create(generic::NSEC3PARAM("1 0 0 -")))
              ->calculate(LabelSequence(Name("com"))));

    // Using unusually large iterations, something larger than the 8-bit range.
    // (expected hash value generated by BIND 9's dnssec-signzone)
    EXPECT_EQ("COG6A52MJ96MNMV3QUCAGGCO0RHCC2Q3",
              NSEC3HashPtr(NSEC3Hash::create(
                               generic::NSEC3PARAM("1 0 256 AABBCCDD")))
              ->calculate(LabelSequence(Name("example.org"))));
}

// Common checks for match cases
template <typename RDATAType>
void
matchCheck(NSEC3Hash& hash, const string& postfix) {
    // If all parameters match, it's considered to be matched.
    EXPECT_TRUE(hash.match(RDATAType("1 0 12 aabbccdd" + postfix)));

    // Algorithm doesn't match
    EXPECT_FALSE(hash.match(RDATAType("2 0 12 aabbccdd" + postfix)));
    // Iterations doesn't match
    EXPECT_FALSE(hash.match(RDATAType("1 0 1 aabbccdd" + postfix)));
    // Salt doesn't match
    EXPECT_FALSE(hash.match(RDATAType("1 0 12 aabbccde" + postfix)));
    // Salt doesn't match: the other has an empty salt
    EXPECT_FALSE(hash.match(RDATAType("1 0 12 -" + postfix)));
    // Flags don't matter
    EXPECT_TRUE(hash.match(RDATAType("1 1 12 aabbccdd" + postfix)));
}

TEST_F(NSEC3HashTest, matchWithNSEC3) {
    {
        SCOPED_TRACE("match NSEC3PARAM based hash against NSEC3 parameters");
        matchCheck<generic::NSEC3>(*test_hash, " " + string(nsec3_common));
    }
    {
        SCOPED_TRACE("match NSEC3 based hash against NSEC3 parameters");
        matchCheck<generic::NSEC3>(*test_hash_nsec3,
                                   " " + string(nsec3_common));
    }
}

TEST_F(NSEC3HashTest, matchWithNSEC3PARAM) {
    {
        SCOPED_TRACE("match NSEC3PARAM based hash against NSEC3 parameters");
        matchCheck<generic::NSEC3PARAM>(*test_hash, "");
    }
    {
        SCOPED_TRACE("match NSEC3 based hash against NSEC3 parameters");
        matchCheck<generic::NSEC3PARAM>(*test_hash_nsec3, "");
    }
}

// A simple faked hash calculator and a dedicated creator for it.
class TestNSEC3Hash : public NSEC3Hash {
    virtual string calculate(const Name&) const {
        return ("00000000000000000000000000000000");
    }
    virtual string calculate(const LabelSequence&) const {
        return ("00000000000000000000000000000000");
    }
    virtual bool match(const generic::NSEC3PARAM&) const {
        return (true);
    }
    virtual bool match(const generic::NSEC3&) const {
        return (true);
    }
};

// This faked creator basically creates the faked calculator regardless of
// the passed NSEC3PARAM or NSEC3.  But if the most significant bit of flags
// is set, it will behave like the default creator.
class TestNSEC3HashCreator : public NSEC3HashCreator {
public:
    virtual NSEC3Hash* create(const generic::NSEC3PARAM& param) const {
        if ((param.getFlags() & 0x80) != 0) {
            return (default_creator_.create(param));
        }
        return (new TestNSEC3Hash);
    }
    virtual NSEC3Hash* create(const generic::NSEC3& nsec3) const {
        if ((nsec3.getFlags() & 0x80) != 0) {
            return (default_creator_.create(nsec3));
        }
        return (new TestNSEC3Hash);
    }
    virtual NSEC3Hash* create(uint8_t, uint16_t,
                              const uint8_t*, size_t) const {
        isc_throw(isc::Unexpected,
                  "This method is not implemented here.");
    }
private:
    DefaultNSEC3HashCreator default_creator_;
};

TEST_F(NSEC3HashTest, setCreator) {
    // Re-check an existing case using the default creator/hash implementation
    EXPECT_EQ("0P9MHAVEQVM6T7VBL5LOP2U3T2RP3TOM",
              test_hash->calculate(Name("example")));
    EXPECT_EQ("0P9MHAVEQVM6T7VBL5LOP2U3T2RP3TOM",
              test_hash->calculate(LabelSequence(Name("example"))));

    // Replace the creator, and confirm the hash values are faked
    TestNSEC3HashCreator test_creator;
    setNSEC3HashCreator(&test_creator);
    // Re-create the hash object with the new creator
    test_hash.reset(NSEC3Hash::create(generic::NSEC3PARAM("1 0 12 aabbccdd")));
    EXPECT_EQ("00000000000000000000000000000000",
              test_hash->calculate(Name("example")));
    EXPECT_EQ("00000000000000000000000000000000",
              test_hash->calculate(LabelSequence(Name("example"))));
    // Same for hash from NSEC3 RDATA
    test_hash.reset(NSEC3Hash::create(generic::NSEC3
                                      ("1 0 12 aabbccdd " +
                                       string(nsec3_common))));
    EXPECT_EQ("00000000000000000000000000000000",
              test_hash->calculate(Name("example")));
    EXPECT_EQ("00000000000000000000000000000000",
              test_hash->calculate(LabelSequence(Name("example"))));

    // If we set a special flag big (0x80) on creation, it will act like the
    // default creator.
    test_hash.reset(NSEC3Hash::create(generic::NSEC3PARAM(
                                          "1 128 12 aabbccdd")));
    EXPECT_EQ("0P9MHAVEQVM6T7VBL5LOP2U3T2RP3TOM",
              test_hash->calculate(Name("example")));
    EXPECT_EQ("0P9MHAVEQVM6T7VBL5LOP2U3T2RP3TOM",
              test_hash->calculate(LabelSequence(Name("example"))));
    test_hash.reset(NSEC3Hash::create(generic::NSEC3
                                      ("1 128 12 aabbccdd " +
                                       string(nsec3_common))));
    EXPECT_EQ("0P9MHAVEQVM6T7VBL5LOP2U3T2RP3TOM",
              test_hash->calculate(Name("example")));
    EXPECT_EQ("0P9MHAVEQVM6T7VBL5LOP2U3T2RP3TOM",
              test_hash->calculate(LabelSequence(Name("example"))));

    // Reset the creator to default, and confirm that
    setNSEC3HashCreator(NULL);
    test_hash.reset(NSEC3Hash::create(generic::NSEC3PARAM("1 0 12 aabbccdd")));
    EXPECT_EQ("0P9MHAVEQVM6T7VBL5LOP2U3T2RP3TOM",
              test_hash->calculate(Name("example")));
    EXPECT_EQ("0P9MHAVEQVM6T7VBL5LOP2U3T2RP3TOM",
              test_hash->calculate(LabelSequence(Name("example"))));
}

} // end namespace

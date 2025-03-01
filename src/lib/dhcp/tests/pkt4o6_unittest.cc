// Copyright (C) 2015-2016 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <dhcp/dhcp6.h>
#include <dhcp/option.h>
#include <dhcp/pkt4.h>
#include <dhcp/pkt6.h>
#include <dhcp/pkt4o6.h>


#include <gtest/gtest.h>

using namespace isc::dhcp;

namespace {

/// @brief A Fixture class dedicated to testing of the Pkt4o6 class that
///        represents a DHCPv4-over-DHCPv6 packet.
class Pkt4o6Test : public ::testing::Test {
protected:
    Pkt4o6Test() :
        data6_(6, 0),
        pkt6_(new Pkt6(&data6_[0], data6_.size())),
        pkt4_(new Pkt4(DHCPDISCOVER, 0x12345678))
    {
        pkt4_->pack();
        const uint8_t* cp = static_cast<const uint8_t*>(
            pkt4_->getBuffer().getData());
        buffer4_.assign(cp, cp + pkt4_->getBuffer().getLength());
    }

protected:
    // commonly used test data
    const std::vector<uint8_t> data6_; // data for Pkt6 (content unimportant)
    Pkt6Ptr pkt6_;                     // DHCPv6 message for 4o6
    Pkt4Ptr pkt4_;                     // DHCPv4 message for 4o6
    OptionBuffer buffer4_;             // wire-format data buffer of pkt4_
};

// This test verifies that the constructors are working as expected.
TEST_F(Pkt4o6Test, construct) {
    // Construct 4o6 packet, unpack the data to examine it
    std::unique_ptr<Pkt4o6> pkt4o6(new Pkt4o6(buffer4_, pkt6_));
    pkt4o6->unpack();
    // Inspect its internal to confirm it's built as expected.  We also test
    // isDhcp4o6() here.
    EXPECT_TRUE(pkt4o6->isDhcp4o6());
    EXPECT_EQ(pkt6_, pkt4o6->getPkt6());
    EXPECT_EQ(DHCPDISCOVER, pkt4o6->getType());

    // Same check for the other constructor.  It relies on the internal
    // behavior of Pkt4's copy constructor, so we need to first unpack pkt4.
    pkt4_.reset(new Pkt4(&buffer4_[0], buffer4_.size()));
    pkt4_->unpack();
    pkt4o6.reset(new Pkt4o6(pkt4_, pkt6_));
    EXPECT_TRUE(pkt4o6->isDhcp4o6());
    EXPECT_EQ(pkt6_, pkt4o6->getPkt6());
    EXPECT_EQ(DHCPDISCOVER, pkt4o6->getType());
}

// This test verifies that the pack() method handles the building
// process correctly.
TEST_F(Pkt4o6Test, pack) {
    // prepare unpacked DHCPv4 packet (see the note in constructor test)
    pkt4_.reset(new Pkt4(&buffer4_[0], buffer4_.size()));
    pkt4_->unpack();

    // Construct 4o6 packet to be tested and pack the data.
    Pkt4o6 pkt4o6(pkt4_, pkt6_);
    pkt4o6.pack();

    // The packed data should be:
    // 4-byte DHCPv6 message header
    // 4-byte header part of DHCPv4 message option
    // Raw DHCPv4 message (data stored in buffer4_)
    EXPECT_EQ(4 + 4 + buffer4_.size(),
              pkt4o6.getPkt6()->getBuffer().getLength());

    // Check the DHCPv4 message option content (Pkt4o6 class is not responsible
    // for making it valid, so we won't examine it)
    const uint8_t* cp = static_cast<const uint8_t*>(
        pkt4o6.getPkt6()->getBuffer().getData());
    EXPECT_EQ(0, cp[4]);
    EXPECT_EQ(D6O_DHCPV4_MSG, cp[5]);
    EXPECT_EQ((buffer4_.size() >> 8) & 0xff, cp[6]);
    EXPECT_EQ(buffer4_.size() & 0xff, cp[7]);
    EXPECT_EQ(0, memcmp(&cp[8], &buffer4_[0], buffer4_.size()));
}

// This test verifies that the flag indicating that the retrieved options
// should be copied is transferred between the DHCPv4 packet and the
// DHCPv6 packet being a member of Pkt4o6 class.
TEST_F(Pkt4o6Test, setCopyRetrievedOptions) {
    // Create Pkt4o6 and initially expect that the flag is set to false.
    Pkt4o6 pkt4o6(pkt4_, pkt6_);
    ASSERT_FALSE(pkt4o6.isCopyRetrievedOptions());
    Pkt6Ptr pkt6 = pkt4o6.getPkt6();
    ASSERT_TRUE(pkt6);
    ASSERT_FALSE(pkt6->isCopyRetrievedOptions());

    // Set the flag to true for Pkt4o6.
    pkt4o6.setCopyRetrievedOptions(true);
    pkt6 = pkt4o6.getPkt6();
    ASSERT_TRUE(pkt6);
    EXPECT_TRUE(pkt6->isCopyRetrievedOptions());

    // Repeat the same test but set the flag to false.
    pkt4o6.setCopyRetrievedOptions(false);
    EXPECT_FALSE(pkt4o6.isCopyRetrievedOptions());
    pkt6 = pkt4o6.getPkt6();
    ASSERT_TRUE(pkt6);
    EXPECT_FALSE(pkt6->isCopyRetrievedOptions());
}

/// @todo: Add a test that handles actual DHCP4o6 traffic capture
///        once we get it. We should add the capture to pkt_captures{4,6}.cc
}

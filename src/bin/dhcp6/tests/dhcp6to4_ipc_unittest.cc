// Copyright (C) 2015-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <gtest/gtest.h>
#include <stdint.h>

#include <asiolink/io_address.h>
#include <dhcp/iface_mgr.h>
#include <dhcp/pkt6.h>
#include <dhcp/tests/iface_mgr_test_config.h>
#include <dhcp/tests/pkt_filter6_test_stub.h>
#include <dhcp6/dhcp6to4_ipc.h>
#include <dhcpsrv/cfgmgr.h>
#include <dhcpsrv/testutils/dhcp4o6_test_ipc.h>
#include <hooks/callout_handle.h>
#include <hooks/hooks_manager.h>
#include <hooks/library_handle.h>
#include <stats/stats_mgr.h>

#include <boost/dynamic_bitset.hpp>

using namespace isc;
using namespace isc::asiolink;
using namespace isc::dhcp;
using namespace isc::dhcp::test;
using namespace isc::stats;
using namespace isc::hooks;
using namespace isc::util;

namespace {

/// @brief Port number used in tests.
const uint16_t TEST_PORT = 32000;

/// @brief Define short name for the test IPC.
typedef Dhcp4o6TestIpc TestIpc;

/// @brief Test fixture class for DHCPv4 endpoint of DHCPv4o6 IPC.
class Dhcp6to4IpcTest : public ::testing::Test {
public:
    /// @brief Constructor
    ///
    /// Configures IPC to use a test port. It also provides a fake
    /// configuration of interfaces and opens IPv6 sockets.
    Dhcp6to4IpcTest()
        : ipc_(Dhcp6to4Ipc::instance()),
          srcIpc_(TEST_PORT, TestIpc::ENDPOINT_TYPE_V4),
          mgr_(StatsMgr::instance()), iface_mgr_test_config_(true) {
        IfaceMgr::instance().openSockets6();
        configurePort(TEST_PORT);
        // Install buffer6_send_callout
        EXPECT_NO_THROW(
            HooksManager::preCalloutsLibraryHandle().registerCallout(
                "buffer6_send", buffer6_send_callout));
        // Let's wipe all existing statistics.
        StatsMgr::instance().removeAll();

        // Reset the flag which we expect to be set in the callout.
        callback_pkt_options_copy_ = false;

        // Open both endpoints.
        EXPECT_NO_THROW(ipc_.open());
        EXPECT_NO_THROW(srcIpc_.open());

        // Get statistics
        pkt6_snd_ = mgr_.getObservation("pkt6-sent");
        d4_resp_ = mgr_.getObservation("pkt6-dhcpv4-response-sent");
        EXPECT_FALSE(pkt6_snd_);
        EXPECT_FALSE(d4_resp_);
    }

    /// @brief Destructor
    ///
    /// Various cleanups.
    virtual ~Dhcp6to4IpcTest() {
        HooksManager::preCalloutsLibraryHandle().deregisterAllCallouts("buffer6_send");
        callback_pkt_.reset();
        bool status = HooksManager::unloadLibraries();
        if (!status) {
            std::cerr << "(fixture dtor) unloadLibraries failed" << std::endl;
        }
    }

    /// @brief Configure DHCP4o6 port.
    ///
    /// @param port New port.
    static void configurePort(const uint16_t port);

    /// @brief Creates an instance of the DHCPv4o6 Message option.
    ///
    /// The option will contain an empty DHCPREQUEST message, with
    /// just the Message Type option inside and nothing else.
    ///
    /// @return Pointer to the instance of the DHCPv4-query Message option.
    OptionPtr createDHCPv4MsgOption() const;

    /// @brief Creates an instance of the DHCPv4o6 SADDR option.
    /// @param cipv6-hintlen 8-bit field expressing the bit mask length of the
    ///     IPv6 prefix specified in cipv6-prefix-hint.
    /// @param cipv6-prefix-hint The IPv6 prefix indicating the preferred prefix
    ///     for the client to bind the received IPv4 configuration to.
    ///
    ///     0                   1                   2                   3
    ///     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    ///     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ///     |   OPTION_DHCP4O6_SADDR_HINT   |         option-length         |
    ///     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ///     |cipv6-hintlen  |                                               |
    ///     +-+-+-+-+-+-+-+-+          cipv6-prefix-hint                    .
    ///     .                          (variable length)                    .
    ///     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ///
    /// @return Pointer to the instance of the DHCPv4 SADDR option.
    OptionPtr createSaddrHintOption(const uint8_t cipv6_hintlen,
                                    const IOAddress& cipv6_prefix_hint) const;

    /// @brief Creates an instance of the DHCPv4o6 SADDR option.
    /// @param cipv6-src-address The IPv6 address that the client has bound the
    ///     allocated IPv4 configuration to.
    ///
    ///     0                   1                   2                   3
    ///     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    ///     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ///     |     OPTION_DHCP4O6_SADDR      |         option-length         |
    ///     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ///     |                                                               |
    ///     +                        cipv6-src-address                      +
    ///     .                           (128 bits)                          .
    ///     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    ///
    /// @return Pointer to the instance of the DHCPv4 SADDR option.
    OptionPtr createSaddrOption(const IOAddress& cipv6_prefix_hint) const;

    /// @brief Open and close IPC.
    ///
    /// Used on invalid port error test to close IPC if @ref open() fails to
    /// throw exception.
    static void openAndClose(Dhcp6to4Ipc& ipc);

    /// @brief Handler for the buffer6_send hook
    ///
    /// @param callout_handle handle passed by the hooks framework
    /// @return always 0
    static int buffer6_send_callout(CalloutHandle& callout_handle) {
        callout_handle.getArgument("response6", callback_pkt_);
        if (callback_pkt_) {
            callback_pkt_options_copy_ =
                callback_pkt_->isCopyRetrievedOptions();
        }
        return (0);
    }

    /// @brief Response Pkt6 shared pointer returned in the callout
    static Pkt6Ptr callback_pkt_;

    /// Flag indicating if copying retrieved options was enabled for
    /// a received packet during callout execution.
    static bool callback_pkt_options_copy_;

    /// @ brief DHCP6 server's IPC endpoint
    Dhcp6to4Ipc& ipc_;

    /// @ brief DHCP4 server's IPC endpoint
    TestIpc srcIpc_;

    /// @brief Statistics manager
    StatsMgr& mgr_;

    /// @brief Contains statistics about sent DHCPv6 messages.
    ObservationPtr pkt6_snd_;

    /// @brief Contains statistics about received DHCPv4 messages.
    ObservationPtr d4_resp_;

private:
    /// @brief Provides fake configuration of interfaces.
    IfaceMgrTestConfig iface_mgr_test_config_;
};

Pkt6Ptr Dhcp6to4IpcTest::callback_pkt_;
bool Dhcp6to4IpcTest::callback_pkt_options_copy_;

void
Dhcp6to4IpcTest::configurePort(const uint16_t port) {
    CfgMgr::instance().getStagingCfg()->setDhcp4o6Port(port);
}

OptionPtr
Dhcp6to4IpcTest::createDHCPv4MsgOption() const {
    // Create the DHCPv4 message.
    Pkt4Ptr pkt(new Pkt4(DHCPREQUEST, 1234));
    // Make a wire representation of the DHCPv4 message.
    pkt->pack();
    OutputBuffer& output_buffer = pkt->getBuffer();
    const uint8_t* data = static_cast<const uint8_t*>(output_buffer.getData());
    OptionBuffer option_buffer(data, data + output_buffer.getLength());

    // Create the DHCPv4 Message option holding the created message.
    OptionPtr opt_msg(new Option(Option::V6, D6O_DHCPV4_MSG, option_buffer));
    return (opt_msg);
}

OptionPtr
Dhcp6to4IpcTest::createSaddrHintOption(
    const uint8_t cipv6_hintlen, const IOAddress& cipv6_prefix_hint) const {
    OutputBuffer outputBuffer(0);
    outputBuffer.writeData(&cipv6_hintlen, sizeof(cipv6_hintlen));

    std::vector<uint8_t> cipv6_prefix_hint_vector = cipv6_prefix_hint.toBytes();
    boost::dynamic_bitset<uint8_t> bits(cipv6_prefix_hint_vector.rbegin(),
                                        cipv6_prefix_hint_vector.rend());

    uint8_t lenBytes = (cipv6_hintlen + 7) / 8;
    for (size_t idx = 0; idx < lenBytes; idx++) {
        boost::dynamic_bitset<uint8_t> tmp = bits >> 120;
        uint8_t val = static_cast<uint8_t>(tmp.to_ulong());

        // Zero padded bits follow when cipv6_hintlen is not divisible by 8.
        if (idx == lenBytes - 1) {
            uint8_t unusedBits = 0xFF;
            unusedBits <<= (8 - (cipv6_hintlen % 8)) % 8;
            val = val & unusedBits;
        }
        bits = bits << 8;

        // Write prefix data
        outputBuffer.writeData(&val, sizeof(val));
    }

    const uint8_t* data = static_cast<const uint8_t*>(outputBuffer.getData());
    OptionBuffer optionBuffer(data, data + outputBuffer.getLength());
    OptionPtr saddr(new Option(Option::V6, D60_S46_BIND_IPV6_PREFIX, optionBuffer));
    return saddr;
}

OptionPtr
Dhcp6to4IpcTest::createSaddrOption(const IOAddress& cipv6_src_address) const {
    // Use the entire 128 address as the option buffer.
    const std::vector<uint8_t> optionData_vector = cipv6_src_address.toBytes();
    const uint8_t* optionData = optionData_vector.data();
    const OptionBuffer optionBuffer(
        optionData, optionData + V6ADDRESS_LEN * sizeof(uint8_t));
    OptionPtr saddr(new Option(Option::V6, DHO_S46_SADDR, optionBuffer));
    return saddr;
}

void
Dhcp6to4IpcTest::openAndClose(Dhcp6to4Ipc& ipc) {
    ipc.open();
    ipc.close();
}

// This test verifies that the IPC returns an error when trying to bind
// to the out of range port.
TEST(Dhcp6to4IpcPortTest, invalidPortError) {
    // Create instance of the IPC endpoint under test with out-of-range port.
    Dhcp6to4IpcTest::configurePort(65535);
    Dhcp6to4Ipc& ipc = Dhcp6to4Ipc::instance();
    EXPECT_THROW(Dhcp6to4IpcTest::openAndClose(ipc), isc::OutOfRange);
}

// This test verifies that the DHCPv4 endpoint of the DHCPv4o6 IPC can
// receive messages.
TEST_F(Dhcp6to4IpcTest, receive) {
    // Create message to be sent over IPC.
    Pkt6Ptr pkt(new Pkt6(DHCPV6_DHCPV4_RESPONSE, 1234));
    pkt->addOption(createDHCPv4MsgOption());
    pkt->setIface("eth0");
    pkt->setIndex(ETH0_INDEX);
    pkt->setRemoteAddr(IOAddress("2001:db8:1::123"));
    ASSERT_NO_THROW(pkt->pack());

    // Reset the callout cached packet
    Dhcp6to4IpcTest::callback_pkt_.reset();

    // Send and wait up to 1 second to receive it.
    ASSERT_NO_THROW(srcIpc_.send(pkt));
    ASSERT_NO_THROW(IfaceMgr::instance().receive6(1, 0));

    // Make sure that the received packet was configured to return copy of
    // retrieved options within a callout.
    EXPECT_TRUE(callback_pkt_options_copy_);

    // Get the forwarded packet from the callout
    Pkt6Ptr forwarded = Dhcp6to4IpcTest::callback_pkt_;
    ASSERT_TRUE(forwarded);

    // Verify the packet received.
    EXPECT_EQ(DHCP6_CLIENT_PORT, forwarded->getRemotePort());
    EXPECT_EQ(forwarded->getType(), pkt->getType());
    EXPECT_TRUE(forwarded->getOption(D6O_DHCPV4_MSG));
    EXPECT_EQ("eth0", forwarded->getIface());
    EXPECT_EQ(ETH0_INDEX, forwarded->getIndex());
    EXPECT_EQ("2001:db8:1::123", forwarded->getRemoteAddr().toText());
}

// Check that the SADDR options can be packed, sent, received & unpacked.
TEST_F(Dhcp6to4IpcTest, saddrHint) {
    // Create packet to be sent over IPC.
    Pkt6Ptr saddrHintPkt(new Pkt6(DHCPV6_DHCPV4_RESPONSE, 1234));
    saddrHintPkt->setIface("eth0");
    saddrHintPkt->setRemoteAddr(IOAddress("2001:db8:1::123"));

    // Add SADDR_HINT option.
    const uint8_t hintLen = 48;
    const IOAddress saddrHint("2001:db8:1::");
    saddrHintPkt->addOption(createSaddrHintOption(hintLen, saddrHint));

    // Reset the callout cached packet
    Dhcp6to4IpcTest::callback_pkt_.reset();

    // Send and wait up to 1 second to receive it.
    ASSERT_NO_THROW(srcIpc_.send(saddrHintPkt));
    ASSERT_NO_THROW(IfaceMgr::instance().receive6(1, 0));

    // Make sure that the received packet was configured to return copy of
    // retrieved options within a callout.
    EXPECT_TRUE(callback_pkt_options_copy_);

    // Get the forwarded packet from the callout
    Pkt6Ptr forwarded = Dhcp6to4IpcTest::callback_pkt_;
    ASSERT_TRUE(forwarded);

    // Verify the received packet.
    EXPECT_EQ(DHCP6_CLIENT_PORT, forwarded->getRemotePort());
    EXPECT_EQ(forwarded->getType(), saddrHintPkt->getType());
    EXPECT_EQ("eth0", forwarded->getIface());
    EXPECT_EQ("2001:db8:1::123", forwarded->getRemoteAddr().toText());
    OptionPtr option = forwarded->getOption(D60_S46_BIND_IPV6_PREFIX);
    EXPECT_TRUE(option);
    // Check cipv6-hintlen.
    OptionBuffer optionBuffer = option->getData();
    InputBuffer inputBuffer(optionBuffer.data(), optionBuffer.size());
    uint8_t receivedHintLen = inputBuffer.readUint8();
    EXPECT_EQ(receivedHintLen, hintLen);
    // Check cipv6-prefix-hint.
    const size_t hintLenBytes = hintLen / 8;
    uint8_t hint[INET6_ADDRSTRLEN + 1];
    std::memset(hint, '\0', INET6_ADDRSTRLEN + 1);
    inputBuffer.readData(hint, hintLenBytes);
    EXPECT_EQ(IOAddress::fromBytes(AF_INET6, hint), saddrHint);

    /// @todo: check if you got a lease after sending saddrHint.
}

TEST_F(Dhcp6to4IpcTest, saddr) {
    // Create packet to be sent over IPC.
    Pkt6Ptr saddrPkt(new Pkt6(DHCPV6_DHCPV4_RESPONSE, 1234));
    saddrPkt->setIface("eth0");
    saddrPkt->setRemoteAddr(IOAddress("2001:db8:1::123"));

    // Add SADDR option.
    const IOAddress saddr("2001:db8:1::1");
    saddrPkt->addOption(createSaddrOption(saddr));

    ASSERT_NO_THROW(saddrPkt->pack());

    // Reset the callout cached packet
    Dhcp6to4IpcTest::callback_pkt_.reset();

    // Send and wait up to 1 second to receive it.
    ASSERT_NO_THROW(srcIpc_.send(saddrPkt));
    ASSERT_NO_THROW(IfaceMgr::instance().receive6(1, 0));

    // Make sure that the received packet was configured to return copy of
    // retrieved options within a callout.
    EXPECT_TRUE(callback_pkt_options_copy_);

    // Get the forwarded packet from the callout
    Pkt6Ptr forwarded = Dhcp6to4IpcTest::callback_pkt_;
    ASSERT_TRUE(forwarded);

    // Verify the received packet.
    EXPECT_EQ(DHCP6_CLIENT_PORT, forwarded->getRemotePort());
    EXPECT_EQ(forwarded->getType(), saddrPkt->getType());
    EXPECT_EQ("eth0", forwarded->getIface());
    EXPECT_EQ("2001:db8:1::123", forwarded->getRemoteAddr().toText());
    OptionPtr option = forwarded->getOption(DHO_S46_SADDR);
    EXPECT_TRUE(option);
    EXPECT_EQ(IOAddress::fromBytes(AF_INET6, option->getData().data()), saddr);
}

// This test verifies that the DHCPv4 endpoint of the DHCPv4o6 IPC can
// receive relayed messages.
// This is currently not supported: it is a known defect addressed by #4296.
TEST_F(Dhcp6to4IpcTest, DISABLED_receiveRelayed) {
    // Create relayed message to be sent over IPC.
    Pkt6Ptr pkt(new Pkt6(DHCPV6_DHCPV4_RESPONSE, 1234));
    pkt->addOption(createDHCPv4MsgOption());
    Pkt6::RelayInfo relay;
    relay.linkaddr_ = IOAddress("3000:1::1");
    relay.peeraddr_ = IOAddress("fe80::1");
    relay.msg_type_ = DHCPV6_RELAY_FORW;
    relay.hop_count_ = 1;
    pkt->relay_info_.push_back(relay);
    pkt->setIface("eth0");
    pkt->setIndex(ETH0_INDEX);
    pkt->setRemoteAddr(IOAddress("2001:db8:1::123"));
    ASSERT_NO_THROW(pkt->pack());

    // Reset the callout cached packet
    Dhcp6to4IpcTest::callback_pkt_.reset();

    // Send and wait up to 1 second to receive it.
    ASSERT_NO_THROW(srcIpc_.send(pkt));
    ASSERT_NO_THROW(IfaceMgr::instance().receive6(1, 0));

    // Get the forwarded packet from the callout
    Pkt6Ptr forwarded = Dhcp6to4IpcTest::callback_pkt_;
    ASSERT_TRUE(forwarded);

    // Verify the packet received.
    EXPECT_EQ(DHCP6_CLIENT_PORT, forwarded->getRemotePort());
    EXPECT_EQ(forwarded->getType(), pkt->getType());
    EXPECT_TRUE(forwarded->getOption(D6O_DHCPV4_MSG));
    EXPECT_EQ("eth0", forwarded->getIface());
    EXPECT_EQ(ETH0_INDEX, forwarded->getIndex());
    EXPECT_EQ("2001:db8:1::123", forwarded->getRemoteAddr().toText());
    EXPECT_EQ(DHCP6_CLIENT_PORT, forwarded->getRemotePort());
}

// This test verifies the client port is enforced also with DHCP4o6.
TEST_F(Dhcp6to4IpcTest, clientPort) {
    // Set the client port.
    ipc_.client_port = 1234;

    // Create message to be sent over IPC.
    Pkt6Ptr pkt(new Pkt6(DHCPV6_DHCPV4_RESPONSE, 1234));
    pkt->addOption(createDHCPv4MsgOption());
    pkt->setIface("eth0");
    pkt->setIndex(ETH0_INDEX);
    pkt->setRemoteAddr(IOAddress("2001:db8:1::123"));
    ASSERT_NO_THROW(pkt->pack());

    // Reset the callout cached packet
    Dhcp6to4IpcTest::callback_pkt_.reset();

    // Send and wait up to 1 second to receive it.
    ASSERT_NO_THROW(srcIpc_.send(pkt));
    ASSERT_NO_THROW(IfaceMgr::instance().receive6(1, 0));

    // Make sure that the received packet was configured to return copy of
    // retrieved options within a callout.
    EXPECT_TRUE(callback_pkt_options_copy_);

    // Get the forwarded packet from the callout
    Pkt6Ptr forwarded = Dhcp6to4IpcTest::callback_pkt_;
    ASSERT_TRUE(forwarded);

    // Verify the packet received.
    EXPECT_EQ(ipc_.client_port, forwarded->getRemotePort());
}

}  // namespace

// Copyright (C) 2013-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>
#include <dhcp/iface_mgr.h>
#include <dhcp/pkt4.h>
#include <dhcp/pkt_filter_inet.h>
#include <errno.h>
#include <cstring>
#include <fcntl.h>

using namespace isc::asiolink;

namespace isc {
namespace dhcp {

const size_t
PktFilterInet::CONTROL_BUF_LEN = CMSG_SPACE(sizeof(struct in6_pktinfo));

SocketInfo
PktFilterInet::openSocket(Iface& iface,
                          const isc::asiolink::IOAddress& addr,
                          const uint16_t port,
                          const bool receive_bcast,
                          const bool send_bcast) {
    struct sockaddr_in addr4;
    memset(&addr4, 0, sizeof(sockaddr));
    addr4.sin_family = AF_INET;
    addr4.sin_port = htons(port);

    // If we are to receive broadcast messages we have to bind
    // to "ANY" address.
    if (receive_bcast && iface.flag_broadcast_) {
        addr4.sin_addr.s_addr = INADDR_ANY;
    } else {
        addr4.sin_addr.s_addr = htonl(addr.toUint32());
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        isc_throw(SocketConfigError, "Failed to create UDP4 socket.");
    }

    // Set the close-on-exec flag.
    if (fcntl(sock, F_SETFD, FD_CLOEXEC) < 0) {
        close(sock);
        isc_throw(SocketConfigError, "Failed to set close-on-exec flag"
                  << " on socket " << sock);
    }

#ifdef SO_BINDTODEVICE
    if (receive_bcast && iface.flag_broadcast_) {
        // Bind to device so as we receive traffic on a specific interface.
        if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, iface.getName().c_str(),
                       iface.getName().length() + 1) < 0) {
            close(sock);
            isc_throw(SocketConfigError, "Failed to set SO_BINDTODEVICE option"
                      << " on socket " << sock);
        }
    }
#endif

    if (send_bcast && iface.flag_broadcast_) {
        // Enable sending to broadcast address.
        int flag = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag)) < 0) {
            close(sock);
            isc_throw(SocketConfigError, "Failed to set SO_BROADCAST option"
                      << " on socket " << sock);
        }
    }

    if (bind(sock, (struct sockaddr *)&addr4, sizeof(addr4)) < 0) {
        close(sock);
        isc_throw(SocketConfigError, "Failed to bind socket " << sock
                  << " to " << addr
                  << "/port=" << port);
    }

    // On Linux systems IP_PKTINFO socket option is supported. This
    // option is used to retrieve destination address of the packet.
#if defined (IP_PKTINFO) && defined (OS_LINUX)
    int flag = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &flag, sizeof(flag)) != 0) {
        close(sock);
        isc_throw(SocketConfigError, "setsockopt: IP_PKTINFO: failed.");
    }

    // On BSD systems IP_RECVDSTADDR is used instead of IP_PKTINFO.
#elif defined (IP_RECVDSTADDR) && defined (OS_BSD)
    int flag = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_RECVDSTADDR, &flag, sizeof(flag)) != 0) {
        close(sock);
        isc_throw(SocketConfigError, "setsockopt: IP_RECVDSTADDR: failed.");
    }
#endif

    SocketInfo sock_desc(addr, port, sock);
    return (sock_desc);

}

Pkt4Ptr
PktFilterInet::receive(Iface& iface, const SocketInfo& socket_info) {
    struct sockaddr_in from_addr;
    uint8_t buf[IfaceMgr::RCVBUFSIZE];
    uint8_t control_buf[CONTROL_BUF_LEN];

    memset(&control_buf[0], 0, CONTROL_BUF_LEN);
    memset(&from_addr, 0, sizeof(from_addr));

    // Initialize our message header structure.
    struct msghdr m;
    memset(&m, 0, sizeof(m));

    // Point so we can get the from address.
    m.msg_name = &from_addr;
    m.msg_namelen = sizeof(from_addr);

    struct iovec v;
    v.iov_base = static_cast<void*>(buf);
    v.iov_len = IfaceMgr::RCVBUFSIZE;
    m.msg_iov = &v;
    m.msg_iovlen = 1;

    // Getting the interface is a bit more involved.
    //
    // We set up some space for a "control message". We have
    // previously asked the kernel to give us packet
    // information (when we initialized the interface), so we
    // should get the destination address from that.
    m.msg_control = &control_buf[0];
    m.msg_controllen = CONTROL_BUF_LEN;

    int result = recvmsg(socket_info.sockfd_, &m, 0);
    if (result < 0) {
        isc_throw(SocketReadError, "failed to receive UDP4 data");
    }

    // We have all data let's create Pkt4 object.
    Pkt4Ptr pkt = Pkt4Ptr(new Pkt4(buf, result));

    pkt->updateTimestamp();

    unsigned int ifindex = iface.getIndex();

    IOAddress from(htonl(from_addr.sin_addr.s_addr));
    uint16_t from_port = htons(from_addr.sin_port);

    // Set receiving interface based on information, which socket was used to
    // receive data. OS-specific info (see os_receive4()) may be more reliable,
    // so this value may be overwritten.
    pkt->setIndex(ifindex);
    pkt->setIface(iface.getName());
    pkt->setRemoteAddr(from);
    pkt->setRemotePort(from_port);
    pkt->setLocalPort(socket_info.port_);

// Linux systems support IP_PKTINFO option which is used to retrieve the
// destination address of the received packet. On BSD systems IP_RECVDSTADDR
// is used instead.
#if defined (IP_PKTINFO) && defined (OS_LINUX)
    struct in_pktinfo* pktinfo;
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&m);

    while (cmsg != NULL) {
        if ((cmsg->cmsg_level == IPPROTO_IP) &&
            (cmsg->cmsg_type == IP_PKTINFO)) {
            pktinfo = reinterpret_cast<struct in_pktinfo*>(CMSG_DATA(cmsg));

            pkt->setIndex(pktinfo->ipi_ifindex);
            pkt->setLocalAddr(IOAddress(htonl(pktinfo->ipi_addr.s_addr)));
            break;

            // This field is useful, when we are bound to unicast
            // address e.g. 192.0.2.1 and the packet was sent to
            // broadcast. This will return broadcast address, not
            // the address we are bound to.

            // XXX: Perhaps we should uncomment this:
            // to_addr = pktinfo->ipi_spec_dst;
        }
        cmsg = CMSG_NXTHDR(&m, cmsg);
    }

#elif defined (IP_RECVDSTADDR) && defined (OS_BSD)
    struct in_addr* to_addr;
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&m);

    while (cmsg != NULL) {
        if ((cmsg->cmsg_level == IPPROTO_IP) &&
            (cmsg->cmsg_type == IP_RECVDSTADDR)) {
            to_addr = reinterpret_cast<struct in_addr*>(CMSG_DATA(cmsg));
            pkt->setLocalAddr(IOAddress(htonl(to_addr->s_addr)));
            break;
        }
        cmsg = CMSG_NXTHDR(&m, cmsg);
    }

#endif

    return (pkt);
}

int
PktFilterInet::send(const Iface&, uint16_t sockfd, const Pkt4Ptr& pkt) {
    uint8_t control_buf[CONTROL_BUF_LEN];
    memset(&control_buf[0], 0, CONTROL_BUF_LEN);

    // Set the target address we're sending to.
    sockaddr_in to;
    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_port = htons(pkt->getRemotePort());
    to.sin_addr.s_addr = htonl(pkt->getRemoteAddr().toUint32());

    struct msghdr m;
    // Initialize our message header structure.
    memset(&m, 0, sizeof(m));
    m.msg_name = &to;
    m.msg_namelen = sizeof(to);

    // Set the data buffer we're sending. (Using this wacky
    // "scatter-gather" stuff... we only have a single chunk
    // of data to send, so we declare a single vector entry.)
    struct iovec v;
    memset(&v, 0, sizeof(v));
    // iov_base field is of void * type. We use it for packet
    // transmission, so this buffer will not be modified.
    v.iov_base = const_cast<void *>(pkt->getBuffer().getData());
    v.iov_len = pkt->getBuffer().getLength();
    m.msg_iov = &v;
    m.msg_iovlen = 1;

// In the future the OS-specific code may be abstracted to a different
// file but for now we keep it here because there is no code yet, which
// is specific to non-Linux systems.
#if defined (IP_PKTINFO) && defined (OS_LINUX)
    // Setting the interface is a bit more involved.
    //
    // We have to create a "control message", and set that to
    // define the IPv4 packet information. We set the source address
    // to handle correctly interfaces with multiple addresses.
    m.msg_control = &control_buf[0];
    m.msg_controllen = CONTROL_BUF_LEN;
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&m);

    // FIXME: Code below assumes that cmsg is not NULL, but
    // CMSG_FIRSTHDR() is coded to return NULL as a possibility.  The
    // following assertion should never fail, but if it did and you came
    // here, fix the code. :)
    assert(cmsg != NULL);

    cmsg->cmsg_level = IPPROTO_IP;
    cmsg->cmsg_type = IP_PKTINFO;
    cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
    struct in_pktinfo* pktinfo =(struct in_pktinfo *)CMSG_DATA(cmsg);
    memset(pktinfo, 0, sizeof(struct in_pktinfo));

    // In some cases the index of the outbound interface is not set. This
    // is a matter of configuration. When the server is configured to
    // determine the outbound interface based on routing information,
    // the index is left unset (negative).
    if (pkt->indexSet()) {
        pktinfo->ipi_ifindex = pkt->getIndex();
    }

    // When the DHCP server is using routing to determine the outbound
    // interface, the local address is also left unset.
    if (!pkt->getLocalAddr().isV4Zero()) {
        pktinfo->ipi_spec_dst.s_addr = htonl(pkt->getLocalAddr().toUint32());
    }

    m.msg_controllen = CMSG_SPACE(sizeof(struct in_pktinfo));
#endif

    pkt->updateTimestamp();

    int result = sendmsg(sockfd, &m, 0);
    if (result < 0) {
        isc_throw(SocketWriteError, "pkt4 send failed: sendmsg() returned "
                  " with an error: " << strerror(errno));
    }

    return (0);
}

} // end of isc::dhcp namespace
} // end of isc namespace

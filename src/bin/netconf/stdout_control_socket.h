// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file stdout_control_socket.h
/// Contains declarations for stdout control socket communication.

#ifndef STDOUT_CONTROL_SOCKET_H
#define STDOUT_CONTROL_SOCKET_H

#include <iostream>

#include <cc/command_interpreter.h>
#include <netconf/control_socket.h>

namespace isc {
namespace netconf {

/// @brief Class for control socket communication over stdout.
///
/// This class is the derived class for control socket communication
/// over stdout.
/// This class implements config-test (always OK) and config-set.
template <isc::dhcp::DhcpSpaceType D>
struct StdoutControlSocket : ControlSocketBase<D> {
    /// @brief Constructor.
    ///
    /// Use std::cout.
    ///
    /// @param ctrl_sock The control socket configuration.
    StdoutControlSocket(CfgControlSocketPtr ctrl_sock)
        : ControlSocketBase<D>(ctrl_sock), output_(std::cout) {
    }

    /// @brief Get configuration.
    ///
    /// Call config-get over the control socket.
    ///
    /// @return The JSON element answer of config-get.
    /// @throw NotImplemented
    isc::data::ElementPtr
    configGet([[maybe_unused]] ChangeCollection const& collection,
              [[maybe_unused]] isc::data::ElementPtr const& sysrepo_config) override final {
        isc_throw(NotImplemented, "No config-get for stdout control socket");
    }

    /// @brief Set configuration.
    ///
    /// Call config-set over the control socket.
    ///
    /// @param config The configuration to set.
    /// @return The JSON element answer of config-set (fixed answer).
    isc::data::ElementPtr
    configSet(isc::data::ElementPtr const& config,
              [[maybe_unused]] isc::data::ElementPtr const& db_config,
              [[maybe_unused]] isc::data::ElementPtr const& sysrepo_config,
              [[maybe_unused]] std::vector<change_result_t> const& change_results) override final {
        prettyPrint(config, output_);
        output_ << std::endl;
        return isc::config::createAnswer();
    }

    /// @brief Test configuration.
    ///
    /// Call config-test over the control socket.
    ///
    /// @param config The configuration to test (ignored).
    /// @return The JSON element answer of config-test (fixed answer).
    isc::data::ElementPtr
    configTest([[maybe_unused]] isc::data::ElementPtr const& config,
               [[maybe_unused]] isc::data::ElementPtr const& db_config,
               [[maybe_unused]] std::vector<change_result_t> const& change_results) override final {
        return isc::config::createAnswer();
    }

    /// @brief Get leases.
    ///
    /// Call leases-get over the control socket.
    ///
    /// @param subnet_id The subnet-id for which leases are returned (ignored).
    /// @param service The target service (ignored).
    /// @return The JSON element answer of leases-get.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr leasesGet(uint32_t /* subnet_id */) override final {
        isc_throw(NotImplemented, "No statistic-get for stdout control socket");
    }

    /// @brief Delete a lease.
    ///
    /// Call delete-lease over the control socket.
    ///
    /// @param type The lease type for the wanted lease (ignored).
    /// @param addr The lease address for the wanted lease (ignored).
    /// @return The JSON element answer of delete-lease.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr deleteLease(uint32_t /*type*/,
                                      std::string const& /*addr*/) override final {
        isc_throw(NotImplemented, "No delete-lease for stdout control socket");
    }

    /// @brief Get statistic.
    ///
    /// Call statistic-get over the control socket.
    ///
    /// @param name Argument provided to the statistic-get command
    /// @return The JSON element answer of statistic-get.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr statisticGet(std::string /*name*/) override final {
        isc_throw(NotImplemented, "No statistic-get for stdout control socket");
    }

    /// @brief Get resource.
    ///
    /// Call resource-get over the control socket.
    ///
    /// @param name Argument provided to the resource-get command
    /// @return The JSON element answer of resource-get.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr resourceGet(std::string /* resource */) override final {
        isc_throw(NotImplemented, "No resource-get for stdout control socket");
    }

protected:
    /// @brief Alternative constructor for tests.
    ///
    /// @param ctrl_sock The control socket configuration.
    /// @param output The output stream.

    StdoutControlSocket(CfgControlSocketPtr ctrl_sock, std::ostream& output)
        : ControlSocketBase<D>(ctrl_sock), output_(output) {
    }

    /// @brief The output stream (std::cout outside tests).
    std::ostream& output_;
};

/// @brief Type definition for the pointer to the @c StdoutControlSocket.
template <isc::dhcp::DhcpSpaceType D>
using StdoutControlSocketPtr = std::shared_ptr<StdoutControlSocket<D>>;

}  // namespace netconf
}  // namespace isc

#endif  // STDOUT_CONTROL_SOCKET_H

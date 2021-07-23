// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file control_socket.h
/// Contains declarations for control socket communication.

#ifndef CONTROL_SOCKET_H
#define CONTROL_SOCKET_H

#include <cc/command_interpreter.h>
#include <dhcpsrv/configuration.h>
#include <exceptions/exceptions.h>
#include <netconf/changes.h>
#include <netconf/netconf_config.h>
#include <util/dhcp.h>
#include <util/filesystem.h>
#include <util/func.h>

namespace isc {
namespace netconf {

using change_result_t = std::tuple<std::string, isc::data::effect_t>;

struct ChangeResult {
    static change_result_t make(std::string const& x, isc::data::effect_t const& y) {
        return std::make_tuple(x, y);
    }
};

struct ControlSocketInterface {
    using master_action_t =
        std::function<std::tuple<int, std::string>(std::string const&, std::string const&)>;
    using shard_action_t = std::function<std::tuple<int, std::string>(
        std::string const&, std::string const&, std::string const&)>;

    virtual ~ControlSocketInterface() = default;

    /// @brief Get configuration.
    ///
    /// Call config-get over the control socket.
    ///
    /// @return The JSON element answer of config-get.
    /// @throw ControlSocketError when a communication error occurs.
    virtual isc::data::ElementPtr configGet(ChangeCollection const& collection,
                                            isc::data::ElementPtr const& sysrepo_config) = 0;

    /// @brief Set configuration.
    ///
    /// Call config-set over the control socket.
    ///
    /// @param config The configuration to set.
    /// @return The JSON element answer of config-set.
    /// @throw ControlSocketError when a communication error occurs.
    virtual isc::data::ElementPtr configSet(isc::data::ElementPtr const& config,
                                            isc::data::ElementPtr const& db_config,
                                            isc::data::ElementPtr const& sysrepo_config,
                                            std::vector<change_result_t> const& change_results) = 0;

    /// @brief Test configuration.
    ///
    /// Call config-test over the control socket.
    ///
    /// @param config The configuration to test.
    /// @return The JSON element answer of config-test.
    /// @throw ControlSocketError when a communication error occurs.
    virtual isc::data::ElementPtr
    configTest(isc::data::ElementPtr const& config,
               isc::data::ElementPtr const& db_config,
               std::vector<change_result_t> const& change_results) = 0;

    /// @brief Get leases.
    ///
    /// Call leases-get over the control socket.
    ///
    /// @param subnet_id The subnet-id for which leases are returned
    /// @return The JSON element answer of leases-get.
    /// @throw ControlSocketError when a communication error occurs.
    virtual isc::data::ElementPtr leasesGet(uint32_t subnet_id) = 0;

    /// @brief Delete a lease.
    ///
    /// Call delete-lease over the control socket.
    ///
    /// @param type The lease type for the wanted lease
    /// @param addr The lease address for the wanted lease
    /// @return The JSON element answer of delete-lease.
    /// @throw ControlSocketError when a communication error occurs.
    virtual isc::data::ElementPtr deleteLease(uint32_t type, std::string const& addr) = 0;

    virtual isc::data::ElementPtr sendCommand([
        [maybe_unused]] isc::data::ElementPtr const& command) {
        isc_throw(NotImplemented, PRETTY_METHOD_NAME());
    }

    virtual void prepareForConfigSet(isc::data::ElementPtr const& config,
                                     isc::data::ElementPtr const& db_config,
                                     isc::data::ElementPtr const& sysrepo_config,
                                     std::vector<change_result_t> const& change_results) = 0;

    virtual std::tuple<int, std::string>
    recreateFileHierarchy(isc::data::ElementPtr const& config,
                          [[maybe_unused]] bool const only_master_ini = false) = 0;

    virtual std::tuple<int, std::string>
    recreateFileHierarchy(isc::data::ElementPtr const& config,
                          shard_action_t const& shard_action,
                          master_action_t const& master_action,
                          bool const only_master_ini = false) = 0;

    /// @brief Get statistic.
    ///
    /// Call statistic-get over the control socket.
    ///
    /// @param name Argument provided to the statistic-get command
    /// @return The JSON element answer of statistic-get.
    /// @throw ControlSocketError when a communication error occurs.
    virtual isc::data::ElementPtr statisticGet(std::string name) = 0;

    /// @brief Get resource.
    ///
    /// Call resource-get over the control socket.
    ///
    /// @param name Argument provided to the resource-get command
    /// @return The JSON element answer of resource-get.
    /// @throw ControlSocketError when a communication error occurs.
    virtual isc::data::ElementPtr resourceGet(std::string resource) = 0;
};

/// @brief Base class for control socket communication.
///
/// This class is the base class for control socket communication.
/// Derived classes implement config-get, config-test and config-set
/// using control sockets of different types. Those classes are used to
/// communicate with other Kea daemons.
template <isc::dhcp::DhcpSpaceType D>
struct ControlSocketBase : ControlSocketInterface {
    /// @brief Constructor.
    ///
    /// @param ctrl_sock The control socket configuration.
    /// @throw ControlSocketError if ctrl_sock is null.
    ControlSocketBase(CfgControlSocketPtr ctrl_sock)
        : workspace_(ctrl_sock->getName()), socket_cfg_(ctrl_sock) {
    }

    /// @brief Destructor (does nothing).
    virtual ~ControlSocketBase() = default;

    /// @brief Getter which returns the socket type.
    ///
    /// @return returns the socket type as a CfgControlSocket::Type.
    CfgControlSocket::Type getType() const {
        return socket_cfg_->getType();
    }

    /// @brief Returns the Unix socket name.
    ///
    /// @return returns the Unix socket name as a std::string.
    const std::string getName() const {
        return socket_cfg_->getName();
    }

    /// @brief Returns the HTTP server URL.
    ///
    /// @return returns the HTTP server URL as an isc::http::Url.
    const isc::http::Url getUrl() const {
        return socket_cfg_->getUrl();
    }

    std::tuple<int, isc::data::ElementPtr>
    configGetParsed(ChangeCollection const& collection,
                    isc::data::ElementPtr const& sysrepo_config) {
        isc::data::ElementPtr answer(configGet(collection, sysrepo_config));
        int return_code;
        isc::data::ElementPtr config(isc::config::parseAnswer(return_code, answer));
        return std::make_tuple(return_code, config);
    }

    virtual void
    prepareForConfigSet([[maybe_unused]] isc::data::ElementPtr const& config,
                        [[maybe_unused]] isc::data::ElementPtr const& db_config,
                        [[maybe_unused]] isc::data::ElementPtr const& sysrepo_config,
                        [[maybe_unused]] std::vector<change_result_t> const& change_results) {
    }

    std::tuple<int, std::string> recreateFileHierarchy(isc::data::ElementPtr const& config,
                                                       bool const only_master_ini = false) {
        cleanupWorkspace();
        return recreateFileHierarchy(
            config,
            [&]([[maybe_unused]] std::string const& master_ini,
                [[maybe_unused]] std::string const& master_directory,
                [[maybe_unused]] std::string const& shard_name) -> std::tuple<int, std::string> {
                return {0, ""};
            },
            [&]([[maybe_unused]] std::string const& master_ini,
                [[maybe_unused]] std::string const& master_directory)
                -> std::tuple<int, std::string> {
                return {0, ""};
            },
            only_master_ini);
    }

    std::tuple<int, std::string> recreateFileHierarchy(isc::data::ElementPtr const& config,
                                                       shard_action_t const& shard_action,
                                                       master_action_t const& master_action,
                                                       bool const only_master_ini = false) {
        cleanupWorkspace();
        return isc::dhcp::ConfigurationManager<
            D, isc::dhcp::ConfigurationConstants::NOT_IETF>::createFileHierarchy(workspace_, config,
                                                                                 shard_action,
                                                                                 master_action,
                                                                                 only_master_ini);
    }

    std::string workspace_;

private:
    void cleanupWorkspace() {
        isc::util::Filesystem::removeRecursively(workspace_, [](std::string const& filename) {
            for (std::string const& i : {"config.timestamp", "credentials.json", "master.ini"}) {
                if (filename == i) {
                    return false;
                }
            }
            return true;
        });
    }

    /// @brief The control socket configuration.
    CfgControlSocketPtr socket_cfg_;
};

/// @brief Type definition for the pointer to the @c ControlSocketBase.
template <isc::dhcp::DhcpSpaceType D>
using ControlSocketBasePtr = std::shared_ptr<ControlSocketBase<D>>;

}  // namespace netconf
}  // namespace isc

#endif  // CONTROL_SOCKET_H

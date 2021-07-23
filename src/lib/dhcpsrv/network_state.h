// Copyright (C) 2017 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef NETWORK_STATE_H
#define NETWORK_STATE_H

#include <cc/data.h>
#include <dhcpsrv/subnet_id.h>
#include <set>
#include <string>

namespace isc {
namespace dhcp {

class NetworkStateImpl;

/// @brief Holds information about DHCP service enabling status.
///
/// When the DHCP server receives a command to disable DHCP service entirely
/// or for specific networks, this has to be recorded to allow for re-enabling
/// DHCP service for these networks as a result of receiving a command from
/// the administrator or when the timeout for re-enabling the service occurs.
///
/// In the future, it will be possible to specify "disabled" parameter for
/// a subnet (or network) in the configuration file to indicate that this subnet
/// should be excluded from the service. When a command is subsequently sent to
/// temporarily disable a service for some other subnets for a specified amount
/// ot time, only these subnets should be re-enabled when the time elapses. This
/// class fulfils this requirement by recording the subnets disabled with a command
/// and re-enabling them when required. The subnets specified as "disabled" in
/// the configuration file should remain disabled until explcitly enabled with a
/// control command.
///
/// This class also allows for disabling the DHCP service globally. In this case
/// the server drops all received packets.
///
/// The "dhcp-disable" and "dhcp-enable" commands are used for globally disabling
/// and enabling the DHCP service. The "dhcp-disable-scopes" and "dhcp-enable-scopes"
/// commands are used to disable and enable DHCP service for subnets and networks.
/// In case of the "dhcp-disable" and "dhcp-disable-scopes" commands, it is possible
/// to specify "max-period" parameter which provides a timeout, after which the
/// settings are reverted (service is re-enabled globally and/or for specific
/// scopes).
///
/// Disabling DHCP service with a timeout is useful to guard against issues when
/// the controlling client dies after disabling the DHCP service on the server,
/// e.g. failover peers may instruct each other to disable the DHCP service while
/// database synchronization takes place. If the peer subsequently dies, the
/// surviving server must re-enable DHCP on its own.
///
/// @todo This class currently supports only the case of globally disabling
/// the DHCP service. Disabling per network/subnet will be added later.
class NetworkState {
public:

    /// @brief DHCP server type.
    enum ServerType {
        DHCPv4,
        DHCPv6
    };

    /// @brief Type of the container holding collection of subnet identifiers.
    typedef std::set<SubnetID> Subnets;

    /// @brief Type of the container holding collection of shared network names.
    typedef std::set<std::string> Networks;

    /// @brief Constructor.
    NetworkState(const ServerType& server_type);

    /// @brief Globally disables DHCP service.
    ///
    /// The DHCP service becomes disabled for all subnets and networks,
    /// regardless of the per scope settings.
    void disableService();

    /// @brief Globally enables DHCP service.
    ///
    /// The DHCP service becomes enabled but per scope settings are in effect.
    /// In order to enable the service for all scopes previously disabled with
    /// a control command, use @c enableAll.
    void enableService();

    /// @brief Enables DHCP service globally and for scopes which have been
    /// disabled as a result of control command.
    void enableAll();

    /// @brief Schedules enabling DHCP service in the future.
    ///
    /// @param seconds Number of seconds after which the service should be enabled
    /// unless @c enableAll is enabled before that time.
    void delayedEnableAll(const unsigned int seconds);

    /// @brief Checks if the DHCP service is globally enabled.
    ///
    /// @return true if the service is globally enabled, false otherwise.
    bool isServiceEnabled() const;

    /// @brief Checks if delayed enabling of DHCP services is scheduled.
    ///
    /// It indicates that the timer is present which counts the time until
    /// @c enableAll function will be called automatically.
    ///
    /// @return true if delayed enabling of the DHCP service is scheduled,
    /// false otherwise.
    bool isDelayedEnableAll() const;

    /// @name Selective disabling/enabling DHCP service per scopes
    //@{

    /// @brief Disable DHCP service for selected subnets.
    ///
    /// @param subnets Collection of subnet identifiers for which the service
    /// should be disabled.
    ///
    /// @throw isc::NotImplemented
    void selectiveDisable(const NetworkState::Subnets& subnets);

    /// @brief Disable DHCP service for selected networks.
    ///
    /// @param networks Collection of shared network names for which the service
    /// should be disabled.
    ///
    /// @throw isc::NotImplemented
    void selectiveDisable(const NetworkState::Networks& networks);

    /// @brief Enable DHCP service for selected subnets.
    ///
    /// @param subnets Collection of subnet identifiers for which the service
    /// should be disabled.
    ///
    /// @throw isc::NotImplemented
    void selectiveEnable(const NetworkState::Subnets& subnets);

    /// @brief Enable DHCP service for selected networks.
    ///
    /// @param networks Collection of shared network names for which the service
    /// should be enabled.
    ///
    /// @throw isc::NotImplemented
    void selectiveEnable(const NetworkState::Networks& networks);

    //@}

private:

    /// @brief Pointer to the @c NetworkState implementation.
    std::shared_ptr<NetworkStateImpl> impl_;
};

/// @brief Pointer to the @c NetworkState object.
typedef std::shared_ptr<NetworkState> NetworkStatePtr;

} // end of namespace isc::dhcp
} // end of namespace isc

#endif // NETWORK_STATE_H

// Copyright (C) 2017-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef NETWORK_H
#define NETWORK_H

#include <cc/cfg_to_element.h>
#include <cc/data.h>
#include <cc/element_value.h>
#include <cc/stamped_element.h>
#include <cc/user_context.h>
#include <dhcp/classify.h>
#include <dhcp/option.h>
#include <dhcpsrv/cfg_option.h>
#include <dhcpsrv/cfg_4o6.h>
#include <dhcpsrv/d2_client_cfg.h>
#include <dhcpsrv/triplet.h>
#include <util/optional.h>
#include <cstdint>
#include <functional>
#include <string>

namespace isc {
namespace dhcp {

/// List of IOAddresses
typedef std::vector<isc::asiolink::IOAddress> IOAddressList;

class Network;

/// @brief Pointer to the @ref Network object.
typedef std::shared_ptr<Network> NetworkPtr;

/// @brief Weak pointer to the @ref Network object.
typedef std::weak_ptr<Network> WeakNetworkPtr;

/// @brief Callback function for @c Network that retrieves globally
/// configured parameters.
typedef std::function<data::ElementPtr()> FetchNetworkGlobalsFn;

/// @brief Common interface representing a network to which the DHCP clients
/// are connected.
///
/// The most common type of network, in Kea's terminology, is a subnet. The
/// @ref Subnet derives from this class. Another types of objects implementing
/// this interface are @ref SharedNetwork4 and @ref SharedNetwork6 objects.
/// They group multiple subnets together to provide means for
/// extending available address pools (a single client may obtain IP
/// address from any of the pools belonging to subnets in the shared
/// network), or for selecting a subnet on a given link, depending on the
/// class of the client (e.g. cable network case: different subnet is
/// selected for cable modems, different one for routers).
///
/// The subnets and shared networks share many data structures, e.g. DHCP
/// options, local interface name, address manipulation methods. Both subnets
/// and shared networks derive from this class to provide the common
/// functionality.
///
/// The DHCP server configuration is complex because many parameters may
/// be specified at different levels of hierarchy. The lower level values,
/// e.g. subnet specific values, take precedence over upper level values,
/// e.g. shared network specific ones. For historical reasons, the DHCP
/// servers expect that the appropriate values are inherited from the
/// upper configuration levels to the lower configuration levels upon
/// the reconfiguration. For example: if a user didn't specify
/// valid-lifetime for a subnet, calling @c Subnet4::getValid() should
/// result in returning a global value of valid-lifetime. In the early
/// Kea days it was achieved by the configuration parsers which would
/// explicitly assign the global valid lifetime to the @c Subnet4
/// instances for which the subnet specific value was not provided. This
/// approach has a major benefit that it is fast. However, it makes
/// the subnets tightly dependent on the global values (and shared
/// network specific values). Modification of the global value must
/// result in modification of this value in all subnets for which
/// there is no explicit value provided. This issue became a serious
/// road block during the implementation of the Configuration Backend.
///
/// The @c Network object has been modified to address the problem of
/// inheritance of global, shared network specific and subnet specific
/// parameters in a generic way, at the same time minimizing the need to
/// change the existing server logic.
///
/// The @c Network object now holds the pointer to the "parent" @c Network
/// object. Thus subnets which belong to a shared network will have
/// that shared network as its parent. Stand-alone subnets, will have
/// no parent.
///
/// The general idea is that the accessor functions of the network
/// will first check if the accessed value is specified or not (that
/// is handled by @c util::Optional object). If the value is specified
/// it is returned. Otherwise, the object will check if there is a
/// parent object it belongs to and will call the appropriate method
/// of that object. If the value is present it is returned. Otherwise
/// the global value is returned.
///
/// Accessing global values from the @c Network object is troublesome.
/// There is no uniform way to access those values. For example, the
/// given network may be in a staging or current configuration and
/// it really has no means to know in which of the two it belongs.
/// In fact, an attempt to pass the pointer to the @c SrvConfig object
/// would cause a circular dependency between the @c Network and the
/// @c SrvConfig. Even if it was possible and the @c Network had
/// access to the specific @c SrvConfig instance, it doesn't handle
/// the cases when the @c SrvConfig instance was modified.
///
/// To deal with the problem of accessing the global parameters in a
/// flexible manner, we elected to use an optional callback function
/// which can be associated with the @c Network object. This callback
/// implements the logic to retrieve global parameters and return them
/// in a well known form, so as the @c Network accessors can use them.
class Network : public virtual isc::data::StampedElement,
                public virtual isc::data::UserContext,
                public isc::data::CfgToElement {
public:
    /// @brief Holds optional information about relay.
    ///
    /// In some cases it is beneficial to have additional information about
    /// a relay configured in the subnet. For now, the structure holds only
    /// IP addresses, but there may potentially be additional parameters added
    /// later, e.g. relay interface-id or relay-id.
    class RelayInfo {
    public:

        /// @brief Adds an address to the list of addresses
        ///
        /// @param addr address to add
        /// @throw BadValue if the address is already in the list
        void addAddress(const asiolink::IOAddress& addr);

        /// @brief Returns const reference to the list of addresses
        ///
        /// @return const reference to the list of addresses
        const IOAddressList& getAddresses() const;

        /// @brief Indicates whether or not the address list has entries
        ///
        /// @return True if the address list is not empty
        bool hasAddresses() const;

        /// @brief Checks the address list for the given address
        ///
        /// @return True if the address is found in the address list
        bool containsAddress(const asiolink::IOAddress& addr) const;

    private:
        /// @brief List of relay IP addresses
        IOAddressList addresses_;
    };


    /// @brief Specifies allowed host reservation mode.
    ///
    typedef enum  {

        /// None - host reservation is disabled. No reservation types
        /// are allowed.
        HR_DISABLED,

        /// Only out-of-pool reservations is allowed. This mode
        /// allows AllocEngine to skip reservation checks when
        /// dealing with with addresses that are in pool.
        HR_OUT_OF_POOL,

        /// Only global reservations are allowed. This mode
        /// instructs AllocEngine to only look at global reservations.
        HR_GLOBAL,

        /// Both out-of-pool and in-pool reservations are allowed. This is the
        /// most flexible mode, where sysadmin have biggest liberty. However,
        /// there is a non-trivial performance penalty for it, as the
        /// AllocEngine code has to check whether there are reservations, even
        /// when dealing with reservations from within the dynamic pools.
        /// @todo - should ALL include global?
        HR_ALL
    } HRMode;

    /// @brief Inheritance "mode" used when fetching an optional @c Network
    /// parameter.
    ///
    /// The following modes are currently supported:
    /// - NONE: no inheritance is used, the network specific value is returned
    ///   regardless if it is specified or not.
    /// - PARENT_NETWORK: parent network specific value is returned or unspecified
    ///   if the parent network doesn't exist.
    /// - GLOBAL: global specific value is returned.
    /// - ALL: inheritance is used on all levels: network specific value takes
    ///   precedence over parent specific value over the global value.
    enum class Inheritance {
        NONE,
        PARENT_NETWORK,
        GLOBAL,
        ALL
    };

    /// Pointer to the RelayInfo structure
    typedef std::shared_ptr<Network::RelayInfo> RelayInfoPtr;

    /// @brief Constructor.
    Network()
        : iface_name_(), client_class_(), t1_(), t2_(), valid_(),
          host_reservation_mode_(HR_ALL, true), cfg_option_(new CfgOption()),
          calculate_tee_times_(), t1_percent_(), t2_percent_(),
          ddns_send_updates_(), ddns_override_no_update_(), ddns_override_client_update_(),
          ddns_replace_client_name_mode_(), ddns_generated_prefix_(), ddns_qualifying_suffix_(),
          hostname_char_set_(), hostname_char_replacement_(), store_extended_info_() {
    }

    /// @brief Virtual destructor.
    ///
    /// Does nothing at the moment.
    virtual ~Network() { };

    /// @brief Sets the optional callback function used to fetch globally
    /// configured parameters.
    ///
    /// @param fetch_globals_fn Pointer to the function.
    void setFetchGlobalsFn(FetchNetworkGlobalsFn fetch_globals_fn) {
        fetch_globals_fn_ = fetch_globals_fn;
    }

    /// @brief Checks if the network is associated with a function used to
    /// fetch globally configured parameters.
    ///
    /// @return true if it is associated, false otherwise.
    bool hasFetchGlobalsFn() const {
        return (static_cast<bool>(fetch_globals_fn_));
    }

    /// @brief Sets local name of the interface for which this network is
    /// selected.
    ///
    /// If the interface is specified, the server will use the network
    /// associated with this local interface to allocate IP addresses and
    /// other resources to a client.
    ///
    /// @param iface_name Interface name.
    void setIface(const util::Optional<std::string>& iface_name) {
        iface_name_ = iface_name;
    }

    /// @brief Returns name of the local interface for which this network is
    /// selected.
    ///
    /// @param inheritance inheritance mode to be used.
    /// @return Interface name as text.
    util::Optional<std::string>
    getIface(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getIface, iface_name_,
                                     inheritance, "interface"));
    };

    /// @brief Sets information about relay
    ///
    /// In some situations where there are shared subnets (i.e. two different
    /// subnets are available on the same physical link), there is only one
    /// relay that handles incoming requests from clients. In such a case,
    /// the usual subnet selection criteria based on relay belonging to the
    /// subnet being selected are no longer sufficient and we need to explicitly
    /// specify a relay. One notable example of such uncommon, but valid
    /// scenario is a cable network, where there is only one CMTS (one relay),
    /// but there are 2 distinct subnets behind it: one for cable modems
    /// and another one for CPEs and other user equipment behind modems.
    /// From manageability perspective, it is essential that modems get addresses
    /// from different subnet, so users won't tinker with their modems.
    ///
    /// Setting this parameter is not needed in most deployments.
    /// This structure holds IP address only for now, but it is expected to
    /// be extended in the future.
    ///
    /// @param relay structure that contains relay information
    void setRelayInfo(const RelayInfo& relay) {
        relay_ = relay;
    }

    /// @brief Returns const reference to relay information
    ///
    /// @note The returned reference is only valid as long as the object
    /// returned it is valid.
    ///
    /// @return const reference to the relay information
    const RelayInfo& getRelayInfo() const {
        return (relay_);
    }

    /// @brief Adds an address to the list addresses in the network's relay info
    ///
    /// @param addr address of the relay
    /// @throw BadValue if the address is already in the list
    void addRelayAddress(const asiolink::IOAddress& addr);

    /// @brief Returns the list of relay addresses from the network's relay info
    ///
    /// @return const reference to the list of addresses
    const IOAddressList& getRelayAddresses() const;

    /// @brief Indicates if network's relay info has relay addresses
    ///
    /// @return True the relay list is not empty, false otherwise
    bool hasRelays() const;

    /// @brief Tests if the network's relay info contains the given address
    ///
    /// @param address address to search for in the relay list
    /// @return True if a relay with the given address is found, false otherwise
    bool hasRelayAddress(const asiolink::IOAddress& address) const;

    /// @brief Checks whether this network supports client that belongs to
    /// specified classes.
    ///
    /// This method checks whether a client that belongs to given classes can
    /// use this network. For example, if this class is reserved for client
    /// class "foo" and the client belongs to classes "foo", "bar" and "baz",
    /// it is supported. On the other hand, client belonging to classes
    /// "foobar" and "zyxxy" is not supported.
    ///
    /// @note: changed the planned white and black lists idea to a simple
    /// client class name.
    ///
    /// @param client_classes list of all classes the client belongs to
    /// @return true if client can be supported, false otherwise
    virtual bool
    clientSupported(const isc::dhcp::ClientClasses& client_classes) const;

    /// @brief Sets the supported class to class class_name
    ///
    /// @param class_name client class to be supported by this network
    void allowClientClass(const isc::dhcp::ClientClass& class_name);

    /// @brief Adds class class_name to classes required to be evaluated.
    ///
    /// @param class_name client class required to be evaluated
    void requireClientClass(const isc::dhcp::ClientClass& class_name);

    /// @brief Returns classes which are required to be evaluated
    const ClientClasses& getRequiredClasses() const;

    /// @brief returns the client class
    ///
    /// @note The returned reference is only valid as long as the object
    /// returned it is valid.
    ///
    /// @param inheritance inheritance mode to be used.
    /// @return client class @ref client_class_
    util::Optional<ClientClass>
    getClientClass(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getClientClass, client_class_,
                                     inheritance));
    }

    /// @brief Return valid-lifetime for addresses in that prefix
    ///
    /// @param inheritance inheritance mode to be used.
    Triplet<uint32_t> getValid(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getValid, valid_, inheritance,
                                     "valid-lifetime"));
    }

    /// @brief Sets new valid lifetime for a network.
    ///
    /// @param valid New valid lifetime in seconds.
    void setValid(const Triplet<uint32_t>& valid) {
        valid_ = valid;
    }

    /// @brief Returns T1 (renew timer), expressed in seconds
    ///
    /// @param inheritance inheritance mode to be used.
    Triplet<uint32_t> getT1(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getT1, t1_, inheritance, "renew-timer"));
    }

    /// @brief Sets new renew timer for a network.
    ///
    /// @param t1 New renew timer value in seconds.
    void setT1(const Triplet<uint32_t>& t1) {
        t1_ = t1;
    }

    /// @brief Returns T2 (rebind timer), expressed in seconds
    ///
    /// @param inheritance inheritance mode to be used.
    Triplet<uint32_t> getT2(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getT2, t2_, inheritance, "rebind-timer"));
    }

    /// @brief Sets new rebind timer for a network.
    ///
    /// @param t2 New rebind timer value in seconds.
    void setT2(const Triplet<uint32_t>& t2) {
        t2_ = t2;
    }

    /// @brief Specifies what type of Host Reservations are supported.
    ///
    /// Host reservations may be either in-pool (they reserve an address that
    /// is in the dynamic pool) or out-of-pool (they reserve an address that is
    /// not in the dynamic pool). HR may also be completely disabled for
    /// performance reasons.
    ///
    /// @param inheritance inheritance mode to be used.
    /// @return Host reservation mode enabled.
    util::Optional<HRMode>
    getHostReservationMode(const Inheritance& inheritance = Inheritance::ALL) const {
        // Inheritance for host reservations is a little different than for other
        // parameters. The reservation at the global level is given as a string.
        // Thus we call getProperty here without a global name to check if the
        // host reservation mode is specified on network level only.
        const util::Optional<HRMode>& hr_mode = getProperty<Network>(&Network::getHostReservationMode,
                                                                     host_reservation_mode_,
                                                                     inheritance);
        // If HR mode is not specified at network level we need this special
        // case code to handle conversion of the globally configured HR
        // mode to an enum.
        if (hr_mode.unspecified() && (inheritance != Inheritance::NONE) &&
            (inheritance != Inheritance::PARENT_NETWORK)) {
            // Get global reservation mode.
            util::Optional<std::string> hr_mode_name;
            hr_mode_name = getGlobalProperty(hr_mode_name, "reservation-mode");
            if (!hr_mode_name.unspecified()) {
                try {
                    // If the HR mode is globally configured, let's convert it from
                    // a string to enum.
                    return (stringToHostReservationMode(hr_mode_name.get()));

                } catch (...) {
                    // This should not really happen because the configuration
                    // parser should have already verified the globally configured
                    // reservation mode. However, we want to be 100% sure that this
                    // method doesn't throw. Let's just return unspecified.
                    return (hr_mode);
                }
            }
        }
        return (hr_mode);
    }

    /// @brief Sets host reservation mode.
    ///
    /// See @ref getHostReservationMode for details.
    ///
    /// @param mode mode to be set
    void setHostReservationMode(const util::Optional<HRMode>& mode) {
        host_reservation_mode_ = mode;
    }

    /// @brief Attempts to convert HRMode enum to text representation.
    ///
    /// Allowed values are "disabled", "off", "out-of-pool", "all".
    /// See Network::HRMode for their exact meaning.
    ///
    /// @param value HR Mode enum.
    ///
    /// @throw BadValue if the value cannot be converted.
    ///
    /// @return one of allowed HRMode values in text representation
    static std::string hostReservationModeToString(HRMode value);

    /// @brief Attempts to convert text representation to HRMode enum.
    ///
    /// Allowed values are "disabled", "off" (alias for disabled),
    /// "out-of-pool" and "all". See @c Network::HRMode for their exact meaning.
    ///
    /// @param hr_mode_name Host Reservation mode in the textual form.
    ///
    /// @throw BadValue if the text cannot be converted.
    ///
    /// @return one of allowed HRMode values
    static HRMode stringToHostReservationMode(const std::string& hr_mode_name);

    /// @brief Returns pointer to the option data configuration for this network.
    CfgOptionPtr getCfgOption() {
        return (cfg_option_);
    }

    /// @brief Returns const pointer to the option data configuration for this
    /// network.
    ConstCfgOptionPtr getCfgOption() const {
        return (cfg_option_);
    }

    /// @brief Returns whether or not T1/T2 calculation is enabled.
    ///
    /// @param inheritance inheritance mode to be used.
    util::Optional<bool>
    getCalculateTeeTimes(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getCalculateTeeTimes,
                                     calculate_tee_times_,
                                     inheritance,
                                     "calculate-tee-times"));
    }

    /// @brief Sets whether or not T1/T2 calculation is enabled.
    ///
    /// @param calculate_tee_times new value of enabled/disabled.
    void setCalculateTeeTimes(const util::Optional<bool>& calculate_tee_times) {
        calculate_tee_times_ = calculate_tee_times;
    }

    /// @brief Returns percentage to use when calculating the T1 (renew timer).
    ///
    /// @param inheritance inheritance mode to be used.
    util::Optional<double>
    getT1Percent(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getT1Percent, t1_percent_,
                                     inheritance, "t1-percent"));
    }

    /// @brief Sets new precentage for calculating T1 (renew timer).
    ///
    /// @param t1_percent New percentage to use.
    void setT1Percent(const util::Optional<double>& t1_percent) {
        t1_percent_ = t1_percent;
    }

    /// @brief Returns percentage to use when calculating the T2 (rebind timer).
    ///
    /// @param inheritance inheritance mode to be used.
    util::Optional<double>
    getT2Percent(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getT2Percent, t2_percent_,
                                     inheritance, "t2-percent"));
    }

    /// @brief Sets new precentage for calculating T2 (rebind timer).
    ///
    /// @param t2_percent New percentage to use.
    void setT2Percent(const util::Optional<double>& t2_percent) {
        t2_percent_ = t2_percent;
    }

    /// @brief Returns ddns-send-updates
    ///
    /// @param inheritance inheritance mode to be used.
    util::Optional<bool>
    getDdnsSendUpdates(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getDdnsSendUpdates, ddns_send_updates_,
                                     inheritance, "ddns-send-updates"));
    }

    /// @brief Sets new ddns-send-updates
    ///
    /// @param ddns_send_updates New value to use.
    void setDdnsSendUpdates(const util::Optional<bool>& ddns_send_updates) {
        ddns_send_updates_ = ddns_send_updates;
    }

    /// @brief Returns ddns-override-no-update
    ///
    /// @param inheritance inheritance mode to be used.
    util::Optional<bool>
    getDdnsOverrideNoUpdate(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getDdnsOverrideNoUpdate,
                                     ddns_override_no_update_,
                                     inheritance, "ddns-override-no-update"));
    }

    /// @brief Sets new ddns-override-no-update
    ///
    /// @param ddns_override_no_update New value to use.
    void setDdnsOverrideNoUpdate(const util::Optional<bool>& ddns_override_no_update) {
        ddns_override_no_update_ = ddns_override_no_update;
    }

    /// @brief Returns ddns-overridie-client-update
    ///
    /// @param inheritance inheritance mode to be used.
    util::Optional<bool>
    getDdnsOverrideClientUpdate(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getDdnsOverrideClientUpdate,
                                     ddns_override_client_update_,
                                     inheritance, "ddns-override-client-update"));
    }

    /// @brief Sets new ddns-override-client-update
    ///
    /// @param ddns_override_client_update New value to use.
    void setDdnsOverrideClientUpdate(const util::Optional<bool>&
                                     ddns_override_client_update) {
        ddns_override_client_update_ = ddns_override_client_update;
    }

    /// @brief Returns ddns-replace-client-name-mode
    ///
    /// @param inheritance inheritance mode to be used.
    util::Optional<D2ClientConfig::ReplaceClientNameMode>
    getDdnsReplaceClientNameMode(const Inheritance& inheritance = Inheritance::ALL) const {
        // Inheritance for ddns-replace-client-name is a little different than for other
        // parameters. The value at the global level is given as a string.
        // Thus we call getProperty here without a global name to check if it
        // is specified on network level only.
        const util::Optional<D2ClientConfig::ReplaceClientNameMode>& mode =
            getProperty<Network>(&Network::getDdnsReplaceClientNameMode,
                                 ddns_replace_client_name_mode_, inheritance);

        // If it is not specified at network level we need this special
        // case code to convert the global string value to an enum.
        if (mode.unspecified() && (inheritance != Inheritance::NONE) &&
            (inheritance != Inheritance::PARENT_NETWORK)) {
            // Get global mode.
            util::Optional<std::string> mode_label;
            mode_label = getGlobalProperty(mode_label, "ddns-replace-client-name");
            if (!mode_label.unspecified()) {
                try {
                    // If the mode is globally configured, convert it to an enum.
                    return (D2ClientConfig::stringToReplaceClientNameMode(mode_label.get()));
                } catch (...) {
                    // This should not really happen because the configuration
                    // parser should have already verified the globally configured
                    // reservation mode. However, we want to be 100% sure that this
                    // method doesn't throw. Let's just return unspecified.
                    return (mode);
                }
            }
        }
        return (mode);
    }

    /// @brief Sets new ddns-replace-client-name-mode
    ///
    /// @param ddns_replace_client_name_mode New value to use.
    void
    setDdnsReplaceClientNameMode(const util::Optional<D2ClientConfig::ReplaceClientNameMode>&
                                 ddns_replace_client_name_mode) {
        ddns_replace_client_name_mode_ = ddns_replace_client_name_mode;
    }

    /// @brief Returns ddns-generated-prefix
    ///
    /// @param inheritance inheritance mode to be used.
    util::Optional<std::string>
    getDdnsGeneratedPrefix(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getDdnsGeneratedPrefix,
                                     ddns_generated_prefix_,
                                     inheritance, "ddns-generated-prefix"));
    }

    /// @brief Sets new ddns-generated-prefix
    ///
    /// @param ddns_generated_prefix New value to use.
    void setDdnsGeneratedPrefix(const util::Optional<std::string>& ddns_generated_prefix) {
        ddns_generated_prefix_ = ddns_generated_prefix;
    }

    /// @brief Returns ddns-qualifying-suffix
    ///
    /// @param inheritance inheritance mode to be used.
    util::Optional<std::string>
    getDdnsQualifyingSuffix(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getDdnsQualifyingSuffix,
                                     ddns_qualifying_suffix_,
                                     inheritance, "ddns-qualifying-suffix"));
    }

    /// @brief Sets new ddns-qualifying-suffix
    ///
    /// @param ddns_qualifying_suffix New value to use.
    void setDdnsQualifyingSuffix(const util::Optional<std::string>& ddns_qualifying_suffix) {
        ddns_qualifying_suffix_ = ddns_qualifying_suffix;
    }

    /// @brief Return the char set regexp used to sanitize client hostnames.
    util::Optional<std::string>
    getHostnameCharSet(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getHostnameCharSet, hostname_char_set_,
                                     inheritance, "hostname-char-set"));
    }

    /// @brief Sets new hostname-char-set
    ///
    /// @param hostname_char_set New value to use.
    void setHostnameCharSet(const util::Optional<std::string>& hostname_char_set) {
        hostname_char_set_ = hostname_char_set;
    }

    /// @brief Return the invalid char replacement used to sanitize client hostnames.
    util::Optional<std::string>
    getHostnameCharReplacement(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getHostnameCharReplacement,
                                     hostname_char_replacement_,
                                     inheritance, "hostname-char-replacement"));
    }

    /// @brief Sets new hostname-char-replacement
    ///
    /// @param hostname_char_replacement New value to use.
    void setHostnameCharReplacement(const util::Optional<std::string>&
                                    hostname_char_replacement) {
        hostname_char_replacement_ = hostname_char_replacement;
    }

    /// @brief Returns store-extended-info
    ///
    /// @param inheritance inheritance mode to be used.
    util::Optional<bool>
    getStoreExtendedInfo(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network>(&Network::getStoreExtendedInfo,
                                     store_extended_info_,
                                     inheritance, "store-extended-info"));
    }

    /// @brief Sets new store-extended-info
    ///
    /// @param store_extended_info New value to use.
    void setStoreExtendedInfo(const util::Optional<bool>& store_extended_info) {
        store_extended_info_ = store_extended_info;
    }

    /// @brief Unparses network object.
    ///
    /// @return A pointer to unparsed network configuration.
    virtual data::ElementPtr toElement() const;

protected:

    /// @brief Gets the optional callback function used to fetch globally
    /// configured parameters.
    ///
    /// @return Pointer to the function.
    FetchNetworkGlobalsFn getFetchGlobalsFn() const {
        return (fetch_globals_fn_);
    }

    /// @brief Returns a value of global configuration parameter with
    /// a given name.
    ///
    /// If the @c ferch_globals_fn_ function is non-null, this method will
    /// invoke this function to retrieve a global value having the given
    /// name. Typically, this method is invoked by @c getProperty when
    /// network specific value of the parameter is not found. In some cases
    /// it may be called by other methods. One such example is the
    /// @c getHostReservationMode which needs to call @c getGlobalProperty
    /// explicitly to convert the global host reservation mode value from
    /// a string to an enum.
    ///
    /// @tparam ReturnType Type of the returned value, e.g.
    /// @c Optional<std::string>.
    ///
    /// @param property Value to be returned when it is specified or when
    /// no global value is found.
    /// @param global_name Name of the global parameter which value should
    /// be returned
    ///
    /// @return Optional value fetched from the global level or the value
    /// of @c property.
    template<typename ReturnType>
    ReturnType getGlobalProperty(ReturnType property,
                                 const std::string& global_name) const {
        if (!global_name.empty() && fetch_globals_fn_) {
            data::ElementPtr globals = fetch_globals_fn_();
            if (globals && (globals->getType() == data::Element::map)) {
                data::ElementPtr global_param = globals->get(global_name);
                if (global_param) {
                    // If there is a global parameter, convert it to the
                    // optional value of the given type and return.
                    return (data::ElementValue<typename ReturnType::ValueType>()(global_param));
                }
            }
        }
        return (property);
    }

    /// @brief The @c getGlobalProperty specialization for Optional<IOAddress>.
    ///
    /// This does two things:
    ///  - uses the string value of the parameter
    ///  - falls back when the value is empty
    ///
    /// @note: use overloading vs specialization because full specialization
    /// is not allowed in this scope.
    ///
    /// @param property Value to be returned when it is specified or when
    /// no global value is found.
    /// @param global_name Name of the global parameter which value should
    /// be returned
    ///
    /// @return Optional value fetched from the global level or the value
    /// of @c property.
    util::Optional<asiolink::IOAddress>
    getGlobalProperty(util::Optional<asiolink::IOAddress> property,
                      const std::string& global_name) const;

    /// @brief Returns a value associated with a network using inheritance.
    ///
    /// This template method provides a generic mechanism to retrieve a
    /// network parameter using inheritance. It is called from public
    /// accessor methods which return an @c OptionalValue or @c Triplet.
    ///
    /// @tparam BaseType Type of this instance, e.g. @c Network, @c Network4
    /// etc, which exposes a method to be called.
    /// @tparam ReturnType Type of the returned value, e.g.
    /// @c Optional<std::string>.
    ///
    /// @param MethodPointer Pointer to the method of the base class which
    /// should be called on the parent network instance (typically on
    /// @c SharedNetwork4 or @c SharedNetwork6) to fetch the parent specific
    /// value if the value is unspecified for this instance.
    /// @param property Value to be returned when it is specified or when
    /// no explicit value is specified on upper inheritance levels.
    /// @param inheritance inheritance mode to be used.
    /// @param global_name Optional name of the global parameter which value
    /// should be returned if the given parameter is not specified on network
    /// level. This value is empty by default, which indicates that the
    /// global value for the given parameter is not supported and shouldn't
    /// be fetched.
    ///
    /// @return Optional value fetched from this instance level, parent
    /// network level or global level
    template<typename BaseType, typename ReturnType>
    ReturnType getProperty(ReturnType(BaseType::*MethodPointer)(const Inheritance&) const,
                           ReturnType property,
                           const Inheritance& inheritance,
                           const std::string& global_name = "") const {

        // If no inheritance is to be used, return the value for this
        // network regardless if it is specified or not.
        if (inheritance == Inheritance::NONE) {
            return (property);

        } else if (inheritance == Inheritance::PARENT_NETWORK) {
            ReturnType parent_property;

            // Check if this instance has a parent network.
            auto parent = std::dynamic_pointer_cast<BaseType>(parent_network_.lock());
            if (parent) {
                parent_property = ((*parent).*MethodPointer)(Network::Inheritance::NONE);
            }
            return (parent_property);

        // If global value requested, return it.
        } else if (inheritance == Inheritance::GLOBAL) {
            return (getGlobalProperty(ReturnType(), global_name));
        }

        // We use inheritance and the value is not specified on the network level.
        // Hence, we need to get the parent network specific value or global value.
        if (property.unspecified()) {
            // Check if this instance has a parent network.
            auto parent = std::dynamic_pointer_cast<BaseType>(parent_network_.lock());
            // If the parent network exists, let's fetch the parent specific
            // value.
            if (parent) {
                // We're using inheritance so ask for the parent specific network
                // and return it only if it is specified.
                auto parent_property = ((*parent).*MethodPointer)(inheritance);
                if (!parent_property.unspecified()) {
                    return (parent_property);
                }
            }

            // The value is not specified on network level. If the value
            // can be specified on global level and there is a callback
            // that returns the global values, try to find this parameter
            // at the global scope.
            return (getGlobalProperty(property, global_name));
        }

        // We haven't found the value at any level, so return the unspecified.
        return (property);
    }

    /// @brief Returns option pointer associated with a network using inheritance.
    ///
    /// This template method provides a generic mechanism to retrieve a
    /// network parameter using inheritance. It is called from public
    /// accessor methods which return an @c OptionPtr.
    ///
    /// @tparam BaseType Type of this instance, e.g. @c Network, @c Network4
    /// etc, which exposes a method to be called.
    ///
    /// @param MethodPointer Pointer to the method of the base class which
    /// should be called on the parent network instance (typically on
    /// @c SharedNetwork4 or @c SharedNetwork6) to fetch the parent specific
    /// value if the value is unspecified for this instance.
    /// @param property the value to return when inheritance mode is NONE, or
    /// when the mode is PARENT_NETWORK and the property has not been specified
    /// by a parent network.
    /// @param inheritance inheritance mode to be used.
    ///
    /// @return Option pointer fetched from this instance level or parent
    /// network level.
    template<typename BaseType>
    OptionPtr
    getOptionProperty(OptionPtr(BaseType::*MethodPointer)(const Inheritance& inheritance) const,
                      OptionPtr property,
                      const Inheritance& inheritance) const {
        if (inheritance == Network::Inheritance::NONE) {
            return (property);

        } else if (inheritance == Network::Inheritance::PARENT_NETWORK) {
            OptionPtr parent_property;
            // Check if this instance has a parent network.
            auto parent = std::dynamic_pointer_cast<BaseType>(parent_network_.lock());
            // If the parent network exists, let's fetch the parent specific
            // value.
            if (parent) {
                parent_property = ((*parent).*MethodPointer)(Network::Inheritance::NONE);
            }
            return (parent_property);

        } else if (inheritance == Network::Inheritance::GLOBAL) {
            return (OptionPtr());
        }

        // We use inheritance and the value is not specified on the network level.
        // Hence, we need to get the parent network specific value.
        if (!property) {
            // Check if this instance has a parent network.
            auto parent = std::dynamic_pointer_cast<BaseType>(parent_network_.lock());
            if (parent) {
                // We're using inheritance so ask for the parent specific network
                // and return it only if it is specified.
                OptionPtr parent_property = (((*parent).*MethodPointer)(inheritance));
                if (parent_property) {
                    return (parent_property);
                }
            }
        }

        // We haven't found the value at any level, so return the unspecified.
        return (property);
    }

    /// @brief Holds interface name for which this network is selected.
    util::Optional<std::string> iface_name_;

    /// @brief Relay information
    ///
    /// See @ref RelayInfo for detailed description.
    RelayInfo relay_;

    /// @brief Optional definition of a client class
    ///
    /// If defined, only clients belonging to that class will be allowed to use
    /// this particular network. The default value for this is an empty string,
    /// which means that any client is allowed, regardless of its class.
    util::Optional<ClientClass> client_class_;

    /// @brief Required classes
    ///
    /// If the network is selected these classes will be added to the
    /// incoming packet and their evaluation will be required.
    ClientClasses required_classes_;

    /// @brief a Triplet (min/default/max) holding allowed renew timer values
    Triplet<uint32_t> t1_;

    /// @brief a Triplet (min/default/max) holding allowed rebind timer values
    Triplet<uint32_t> t2_;

    /// @brief a Triplet (min/default/max) holding allowed valid lifetime values
    Triplet<uint32_t> valid_;

    /// @brief Specifies host reservation mode
    ///
    /// See @ref HRMode type for details.
    util::Optional<HRMode> host_reservation_mode_;

    /// @brief Pointer to the option data configuration for this subnet.
    CfgOptionPtr cfg_option_;

    /// @brief Enables calculation of T1 and T2 timers
    util::Optional<bool> calculate_tee_times_;

    /// @brief Percentage of the lease lifetime to use when calculating T1 timer
    util::Optional<double> t1_percent_;

    /// @brief Percentage of the lease lifetime to use when calculating T2 timer
    util::Optional<double> t2_percent_;

    /// @brief Should Kea perform DNS updates. Used to provide scoped enabling
    /// and disabling of updates.
    util::Optional<bool> ddns_send_updates_;

    /// @brief Should Kea perform updates, even if client requested no updates.
    /// Overrides the client request for no updates via the N flag.
    util::Optional<bool> ddns_override_no_update_;

    /// @brief Should Kea perform updates, even if client requested delegation.
    util::Optional<bool> ddns_override_client_update_;

    /// @brief How Kea should handle the domain-name supplied by the client.
    util::Optional<D2ClientConfig::ReplaceClientNameMode> ddns_replace_client_name_mode_;

    /// @brief Prefix Kea should use when generating domain-names.
    util::Optional<std::string> ddns_generated_prefix_;

    /// @brief Suffix Kea should use when to qualify partial domain-names.
    util::Optional<std::string> ddns_qualifying_suffix_;

    /// @brief Regular expression describing invalid characters for client
    /// hostnames.
    util::Optional<std::string> hostname_char_set_;

    /// @brief A string to replace invalid characters when scrubbing hostnames.
    /// Meaningful only if hostname_char_set_ is not empty.
    util::Optional<std::string> hostname_char_replacement_;

    /// @brief Should Kea store addtional client query data (e.g. relay-agent-info)
    /// on the lease.
    util::Optional<bool> store_extended_info_;

    /// @brief Pointer to another network that this network belongs to.
    ///
    /// The most common case is that this instance is a subnet which belongs
    /// to a shared network and the @c parent_network_ points to the shared
    /// network object. If the network instance (subnet) doesn't belong to
    /// a shared network this pointer is null.
    WeakNetworkPtr parent_network_;

    /// @brief Pointer to the optional callback used to fetch globally
    /// configured parameters inherited to the @c Network object.
    FetchNetworkGlobalsFn fetch_globals_fn_;
};

/// @brief Specialization of the @ref Network object for DHCPv4 case.
class Network4 : public virtual Network {
public:

    /// @brief Constructor.
    Network4()
        : Network(), match_client_id_(true, true), authoritative_(),
          siaddr_(), sname_(), filename_() {
    }

    /// @brief Returns the flag indicating if the client identifiers should
    /// be used to identify the client's lease.
    ///
    /// @param inheritance inheritance mode to be used.
    /// @return true if client identifiers should be used, false otherwise.
    util::Optional<bool>
    getMatchClientId(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network4>(&Network4::getMatchClientId,
                                      match_client_id_,
                                      inheritance,
                                      "match-client-id"));
    }

    /// @brief Sets the flag indicating if the client identifier should be
    /// used to identify the client's lease.
    ///
    /// @param match If this value is true, the client identifiers are not
    /// used for lease lookup.
    void setMatchClientId(const util::Optional<bool>& match) {
        match_client_id_ = match;
    }

    /// @brief Returns the flag indicating if requests for unknown IP addresses
    /// should be rejected with DHCPNAK instead of ignored.
    ///
    /// @param inheritance inheritance mode to be used.w
    /// @return true if requests for unknown IP addresses should be rejected,
    /// false otherwise.
    util::Optional<bool>
    getAuthoritative(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network4>(&Network4::getAuthoritative, authoritative_,
                                      inheritance, "authoritative"));
    }

    /// @brief Sets the flag indicating if requests for unknown IP addresses
    /// should be rejected with DHCPNAK instead of ignored.
    ///
    /// @param authoritative If this value is true, the requests for unknown IP
    /// addresses will be rejected with DHCPNAK messages
    void setAuthoritative(const util::Optional<bool>& authoritative) {
        authoritative_ = authoritative;
    }

    /// @brief Sets siaddr for the network.
    ///
    /// Will be used for siaddr field (the next server) that typically is used
    /// as TFTP server. If not specified, the default value of 0.0.0.0 is
    /// used.
    void setSiaddr(const util::Optional<asiolink::IOAddress>& siaddr);

    /// @brief Returns siaddr for this network.
    ///
    /// @return siaddr value
    util::Optional<asiolink::IOAddress>
    getSiaddr(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network4>(&Network4::getSiaddr, siaddr_,
                                      inheritance, "next-server"));
    }

    /// @brief Sets server hostname for the network.
    ///
    /// Will be used for server hostname field (may be empty if not defined)
    void setSname(const util::Optional<std::string>& sname);

    /// @brief Returns server hostname for this network.
    ///
    /// @param inheritance inheritance mode to be used.
    /// @return server hostname value
    util::Optional<std::string>
    getSname(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network4>(&Network4::getSname, sname_,
                                      inheritance, "server-hostname"));
    }

    /// @brief Sets boot file name for the network.
    ///
    /// Will be used for boot file name (may be empty if not defined)
    void setFilename(const util::Optional<std::string>& filename);

    /// @brief Returns boot file name for this subnet
    ///
    /// @param inheritance inheritance mode to be used.
    /// @return boot file name value
    util::Optional<std::string>
    getFilename(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network4>(&Network4::getFilename, filename_,
                                      inheritance, "boot-file-name"));
    }

    /// @brief Unparses network object.
    ///
    /// @return A pointer to unparsed network configuration.
    virtual data::ElementPtr toElement() const;

    /// @brief Returns binary representation of the dhcp-server-identifier option (54).
    ///
    /// @return Server identifier option as IPv4 address. Zero IPv4 address
    /// indicates that server identifier hasn't been specified.
    virtual asiolink::IOAddress getServerId() const;

private:

    /// @brief Should server use client identifiers for client lease
    /// lookup.
    util::Optional<bool> match_client_id_;

    /// @brief Should requests for unknown IP addresses be rejected.
    util::Optional<bool> authoritative_;

    /// @brief siaddr value for this subnet
    util::Optional<asiolink::IOAddress> siaddr_;

    /// @brief server hostname for this subnet
    util::Optional<std::string> sname_;

    /// @brief boot file name for this subnet
    util::Optional<std::string> filename_;
};

/// @brief Specialization of the @ref Network object for DHCPv6 case.
class Network6 : public virtual Network {
public:

    /// @brief Constructor.
    Network6()
        : Network(), preferred_(), interface_id_(), rapid_commit_() {
    }

    /// @brief Returns preferred lifetime (in seconds)
    ///
    /// @param inheritance inheritance mode to be used.
    /// @return a triplet with preferred lifetime
    Triplet<uint32_t>
    getPreferred(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getProperty<Network6>(&Network6::getPreferred, preferred_,
                                      inheritance, "preferred-lifetime"));
    }

    /// @brief Sets new preferred lifetime for a network.
    ///
    /// @param preferred New preferred lifetime in seconds.
    void setPreferred(const Triplet<uint32_t>& preferred) {
        preferred_ = preferred;
    }

    /// @brief Returns interface-id value (if specified)
    ///
    /// @param inheritance inheritance mode to be used.
    /// @return interface-id option (if defined)
    OptionPtr getInterfaceId(const Inheritance& inheritance = Inheritance::ALL) const {
        return (getOptionProperty<Network6>(&Network6::getInterfaceId, interface_id_,
                                            inheritance));
    }

    /// @brief sets interface-id option (if defined)
    ///
    /// @param ifaceid pointer to interface-id option
    void setInterfaceId(const OptionPtr& ifaceid) {
        interface_id_ = ifaceid;
    }

    /// @brief Returns boolean value indicating that the Rapid Commit option
    /// is supported or unsupported for the subnet.
    ///
    /// @param inheritance inheritance mode to be used.
    /// @return true if the Rapid Commit option is supported, false otherwise.
    util::Optional<bool>
    getRapidCommit(const Inheritance& inheritance = Inheritance::ALL) const {

        return (getProperty<Network6>(&Network6::getRapidCommit, rapid_commit_,
                                      inheritance, "rapid-commit"));
    }

    /// @brief Enables or disables Rapid Commit option support for the subnet.
    ///
    /// @param rapid_commit A boolean value indicating that the Rapid Commit
    /// option support is enabled (if true), or disabled (if false).
    void setRapidCommit(const util::Optional<bool>& rapid_commit) {
        rapid_commit_ = rapid_commit;
    };

    /// @brief Unparses network object.
    ///
    /// @return A pointer to unparsed network configuration.
    virtual data::ElementPtr toElement() const;

private:

    /// @brief a triplet with preferred lifetime (in seconds)
    Triplet<uint32_t> preferred_;

    /// @brief specifies optional interface-id
    OptionPtr interface_id_;

    /// @brief A flag indicating if Rapid Commit option is supported
    /// for this network.
    ///
    /// It's default value is false, which indicates that the Rapid
    /// Commit is disabled for the subnet.
    util::Optional<bool> rapid_commit_;
};

} // end of namespace isc::dhcp
} // end of namespace isc

#endif // NETWORK_H

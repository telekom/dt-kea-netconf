// Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef ISC_ADAPTOR_CONFIG_H
#define ISC_ADAPTOR_CONFIG_H 1

#include <list>
#include <yang/adaptor_host.h>
#include <yang/adaptor_option.h>
#include <yang/adaptor_pool.h>
#include <yang/adaptor_subnet.h>

namespace isc {
namespace yang {

/// @brief JSON adaptor for Kea server configurations.
///
/// Currently only from JSON to YANG for DHCPv4 and DHCPv6 available
/// as preProcess4 and preProcess6 class methods, filling some required
/// (by YANG) fields (e.g. subnet IDs, or option code and space), or
/// transforming a hand-written JSON configuration into a canonical form.
struct AdaptorConfig : AdaptorHost, AdaptorOption, AdaptorSubnet {
    /// @brief Destructor.
    virtual ~AdaptorConfig() = default;

    /// @brief Pre process a DHCPv4 configuration.
    ///
    /// Assign subnet IDs, check and set defaults in options, etc.
    /// Note even though the parameter is a ElementPtr and is not modified,
    /// sub-structures can modify it, so if you need a copy do a deep one.
    ///
    /// @param config The configuration.
    /// @throw MissingKey when a required key is missing.
    /// @throw BadValue when null or not a map or deprecated Logging present.
    /// @note Does nothing if "Dhcp4" is not present in the map.
    static void preProcess4(isc::data::ElementPtr config);

    /// @brief Pre process a DHCPv6 configuration.
    ///
    /// Assign subnet IDs, check and set default in options, etc.
    /// Note even though the parameter is a ElementPtr and is not modified,
    /// sub-structures can modify it, so if you need a copy do a deep one.
    ///
    /// @param config The configuration.
    /// @throw MissingKey when a required key is missing.
    /// @throw BadValue when null or not a map or deprecated Logging present.
    /// @note Does nothing if "Dhcp6" is not present in the map.
    static void preProcess6(isc::data::ElementPtr config);

    /// @brief Pre process a distributed DHCPv4 configuration.
    ///
    /// Assign subnet IDs, check and set defaults in options, etc.
    /// Note even though the parameter is a ElementPtr and is not modified,
    /// sub-structures can modify it, so if you need a copy do a deep one.
    ///
    /// @param config The configuration.
    /// @throw MissingKey when a required key is missing.
    /// @throw BadValue when null or not a map
    /// @note Does nothing if "Dhcp4" is not present in the map.
    static void preProcess4Distributed(isc::data::ElementPtr config);

    /// @brief Pre process a distributed DHCPv6 configuration.
    ///
    /// Assign subnet IDs, check and set default in options, etc.
    /// Note even though the parameter is a ElementPtr and is not modified,
    /// sub-structures can modify it, so if you need a copy do a deep one.
    ///
    /// @param config The configuration.
    /// @throw MissingKey when a required key is missing.
    /// @throw BadValue when null or not a map
    /// @note Does nothing if "Dhcp6" is not present in the map.
    static void preProcess6Distributed(isc::data::ElementPtr config);

protected:
    /// @brief Collects subnet-ids on all subnets.
    ///
    /// It will go over all subnets and collect their ids. It will then return
    /// true if all subnets have ids. If the subnets list is empty, it will also
    /// return true. False will be returned if there is at least one subnet that
    /// doesn't have subnet-id.
    ///
    /// @param subnets The subnet list.
    /// @param set The reference to the set of assigned IDs.
    /// @return True if all subnets have an ID, false otherwise.
    static bool subnetsCollectID(isc::data::ElementPtr subnets, SubnetIDSet& set);

    /// @brief Collects subnet-ids in all subnets in all shared network list.
    ///
    /// It will go over all subnets in all shared networks specified to collect
    /// their subnet-ids. It will then return true if all subnets have ids. If
    /// the subnets list is empty, it will also return true. False will be
    /// returned if there is at least one subnet that doesn't have subnet-id.
    ///
    /// @param networks The shared network list.
    /// @param set The reference to the set of assigned IDs.
    /// @param subsel The subnet list name.
    /// @return True if all subnets have an ID, false otherwise.
    static bool sharedNetworksCollectID(isc::data::ElementPtr networks,
                                        SubnetIDSet& set,
                                        const std::string& subsel);

    /// @brief Assigns subnet-id to every subnet in a subnet list.
    ///
    /// Only those subnets that don't have one subnet-id assigned yet,
    /// will get a new subnet-id value.
    ///
    /// @param subnets The subnet list.
    /// @param set The reference to the set of assigned IDs.
    /// @param next The next ID.
    /// @return True if all subnets have an ID, false otherwise.
    static void
    subnetsAssignID(isc::data::ElementPtr subnets, SubnetIDSet& set, isc::dhcp::SubnetID& next);

    /// @brief Assigns subnet-id to every subnet in a shared network list.
    ///
    /// Only those subnets that don't have one subnet-id assigned yet,
    /// will get a new subnet-id value.
    ///
    /// @param networks The shared network list.
    /// @param set The reference to the set of assigned IDs.
    /// @param next The next ID.
    /// @param subsel The subnet list name.
    /// @return True if all subnets have an ID, false otherwise.
    static void sharedNetworksAssignID(isc::data::ElementPtr networks,
                                       SubnetIDSet& set,
                                       isc::dhcp::SubnetID& next,
                                       const std::string& subsel);

    /// @brief Sanitizes all pools in a pools list.
    ///
    /// Goes over each pool and cleans up its definition (removes extra spaces,
    /// adds one space before and after - ).
    ///
    /// @param pools The pool list.
    static void sanitizePools(isc::data::ElementPtr pools);

    /// @brief Sanitizes all pools in a subnets list.
    ///
    /// @param subnets The subnet list.
    static void sanitizePoolsInSubnets(isc::data::ElementPtr subnets);

    /// @brief Sanitizes all pools in all subnets in a shared network list.
    ///
    /// @param networks The shared network list.
    /// @param subsel The subnet list name.
    static void sanitizePoolsInSharedNetworks(isc::data::ElementPtr networks,
                                              const std::string& subsel);

    /// @brief Collect option definitions from an option definition list.
    ///
    /// Collects options definitions, but also sets missing option space
    /// with default.
    ///
    /// @param defs The option definition list.
    /// @param space The default space name (missing will be filled with this)
    /// @param codes Option definitions.
    static void
    sanitizeOptionDefList(isc::data::ElementPtr defs, const std::string& space, OptionCodes& codes);

    /// @brief Set missing option codes to an option data list.
    ///
    /// @param options The option data list.
    /// @param space The default space name.
    /// @param codes Option definitions.
    static void sanitizeOptionDataList(isc::data::ElementPtr options,
                                       const std::string& space,
                                       const OptionCodes& codes);

    /// @brief Collect option definitions from a client class list
    /// and set missing option codes.
    ///
    /// @param classes The client class list.
    /// @param space The default space name.
    /// @param codes Option definitions.
    static void sanitizeOptionClasses(isc::data::ElementPtr classes,
                                      const std::string& space,
                                      OptionCodes& codes);

    /// @brief Set missing option codes to a pool list.
    ///
    /// @param pools The pool list.
    /// @param space The default space name.
    /// @param codes Option definitions.
    static void sanitizeOptionPools(isc::data::ElementPtr pools,
                                    const std::string& space,
                                    const OptionCodes& codes);

    /// @brief Set missing option codes to a host reservation list.
    ///
    /// @param hosts The host reservation list.
    /// @param space The default space name.
    /// @param codes Option definitions.
    static void sanitizeOptionHosts(isc::data::ElementPtr hosts,
                                    const std::string& space,
                                    const OptionCodes& codes);

    /// @brief Set missing option codes to a subnet list.
    ///
    /// @param subnets The subnet list.
    /// @param space The default space name.
    /// @param codes Option definitions.
    static void sanitizeOptionSubnets(isc::data::ElementPtr subnets,
                                      const std::string& space,
                                      const OptionCodes& codes);

    /// @brief Set missing option codes to a shared network list.
    ///
    /// @param networks The shared network list.
    /// @param space The default space name.
    /// @param codes Option definitions.
    static void sanitizeOptionSharedNetworks(isc::data::ElementPtr networks,
                                             const std::string& space,
                                             const OptionCodes& codes);

    /// @brief Process require client classes in a pool list.
    ///
    /// Remove empty require client class list.
    ///
    /// @param pools The pool list.
    static void sanitizeRequireClassesPools(isc::data::ElementPtr pools);

    /// @brief Process require client classes in a subnet list.
    ///
    /// Remove empty require client class lists.
    ///
    /// @param subnets The subnet list.
    static void sanitizeRequireClassesSubnets(isc::data::ElementPtr subnets);

    /// @brief Process require client classes in a shared network list.
    ///
    /// Remove empty require client class lists.
    ///
    /// @param networks The shared network list.
    /// @param subsel The subnet list name.
    static void requireClassesSharedNetworks(isc::data::ElementPtr networks,
                                             const std::string& subsel);

    /// @brief Process host reservation list.
    ///
    /// Quote when needed flex-id identifiers.
    ///
    /// @param hosts The host reservation list.
    static void sanitizeHostList(isc::data::ElementPtr hosts);

    /// @brief Process host reservations in a subnet list.
    ///
    /// Quote when needed flex-id identifiers.
    ///
    /// @param subnets The subnet list.
    static void sanitizeHostSubnets(isc::data::ElementPtr subnets);

    /// @brief Process host reservations in a shared network list.
    ///
    /// Quote when needed flex-id identifiers.
    ///
    /// @param networks The shared network list.
    /// @param space The default space name.
    static void SanitizeHostsInSharedNetworks(isc::data::ElementPtr networks,
                                              const std::string& space);

    /// @brief Sanitizes relay information in subnets in a subnet list.
    ///
    /// Force the use of ip-addresses when it finds an ip-address entry.
    ///
    /// @param subnets The subnet list.
    static void sanitizeRelaySubnets(isc::data::ElementPtr subnets);

    /// @brief Sanitizes relay information in a shared network list.
    ///
    /// Force the use of ip-addresses when it finds an ip-address entry.
    ///
    /// @param networks The shared network list.
    /// @param subsel The subnet list name.
    static void sanitizeRelayInSharedNetworks(isc::data::ElementPtr networks,
                                              const std::string& subsel);

    /// @brief Update (hosts) database.
    ///
    /// Force the use of hosts-databases vs. hosts-database.
    ///
    /// @param dhcp The DHCP server.
    static void sanitizeDatabase(isc::data::ElementPtr dhcp);

    /// @brief Update relay supplied options.
    ///
    /// Remove empty relay supplied option list.
    ///
    /// @param dhcp The DHCPv6 server.
    static void sanitizeRelaySuppliedOptions(isc::data::ElementPtr dhcp);

    /// @brief Pre process a configuration.
    ///
    /// Assign subnet IDs, check and set default in options, etc.
    ///
    /// @param dhcp The server configuration.
    /// @param subsel The subnet list name.
    /// @param space The default option space name.
    /// @throw MissingKey when a required key is missing.
    static void
    preProcess(isc::data::ElementPtr dhcp, const std::string& subsel, const std::string& space);
};

}  // namespace yang
}  // namespace isc

#endif  // ISC_ADAPTOR_CONFIG_H

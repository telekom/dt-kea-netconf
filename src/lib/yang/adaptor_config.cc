// Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <yang/adaptor_config.h>

using namespace std;
using namespace isc::data;
using namespace isc::dhcp;

namespace {
const string DHCP4_SPACE = "dhcp4";
const string DHCP6_SPACE = "dhcp6";
}  // namespace

namespace isc {
namespace yang {

bool AdaptorConfig::subnetsCollectID(ElementPtr subnets, SubnetIDSet& set) {
    bool have_ids = true;

    if (!subnets || subnets->empty()) {
        // There are no subnets defined, so technically there are no ids.
        // However, the flag is used to determine whether the code later
        // needs to call assignIDs. Since there is no need to assign
        // anything, the code returns true here.
        return (true);
    }

    // If there are subnets defined, let's go over them one by one and
    // collect subnet-ids used in them.
    for (ElementPtr subnet : subnets->listValue()) {
        if (!collectID(subnet, set)) {
            have_ids = false;
        }
    }
    return (have_ids);
}

bool AdaptorConfig::sharedNetworksCollectID(ElementPtr networks,
                                            SubnetIDSet& set,
                                            const string& subsel) {
    if (!networks || networks->empty()) {
        // There are no shared networks defined, so technically there are no
        // ids. However, the flag is used to determine whether the code later
        // needs to call assignIDs. Since there is no need to assign anything,
        // the code returns true here.
        return (true);
    }

    // This determines if EVERY subnet has subnet-id defined.
    bool have_ids = true;
    for (size_t i = 0; i < networks->size(); ++i) {
        ElementPtr network = networks->get(i);
        ElementPtr subnets = network->get(subsel);
        if (subnets) {
            if (!subnets->empty()) {
                // If there are subnets, collect their subnet-ids. If any
                // of them doesn't have a subnet-id, return false.
                if (!subnetsCollectID(subnets, set)) {
                    have_ids = false;
                }
            } else {
                // There's empty subnets list, so just remove it.
                network->remove(subsel);
            }
        }
    }
    return (have_ids);
}

void AdaptorConfig::subnetsAssignID(ElementPtr subnets, SubnetIDSet& set, SubnetID& next) {
    if (!subnets || subnets->empty()) {
        // nothing to do here.
        return;
    }

    for (size_t i = 0; i < subnets->size(); ++i) {
        ElementPtr subnet = subnets->get(i);
        assignID(subnet, set, next);
    }
}

void AdaptorConfig::sharedNetworksAssignID(ElementPtr networks,
                                           SubnetIDSet& set,
                                           SubnetID& next,
                                           const string& subsel) {
    if (!networks || networks->empty()) {
        // nothing to do here.
        return;
    }

    for (ElementPtr network : networks->listValue()) {
        ElementPtr subnets = network->get(subsel);
        if (!subnets || subnets->empty()) {
            continue;
        }

        for (size_t i = 0; i < subnets->size(); ++i) {
            ElementPtr subnet = subnets->get(i);
            assignID(subnet, set, next);
        }
    }
}

void AdaptorConfig::sanitizePools(ElementPtr pools) {
    if (!pools || pools->empty()) {
        // nothing to do here.
        return;
    }

    // Canonize (clean up name, remove extra spaces, add one space where
    // needed) every pool on the list.
    for (size_t i = 0; i < pools->size(); ++i) {
        ElementPtr pool = pools->get(i);
        AdaptorPool::canonizePool(pool);
    }
}

void AdaptorConfig::sanitizePoolsInSubnets(ElementPtr subnets) {
    if (!subnets || subnets->empty()) {
        // nothing to do here.
        return;
    }

    for (ElementPtr subnet : subnets->listValue()) {
        sanitizePools(subnet->get("pools"));
    }
}

void AdaptorConfig::sanitizePoolsInSharedNetworks(ElementPtr networks, const string& subsel) {
    if (!networks || networks->empty()) {
        // nothing to do here.
        return;
    }

    for (ElementPtr network : networks->listValue()) {
        sanitizePoolsInSubnets(network->get(subsel));
    }
}

void AdaptorConfig::sanitizeOptionDefList(ElementPtr defs,
                                          const string& space,
                                          OptionCodes& codes) {
    if (!defs || defs->empty()) {
        // nothing to do here.
        return;
    }

    // Do sanity checks on every option definition and fill any missing
    // fields with default values.
    for (size_t i = 0; i < defs->size(); ++i) {
        ElementPtr def = defs->get(i);
        checkCode(def);
        checkType(def);
        setSpace(def, space);
        collect(def, codes);
    }
}

void AdaptorConfig::sanitizeOptionDataList(ElementPtr options,
                                           const string& space,
                                           const OptionCodes& codes) {
    if (!options || options->empty()) {
        // nothing to do here.
        return;
    }

    // Sanitize option-data. The only missing elements we may possibly
    // need to fill are option space and option code.
    for (size_t i = 0; i < options->size(); ++i) {
        ElementPtr option = options->get(i);
        setSpace(option, space);
        setCode(option, codes);
    }
}

void AdaptorConfig::sanitizeOptionClasses(ElementPtr classes,
                                          const string& space,
                                          OptionCodes& codes) {
    if (!classes || classes->empty()) {
        // nothing to do here.
        return;
    }

    // For every client class defined...
    for (size_t i = 0; i < classes->size(); ++i) {
        ElementPtr cclass = classes->get(i);

        if (space == DHCP4_SPACE) {
            ElementPtr options = cclass->get("option-def");
            if (options) {
                if (!options->empty()) {
                    // If present, sanitize it.
                    sanitizeOptionDefList(options, space, codes);
                } else {
                    // If empty, remove it.
                    cclass->remove("option-def");
                }
            }
        }

        // also sanitize option data.
        ElementPtr options = cclass->get("option-data");
        if (options) {
            if (!options->empty()) {
                // If present, sanitize it.
                sanitizeOptionDataList(options, space, codes);
            } else {
                // If empty, remove it.
                cclass->remove("option-data");
            }
        }
    }
}

void AdaptorConfig::sanitizeOptionPools(ElementPtr pools,
                                        const string& space,
                                        const OptionCodes& codes) {
    if (!pools || pools->empty()) {
        // nothing to do here.
        return;
    }

    for (size_t i = 0; i < pools->size(); ++i) {
        ElementPtr pool = pools->get(i);
        ElementPtr options = pool->get("option-data");
        if (options) {
            if (!options->empty()) {
                sanitizeOptionDataList(options, space, codes);
            } else {
                pool->remove("option-data");
            }
        }
    }
}

void AdaptorConfig::sanitizeOptionHosts(ElementPtr hosts,
                                        const string& space,
                                        const OptionCodes& codes) {
    if (!hosts || hosts->empty()) {
        // nothing to do here.
        return;
    }

    for (size_t i = 0; i < hosts->size(); ++i) {
        ElementPtr host = hosts->get(i);
        ElementPtr options = host->get("option-data");
        if (options) {
            if (!options->empty()) {
                sanitizeOptionDataList(options, space, codes);
            } else {
                host->remove("option-data");
            }
        }
    }
}

void AdaptorConfig::sanitizeOptionSubnets(ElementPtr subnets,
                                          const string& space,
                                          const OptionCodes& codes) {
    if (!subnets || subnets->empty()) {
        // nothing to do here.
        return;
    }

    for (size_t i = 0; i < subnets->size(); ++i) {
        ElementPtr subnet = subnets->get(i);

        // Let's try to sanitize option-data first.
        ElementPtr options = subnet->get("option-data");
        if (options) {
            if (!options->empty()) {
                sanitizeOptionDataList(options, space, codes);
            } else {
                subnet->remove("option-data");
            }
        }

        // Then try to sanitize pools.
        ElementPtr pools = subnet->get("pools");
        if (pools) {
            if (!pools->empty()) {
                sanitizeOptionPools(pools, space, codes);
            } else {
                subnet->remove("pools");
            }
        }

        // If this is v6, also sanitize pd-pools.
        if (space == DHCP6_SPACE) {
            ElementPtr pools = subnet->get("pd-pools");
            if (pools) {
                if (!pools->empty()) {
                    sanitizeOptionPools(pools, space, codes);
                } else {
                    subnet->remove("pd-pools");
                }
            }
        }

        // Finally, sanitize host reservations.
        ElementPtr hosts = subnet->get("reservations");
        if (hosts) {
            if (!hosts->empty()) {
                sanitizeOptionHosts(hosts, space, codes);
            } else {
                subnet->remove("reservations");
            }
        }
    }
}

void AdaptorConfig::sanitizeOptionSharedNetworks(ElementPtr networks,
                                                 const string& space,
                                                 const OptionCodes& codes) {
    if (!networks || networks->empty()) {
        // nothing to do here.
        return;
    }

    // For every shared network...
    for (size_t i = 0; i < networks->size(); ++i) {
        ElementPtr network = networks->get(i);

        // try to sanitize shared network options first.
        ElementPtr options = network->get("option-data");
        if (options) {
            if (!options->empty()) {
                sanitizeOptionDataList(options, space, codes);
            } else {
                network->remove("option-data");
            }
        }
        string subnet = "subnet";
        if (space == DHCP4_SPACE) {
            subnet += "4";
        } else {
            subnet += "6";
        }

        // Now try to sanitize subnets.
        ElementPtr subnets = network->get(subnet);
        if (subnets) {
            if (!subnets->empty()) {
                sanitizeOptionSubnets(subnets, space, codes);
            } else {
                network->remove(subnet);
            }
        }
    }
}

void AdaptorConfig::sanitizeRequireClassesPools(ElementPtr pools) {
    if (!pools || pools->empty()) {
        // nothing to do here.
        return;
    }

    for (size_t i = 0; i < pools->size(); ++i) {
        ElementPtr pool = pools->get(i);
        ElementPtr require(pool->get("require-client-classes"));
        if (require && require->empty()) {
            pool->remove("require-client-classes");
        }
    }
}

void AdaptorConfig::sanitizeRequireClassesSubnets(ElementPtr subnets) {
    if (!subnets || subnets->empty()) {
        // nothing to do here.
        return;
    }

    for (size_t i = 0; i < subnets->size(); ++i) {
        ElementPtr subnet = subnets->get(i);
        sanitizeRequireClassesPools(subnet->get("pools"));
        sanitizeRequireClassesPools(subnet->get("pd-pools"));
        ElementPtr require(subnet->get("require-client-classes"));
        if (require && require->empty()) {
            subnet->remove("require-client-classes");
        }
    }
}

void AdaptorConfig::requireClassesSharedNetworks(ElementPtr networks, const string& subsel) {
    if (!networks || networks->empty()) {
        // nothing to do here.
        return;
    }

    for (size_t i = 0; i < networks->size(); ++i) {
        ElementPtr network = networks->get(i);
        sanitizeRequireClassesSubnets(network->get(subsel));
        ElementPtr require(network->get("require-client-classes"));
        if (require && require->empty()) {
            network->remove("require-client-classes");
        }
    }
}

void AdaptorConfig::sanitizeHostList(ElementPtr hosts) {

    if (!hosts || hosts->empty()) {
        // nothing to do here.
        return;
    }

    for (size_t i = 0; i < hosts->size(); ++i) {
        ElementPtr host = hosts->get(i);
        quoteIdentifier(host);
    }
}

void AdaptorConfig::sanitizeHostSubnets(ElementPtr subnets) {

    if (!subnets || subnets->empty()) {
        // nothing to do here.
        return;
    }

    for (ElementPtr subnet : subnets->listValue()) {
        sanitizeHostList(subnet->get("reservations"));
    }
}

void AdaptorConfig::SanitizeHostsInSharedNetworks(ElementPtr networks, const string& space) {
    if (!networks || networks->empty()) {
        // nothing to do here.
        return;
    }

    for (ElementPtr network : networks->listValue()) {
        if (space == DHCP4_SPACE) {
            sanitizeHostSubnets(network->get("subnet4"));
        } else {
            sanitizeHostSubnets(network->get("subnet6"));
        }
    }
}

void AdaptorConfig::sanitizeRelaySubnets(ElementPtr subnets) {
    if (!subnets || subnets->empty()) {
        // nothing to do here.
        return;
    }

    for (size_t i = 0; i < subnets->size(); ++i) {
        ElementPtr subnet = subnets->get(i);
        updateRelay(subnet);
    }
}

void AdaptorConfig::sanitizeRelayInSharedNetworks(ElementPtr networks, const string& subsel) {
    if (!networks || networks->empty()) {
        // nothing to do here.
        return;
    }

    for (size_t i = 0; i < networks->size(); ++i) {
        ElementPtr network = networks->get(i);
        updateRelay(network);
        sanitizeRelaySubnets(network->get(subsel));
    }
}

void AdaptorConfig::sanitizeDatabase(ElementPtr dhcp) {
    ElementPtr database = dhcp->get("hosts-database");
    if (!database) {
        // nothing to do here.
        return;
    }

    dhcp->remove("hosts-database");
    ElementPtr list = Element::createList();
    list->add(database);
    dhcp->set("hosts-databases", list);
}

void AdaptorConfig::sanitizeRelaySuppliedOptions(ElementPtr dhcp) {
    ElementPtr options = dhcp->get("relay-supplied-options");
    if (!options || !options->empty()) {
        // nothing to do here.
        return;
    }
    dhcp->remove("relay-supplied-options");
}

void AdaptorConfig::preProcess(ElementPtr dhcp, const string& subsel, const string& space) {
    if (!dhcp) {
        isc_throw(BadValue, "preProcess: null DHCP config");
    }
    bool have_ids = true;
    SubnetIDSet set;
    ElementPtr subnets = dhcp->get(subsel);
    if (subnets) {
        if (!subnets->empty()) {
            if (!subnetsCollectID(subnets, set)) {
                have_ids = false;
            }
        } else {
            dhcp->remove(subsel);
        }
    }
    ElementPtr networks = dhcp->get("shared-networks");
    if (networks) {
        if (!networks->empty()) {
            if (!sharedNetworksCollectID(networks, set, subsel)) {
                have_ids = false;
            }
        } else {
            dhcp->remove("shared-networks");
        }
    }

    if (!have_ids) {
        SubnetID next(1);
        subnetsAssignID(subnets, set, next);
        sharedNetworksAssignID(networks, set, next, subsel);
    }

    OptionCodes codes;
    initCodes(codes, space);
    ElementPtr defs = dhcp->get("option-def");
    if (defs) {
        if (!defs->empty()) {
            sanitizeOptionDefList(defs, space, codes);
        } else {
            dhcp->remove("option-def");
        }
    }
    ElementPtr options = dhcp->get("option-data");
    if (options) {
        if (!options->empty()) {
            sanitizeOptionDataList(options, space, codes);
        } else {
            dhcp->remove("option-data");
        }
    }
    ElementPtr classes = dhcp->get("client-classes");
    if (classes) {
        if (!classes->empty()) {
            sanitizeOptionClasses(classes, space, codes);
        } else {
            dhcp->remove("client-classes");
        }
    }
    ElementPtr hosts = dhcp->get("reservations");
    if (hosts) {
        if (!hosts->empty()) {
            sanitizeHostList(hosts);
            sanitizeOptionHosts(hosts, space, codes);
        } else {
            dhcp->remove("reservations");
        }
    }
    sanitizeOptionSubnets(subnets, space, codes);
    sanitizeOptionSharedNetworks(networks, space, codes);

    sanitizePoolsInSubnets(subnets);
    sanitizePoolsInSharedNetworks(networks, subsel);

    sanitizeHostSubnets(subnets);
    SanitizeHostsInSharedNetworks(networks, space);

    sanitizeRelaySubnets(subnets);
    sanitizeRelayInSharedNetworks(networks, subsel);

    sanitizeRequireClassesSubnets(subnets);
    requireClassesSharedNetworks(networks, subsel);

    sanitizeDatabase(dhcp);

    if (space == DHCP6_SPACE) {
        sanitizeRelaySuppliedOptions(dhcp);
    }
}

void AdaptorConfig::preProcess4(ElementPtr config) {
    if (!config) {
        isc_throw(BadValue, "preProcess4: null config");
    }
    if (config->getType() != Element::map) {
        isc_throw(BadValue, "preProcess4: not map: " << config->str());
    }
    if (config->contains("Logging")) {
        isc_throw(BadValue, "preProcess4: got Logging object");
    }
    ElementPtr dhcp = config->get("Dhcp4");
    if (!dhcp) {
        return;
    }
    preProcess(dhcp, "subnet4", DHCP4_SPACE);
}

void AdaptorConfig::preProcess6(ElementPtr config) {
    if (!config) {
        isc_throw(BadValue, "preProcess6: null config");
    }
    if (config->getType() != Element::map) {
        isc_throw(BadValue, "preProcess6: not map: " << config->str());
    }
    if (config->contains("Logging")) {
        isc_throw(BadValue, "preProcess6: got Logging object");
    }
    ElementPtr dhcp = config->get("Dhcp6");
    if (!dhcp) {
        return;
    }
    preProcess(dhcp, "subnet6", DHCP6_SPACE);
}

void AdaptorConfig::preProcess4Distributed(ElementPtr config) {
    if (!config) {
        isc_throw(BadValue, "preProcess4Distributed: null config");
    }
    if (config->getType() != Element::map) {
        isc_throw(BadValue, "preProcess4Distributed: not map: " << config->str());
    }
    ElementPtr masters = config->get("masters");
    if (masters) {
        for (auto master : masters->listValue()) {
            ElementPtr shards = master->get("shards");
            if (shards) {
                for (auto shard : shards->listValue()) {
                    ElementPtr shard_config = shard->get("shard-config");
                    if (shard_config) {
                        ElementPtr dhcp = config->get("Dhcp4");
                        if (!dhcp) {
                            return;
                        }
                        preProcess(dhcp, "subnet4", DHCP4_SPACE);
                    }
                }
            }
        }
    }
}

void AdaptorConfig::preProcess6Distributed(ElementPtr config) {
    if (!config) {
        isc_throw(BadValue, "preProcess6Distributed: null config");
    }
    if (config->getType() != Element::map) {
        isc_throw(BadValue, "preProcess6Distributed: not map: " << config->str());
    }
    ElementPtr masters = config->get("masters");
    if (masters) {
        for (auto master : masters->listValue()) {
            ElementPtr shards = master->get("shards");
            if (shards) {
                for (auto shard : shards->listValue()) {
                    ElementPtr shard_config = shard->get("shard-config");
                    if (shard_config) {
                        ElementPtr dhcp = config->get("Dhcp6");
                        if (!dhcp) {
                            return;
                        }
                        preProcess(dhcp, "subnet6", DHCP6_SPACE);
                    }
                }
            }
        }
    }
}

}  // namespace yang
}  // namespace isc

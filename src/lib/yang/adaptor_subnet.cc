// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <yang/adaptor_subnet.h>

using namespace std;
using namespace isc::data;
using namespace isc::dhcp;

namespace isc {
namespace yang {

bool AdaptorSubnet::collectID(ElementPtr subnet, SubnetIDSet& set) {
    ElementPtr id = subnet->get("id");
    if (id) {
        set.insert(static_cast<SubnetID>(id->intValue()));
        return (true);
    }
    return (false);
}

void AdaptorSubnet::assignID(ElementPtr subnet, SubnetIDSet& set, SubnetID& next) {
    ElementPtr id = subnet->get("id");
    if (!id) {
        // Skip already used.
        while (set.count(next) > 0) {
            ++next;
        }
        subnet->set("id", Element::create(static_cast<long long>(next)));
        set.insert(next);
        ++next;
    }
}

void AdaptorSubnet::updateRelay(ElementPtr subnet) {
    ElementPtr relay = subnet->get("relay");
    if (!relay) {
        return;
    }
    ElementPtr addresses = relay->get("ip-addresses");
    if (!addresses) {
        ElementPtr address = relay->get("ip-address");
        if (!address) {
            subnet->remove("relay");
            return;
        }
        ElementPtr addr = Element::create(address->stringValue());
        ElementPtr addrs = Element::createList();
        addrs->add(addr);
        ElementPtr updated = Element::createMap();
        updated->set("ip-addresses", addrs);
        subnet->set("relay", updated);
    } else if (addresses->size() == 0) {
        subnet->remove("relay");
    }
}

}  // namespace yang
}  // namespace isc

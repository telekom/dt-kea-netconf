// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <boost/lexical_cast.hpp>
#include <sstream>
#include <yang/adaptor.h>
#include <yang/translator_pd_pool.h>
#include <yang/yang_models.h>

using namespace std;
using namespace isc::data;
using namespace sysrepo;

namespace isc {
namespace yang {

TranslatorPdPool::TranslatorPdPool(S_Session session, const string& model)
    : TranslatorBasic(session, model), TranslatorOptionData(session, model),
      TranslatorOptionDataList(session, model) {
}

ElementPtr TranslatorPdPool::getPdPool(const string& xpath) {
    try {
        if (model_ == IETF_DHCPV6_SERVER) {
            return (getPdPoolIetf6(xpath));
        } else if ((model_ == KEA_DHCP6_SERVER)) {
            return (getPdPoolKea(xpath));
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError,
                  "sysrepo error getting pd-pool at '" << xpath << "': " << ex.what());
    }
    isc_throw(NotImplemented, "getPdPool not implemented for the model: " << model_);
}

ElementPtr TranslatorPdPool::getPdPoolIetf6(const string& xpath) {
    ElementPtr result = Element::createMap();
    ElementPtr pref = getItem(xpath + "/prefix");
    if (!pref) {
        isc_throw(BadValue, "getPdPoolIetf6: prefix is required");
    }
    const string& prefix = pref->stringValue();
    size_t slash = prefix.find("/");
    if (slash == string::npos) {
        isc_throw(BadValue, "getPdPoolIetf6: no '/' in prefix '" << prefix << "'");
    }
    const string& address = prefix.substr(0, slash);
    if (address.empty()) {
        isc_throw(BadValue, "getPdPoolIetf6: malformed prefix '" << prefix << "'");
    }
    result->set("prefix", Element::create(address));
    // Silly: the prefix length is specified twice...
    ElementPtr preflen = getItem(xpath + "/prefix-length");
    if (!preflen) {
        isc_throw(BadValue, "getPdPoolIetf6: prefix length is required");
    }
    result->set("prefix-len", preflen);
    ElementPtr valid_lifetime = getItem(xpath + "/valid-lifetime");
    if (valid_lifetime) {
        result->set("valid-lifetime", valid_lifetime);
    }
    ElementPtr preferred_lifetime = getItem(xpath + "/preferred-lifetime");
    if (preferred_lifetime) {
        result->set("preferred-lifetime", preferred_lifetime);
    }
    ElementPtr renew_time = getItem(xpath + "/renew-time");
    if (renew_time) {
        result->set("renew-timer", renew_time);
    }
    ElementPtr rebind_time = getItem(xpath + "/rebind-time");
    if (rebind_time) {
        result->set("rebind-timer", rebind_time);
    }
    // Skip rapid-commit.
    ElementPtr guard = getItem(xpath + "/client-class");
    if (guard) {
        result->set("client-class", guard);
    }
    // no require-client-classes nor user-context.
    // Skip max-pd-space-utilization.
    // @todo option-data.
    return (result);
}

ElementPtr TranslatorPdPool::getPdPoolKea(const string& xpath) {
    ElementPtr result = Element::createMap();

    ElementPtr pref = getItem(xpath + "/prefix");
    if (!pref) {
        isc_throw(BadValue, "getPdPoolKea: no prefix defined at " << xpath);
    }
    const string& address = pref->stringValue();

    ElementPtr prefix_len = getItem(xpath + "/prefix-len");
    if (!prefix_len) {
        isc_throw(BadValue, "getPdPoolKea: no prefix-len defined at " << xpath);
    }
    int const length = prefix_len->intValue();

    if (address.empty()) {
        isc_throw(BadValue, "getPdPoolKea: malformed prefix '" << address << "/" << length << "'");
    }

    result->set("prefix", Element::create(address));
    result->set("prefix-len", Element::create(length));

    ElementPtr delegated = getItem(xpath + "/delegated-len");
    if (delegated) {
        int const delegated_len = delegated->intValue();
        result->set("delegated-len", Element::create(delegated_len));
    }

    ElementPtr xpref = getItem(xpath + "/excluded-prefix");
    if (xpref) {
        const string& xaddress = xpref->stringValue();
        if (xaddress.empty()) {
            isc_throw(BadValue, "getPdPoolKea: empty excluded address");
        }
        result->set("excluded-prefix", Element::create(xaddress));

        ElementPtr xprefix_len = getItem(xpath + "/excluded-prefix-len");
        if (xprefix_len) {
            int const xlength = xprefix_len->intValue();
            result->set("excluded-prefix-len", Element::create(xlength));
        }
    }

    ElementPtr options = getOptionDataList(xpath);
    if (options && options->size() > 0) {
        result->set("option-data", options);
    }
    ElementPtr guard = getItem(xpath + "/client-class");
    if (guard) {
        result->set("client-class", guard);
    }
    ElementPtr required = getItems(xpath + "/require-client-classes");
    if (required && (required->size() > 0)) {
        result->set("require-client-classes", required);
    }
    ElementPtr context = getItem(xpath + "/user-context");
    if (context) {
        result->set("user-context", Element::fromJSON(context->stringValue()));
    }
    return (result);
}

void TranslatorPdPool::setPdPool(const string& xpath, ElementPtr elem) {
    try {
        if (model_ == IETF_DHCPV6_SERVER) {
            setPdPoolIetf6(xpath, elem);
        } else if ((model_ == KEA_DHCP6_SERVER)) {
            setPdPoolKea(xpath, elem);
        } else {
            isc_throw(NotImplemented, "setPdPool not implemented for the model: " << model_);
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error setting pd-pool '" << elem->str() << "' at '"
                                                                  << xpath << "': " << ex.what());
    }
}

void TranslatorPdPool::setPdPoolIetf6(const string& xpath, ElementPtr elem) {
    ElementPtr base = elem->get("prefix");
    ElementPtr length = elem->get("prefix-len");
    if (!base || !length) {
        isc_throw(BadValue, "setPdPoolIetf6 requires prefix and prefix length: " << elem->str());
    }
    ostringstream prefix;
    prefix << base->stringValue() << "/" << length->intValue();
    setItem(xpath + "/prefix", Element::create(prefix.str()), SR_STRING_T);
    setItem(xpath + "/prefix-length", length, SR_UINT8_T);
    ElementPtr valid_lifetime = elem->get("valid-lifetime");
    if (valid_lifetime) {
        setItem(xpath + "/valid-lifetime", valid_lifetime, SR_UINT32_T);
    }
    ElementPtr preferred_lifetime = elem->get("preferred-lifetime");
    if (preferred_lifetime) {
        setItem(xpath + "/preferred-lifetime", preferred_lifetime, SR_UINT32_T);
    }
    ElementPtr renew_timer = elem->get("renew-timer");
    if (renew_timer) {
        setItem(xpath + "/renew-time", renew_timer, SR_UINT32_T);
    }
    ElementPtr rebind_timer = elem->get("rebind-timer");
    if (rebind_timer) {
        setItem(xpath + "/rebind-time", rebind_timer, SR_UINT32_T);
    }
    // Skip rapid-commit.
    ElementPtr guard = elem->get("client-class");
    if (guard) {
        setItem(xpath + "/client-class", guard, SR_STRING_T);
    }
    // Set max pd space utilization to disabled.
    setItem(xpath + "/max-pd-space-utilization", Element::create(string("disabled")), SR_ENUM_T);
    // @todo option-data.
}

void TranslatorPdPool::setPdPoolKea(const string& xpath, ElementPtr elem) {
    // Skip prefix as it is the key.
    bool created = false;
    ElementPtr e_prefix_len = elem->get("prefix-len");
    if (e_prefix_len) {
        uint8_t const prefix_len = e_prefix_len->intValue();
        setItem(xpath + "/prefix-len", Element::create(prefix_len), SR_UINT8_T);
    }
    ElementPtr delegated = elem->get("delegated-len");
    if (delegated) {
        uint8_t const delegated_len = delegated->intValue();
        setItem(xpath + "/delegated-len", Element::create(delegated_len), SR_UINT8_T);
    }
    ElementPtr xprefix = elem->get("excluded-prefix");
    ElementPtr xlen = elem->get("excluded-prefix-len");
    if (xprefix && xlen) {
        setItem(xpath + "/excluded-prefix", Element::create(xprefix->stringValue()), SR_STRING_T);
        setItem(xpath + "/excluded-prefix-len", Element::create(xlen->intValue()), SR_UINT8_T);
        created = true;
    }
    ElementPtr options = elem->get("option-data");
    if (options && options->size() > 0) {
        setOptionDataList(xpath, options);
        created = true;
    }
    ElementPtr guard = elem->get("client-class");
    if (guard) {
        setItem(xpath + "/client-class", guard, SR_STRING_T);
        created = true;
    }
    ElementPtr required = elem->get("require-client-classes");
    if (required && (required->size() > 0)) {
        for (ElementPtr rclass : required->listValue()) {
            setItem(xpath + "/require-client-classes", rclass, SR_STRING_T);
            created = true;
        }
    }
    ElementPtr context = Adaptor::getContext(elem);
    if (context) {
        setItem(xpath + "/user-context", Element::create(context->str()), SR_STRING_T);
        created = true;
    }
    // There are no mandatory fields outside the keys so force creation.
    if (!created) {
        ElementPtr list = Element::createList();
        setItem(xpath, list, SR_LIST_T);
    }
}

TranslatorPdPools::TranslatorPdPools(S_Session session, const string& model)
    : TranslatorBasic(session, model), TranslatorOptionData(session, model),
      TranslatorOptionDataList(session, model), TranslatorPdPool(session, model) {
}

ElementPtr TranslatorPdPools::getPdPools(const string& xpath) {
    try {
        if (model_ == IETF_DHCPV6_SERVER) {
            return getPdPoolsIetf(xpath);
        } else if (model_ == KEA_DHCP6_SERVER) {
            return getPdPoolsKea(xpath);
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError,
                  "sysrepo error getting pd-pools at '" << xpath << "': " << ex.what());
    }
    isc_throw(NotImplemented, "getPdPools not implemented for the model: " << model_);
}

ElementPtr TranslatorPdPools::getPdPoolsIetf(string const& xpath) {
    return getList<TranslatorPdPool>(xpath + "/pd-pool", *this, &TranslatorPdPool::getPdPool);
}

ElementPtr TranslatorPdPools::getPdPoolsKea(string const& xpath) {
    return getList<TranslatorPdPool>(xpath + "/pd-pools", *this, &TranslatorPdPool::getPdPool);
}

void TranslatorPdPools::setPdPools(const string& xpath, ElementPtr elem) {
    try {
        if (model_ == IETF_DHCPV6_SERVER) {
            setPdPoolsId(xpath, elem);
        } else if ((model_ == KEA_DHCP6_SERVER)) {
            setPdPoolsPrefix(xpath, elem);
        } else {
            isc_throw(NotImplemented, "setPdPools not implemented for the model: " << model_);
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error setting pools '" << elem->str() << "' at '" << xpath
                                                                << "': " << ex.what());
    }
}

void TranslatorPdPools::setPdPoolsId(const string& xpath, ElementPtr elem) {
    for (size_t i = 0; i < elem->size(); ++i) {
        ElementPtr pool = elem->get(i);
        ostringstream prefix;
        prefix << xpath << "/pd-pool[pool-id='" << i << "']";
        setPdPool(prefix.str(), pool);
    }
}

void TranslatorPdPools::setPdPoolsPrefix(const string& xpath, ElementPtr elem) {
    for (size_t i = 0; i < elem->size(); ++i) {
        ElementPtr pool = elem->get(i);
        if (!pool->contains("prefix") || !pool->contains("prefix-len")) {
            isc_throw(BadValue, "pd-pool requires prefix and prefix length: " << pool->str());
        }
        ostringstream prefix;
        prefix << xpath << "/pd-pools[prefix='" << pool->get("prefix")->stringValue() << "']";
        setPdPool(prefix.str(), pool);
    }
}

}  // namespace yang
}  // namespace isc

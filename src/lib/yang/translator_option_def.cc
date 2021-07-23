// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <sstream>
#include <yang/adaptor.h>
#include <yang/translator_option_def.h>
#include <yang/yang_models.h>

using namespace std;
using namespace isc::data;
using namespace sysrepo;

namespace isc {
namespace yang {

TranslatorOptionDef::TranslatorOptionDef(S_Session session, const string& model)
    : TranslatorBasic(session, model) {
}

ElementPtr TranslatorOptionDef::getOptionDef(const string& xpath) {
    try {
        if ((model_ == KEA_DHCP4_SERVER) || (model_ == KEA_DHCP6_SERVER)) {
            return (getOptionDefKea(xpath));
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError,
                  "sysrepo error getting option definition at '" << xpath << "': " << ex.what());
    }
    isc_throw(NotImplemented, "getOptionDef not implemented for the model: " << model_);
}

ElementPtr TranslatorOptionDef::getOptionDefKea(const string& xpath) {
    ElementPtr code = getItem(xpath + "/code");
    ElementPtr name = getItem(xpath + "/name");
    ElementPtr type = getItem(xpath + "/type");
    ElementPtr space = getItem(xpath + "/space");
    if (!code || !space) {
        // Can't happen as code and space are the keys.
        isc_throw(Unexpected, "getOptionDefKea requires code and space: " << xpath);
    }
    if (!name || !type) {
        isc_throw(BadValue, "getOptionDefKea requires name and type: " << xpath);
    }
    ElementPtr result = Element::createMap();
    result->set("code", code);
    result->set("name", name);
    result->set("type", type);
    result->set("space", getItem(xpath + "/space"));
    ElementPtr record = getItem(xpath + "/record-types");
    if (record) {
        result->set("record-types", record);
    }
    ElementPtr array = getItem(xpath + "/array");
    if (array) {
        result->set("array", array);
    }
    ElementPtr encapsulate = getItem(xpath + "/encapsulate");
    if (encapsulate) {
        result->set("encapsulate", encapsulate);
    }
    ElementPtr context = getItem(xpath + "/user-context");
    if (context) {
        result->set("user-context", Element::fromJSON(context->stringValue()));
    }
    return (result);
}

void TranslatorOptionDef::setOptionDef(const string& xpath, ElementPtr elem) {
    try {
        if ((model_ == KEA_DHCP4_SERVER) || (model_ == KEA_DHCP6_SERVER)) {
            setOptionDefKea(xpath, elem);
        } else {
            isc_throw(NotImplemented, "setOptionDef not implemented for the model: " << model_);
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error setting option definition '"
                                    << elem->str() << "' at '" << xpath << "': " << ex.what());
    }
}

void TranslatorOptionDef::setOptionDefKea(const string& xpath, ElementPtr elem) {
    // Skip code and space as they are the keys.
    ElementPtr name = elem->get("name");
    if (!name) {
        isc_throw(BadValue, "option definition with name: " << elem->str());
    }
    setItem(xpath + "/name", name, SR_STRING_T);
    ElementPtr type = elem->get("type");
    if (!type) {
        isc_throw(BadValue, "option definition with type: " << elem->str());
    }
    setItem(xpath + "/type", type, SR_STRING_T);
    ElementPtr record = elem->get("record-types");
    if (record) {
        setItem(xpath + "/record-types", record, SR_STRING_T);
    }
    ElementPtr array = elem->get("array");
    if (array) {
        setItem(xpath + "/array", array, SR_BOOL_T);
    }
    ElementPtr encapsulate = elem->get("encapsulate");
    if (encapsulate) {
        setItem(xpath + "/encapsulate", encapsulate, SR_STRING_T);
    }
    ElementPtr context = Adaptor::getContext(elem);
    if (context) {
        setItem(xpath + "/user-context", Element::create(context->str()), SR_STRING_T);
    }
}

TranslatorOptionDefList::TranslatorOptionDefList(S_Session session, const string& model)
    : TranslatorBasic(session, model), TranslatorOptionDef(session, model) {
}

ElementPtr TranslatorOptionDefList::getOptionDefList(const string& xpath) {
    try {
        if ((model_ == KEA_DHCP4_SERVER) || (model_ == KEA_DHCP6_SERVER)) {
            return (getOptionDefListKea(xpath));
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error getting option definition list at '"
                                    << xpath << "': " << ex.what());
    }
    isc_throw(NotImplemented, "getOptionDefList not implemented for the model: " << model_);
}

ElementPtr TranslatorOptionDefList::getOptionDefListKea(const string& xpath) {
    return getList<TranslatorOptionDef>(xpath + "/option-def", *this,
                                        &TranslatorOptionDefList::getOptionDef);
}

void TranslatorOptionDefList::setOptionDefList(const string& xpath, ElementPtr elem) {
    try {
        if ((model_ == KEA_DHCP4_SERVER) || (model_ == KEA_DHCP6_SERVER)) {
            setOptionDefListKea(xpath, elem);
        } else {
            isc_throw(NotImplemented, "setOptionDefList not implemented for the model: " << model_);
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error setting option definition list '"
                                    << elem->str() << "' at '" << xpath << "': " << ex.what());
    }
}

void TranslatorOptionDefList::setOptionDefListKea(const string& xpath, ElementPtr elem) {
    for (size_t i = 0; i < elem->size(); ++i) {
        ElementPtr def = elem->get(i);
        if (!def->contains("code")) {
            isc_throw(BadValue, "option definition without code: " << def->str());
        }
        unsigned code = static_cast<unsigned>(def->get("code")->intValue());
        if (!def->contains("space")) {
            isc_throw(BadValue, "option definition without space: " << def->str());
        }
        string space = def->get("space")->stringValue();
        ostringstream keys;
        keys << xpath << "/option-def[code='" << code << "'][space='" << space << "']";
        setOptionDef(keys.str(), def);
    }
}

}  // namespace yang
}  // namespace isc

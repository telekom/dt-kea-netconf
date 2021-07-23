// Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <sstream>
#include <yang/adaptor.h>
#include <yang/translator_logger.h>
#include <yang/yang_models.h>

using namespace std;
using namespace isc::data;
using namespace sysrepo;

namespace isc {
namespace yang {

TranslatorLogger::TranslatorLogger(S_Session session, const string& model)
    : TranslatorBasic(session, model) {
}

ElementPtr TranslatorLogger::getLogger(const string& xpath) {
    try {
        if ((model_ == KEA_DHCP4_SERVER) || (model_ == KEA_DHCP6_SERVER) ||
            (model_ == KEA_DHCP_DDNS) || (model_ == KEA_CTRL_AGENT)) {
            return (getLoggerKea(xpath));
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error getting logger at '" << xpath << "': " << ex.what());
    }
    isc_throw(NotImplemented, "getLogger not implemented for the model: " << model_);
}

ElementPtr TranslatorLogger::getLoggerKea(const string& xpath) {
    ElementPtr name = getItem(xpath + "/name");
    if (!name) {
        // Can't happen as name is the key.
        isc_throw(Unexpected, "getLoggerKea requires name: " << xpath);
    }
    ElementPtr result = Element::createMap();
    result->set("name", name);
    ElementPtr options = getOutputOptions(xpath);
    if (options && (options->size() > 0)) {
        result->set("output_options", options);
    }
    checkAndGetLeaf(result, xpath, "debuglevel");
    checkAndGetLeaf(result, xpath, "severity");
    ElementPtr context = getItem(xpath + "/user-context");
    if (context) {
        result->set("user-context", Element::fromJSON(context->stringValue()));
    }
    return (result);
}

ElementPtr TranslatorLogger::getOutputOption(const string& xpath) {
    ElementPtr output = getItem(xpath + "/output");
    if (!output) {
        // Can't happen as output is the key.
        isc_throw(Unexpected, "getOutputOption requires (!output): " << xpath);
    }
    ElementPtr result = Element::createMap();
    result->set("output", output);
    ElementPtr maxver = getItem(xpath + "/maxver");
    if (maxver) {
        result->set("maxver", maxver);
    }
    ElementPtr maxsize = getItem(xpath + "/maxsize");
    if (maxsize) {
        result->set("maxsize", maxsize);
    }
    ElementPtr flush = getItem(xpath + "/flush");
    if (flush) {
        result->set("flush", flush);
    }
    ElementPtr pattern = getItem(xpath + "/pattern");
    if (pattern) {
        result->set("pattern", pattern);
    }
    return (result);
}

ElementPtr TranslatorLogger::getOutputOptions(const string& xpath) {
    return getList(xpath + "/output_options", *this, &TranslatorLogger::getOutputOption);
}

void TranslatorLogger::setLogger(const string& xpath, ElementPtr elem) {
    try {
        if ((model_ == KEA_DHCP4_SERVER) || (model_ == KEA_DHCP6_SERVER) ||
            (model_ == KEA_DHCP_DDNS) || (model_ == KEA_CTRL_AGENT)) {
            setLoggerKea(xpath, elem);
        } else {
            isc_throw(NotImplemented, "setLogger not implemented for the model: " << model_);
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error setting logger '" << elem->str() << "' at '" << xpath
                                                                 << "': " << ex.what());
    }
}

void TranslatorLogger::setLoggerKea(const string& xpath, ElementPtr elem) {
    // Skip name as it is the key.
    ElementPtr options = elem->get("output_options");
    if (options && (options->size() > 0)) {
        setOutputOptions(xpath, options);
    }
    checkAndSetLeaf(elem, xpath, "debuglevel", SR_UINT8_T);
    checkAndSetLeaf(elem, xpath, "severity", SR_ENUM_T);
    ElementPtr context = Adaptor::getContext(elem);
    if (context) {
        setItem(xpath + "/user-context", Element::create(context->str()), SR_STRING_T);
    }
}

void TranslatorLogger::setOutputOption(const string& xpath, ElementPtr elem) {
    bool created = false;
    // Skip output as it is the key.
    ElementPtr maxver = elem->get("maxver");
    if (maxver) {
        setItem(xpath + "/maxver", maxver, SR_UINT32_T);
        created = true;
    }
    ElementPtr maxsize = elem->get("maxsize");
    if (maxsize) {
        setItem(xpath + "/maxsize", maxsize, SR_UINT32_T);
        created = true;
    }
    ElementPtr flush = elem->get("flush");
    if (flush) {
        setItem(xpath + "/flush", flush, SR_BOOL_T);
        created = true;
    }
    ElementPtr pattern = elem->get("pattern");
    if (pattern) {
        setItem(xpath + "/pattern", pattern, SR_STRING_T);
        created = true;
    }
    // There are no mandatory fields outside the key so force creation.
    if (!created) {
        ElementPtr list = Element::createList();
        setItem(xpath, list, SR_LIST_T);
    }
}

void TranslatorLogger::setOutputOptions(const string& xpath, ElementPtr elem) {
    for (size_t i = 0; i < elem->size(); ++i) {
        ElementPtr option = elem->get(i);
        if (!option->contains("output")) {
            isc_throw(BadValue, "output_options without output: " << option->str());
        }
        string output = option->get("output")->stringValue();
        ostringstream key;
        key << xpath << "/output_options[output='" << output << "']";
        setOutputOption(key.str(), option);
    }
}

TranslatorLoggers::TranslatorLoggers(S_Session session, const string& model)
    : TranslatorBasic(session, model), TranslatorLogger(session, model) {
}

ElementPtr TranslatorLoggers::getLoggers(const string& xpath) {
    try {
        if ((model_ == KEA_DHCP4_SERVER) || (model_ == KEA_DHCP6_SERVER) ||
            (model_ == KEA_DHCP_DDNS) || (model_ == KEA_CTRL_AGENT)) {
            return (getLoggersKea(xpath));
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError,
                  "sysrepo error getting loggeres at '" << xpath << "': " << ex.what());
    }
    isc_throw(NotImplemented, "getLoggers not implemented for the model: " << model_);
}

ElementPtr TranslatorLoggers::getLoggersKea(const string& xpath) {
    return getList<TranslatorLogger>(xpath + "/loggers", *this, &TranslatorLogger::getLogger);
}

void TranslatorLoggers::setLoggers(const string& xpath, ElementPtr elem) {
    try {
        if ((model_ == KEA_DHCP4_SERVER) || (model_ == KEA_DHCP6_SERVER) ||
            (model_ == KEA_DHCP_DDNS) || (model_ == KEA_CTRL_AGENT)) {
            setLoggersKea(xpath, elem);
        } else {
            isc_throw(NotImplemented, "setLoggers not implemented for the model: " << model_);
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error setting loggeres '" << elem->str() << "' at '"
                                                                   << xpath << "': " << ex.what());
    }
}

void TranslatorLoggers::setLoggersKea(const string& xpath, ElementPtr elem) {
    for (size_t i = 0; i < elem->size(); ++i) {
        ElementPtr logger = elem->get(i);
        if (!logger->contains("name")) {
            isc_throw(BadValue, "logger without name: " << logger->str());
        }
        string name = logger->get("name")->stringValue();
        ostringstream key;
        key << xpath << "/loggers[name='" << name << "']";
        setLogger(key.str(), logger);
    }
}

}  // namespace yang
}  // namespace isc

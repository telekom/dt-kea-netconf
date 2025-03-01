// Copyright (C) 2018-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <dhcp/docsis3_option_defs.h>
#include <dhcp/std_option_defs.h>
#include <yang/adaptor_option.h>

using namespace std;
using namespace isc::data;
using namespace isc::dhcp;

namespace isc {
namespace yang {

void AdaptorOption::setSpace(ElementPtr option, const string& space) {
    if (!option->contains("space")) {
        option->set("space", Element::create(space));
    }
}

void AdaptorOption::checkType(ElementPtr option) {
    if (!option->contains("type")) {
        isc_throw(MissingKey, "missing type in option definition " << option->str());
    }
}

void AdaptorOption::checkCode(ElementPtr option) {
    if (!option->contains("code")) {
        isc_throw(MissingKey, "missing code in option " << option->str());
    }
}

void AdaptorOption::collect(ElementPtr option, OptionCodes& codes) {
    ElementPtr name = option->get("name");
    if (name) {
        ElementPtr space = option->get("space");
        ElementPtr code = option->get("code");
        string index = space->stringValue() + "@" + name->stringValue();
        uint16_t val = static_cast<uint16_t>(code->intValue());
        codes.insert(std::pair<string, uint16_t>(index, val));
    }
}

void AdaptorOption::setCode(ElementPtr option, const OptionCodes& codes) {
    ElementPtr code = option->get("code");
    if (!code) {
        ElementPtr name = option->get("name");
        if (!name) {
            isc_throw(MissingKey, "missing name and code in option " << option->str());
        }
        ElementPtr space = option->get("space");
        string index = space->stringValue() + "@" + name->stringValue();
        OptionCodes::const_iterator it = codes.find(index);
        if (it == codes.end()) {
            isc_throw(MissingKey, "can't get code from option " << option->str());
        }
        option->set("code", Element::create(static_cast<int>(it->second)));
    }
}

void
AdaptorOption::initCodes(OptionCodes& codes, const string& space) {
    if (space == DHCP4_OPTION_SPACE) {
        initCodesInternal(codes, space, STANDARD_V4_OPTION_DEFINITIONS,
                          STANDARD_V4_OPTION_DEFINITIONS_SIZE);
        initCodesInternal(codes, space, LAST_RESORT_V4_OPTION_DEFINITIONS,
                          LAST_RESORT_V4_OPTION_DEFINITIONS_SIZE);
        initCodesInternal(codes, "vendor-4491", DOCSIS3_V4_OPTION_DEFINITIONS,
                          DOCSIS3_V4_OPTION_DEFINITIONS_SIZE);
    } else if (space == DHCP6_OPTION_SPACE) {
        initCodesInternal(codes, space, STANDARD_V6_OPTION_DEFINITIONS,
                          STANDARD_V6_OPTION_DEFINITIONS_SIZE);
        initCodesInternal(codes, "vendor-4491", DOCSIS3_V6_OPTION_DEFINITIONS,
                          DOCSIS3_V6_OPTION_DEFINITIONS_SIZE);
        initCodesInternal(codes, MAPE_V6_OPTION_SPACE, MAPE_V6_OPTION_DEFINITIONS,
                          MAPE_V6_OPTION_DEFINITIONS_SIZE);
        initCodesInternal(codes, MAPT_V6_OPTION_SPACE, MAPT_V6_OPTION_DEFINITIONS,
                          MAPT_V6_OPTION_DEFINITIONS_SIZE);
        initCodesInternal(codes, LW_V6_OPTION_SPACE, LW_V6_OPTION_DEFINITIONS,
                          LW_V6_OPTION_DEFINITIONS_SIZE);
        initCodesInternal(codes, V4V6_RULE_OPTION_SPACE, V4V6_RULE_OPTION_DEFINITIONS,
                          V4V6_RULE_OPTION_DEFINITIONS_SIZE);
        initCodesInternal(codes, V4V6_BIND_OPTION_SPACE, V4V6_BIND_OPTION_DEFINITIONS,
                          V4V6_BIND_OPTION_DEFINITIONS_SIZE);
        initCodesInternal(codes, "vendor-2495", ISC_V6_OPTION_DEFINITIONS,
                          ISC_V6_OPTION_DEFINITIONS_SIZE);
    }
}

void AdaptorOption::initCodesInternal(OptionCodes& codes,
                                      const string& space,
                                      const OptionDefParams* params,
                                      size_t params_size) {
    for (size_t i = 0; i < params_size; ++i) {
        string index = space + "@" + params[i].name;
        codes.insert(std::pair<string, uint16_t>(index, params[i].code));
    }
}

}  // namespace yang
}  // namespace isc

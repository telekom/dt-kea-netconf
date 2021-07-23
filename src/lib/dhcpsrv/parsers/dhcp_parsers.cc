// Copyright (C) 2013-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <dhcp/dhcp4.h>
#include <dhcp/iface_mgr.h>
#include <dhcp/libdhcp++.h>
#include <dhcpsrv/cfg_mac_source.h>
#include <dhcpsrv/cfg_option.h>
#include <dhcpsrv/cfgmgr.h>
#include <dhcpsrv/dhcpsrv_log.h>
#include <dhcpsrv/parsers/dhcp_parsers.h>
#include <dhcpsrv/parsers/host_reservation_parser.h>
#include <dhcpsrv/parsers/host_reservations_list_parser.h>
#include <dhcpsrv/parsers/option_data_parser.h>
#include <dhcpsrv/parsers/simple_parser4.h>
#include <dhcpsrv/parsers/simple_parser6.h>
#include <util/encode/hex.h>
#include <util/strutil.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <iomanip>
#include <map>
#include <string>
#include <vector>

using namespace std;
using namespace isc::asiolink;
using namespace isc::data;
using namespace isc::util;

namespace isc {
namespace dhcp {

// ******************** MACSourcesListConfigParser *************************

void
MACSourcesListConfigParser::parse(CfgMACSource& mac_sources, ElementPtr value) {
    CfgIface cfg_iface;
    uint32_t source = 0;
    size_t cnt = 0;

    // By default, there's only one source defined: ANY.
    // If user specified anything, we need to get rid of that default.
    mac_sources.clear();

    BOOST_FOREACH(ElementPtr source_elem, value->listValue()) {
        std::string source_str = source_elem->stringValue();
        try {
            source = CfgMACSource::MACSourceFromText(source_str);
            mac_sources.add(source);
            ++cnt;
        } catch (const InvalidParameter& ex) {
            isc_throw(DhcpConfigError, "The mac-sources value '" << source_str
                      << "' was specified twice (" << value->getPosition() << ")");
        } catch (const std::exception& ex) {
            isc_throw(DhcpConfigError, "Failed to convert '"
                      << source_str << "' to any recognized MAC source:"
                      << ex.what() << " (" << value->getPosition() << ")");
        }
    }

    if (!cnt) {
        isc_throw(DhcpConfigError, "If specified, MAC Sources cannot be empty");
    }
}

// ******************** ControlSocketParser *************************
void ControlSocketParser::sanityChecks(isc::data::ElementPtr const& value) {
    if (!value) {
        // Sanity check: not supposed to fail.
        isc_throw(DhcpConfigError, "Logic error: specified control-socket is null");
    }

    if (value->getType() != Element::map) {
        // Sanity check: not supposed to fail.
        isc_throw(DhcpConfigError, "Specified control-socket is expected to be a map"
                  ", i.e. a structure defined within { }");
    }
}

void ControlSocketParser::parse(SrvConfig& srv_cfg, isc::data::ElementPtr value) {
    sanityChecks(value);
    srv_cfg.setControlSocketInfo(value);
}

void ControlSocketParser::parseForNotifications(SrvConfig& srv_cfg, isc::data::ElementPtr const& value) {
    sanityChecks(value);
    srv_cfg.setNotificationsSocketInfo(value);
}

template<typename SearchKey>
OptionDefinitionPtr
OptionDataParser::findOptionDefinition(const std::string& option_space,
                                       const SearchKey& search_key) const {
    OptionDefinitionPtr def = LibDHCP::getOptionDef(option_space, search_key);

    if (!def) {
        // Check if this is a vendor-option. If it is, get vendor-specific
        // definition.
        uint32_t vendor_id = LibDHCP::optionSpaceToVendorId(option_space);
        if (vendor_id) {
            const Option::Universe u = address_family_ == AF_INET ?
                Option::V4 : Option::V6;
            def = LibDHCP::getVendorOptionDef(u, vendor_id, search_key);
        }
    }

    if (!def) {
        // Check if this is an option specified by a user.
        def = CfgMgr::instance().getStagingCfg()->getCfgOptionDef()
            ->get(option_space, search_key);
    }

    return (def);
}

// ******************************** OptionDefParser ****************************

OptionDefParser::OptionDefParser(const uint16_t address_family)
    : address_family_(address_family) {
}

std::pair<isc::dhcp::OptionDefinitionPtr, std::string>
OptionDefParser::parse(ElementPtr option_def) {

    // Check parameters.
    if (address_family_ == AF_INET) {
        checkKeywords(SimpleParser4::OPTION4_DEF_PARAMETERS, option_def);
    } else {
        checkKeywords(SimpleParser6::OPTION6_DEF_PARAMETERS, option_def);
    }

    // Get mandatory parameters.
    std::string name = getString(option_def, "name");
    int64_t code64 = getInteger(option_def, "code");
    std::string type = getString(option_def, "type");

    // Get optional parameters. Whoever called this parser, should have
    // called SimpleParser::setDefaults first.
    bool array_type = getBoolean(option_def, "array");
    std::string record_types = getString(option_def, "record-types");
    std::string space = getString(option_def, "space");
    std::string encapsulates = getString(option_def, "encapsulate");
    ElementPtr user_context = option_def->get("user-context");

    // Check code value.
    if (code64 < 0) {
        isc_throw(DhcpConfigError, "option code must not be negative "
                  "(" << getPosition("code", option_def) << ")");
    } else if (address_family_ == AF_INET &&
               code64 > std::numeric_limits<uint8_t>::max()) {
        isc_throw(DhcpConfigError, "invalid option code '" << code64
                  << "', it must not be greater than '"
                  << static_cast<int>(std::numeric_limits<uint8_t>::max())
                  << "' (" << getPosition("code", option_def) << ")");
    } else if (address_family_ == AF_INET6 &&
               code64 > std::numeric_limits<uint16_t>::max()) {
        isc_throw(DhcpConfigError, "invalid option code '" << code64
                  << "', it must not be greater than '"
                  << std::numeric_limits<uint16_t>::max()
                  << "' (" << getPosition("code", option_def) << ")");
    }
    uint32_t code = static_cast<uint32_t>(code64);

    // Validate space name.
    if (!OptionSpace::validateName(space)) {
        isc_throw(DhcpConfigError, "invalid option space name '"
                  << space << "' ("
                  << getPosition("space", option_def) << ")");
    }

    // Protect against definition of options 0 (PAD) or 255 (END)
    // in (and only in) the dhcp4 space.
    if (space == DHCP4_OPTION_SPACE) {
        if (code == DHO_PAD) {
            isc_throw(DhcpConfigError, "invalid option code '0': "
                      << "reserved for PAD ("
                      << getPosition("code", option_def) << ")");
        } else if (code == DHO_END) {
            isc_throw(DhcpConfigError, "invalid option code '255': "
                      << "reserved for END ("
                      << getPosition("code", option_def) << ")");
        }
    }

    // For dhcp6 space the value 0 is reserved.
    if (space == DHCP6_OPTION_SPACE) {
        if (code == 0) {
            isc_throw(DhcpConfigError, "invalid option code '0': "
                      << "reserved value ("
                      << getPosition("code", option_def) << ")");
        }
    }

    // Create option definition.
    OptionDefinitionPtr def;
    // We need to check if user has set encapsulated option space
    // name. If so, different constructor will be used.
    if (!encapsulates.empty()) {
        // Arrays can't be used together with sub-options.
        if (array_type) {
            isc_throw(DhcpConfigError, "option '" << space << "."
                      << "name" << "', comprising an array of data"
                      << " fields may not encapsulate any option space ("
                      << option_def->getPosition() << ")");

        } else if (encapsulates == space) {
            isc_throw(DhcpConfigError, "option must not encapsulate"
                      << " an option space it belongs to: '"
                      << space << "." << name << "' is set to"
                      << " encapsulate '" << space << "' ("
                      << option_def->getPosition() << ")");

        } else {
            def.reset(new OptionDefinition(name, code, type,
                        encapsulates.c_str()));
        }

    } else {
        def.reset(new OptionDefinition(name, code, type, array_type));

    }

    if (user_context) {
        def->setContext(user_context);
    }

    // Split the list of record types into tokens.
    std::vector<std::string> record_tokens =
    isc::util::str::tokens(record_types, ",");
    // Iterate over each token and add a record type into
    // option definition.
    BOOST_FOREACH(std::string record_type, record_tokens) {
        try {
            boost::trim(record_type);
            if (!record_type.empty()) {
                    def->addRecordField(record_type);
            }
        } catch (const Exception& ex) {
            isc_throw(DhcpConfigError, "invalid record type values"
                      << " specified for the option definition: "
                      << ex.what() << " ("
                      << getPosition("record-types", option_def) << ")");
        }
    }

    // Validate the definition.
    try {
        def->validate();
    } catch (const std::exception& ex) {
        isc_throw(DhcpConfigError, ex.what()
                  << " (" << option_def->getPosition() << ")");
    }

    // Option definition has been created successfully.
    return make_pair(def, space);
}

// ******************************** OptionDefListParser ************************

OptionDefListParser::OptionDefListParser(const uint16_t address_family)
    : address_family_(address_family) {
}

void
OptionDefListParser::parse(CfgOptionDefPtr storage, ElementPtr option_def_list) {
    if (!option_def_list) {
        // Sanity check: not supposed to fail.
        isc_throw(DhcpConfigError, "parser error: a pointer to a list of"
                  << " option definitions is NULL ("
                  << option_def_list->getPosition() << ")");
    }

    OptionDefParser parser(address_family_);
    BOOST_FOREACH(ElementPtr option_def, option_def_list->listValue()) {
        OptionDefinitionTuple def;

        def = parser.parse(option_def);
        try {
            storage->add(def.first, def.second);
        } catch (const std::exception& ex) {
            // Append position if there is a failure.
            isc_throw(DhcpConfigError, ex.what() << " ("
                      << option_def->getPosition() << ")");
        }
    }

    // All definitions have been prepared. Put them as runtime options into
    // the libdhcp++.
    LibDHCP::setRuntimeOptionDefs(storage->getContainer());
}

//****************************** RelayInfoParser ********************************
RelayInfoParser::RelayInfoParser(const Option::Universe& family)
    : family_(family) {
}

void
RelayInfoParser::parse(const isc::dhcp::Network::RelayInfoPtr& relay_info,
                       ElementPtr relay_elem) {

    if (relay_elem->getType() != Element::map) {
        isc_throw(DhcpConfigError, "relay must be a map");
    }

    ElementPtr address = relay_elem->get("ip-address");
    ElementPtr addresses = relay_elem->get("ip-addresses");

    if (address && addresses) {
        isc_throw(DhcpConfigError,
            "specify either ip-address or ip-addresses, not both");
    }

    if (!address && !addresses) {
        isc_throw(DhcpConfigError, "ip-addresses is required");
    }

    // Create our resultant RelayInfo structure
    *relay_info = isc::dhcp::Network::RelayInfo();

    if (address) {
        addAddress("ip-address", getString(relay_elem, "ip-address"),
                   relay_elem, relay_info);
        LOG_DEBUG(dhcpsrv_logger, DHCPSRV_DBG_TRACE_DETAIL,
                  DHCPSRV_CFGMGR_RELAY_IP_ADDRESS_DEPRECATED)
                  .arg(getPosition("ip-address", relay_elem));
        return;
    }

    if (addresses->getType() != Element::list) {
        isc_throw(DhcpConfigError, "ip-addresses must be a list "
                  << " (" << getPosition("ip-addresses", relay_elem) << ")");
    }

    BOOST_FOREACH(ElementPtr address_element, addresses->listValue()) {
        addAddress("ip-addresses", address_element->stringValue(),
                   relay_elem, relay_info);
    }
}

void
RelayInfoParser::addAddress(const std::string& name,
                            const std::string& address_str,
                            ElementPtr relay_elem,
                            const isc::dhcp::Network::RelayInfoPtr& relay_info) {
    std::unique_ptr<isc::asiolink::IOAddress> ip;
    try {
        ip.reset(new isc::asiolink::IOAddress(address_str));
    } catch (const std::exception& ex) {
        isc_throw(DhcpConfigError, "address " << address_str
                  << " is not a valid: "
                  << (family_ == Option::V4 ? "IPv4" : "IPv6")
                  << "address"
                  << " (" << getPosition(name, relay_elem) << ")");
    }

    // Check if the address family matches.
    if ((ip->isV4() && family_ != Option::V4) ||
        (ip->isV6() && family_ != Option::V6) ) {
        isc_throw(DhcpConfigError, "address " << address_str
                  << " is not a: "
                  << (family_ == Option::V4 ? "IPv4" : "IPv6")
                  << "address"
                  << " (" << getPosition(name, relay_elem) << ")");
    }

    try {
        relay_info->addAddress(*ip);
    } catch (const std::exception& ex) {
        isc_throw(DhcpConfigError, "cannot add address: " << address_str
                  << "to relay info: " << ex.what()
                  << " (" << getPosition(name, relay_elem) << ")");
    }
}

//****************************** PoolParser ********************************

void
PoolParser::parse(PoolStoragePtr pools,
                  ElementPtr pool_structure,
                  const uint16_t address_family) {

    if (address_family == AF_INET) {
        checkKeywords(SimpleParser4::POOL4_PARAMETERS, pool_structure);
    } else {
        checkKeywords(SimpleParser6::POOL6_PARAMETERS, pool_structure);
    }

    ElementPtr text_pool = pool_structure->get("pool");

    ElementPtr const& max_address_count(pool_structure->get("max-address-utilization"));

    if (!text_pool) {
        isc_throw(DhcpConfigError, "Mandatory 'pool' entry missing in "
                  "definition: (" << pool_structure->getPosition() << ")");
    }

    // That should be a single pool representation. It should contain
    // text is form prefix/len or first - last. Note that spaces
    // are allowed
    string txt = text_pool->stringValue();

    // first let's remove any whitespaces
    isc::util::str::erase_all(txt, ' '); // space
    isc::util::str::erase_all(txt, '\t'); // tabulation

    PoolPtr pool;

    // Is this prefix/len notation?
    size_t pos = txt.find("/");
    if (pos != string::npos) {
        isc::asiolink::IOAddress addr("::");
        uint8_t len = 0;
        try {
            addr = isc::asiolink::IOAddress(txt.substr(0, pos));

            // start with the first character after /
            string prefix_len = txt.substr(pos + 1);

            // It is lexical cast to int and then downcast to uint8_t.
            // Direct cast to uint8_t (which is really an unsigned char)
            // will result in interpreting the first digit as output
            // value and throwing exception if length is written on two
            // digits (because there are extra characters left over).

            // No checks for values over 128. Range correctness will
            // be checked in Pool4 constructor, here we only check
            // the representation fits in an uint8_t as this can't
            // be done by a direct lexical cast as explained...
            int val_len = boost::lexical_cast<int>(prefix_len);
            if ((val_len < std::numeric_limits<uint8_t>::min()) ||
                (val_len > std::numeric_limits<uint8_t>::max())) {
                // This exception will be handled 4 line later!
                isc_throw(OutOfRange, "");
            }
            len = static_cast<uint8_t>(val_len);
        } catch (...)  {
            isc_throw(DhcpConfigError, "Failed to parse pool "
                      "definition: " << txt << " ("
                      << text_pool->getPosition() << ")");
        }

        try {
            if (max_address_count) {
                pool = poolMaker(addr, len, 0, max_address_count->intValue());
            } else {
                pool = poolMaker(addr, len);
            }
            pools->push_back(pool);
        } catch (OutOfRange const&) {
            // 0% threshold
            return;
        } catch (const std::exception& ex) {
            isc_throw(DhcpConfigError, "Failed to create pool defined by: "
                                           << txt << " (" << text_pool->getPosition() << "): "
                                           << ex.what());
        }

    } else {
        isc::asiolink::IOAddress min("::");
        isc::asiolink::IOAddress max("::");

        // Is this min-max notation?
        pos = txt.find("-");
        if (pos != string::npos) {
            // using min-max notation
            try {
                min = isc::asiolink::IOAddress(txt.substr(0, pos));
                max = isc::asiolink::IOAddress(txt.substr(pos + 1));
            } catch (...)  {
                isc_throw(DhcpConfigError, "Failed to parse pool "
                          "definition: " << txt << " ("
                          << text_pool->getPosition() << ")");
            }

            try {
                if (max_address_count) {
                    pool = poolMaker(min, max, 0, max_address_count->intValue());
                } else {
                    pool = poolMaker(min, max);
                }
                pools->push_back(pool);
            } catch (OutOfRange const&) {
                // 0% threshold
                return;
            } catch (const std::exception& ex) {
                isc_throw(DhcpConfigError, "Failed to create pool defined by: "
                                               << txt << " (" << text_pool->getPosition() << "): "
                                               << ex.what());
            }
        }
    }

    if (!pool) {
        isc_throw(DhcpConfigError, "invalid pool definition: "
                  << text_pool->stringValue() <<
                  ". There are two acceptable formats <min address-max address>"
                  " or <prefix/len> ("
                  << text_pool->getPosition() << ")");
    }

    // If there's user-context specified, store it.
    ElementPtr user_context = pool_structure->get("user-context");
    if (user_context) {
        // The grammar accepts only maps but still check it.
        if (user_context->getType() != Element::map) {
            isc_throw(isc::dhcp::DhcpConfigError, "User context has to be a map ("
                      << user_context->getPosition() << ")");
        }
        pool->setContext(user_context);
    }

    // Parser pool specific options.
    ElementPtr option_data = pool_structure->get("option-data");
    if (option_data) {
        try {
            CfgOptionPtr cfg = pool->getCfgOption();
            OptionDataListParser option_parser(address_family);
            option_parser.parse(cfg, option_data);
        } catch (const std::exception& ex) {
            isc_throw(isc::dhcp::DhcpConfigError, ex.what()
                      << " (" << option_data->getPosition() << ")");
        }
    }

    // Client-class.
    ElementPtr client_class = pool_structure->get("client-class");
    if (client_class) {
        string cclass = client_class->stringValue();
        if (!cclass.empty()) {
            pool->allowClientClass(cclass);
        }
    }

    // Try setting up required client classes.
    ElementPtr class_list = pool_structure->get("require-client-classes");
    if (class_list) {
        const std::vector<data::ElementPtr>& classes = class_list->listValue();
        for (auto cclass = classes.cbegin();
             cclass != classes.cend(); ++cclass) {
            if (((*cclass)->getType() != Element::string) ||
                (*cclass)->stringValue().empty()) {
                isc_throw(DhcpConfigError, "invalid class name ("
                          << (*cclass)->getPosition() << ")");
            }
            pool->requireClientClass((*cclass)->stringValue());
        }
    }

    ElementPtr pool_ID(pool_structure->get("id"));
    if (pool_ID) {
        pool->setId(pool_ID->intValue());
    }
}

//****************************** Pool4Parser *************************

PoolPtr
Pool4Parser::poolMaker(IOAddress& addr, uint32_t len, int32_t, int8_t const threshold /* = 100 */) {
    return std::make_shared<Pool4>(addr, len, threshold);
}

PoolPtr Pool4Parser::poolMaker(IOAddress& min,
                               IOAddress& max,
                               int32_t,
                               int8_t const threshold /* = 100 */) {
    return std::make_shared<Pool4>(min, max, threshold);
}

//****************************** Pool4ListParser *************************

void
Pools4ListParser::parse(PoolStoragePtr pools, ElementPtr pools_list) {
    BOOST_FOREACH(ElementPtr pool, pools_list->listValue()) {
        Pool4Parser parser;
        parser.parse(pools, pool, AF_INET);
    }
}

//****************************** SubnetConfigParser *************************

SubnetConfigParser::SubnetConfigParser(uint16_t family, bool check_iface)
    : pools_(new PoolStorage()),
      address_family_(family),
      options_(new CfgOption()),
      check_iface_(check_iface) {
    relay_info_.reset(new isc::dhcp::Network::RelayInfo());
}

SubnetPtr
SubnetConfigParser::parse(ElementPtr subnet) {

    ElementPtr options_params = subnet->get("option-data");
    if (options_params) {
        OptionDataListParser opt_parser(address_family_);
        opt_parser.parse(options_, options_params);
    }

    ElementPtr relay_params = subnet->get("relay");
    if (relay_params) {
        Option::Universe u = (address_family_ == AF_INET) ? Option::V4 : Option::V6;
        RelayInfoParser parser(u);
        parser.parse(relay_info_, relay_params);
    }

    // Create a subnet.
    try {
        createSubnet(subnet);
    } catch (const std::exception& ex) {
        isc_throw(DhcpConfigError,
                  "subnet configuration failed: " << ex.what());
    }

    return (subnet_);
}

Subnet::AllocationType
SubnetConfigParser::allocationTypeFromText(const std::string& txt) {
    if (txt.compare("default") == 0) {
        return (Subnet::ALLOCATION_TYPE_DEFAULT);
    } else if (txt.compare("privacy") == 0) {
        return (Subnet::ALLOCATION_TYPE_PRIVACY);
    } else {
        isc_throw(BadValue, "Can't convert '" << txt
                  << "' into any valid allocation-type values");
    }
}

Network::HRMode
SubnetConfigParser::hrModeFromText(const std::string& txt) {
    if ( (txt.compare("disabled") == 0) ||
         (txt.compare("off") == 0) )  {
        return (Network::HR_DISABLED);
    } else if (txt.compare("out-of-pool") == 0) {
        return (Network::HR_OUT_OF_POOL);
    } else if (txt.compare("global") == 0) {
        return (Network::HR_GLOBAL);
    } else if (txt.compare("all") == 0) {
        return (Network::HR_ALL);
    } else {
        // Should never happen...
        isc_throw(BadValue, "Can't convert '" << txt
                  << "' into any valid reservation-mode values");
    }
}

void
SubnetConfigParser::createSubnet(ElementPtr params) {
    std::string subnet_txt;
    try {
        subnet_txt = getString(params, "subnet");
    } catch (const DhcpConfigError &) {
        // rethrow with precise error
        isc_throw(DhcpConfigError,
                 "mandatory 'subnet' parameter is missing for a subnet being"
                  " configured (" << params->getPosition() << ")");
    }

    // Remove any spaces or tabs.
    isc::util::str::erase_all(subnet_txt, ' ');
    isc::util::str::erase_all(subnet_txt, '\t');

    // The subnet format is prefix/len. We are going to extract
    // the prefix portion of a subnet string to create IOAddress
    // object from it. IOAddress will be passed to the Subnet's
    // constructor later on. In order to extract the prefix we
    // need to get all characters preceding "/".
    size_t pos = subnet_txt.find("/");
    if (pos == string::npos) {
        ElementPtr elem = params->get("subnet");
        isc_throw(DhcpConfigError,
                  "Invalid subnet syntax (prefix/len expected):" << subnet_txt
                  << " (" << elem->getPosition() << ")");
    }

    // Try to create the address object. It also validates that
    // the address syntax is ok.
    isc::asiolink::IOAddress addr(subnet_txt.substr(0, pos));

    // Now parse out the prefix length.
    unsigned int len;
    try {
        len = boost::lexical_cast<unsigned int>(subnet_txt.substr(pos + 1));
    } catch (const boost::bad_lexical_cast&) {
        ElementPtr elem = params->get("subnet");
        isc_throw(DhcpConfigError, "prefix length: '" <<
                  subnet_txt.substr(pos+1) << "' is not an integer ("
                  << elem->getPosition() << ")");
    }

    // Sanity check the prefix length
    if ((addr.isV6() && len > 128) ||
        (addr.isV4() && len > 32)) {
        ElementPtr elem = params->get("subnet");
        isc_throw(BadValue,
                  "Invalid prefix length specified for subnet: " << len
                  << " (" <<  elem->getPosition() << ")");
    }

    // Call the subclass's method to instantiate the subnet
    initSubnet(params, addr, len);

    // Add pools to it.
    for (PoolStorage::iterator it = pools_->begin(); it != pools_->end();
         ++it) {
        try {
            subnet_->addPool(*it);
        } catch (const BadValue& ex) {
            // addPool() can throw BadValue if the pool is overlapping or
            // is out of bounds for the subnet.
            isc_throw(DhcpConfigError,
                      ex.what() << " (" << params->getPosition() << ")");
        }
    }

    // Configure allocation type, if defined

    // Get allocation type. If it is defined, then the addresses from this
    // subnet will be allocated according to the allocation type.
    std::string allocationType;
    try {
        allocationType = getString(params, "allocation-type");
        subnet_->setAllocationType(allocationTypeFromText(allocationType));
    } catch (const DhcpConfigError &) {
        // allocation type not mandatory so swallow the exception
    }

    // If there's user-context specified, store it.
    ElementPtr user_context = params->get("user-context");
    if (user_context) {
        // The grammar accepts only maps but still check it.
        if (user_context->getType() != Element::map) {
            isc_throw(isc::dhcp::DhcpConfigError, "User context has to be a map ("
                      << user_context->getPosition() << ")");
        }
        subnet_->setContext(user_context);
    }

    // In order to take advantage of the dynamic inheritance of global
    // parameters to a subnet we need to set a callback function for each
    // subnet to allow for fetching global parameters.
    subnet_->setFetchGlobalsFn([]() -> ElementPtr {
        return (CfgMgr::instance().getCurrentCfg()->getConfiguredGlobals());
    });

}

//****************************** Subnet4ConfigParser *************************

Subnet4ConfigParser::Subnet4ConfigParser(bool check_iface)
    : SubnetConfigParser(AF_INET, check_iface) {
}

Subnet4Ptr
Subnet4ConfigParser::parse(ElementPtr subnet) {
    // Check parameters.
    checkKeywords(SimpleParser4::SUBNET4_PARAMETERS, subnet);

    /// Parse Pools first.
    ElementPtr pools = subnet->get("pools");
    if (pools) {
        Pools4ListParser parser;
        parser.parse(pools_, pools);
    }

    SubnetPtr generic = SubnetConfigParser::parse(subnet);

    if (!generic) {
        // Sanity check: not supposed to fail.
        isc_throw(DhcpConfigError,
                  "Failed to create an IPv4 subnet (" <<
                  subnet->getPosition() << ")");
    }

    Subnet4Ptr sn4ptr = std::dynamic_pointer_cast<Subnet4>(subnet_);
    if (!sn4ptr) {
        // If we hit this, it is a programming error.
        isc_throw(Unexpected,
                  "Invalid Subnet4 cast in Subnet4ConfigParser::parse");
    }

    // Set relay information if it was parsed
    if (relay_info_) {
        sn4ptr->setRelayInfo(*relay_info_);
    }

    // Parse Host Reservations for this subnet if any.
    ElementPtr reservations = subnet->get("reservations");
    if (reservations) {
        HostCollection hosts;
        HostReservationsListParser<HostReservationParser4> parser;
        parser.parse(subnet_->getID(), reservations, hosts);
        for (auto h = hosts.begin(); h != hosts.end(); ++h) {
            validateResv(sn4ptr, *h);
            CfgMgr::instance().getStagingCfg()->getCfgHosts()->add(*h);
        }
    }

    return (sn4ptr);
}

void
Subnet4ConfigParser::initSubnet(data::ElementPtr params,
                                asiolink::IOAddress addr, uint8_t len) {
    // Subnet ID is optional. If it is not supplied the value of 0 is used,
    // which means autogenerate. The value was inserted earlier by calling
    // SimpleParser4::setAllDefaults.
    SubnetID subnet_id = static_cast<SubnetID>(getInteger(params, "id"));

    Subnet4Ptr subnet4(new Subnet4(addr, len, Triplet<uint32_t>(),
                                   Triplet<uint32_t>(), Triplet<uint32_t>(),
                                   subnet_id));
    subnet_ = subnet4;

    // Parse parameters common to all Network derivations.
    NetworkPtr network = std::dynamic_pointer_cast<Network>(subnet4);
    parseCommon(params, network);

    std::ostringstream output;
    output << addr << "/" << static_cast<int>(len) << " with params: ";

    bool has_renew = !subnet4->getT1().unspecified();
    bool has_rebind = !subnet4->getT2().unspecified();
    int64_t renew = -1;
    int64_t rebind = -1;

    // t1 and t2 are optional may be not specified.
    if (has_renew) {
        renew = subnet4->getT1().get();
        output << "t1=" << renew << ", ";
    }
    if (has_rebind) {
        rebind = subnet4->getT2().get();
        output << "t2=" << rebind << ", ";
    }

    if (has_renew && has_rebind && (renew > rebind)) {
        isc_throw(DhcpConfigError, "the value of renew-timer" << " (" << renew
                  << ") is greater than the value of rebind-timer" << " ("
                  << rebind << ")");
    }

    if (!subnet4->getValid().unspecified()) {
        output << "valid-lifetime=" << subnet4->getValid().get();
    }

    LOG_INFO(dhcpsrv_logger, DHCPSRV_CFGMGR_NEW_SUBNET4).arg(output.str());

    // Set the match-client-id value for the subnet.
    if (params->contains("match-client-id")) {
        bool match_client_id = getBoolean(params, "match-client-id");
        subnet4->setMatchClientId(match_client_id);
    }

    // Set the authoritative value for the subnet.
    if (params->contains("authoritative")) {
        bool authoritative = getBoolean(params, "authoritative");
        subnet4->setAuthoritative(authoritative);
    }

    // Set next-server. The default value is 0.0.0.0. Nevertheless, the
    // user could have messed that up by specifying incorrect value.
    // To avoid using 0.0.0.0, user can specify "".
    if (params->contains("next-server")) {
        string next_server;
        try {
            next_server = getString(params, "next-server");
            if (!next_server.empty()) {
                subnet4->setSiaddr(IOAddress(next_server));
            }
        } catch (...) {
            ElementPtr next = params->get("next-server");
            string pos;
            if (next) {
                pos = next->getPosition().str();
            } else {
                pos = params->getPosition().str();
            }
            isc_throw(DhcpConfigError, "invalid parameter next-server : "
                      << next_server << "(" << pos << ")");
        }
    }

    // Set server-hostname.
    if (params->contains("server-hostname")) {
        std::string sname = getString(params, "server-hostname");
        if (!sname.empty()) {
            if (sname.length() >= Pkt4::MAX_SNAME_LEN) {
                ElementPtr error = params->get("server-hostname");
                isc_throw(DhcpConfigError, "server-hostname must be at most "
                          << Pkt4::MAX_SNAME_LEN - 1 << " bytes long, it is "
                          << sname.length() << " ("
                          << error->getPosition() << ")");
            }
            subnet4->setSname(sname);
        }
    }

    // Set boot-file-name.
    if (params->contains("boot-file-name")) {
        std::string filename =getString(params, "boot-file-name");
        if (!filename.empty()) {
            if (filename.length() > Pkt4::MAX_FILE_LEN) {
                ElementPtr error = params->get("boot-file-name");
                isc_throw(DhcpConfigError, "boot-file-name must be at most "
                          << Pkt4::MAX_FILE_LEN - 1 << " bytes long, it is "
                          << filename.length() << " ("
                          << error->getPosition() << ")");
            }
            subnet4->setFilename(filename);
        }
    }

    // Get interface name. If it is defined, then the subnet is available
    // directly over specified network interface.
    if (params->contains("interface")) {
        std::string iface = getString(params, "interface");
        if (!iface.empty()) {
            if (IfaceMgr::instance().isServerMode() && check_iface_ &&
                !IfaceMgr::instance().getIface(iface)) {
                ElementPtr error(params->get("interface"));
                isc_throw(DhcpConfigError, "Specified network interface name " << iface
                          << " for subnet " << subnet4->toText()
                          << " is not present in the system ("
                          << error->getPosition() << ")");
            }
            subnet4->setIface(iface);
        }
    }

    // Let's set host reservation mode.
    parseHostReservationMode(params, network);

    // Try setting up client class.
    if (params->contains("client-class")) {
        string client_class = getString(params, "client-class");
        if (!client_class.empty()) {
            subnet4->allowClientClass(client_class);
        }
    }

    // Try setting up required client classes.
    ElementPtr class_list = params->get("require-client-classes");
    if (class_list) {
        const std::vector<data::ElementPtr>& classes = class_list->listValue();
        for (auto cclass = classes.cbegin();
             cclass != classes.cend(); ++cclass) {
            if (((*cclass)->getType() != Element::string) ||
                (*cclass)->stringValue().empty()) {
                isc_throw(DhcpConfigError, "invalid class name ("
                          << (*cclass)->getPosition() << ")");
            }
            subnet4->requireClientClass((*cclass)->stringValue());
        }
    }

    // 4o6 specific parameter: 4o6-interface.
    if (params->contains("4o6-interface")) {
        string iface4o6 = getString(params, "4o6-interface");
        if (!iface4o6.empty()) {
            subnet4->get4o6().setIface4o6(iface4o6);
            subnet4->get4o6().enabled(true);
        }
    }

    // 4o6 specific parameter: 4o6-subnet.
    if (params->contains("4o6-subnet")) {
        string subnet4o6 = getString(params, "4o6-subnet");
        if (!subnet4o6.empty()) {
            size_t slash = subnet4o6.find("/");
            if (slash == std::string::npos) {
                isc_throw(DhcpConfigError, "Missing / in the 4o6-subnet parameter:"
                          << subnet4o6 << ", expected format: prefix6/length");
            }
            string prefix = subnet4o6.substr(0, slash);
            string lenstr = subnet4o6.substr(slash + 1);

            uint8_t len = 128;
            try {
                len = boost::lexical_cast<unsigned int>(lenstr.c_str());
            } catch (const boost::bad_lexical_cast &) {
                isc_throw(DhcpConfigError, "Invalid prefix length specified in "
                          "4o6-subnet parameter: " << subnet4o6 << ", expected 0..128 value");
            }
            subnet4->get4o6().setSubnet4o6(IOAddress(prefix), len);
            subnet4->get4o6().enabled(true);
        }
    }

    // Try 4o6 specific parameter: 4o6-interface-id
    if (params->contains("4o6-interface-id")) {
        std::string ifaceid = getString(params, "4o6-interface-id");
        if (!ifaceid.empty()) {
            OptionBuffer tmp(ifaceid.begin(), ifaceid.end());
            OptionPtr opt(new Option(Option::V6, D6O_INTERFACE_ID, tmp));
            subnet4->get4o6().setInterfaceId(opt);
            subnet4->get4o6().enabled(true);
        }
    }

    /// client-class processing is now generic and handled in the common
    /// code (see isc::data::SubnetConfigParser::createSubnet)

    // address plus port specific parameter: v4-psid-offset. If not explicitly specified,
    // it will have the default value of "0".
    if (params->contains("v4-psid-offset")) {
        uint8_t const psid_offset = getInteger(params, "v4-psid-offset");
        if (psid_offset) {
            subnet4->get4o6().setPsidOffset(psid_offset);
            subnet4->get4o6().enabled(true);
        }
    }

    // address plus port specific parameter: v4-psid-len. If not explicitly
    // specified,
    // it will have the default value of "0".
    if (params->contains("v4-psid-len")) {
        uint8_t psid_len = getInteger(params, "v4-psid-len");
        if (psid_len) {
            subnet4->get4o6().setPsidLen(psid_len);
            subnet4->get4o6().enabled(true);
        }
    }


    if (params->contains("v4-excluded-psids")) {
        ElementPtr excluded_psids = params->get("v4-excluded-psids");
        if (excluded_psids) {
            PSIDContainer excludedPSIDs;
            BOOST_FOREACH (ElementPtr psid, excluded_psids->listValue()) {
                int64_t value;
                psid->getValue(value);
                excludedPSIDs.insert(static_cast<uint16_t>(value));
            }
            subnet4->setExcludedPSIDs(excludedPSIDs);
        }
    }

    // Here globally defined options were merged to the subnet specific
    // options but this is no longer the case (they have a different
    // and not consecutive priority).

    // Copy options to the subnet configuration.
    options_->copyTo(*subnet4->getCfgOption());

    // Parse t1-percent and t2-percent
    parseTeePercents(params, network);

    // Parse DDNS parameters
    parseDdnsParams(params, network);
}

void
Subnet4ConfigParser::validateResv(const Subnet4Ptr& subnet, ConstHostPtr host) {
    const IOAddress& address = host->getIPv4Reservation();
    if (!address.isV4Zero() && !subnet->inRange(address)) {
        isc_throw(DhcpConfigError, "specified reservation '" << address
                  << "' is not within the IPv4 subnet '"
                  << subnet->toText() << "'");
    }
}

//**************************** Subnets4ListConfigParser **********************

Subnets4ListConfigParser::Subnets4ListConfigParser(bool check_iface)
    : check_iface_(check_iface) {
}

size_t
Subnets4ListConfigParser::parse(SrvConfigPtr cfg, ElementPtr subnets_list) {
    size_t cnt = 0;
    BOOST_FOREACH(ElementPtr subnet_json, subnets_list->listValue()) {

        Subnet4ConfigParser parser(check_iface_);
        Subnet4Ptr subnet = parser.parse(subnet_json);
        if (subnet) {

            // Adding a subnet to the Configuration Manager may fail if the
            // subnet id is invalid (duplicate). Thus, we catch exceptions
            // here to append a position in the configuration string.
            try {
                cfg->getCfgSubnets4()->add(subnet);
                cnt++;
            } catch (const std::exception& ex) {
                isc_throw(DhcpConfigError, ex.what() << " ("
                          << subnet_json->getPosition() << ")");
            }
        }
    }
    return (cnt);
}

size_t
Subnets4ListConfigParser::parse(Subnet4Collection& subnets,
                                data::ElementPtr subnets_list) {
    size_t cnt = 0;
    BOOST_FOREACH(ElementPtr subnet_json, subnets_list->listValue()) {

        Subnet4ConfigParser parser(check_iface_);
        Subnet4Ptr subnet = parser.parse(subnet_json);
        if (subnet) {
            try {
                auto ret = subnets.insert(subnet);
                if (!ret.second) {
                    isc_throw(Unexpected,
                              "can't store subnet because of conflict");
                }
                ++cnt;
            } catch (const std::exception& ex) {
                isc_throw(DhcpConfigError, ex.what() << " ("
                          << subnet_json->getPosition() << ")");
            }
        }
    }
    return (cnt);
}


//**************************** Pool6Parser *********************************

PoolPtr Pool6Parser::poolMaker(IOAddress& addr,
                               uint32_t len,
                               int32_t ptype,
                               int8_t const threshold /* = 100 */) {
    return std::make_shared<Pool6>(static_cast<isc::dhcp::Lease::Type>(ptype), addr, len, 128,
                                   threshold);
}

PoolPtr Pool6Parser::poolMaker(IOAddress& min,
                               IOAddress& max,
                               int32_t ptype,
                               int8_t const threshold /* = 100 */) {
    return std::make_shared<Pool6>(static_cast<isc::dhcp::Lease::Type>(ptype), min, max, threshold);
}

//**************************** Pool6ListParser ***************************

void
Pools6ListParser::parse(PoolStoragePtr pools, ElementPtr pools_list) {
    BOOST_FOREACH(ElementPtr pool, pools_list->listValue()) {
        Pool6Parser parser;
        parser.parse(pools, pool, AF_INET6);
    }
}

//**************************** PdPoolParser ******************************

PdPoolParser::PdPoolParser() : options_(new CfgOption()) {
}

void
PdPoolParser::parse(PoolStoragePtr pools, ElementPtr pd_pool_) {
    checkKeywords(SimpleParser6::PD_POOL6_PARAMETERS, pd_pool_);

    std::string addr_str = getString(pd_pool_, "prefix");

    uint8_t prefix_len = getUint8(pd_pool_, "prefix-len");

    uint8_t delegated_len = getUint8(pd_pool_, "delegated-len");

    std::string excluded_prefix_str = "::";
    if (pd_pool_->contains("excluded-prefix")) {
        excluded_prefix_str = getString(pd_pool_, "excluded-prefix");
    }

    uint8_t excluded_prefix_len = 0;
    if (pd_pool_->contains("excluded-prefix-len")) {
        excluded_prefix_len = getUint8(pd_pool_, "excluded-prefix-len");
    }

    ElementPtr option_data = pd_pool_->get("option-data");
    if (option_data) {
        OptionDataListParser opts_parser(AF_INET6);
        opts_parser.parse(options_, option_data);
    }

    ElementPtr user_context = pd_pool_->get("user-context");
    if (user_context) {
        user_context_ = user_context;
    }

    ElementPtr client_class = pd_pool_->get("client-class");
    if (client_class) {
        client_class_ = client_class;
    }

    ElementPtr class_list = pd_pool_->get("require-client-classes");

    ElementPtr const& max_pd_space_utilization(pd_pool_->get("max-pd-space-utilization"));

    // Check the pool parameters. It will throw an exception if any
    // of the required parameters are invalid.
    try {
        // Attempt to construct the local pool.
        if (max_pd_space_utilization) {
            pool_.reset(new Pool6(IOAddress(addr_str), prefix_len, delegated_len,
                                  IOAddress(excluded_prefix_str), excluded_prefix_len,
                                  max_pd_space_utilization->intValue()));
        } else {
            pool_.reset(new Pool6(IOAddress(addr_str), prefix_len, delegated_len,
                                  IOAddress(excluded_prefix_str), excluded_prefix_len));
        }
        // Merge options specified for a pool into pool configuration.
        options_->copyTo(*pool_->getCfgOption());
    } catch (const std::exception& ex) {
        // Some parameters don't exist or are invalid. Since we are not
        // aware whether they don't exist or are invalid, let's append
        // the position of the pool map element.
        isc_throw(isc::dhcp::DhcpConfigError, ex.what()
                  << " (" << pd_pool_->getPosition() << ")");
    }

    if (user_context_) {
        pool_->setContext(user_context_);
    }

    if (client_class_) {
        string cclass = client_class_->stringValue();
        if (!cclass.empty()) {
            pool_->allowClientClass(cclass);
        }
    }

    if (class_list) {
        const std::vector<data::ElementPtr>& classes = class_list->listValue();
        for (auto cclass = classes.cbegin();
             cclass != classes.cend(); ++cclass) {
            if (((*cclass)->getType() != Element::string) ||
                (*cclass)->stringValue().empty()) {
                isc_throw(DhcpConfigError, "invalid class name ("
                          << (*cclass)->getPosition() << ")");
            }
            pool_->requireClientClass((*cclass)->stringValue());
        }
    }

    ElementPtr pool_ID(pd_pool_->get("id"));
    if (pool_ID) {
        pool_->setId(pool_ID->intValue());
    }

    // Add the local pool to the external storage ptr.
    pools->push_back(pool_);
}

//**************************** PdPoolsListParser ************************

void
PdPoolsListParser::parse(PoolStoragePtr pools, ElementPtr pd_pool_list) {
    // Loop through the list of pd pools.
    BOOST_FOREACH(ElementPtr pd_pool, pd_pool_list->listValue()) {
        PdPoolParser parser;
        parser.parse(pools, pd_pool);
    }
}

//**************************** Subnet6ConfigParser ***********************

Subnet6ConfigParser::Subnet6ConfigParser(bool check_iface)
    : SubnetConfigParser(AF_INET6, check_iface) {
}

Subnet6Ptr
Subnet6ConfigParser::parse(ElementPtr subnet) {
    // Check parameters.
    checkKeywords(SimpleParser6::SUBNET6_PARAMETERS, subnet);

    /// Parse all pools first.
    ElementPtr pools = subnet->get("pools");
    if (pools) {
        Pools6ListParser parser;
        parser.parse(pools_, pools);
    }
    ElementPtr pd_pools = subnet->get("pd-pools");
    if (pd_pools) {
        PdPoolsListParser parser;
        parser.parse(pools_, pd_pools);
    }

    SubnetPtr generic = SubnetConfigParser::parse(subnet);

    if (!generic) {
        // Sanity check: not supposed to fail.
        isc_throw(DhcpConfigError,
                  "Failed to create an IPv6 subnet (" <<
                  subnet->getPosition() << ")");
    }

    Subnet6Ptr sn6ptr = std::dynamic_pointer_cast<Subnet6>(subnet_);
    if (!sn6ptr) {
        // If we hit this, it is a programming error.
        isc_throw(Unexpected,
                  "Invalid Subnet6 cast in Subnet6ConfigParser::parse");
    }

    // Set relay information if it was provided
    if (relay_info_) {
        sn6ptr->setRelayInfo(*relay_info_);
    }

    // Parse Host Reservations for this subnet if any.
    ElementPtr reservations = subnet->get("reservations");
    if (reservations) {
        HostCollection hosts;
        HostReservationsListParser<HostReservationParser6> parser;
        parser.parse(subnet_->getID(), reservations, hosts);
        for (auto h = hosts.begin(); h != hosts.end(); ++h) {
            validateResvs(sn6ptr, *h);
            CfgMgr::instance().getStagingCfg()->getCfgHosts()->add(*h);
        }
    }

    return (sn6ptr);
}

// Unused?
void
Subnet6ConfigParser::duplicate_option_warning(uint32_t code,
                                              asiolink::IOAddress& addr) {
    LOG_WARN(dhcpsrv_logger, DHCPSRV_CFGMGR_OPTION_DUPLICATE)
        .arg(code).arg(addr.toText());
}

void
Subnet6ConfigParser::initSubnet(data::ElementPtr params,
                                asiolink::IOAddress addr, uint8_t len) {
    // Subnet ID is optional. If it is not supplied the value of 0 is used,
    // which means autogenerate. The value was inserted earlier by calling
    // SimpleParser6::setAllDefaults.
    SubnetID subnet_id = static_cast<SubnetID>(getInteger(params, "id"));

    // We want to log whether rapid-commit is enabled, so we get this
    // before the actual subnet creation.
    Optional<bool> rapid_commit;
    if (params->contains("rapid-commit")) {
        rapid_commit = getBoolean(params, "rapid-commit");
    }

    // Parse preferred lifetime as it is not parsed by the common function.
    Triplet<uint32_t> pref = parseLifetime(params, "preferred-lifetime");

    // Create a new subnet.
    Subnet6* subnet6 = new Subnet6(addr, len, Triplet<uint32_t>(),
                                   Triplet<uint32_t>(),
                                   pref,
                                   Triplet<uint32_t>(),
                                   subnet_id);
    subnet_.reset(subnet6);

    // Parse parameters common to all Network derivations.
    NetworkPtr network = std::dynamic_pointer_cast<Network>(subnet_);
    parseCommon(params, network);

    // Enable or disable Rapid Commit option support for the subnet.
    if (!rapid_commit.unspecified()) {
        subnet6->setRapidCommit(rapid_commit);
    }

    std::ostringstream output;
    output << addr << "/" << static_cast<int>(len) << " with params: ";
    // t1 and t2 are optional may be not specified.

    bool has_renew = !subnet6->getT1().unspecified();
    bool has_rebind = !subnet6->getT2().unspecified();
    int64_t renew = -1;
    int64_t rebind = -1;

    if (has_renew) {
        renew = subnet6->getT1().get();
        output << "t1=" << renew << ", ";
    }
    if (has_rebind) {
        rebind = subnet6->getT2().get();
        output << "t2=" << rebind << ", ";
    }

    if (has_renew && has_rebind && (renew > rebind)) {
        isc_throw(DhcpConfigError, "the value of renew-timer" << " (" << renew
                  << ") is greater than the value of rebind-timer" << " ("
                  << rebind << ")");
    }

    if (!subnet6->getPreferred().unspecified()) {
        output << "preferred-lifetime=" << subnet6->getPreferred().get() << ", ";
    }
    if (!subnet6->getValid().unspecified()) {
        output << "valid-lifetime=" << subnet6->getValid().get();
    }
    if (!subnet6->getRapidCommit().unspecified()) {
        output << ", rapid-commit is "
               << boolalpha << subnet6->getRapidCommit().get();
    }

    LOG_INFO(dhcpsrv_logger, DHCPSRV_CFGMGR_NEW_SUBNET6).arg(output.str());

    // Get interface-id option content. For now we support string
    // representation only
    Optional<std::string> ifaceid;
    if (params->contains("interface-id")) {
        ifaceid = getString(params, "interface-id");
    }

    Optional<std::string> iface;
    if (params->contains("interface")) {
        iface = getString(params, "interface");
    }

    // Specifying both interface for locally reachable subnets and
    // interface id for relays is mutually exclusive. Need to test for
    // this condition.
    if (!ifaceid.unspecified() && !iface.unspecified() && !ifaceid.empty() &&
        !iface.empty()) {
        isc_throw(isc::dhcp::DhcpConfigError,
                  "parser error: interface (defined for locally reachable "
                  "subnets) and interface-id (defined for subnets reachable"
                  " via relays) cannot be defined at the same time for "
                  "subnet " << addr << "/" << (int)len << "("
                  << params->getPosition() << ")");
    }

    // Configure interface-id for remote interfaces, if defined
    if (!ifaceid.unspecified() && !ifaceid.empty()) {
        std::string ifaceid_value = ifaceid.get();
        OptionBuffer tmp(ifaceid_value.begin(), ifaceid_value.end());
        OptionPtr opt(new Option(Option::V6, D6O_INTERFACE_ID, tmp));
        subnet6->setInterfaceId(opt);
    }

    // Get interface name. If it is defined, then the subnet is available
    // directly over specified network interface.
    if (!iface.unspecified() && !iface.empty()) {
        if (IfaceMgr::instance().isServerMode() && check_iface_ &&
            !IfaceMgr::instance().getIface(iface)) {
            ElementPtr error(params->get("interface"));
            isc_throw(DhcpConfigError, "Specified network interface name " << iface
                      << " for subnet " << subnet6->toText()
                      << " is not present in the system ("
                      << error->getPosition() << ")");
        }
        subnet6->setIface(iface);
    }

    // Let's set host reservation mode. If not specified, the default value of
    // all will be used.
    parseHostReservationMode(params, network);

    // Try setting up client class.
    if (params->contains("client-class")) {
        string client_class = getString(params, "client-class");
        if (!client_class.empty()) {
            subnet6->allowClientClass(client_class);
        }
    }

    if (params->contains("require-client-classes")) {
        // Try setting up required client classes.
        ElementPtr class_list = params->get("require-client-classes");
        if (class_list) {
            const std::vector<data::ElementPtr>& classes = class_list->listValue();
            for (auto cclass = classes.cbegin();
                 cclass != classes.cend(); ++cclass) {
                if (((*cclass)->getType() != Element::string) ||
                    (*cclass)->stringValue().empty()) {
                    isc_throw(DhcpConfigError, "invalid class name ("
                              << (*cclass)->getPosition() << ")");
                }
                subnet6->requireClientClass((*cclass)->stringValue());
            }
        }
    }

    /// client-class processing is now generic and handled in the common
    /// code (see isc::data::SubnetConfigParser::createSubnet)

    // Copy options to the subnet configuration.
    options_->copyTo(*subnet6->getCfgOption());

    // Parse t1-percent and t2-percent
    parseTeePercents(params, network);

    // Parse DDNS parameters
    parseDdnsParams(params, network);
}

void
Subnet6ConfigParser::validateResvs(const Subnet6Ptr& subnet, ConstHostPtr host) {
    IPv6ResrvRange range = host->getIPv6Reservations(IPv6Resrv::TYPE_NA);
    for (auto it = range.first; it != range.second; ++it) {
        const IOAddress& address = it->second.getPrefix();
        if (!subnet->inRange(address)) {
            isc_throw(DhcpConfigError, "specified reservation '" << address
                      << "' is not within the IPv6 subnet '"
                      << subnet->toText() << "'");
        }
    }
}

//**************************** Subnet6ListConfigParser ********************

Subnets6ListConfigParser::Subnets6ListConfigParser(bool check_iface)
    : check_iface_(check_iface) {
}

size_t
Subnets6ListConfigParser::parse(SrvConfigPtr cfg, ElementPtr subnets_list) {
    size_t cnt = 0;
    BOOST_FOREACH(ElementPtr subnet_json, subnets_list->listValue()) {

        Subnet6ConfigParser parser(check_iface_);
        Subnet6Ptr subnet = parser.parse(subnet_json);

        // Adding a subnet to the Configuration Manager may fail if the
        // subnet id is invalid (duplicate). Thus, we catch exceptions
        // here to append a position in the configuration string.
        try {
            cfg->getCfgSubnets6()->add(subnet);
            cnt++;
        } catch (const std::exception& ex) {
            isc_throw(DhcpConfigError, ex.what() << " ("
                      << subnet_json->getPosition() << ")");
        }
    }
    return (cnt);
}

size_t
Subnets6ListConfigParser::parse(Subnet6Collection& subnets,
                                ElementPtr subnets_list) {
    size_t cnt = 0;
    BOOST_FOREACH(ElementPtr subnet_json, subnets_list->listValue()) {

        Subnet6ConfigParser parser(check_iface_);
        Subnet6Ptr subnet = parser.parse(subnet_json);
        if (subnet) {
            try {
                auto ret = subnets.insert(subnet);
                if (!ret.second) {
                    isc_throw(Unexpected,
                              "can't store subnet because of conflict");
                }
                ++cnt;
            } catch (const std::exception& ex) {
                isc_throw(DhcpConfigError, ex.what() << " ("
                          << subnet_json->getPosition() << ")");
            }
        }
    }
    return (cnt);
}


//**************************** D2ClientConfigParser **********************

dhcp_ddns::NameChangeProtocol
D2ClientConfigParser::getProtocol(ElementPtr scope,
                                  const std::string& name) {
    return (getAndConvert<dhcp_ddns::NameChangeProtocol,
            dhcp_ddns::stringToNcrProtocol>
            (scope, name, "NameChangeRequest protocol"));
}

dhcp_ddns::NameChangeFormat
D2ClientConfigParser::getFormat(ElementPtr scope,
                                const std::string& name) {
    return (getAndConvert<dhcp_ddns::NameChangeFormat,
            dhcp_ddns::stringToNcrFormat>
            (scope, name, "NameChangeRequest format"));
}

D2ClientConfig::ReplaceClientNameMode
D2ClientConfigParser::getMode(ElementPtr scope,
                              const std::string& name) {
    return (getAndConvert<D2ClientConfig::ReplaceClientNameMode,
            D2ClientConfig::stringToReplaceClientNameMode>
            (scope, name, "ReplaceClientName mode"));
}

D2ClientConfigPtr
D2ClientConfigParser::parse(isc::data::ElementPtr client_config) {
    D2ClientConfigPtr new_config;

    // Get all parameters that are needed to create the D2ClientConfig.
    bool enable_updates = getBoolean(client_config, "enable-updates");

    IOAddress server_ip = getAddress(client_config, "server-ip");

    uint32_t server_port = getUint32(client_config, "server-port");

    std::string sender_ip_str = getString(client_config, "sender-ip");

    uint32_t sender_port = getUint32(client_config, "sender-port");

    uint32_t max_queue_size = getUint32(client_config, "max-queue-size");

    dhcp_ddns::NameChangeProtocol ncr_protocol =
        getProtocol(client_config, "ncr-protocol");

    dhcp_ddns::NameChangeFormat ncr_format =
        getFormat(client_config, "ncr-format");

    IOAddress sender_ip(uint64_t(0));
    if (sender_ip_str.empty()) {
        // The default sender IP depends on the server IP family
        sender_ip = (server_ip.isV4() ? IOAddress::IPV4_ZERO_ADDRESS() :
                                        IOAddress::IPV6_ZERO_ADDRESS());
    } else {
        try {
            sender_ip = IOAddress(sender_ip_str);
        } catch (const std::exception& ex) {
            isc_throw(DhcpConfigError, "invalid address (" << sender_ip_str
                      << ") specified for parameter 'sender-ip' ("
                      << getPosition("sender-ip", client_config) << ")");
        }
    }

    // Now we check for logical errors. This repeats what is done in
    // D2ClientConfig::validate(), but doing it here permits us to
    // emit meaningful parameter position info in the error.
    if (ncr_format != dhcp_ddns::FMT_JSON) {
        isc_throw(D2ClientError, "D2ClientConfig error: NCR Format: "
                  << dhcp_ddns::ncrFormatToString(ncr_format)
                  << " is not supported. ("
                  << getPosition("ncr-format", client_config) << ")");
    }

    if (ncr_protocol != dhcp_ddns::NCR_UDP) {
        isc_throw(D2ClientError, "D2ClientConfig error: NCR Protocol: "
                  << dhcp_ddns::ncrProtocolToString(ncr_protocol)
                  << " is not supported. ("
                  << getPosition("ncr-protocol", client_config) << ")");
    }

    if (sender_ip.getFamily() != server_ip.getFamily()) {
        isc_throw(D2ClientError,
                  "D2ClientConfig error: address family mismatch: "
                  << "server-ip: " << server_ip.toText()
                  << " is: " << (server_ip.isV4() ? "IPv4" : "IPv6")
                  << " while sender-ip: "  << sender_ip.toText()
                  << " is: " << (sender_ip.isV4() ? "IPv4" : "IPv6")
                  << " (" << getPosition("sender-ip", client_config) << ")");
    }

    if (server_ip == sender_ip && server_port == sender_port) {
        isc_throw(D2ClientError,
                  "D2ClientConfig error: server and sender cannot"
                  " share the exact same IP address/port: "
                  << server_ip.toText() << "/" << server_port
                  << " (" << getPosition("sender-ip", client_config) << ")");
    }

    try {
        // Attempt to create the new client config.
        new_config.reset(new D2ClientConfig(enable_updates,
                                            server_ip,
                                            server_port,
                                            sender_ip,
                                            sender_port,
                                            max_queue_size,
                                            ncr_protocol,
                                            ncr_format));
    }  catch (const std::exception& ex) {
        isc_throw(DhcpConfigError, ex.what() << " ("
                  << client_config->getPosition() << ")");
    }

    // Add user context
    ElementPtr user_context = client_config->get("user-context");
    if (user_context) {
        new_config->setContext(user_context);
    }

    return(new_config);
}

/// @brief This table defines default values for D2 client configuration
const SimpleDefaults D2ClientConfigParser::D2_CLIENT_CONFIG_DEFAULTS = {
    // enable-updates is unconditionally required
    { "server-ip", Element::string, "127.0.0.1" },
    { "server-port", Element::integer, "53001" },
    // default sender-ip depends on server-ip family, so we leave default blank
    // parser knows to use the appropriate ZERO address based on server-ip
    { "sender-ip", Element::string, "" },
    { "sender-port", Element::integer, "0" },
    { "max-queue-size", Element::integer, "1024" },
    { "ncr-protocol", Element::string, "UDP" },
    { "ncr-format", Element::string, "JSON" }
};

size_t
D2ClientConfigParser::setAllDefaults(isc::data::ElementPtr d2_config) {
    return SimpleParser::setDefaults(d2_config, D2_CLIENT_CONFIG_DEFAULTS);
}

} // namespace dhcp
} // namespace isc

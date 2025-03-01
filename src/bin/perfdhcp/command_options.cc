// Copyright (C) 2012-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <perfdhcp/command_options.h>

#include <exceptions/exceptions.h>
#include <dhcp/iface_mgr.h>
#include <dhcp/duid.h>
#include <dhcp/option.h>
#include <cfgrpt/config_report.h>
#include <util/encode/hex.h>
#include <asiolink/io_error.h>

#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fstream>
#include <thread>
#include <getopt.h>

#ifdef HAVE_OPTRESET
extern int optreset;
#endif

using namespace std;
using namespace isc;
using namespace isc::dhcp;

namespace isc {
namespace perfdhcp {

// Refer to config_report so it will be embedded in the binary
const char* const* perfdhcp_config_report = isc::detail::config_report;

CommandOptions::LeaseType::LeaseType()
    : type_(ADDRESS) {
}

CommandOptions::LeaseType::LeaseType(const Type lease_type)
    : type_(lease_type) {
}

bool
CommandOptions::LeaseType::is(const Type lease_type) const {
    return (lease_type == type_);
}

bool
CommandOptions::LeaseType::includes(const Type lease_type) const {
    return (is(ADDRESS_AND_PREFIX) || (lease_type == type_));
}

void
CommandOptions::LeaseType::set(const Type lease_type) {
    type_ = lease_type;
}

void
CommandOptions::LeaseType::fromCommandLine(const std::string& cmd_line_arg) {
    if (cmd_line_arg == "address-only") {
        type_ = ADDRESS;

    } else if (cmd_line_arg == "prefix-only") {
        type_ = PREFIX;

    } else if (cmd_line_arg == "address-and-prefix") {
        type_ = ADDRESS_AND_PREFIX;

    } else {
        isc_throw(isc::InvalidParameter, "value of lease-type: -e<lease-type>,"
                  " must be one of the following: 'address-only' or"
                  " 'prefix-only'");
    }
}

std::string
CommandOptions::LeaseType::toText() const {
    switch (type_) {
    case ADDRESS:
        return ("address-only (IA_NA option added to the client's request)");
    case PREFIX:
        return ("prefix-only (IA_PD option added to the client's request)");
    case ADDRESS_AND_PREFIX:
        return ("address-and-prefix (Both IA_NA and IA_PD options added to the"
                " client's request)");
    default:
        isc_throw(Unexpected, "internal error: undefined lease type code when"
                  " returning textual representation of the lease type");
    }
}

void
CommandOptions::reset() {
    // Default mac address used in DHCP messages
    // if -b mac=<mac-address> was not specified
    uint8_t mac[6] = { 0x0, 0xC, 0x1, 0x2, 0x3, 0x4 };

    // Default packet drop time if -D<drop-time> parameter
    // was not specified
    double dt[2] = { 1., 1. };

    // We don't use constructor initialization list because we
    // will need to reset all members many times to perform unit tests
    ipversion_ = 0;
    exchange_mode_ = DORA_SARR;
    lease_type_.set(LeaseType::ADDRESS);
    rate_ = 0;
    renew_rate_ = 0;
    release_rate_ = 0;
    report_delay_ = 0;
    clean_report_ = false;
    clean_report_separator_ = "";
    clients_num_ = 0;
    mac_template_.assign(mac, mac + 6);
    duid_template_.clear();
    base_.clear();
    addr_unique_ = false;
    mac_list_file_.clear();
    mac_list_.clear();
    giaddr_list_file_.clear();
    giaddr_list_.clear();
    multi_subnet_ = false;
    num_request_.clear();
    exit_wait_time_ = 0;
    period_ = 0;
    drop_time_set_ = 0;
    drop_time_.assign(dt, dt + 2);
    max_drop_.clear();
    max_pdrop_.clear();
    localname_.clear();
    is_interface_ = false;
    preload_ = 0;
    local_port_ = 0;
    remote_port_ = 0;
    seeded_ = false;
    seed_ = 0;
    broadcast_ = false;
    rapid_commit_ = false;
    use_first_ = false;
    template_file_.clear();
    rnd_offset_.clear();
    xid_offset_.clear();
    elp_offset_ = -1;
    sid_offset_ = -1;
    rip_offset_ = -1;
    diags_.clear();
    wrapped_.clear();
    server_name_.clear();
    v6_relay_encapsulation_level_ = 0;
    renew_only_ = false;
    request_count_for_renews_ = 0;
    load_replies_from_file_ = false;
    save_replies_to_file_ = false;
    generateDuidTemplate();
    extra_opts_.clear();
    if (std::thread::hardware_concurrency() == 1) {
        single_thread_mode_ = true;
    } else {
        single_thread_mode_ = false;
    }
    scenario_ = Scenario::BASIC;
}

bool
CommandOptions::parse(int argc, char** const argv, bool print_cmd_line) {
    // Reset internal variables used by getopt
    // to eliminate undefined behavior when
    // parsing different command lines multiple times

#ifdef __GLIBC__
    // Warning: non-portable code. This is due to a bug in glibc's
    // getopt() which keeps internal state about an old argument vector
    // (argc, argv) from last call and tries to scan them when a new
    // argument vector (argc, argv) is passed. As the old vector may not
    // be main()'s arguments, but heap allocated and may have been freed
    // since, this becomes a use after free and results in random
    // behavior. According to the NOTES section in glibc getopt()'s
    // manpage, setting optind=0 resets getopt()'s state. Though this is
    // not required in our usage of getopt(), the bug still happens
    // unless we set optind=0.
    //
    // Setting optind=0 is non-portable code.
    optind = 0;
#else
    optind = 1;
#endif

    // optreset is declared on BSD systems and is used to reset internal
    // state of getopt(). When parsing command line arguments multiple
    // times with getopt() the optreset must be set to 1 every time before
    // parsing starts. Failing to do so will result in random behavior of
    // getopt().
#ifdef HAVE_OPTRESET
    optreset = 1;
#endif

    opterr = 0;

    // Reset values of class members
    reset();

    // Informs if program has been run with 'h' or 'v' option.
    bool help_or_version_mode = initialize(argc, argv, print_cmd_line);
    if (!help_or_version_mode) {
        validate();
    }
    return (help_or_version_mode);
}

const int LONG_OPT_SCENARIO = 300;

bool
CommandOptions::initialize(int argc, char** argv, bool print_cmd_line) {
    int opt = 0;                // Subsequent options returned by getopt()
    std::string drop_arg;       // Value of -D<value>argument
    size_t percent_loc = 0;     // Location of % sign in -D<value>
    double drop_percent = 0;    // % value (1..100) in -D<value%>
    int num_drops = 0;          // Max number of drops specified in -D<value>
    int num_req = 0;            // Max number of dropped
                                // requests in -n<max-drops>
    int offset_arg = 0;         // Temporary variable holding offset arguments
    std::string sarg;           // Temporary variable for string args

    std::ostringstream stream;
    stream << "perfdhcp";
    int num_mac_list_files = 0;
    int num_subnet_list_files = 0;

    struct option long_options[] = {
        {"scenario", required_argument, 0, LONG_OPT_SCENARIO},
        {0,          0,                 0, 0}
    };

    // In this section we collect argument values from command line
    // they will be tuned and validated elsewhere
    while((opt = getopt_long(argc, argv,
                             "huv46A:r:t:R:b:n:p:d:D:l:P:a:L:N:M:s:z:iBc1yY"
                             "J:T:X:O:o:E:S:I:x:W:w:e:f:F:g:C:",
                             long_options, NULL)) != -1) {
        stream << " -" << static_cast<char>(opt);
        if (optarg) {
            stream << " " << optarg;
        }
        switch (opt) {
        case '1':
            use_first_ = true;
            break;

        // Simulate DHCPv6 relayed traffic.
        case 'A':
            // @todo: At the moment we only support simulating a single relay
            // agent. In the future we should extend it to up to 32.
            // See comment in https://github.com/isc-projects/kea/pull/22#issuecomment-243405600
            v6_relay_encapsulation_level_ =
                static_cast<uint8_t>(positiveInteger("-A<encapsulation-level> must"
                                                     " be a positive integer"));
            if (v6_relay_encapsulation_level_ != 1) {
                isc_throw(isc::InvalidParameter, "-A only supports 1 at the moment.");
            }
            break;

        case 'u':
            addr_unique_ = true;
            break;

        case '4':
            check(ipversion_ == 6, "IP version already set to 6");
            ipversion_ = 4;
            break;

        case '6':
            check(ipversion_ == 4, "IP version already set to 4");
            ipversion_ = 6;
            break;

        case 'b':
            check(base_.size() > 3, "-b<value> already specified,"
                  " unexpected occurrence of 5th -b<value>");
            base_.push_back(optarg);
            decodeBase(base_.back());
            break;

        case 'B':
            broadcast_ = true;
            break;

        case 'c':
            rapid_commit_ = true;
            break;

        case 'C':
            clean_report_ = true;
            clean_report_separator_ = optarg;
            break;

        case 'd':
            check(drop_time_set_ > 1,
                  "maximum number of drops already specified, "
                  "unexpected 3rd occurrence of -d<value>");
            try {
                drop_time_[drop_time_set_] =
                    boost::lexical_cast<double>(optarg);
            } catch (const boost::bad_lexical_cast&) {
                isc_throw(isc::InvalidParameter,
                          "value of drop time: -d<value>"
                          " must be positive number");
            }
            check(drop_time_[drop_time_set_] <= 0.,
                  "drop-time must be a positive number");
            drop_time_set_ = true;
            break;

        case 'D':
            drop_arg = std::string(optarg);
            percent_loc = drop_arg.find('%');
            check(max_pdrop_.size() > 1 || max_drop_.size() > 1,
                  "values of maximum drops: -D<value> already "
                  "specified, unexpected 3rd occurrence of -D<value>");
            if ((percent_loc) != std::string::npos) {
                try {
                    drop_percent =
                        boost::lexical_cast<double>(drop_arg.substr(0, percent_loc));
                } catch (const boost::bad_lexical_cast&) {
                    isc_throw(isc::InvalidParameter,
                              "value of drop percentage: -D<value%>"
                              " must be 0..100");
                }
                check((drop_percent <= 0) || (drop_percent >= 100),
                  "value of drop percentage: -D<value%> must be 0..100");
                max_pdrop_.push_back(drop_percent);
            } else {
                num_drops = positiveInteger("value of max drops number:"
                                            " -D<value> must be a positive integer");
                max_drop_.push_back(num_drops);
            }
            break;

        case 'e':
            initLeaseType();
            break;

        case 'E':
            elp_offset_ = nonNegativeInteger("value of time-offset: -E<value>"
                                             " must not be a negative integer");
            break;

        case 'f':
            renew_rate_ = positiveInteger("value of the renew rate: -f<renew-rate>"
                                          " must be a positive integer");
            break;

        case 'F':
            release_rate_ = positiveInteger("value of the release rate:"
                                            " -F<release-rate> must be a"
                                            " positive integer");
            break;

        case 'g': {
            auto optarg_text = std::string(optarg);
            if (optarg_text == "single") {
                single_thread_mode_ = true;
            } else if (optarg_text == "multi") {
                single_thread_mode_ = false;
            } else {
                isc_throw(InvalidParameter, "value of thread mode (-g) '" << optarg << "' is wrong - should be '-g single' or '-g multi'");
            }
            break;
        }
        case 'h':
            usage();
            return (true);

        case 'i':
            exchange_mode_ = DO_SA;
            break;

        case 'I':
            rip_offset_ = positiveInteger("value of ip address offset:"
                                          " -I<value> must be a"
                                          " positive integer");
            break;

        case 'J':
            check(num_subnet_list_files >= 1, "only one -J option can be specified");
            num_subnet_list_files++;
            giaddr_list_file_ = std::string(optarg);
            loadGiaddr();
            break;

        case 'l':
            localname_ = std::string(optarg);
            initIsInterface();
            break;

        case 'L':
            local_port_ = nonNegativeInteger("value of local port:"
                                             " -L<value> must not be a"
                                             " negative integer");
            check(local_port_ >
                  static_cast<int>(std::numeric_limits<uint16_t>::max()),
                  "local-port must be lower than " +
                  boost::lexical_cast<std::string>(std::numeric_limits<uint16_t>::max()));
            break;

        case 'N':
            remote_port_ = nonNegativeInteger("value of remote port:"
                                              " -N<value> must not be a"
                                              " negative integer");
            check(remote_port_ >
                  static_cast<int>(std::numeric_limits<uint16_t>::max()),
                  "remote-port must be lower than " +
                  boost::lexical_cast<std::string>(std::numeric_limits<uint16_t>::max()));
            break;

        case 'M':
            check(num_mac_list_files >= 1, "only one -M option can be specified");
            num_mac_list_files++;
            mac_list_file_ = std::string(optarg);
            loadMacs();
            break;

        case 'W':
            exit_wait_time_ = nonNegativeInteger("value of exist wait time: "
                                                  "-W<value> must not be a "
                                                  "negative integer");
            break;

        case 'n':
            num_req = positiveInteger("value of num-request:"
                                      " -n<value> must be a positive integer");
            if (num_request_.size() >= 2) {
                isc_throw(isc::InvalidParameter,
                          "value of maximum number of requests: -n<value> "
                          "already specified, unexpected 3rd occurrence"
                          " of -n<value>");
            }
            num_request_.push_back(num_req);
            break;

        case 'O':
            if (rnd_offset_.size() < 2) {
                offset_arg = positiveInteger("value of random offset: "
                                             "-O<value> must be greater than 3");
            } else {
                isc_throw(isc::InvalidParameter,
                          "random offsets already specified,"
                          " unexpected 3rd occurrence of -O<value>");
            }
            check(offset_arg < 3, "value of random random-offset:"
                  " -O<value> must be greater than 3 ");
            rnd_offset_.push_back(offset_arg);
            break;
        case 'o': {

            // we must know how to contruct the option: whether it's v4 or v6.
            check( (ipversion_ != 4) && (ipversion_ != 6),
                   "-4 or -6 must be explicitly specified before -o is used.");

            // custom option (expected format: code,hexstring)
            std::string opt_text = std::string(optarg);
            size_t coma_loc = opt_text.find(',');
            check(coma_loc == std::string::npos,
                  "-o option must provide option code, a coma and hexstring for"
                  " the option content, e.g. -o60,646f63736973 for sending option"
                  " 60 (class-id) with the value 'docsis'");
            int code = 0;

            // Try to parse the option code
            try {
                code = boost::lexical_cast<int>(opt_text.substr(0,coma_loc));
                check(code <= 0, "Option code can't be negative");
            } catch (const boost::bad_lexical_cast&) {
                isc_throw(InvalidParameter, "Invalid option code specified for "
                          "-o option, expected format: -o<integer>,<hexstring>");
            }

            // Now try to interpret the hexstring
            opt_text = opt_text.substr(coma_loc + 1);
            std::vector<uint8_t> bin;
            try {
                isc::util::encode::decodeHex(opt_text, bin);
            } catch (const BadValue& e) {
                isc_throw(InvalidParameter, "Error during encoding option -o:"
                          << e.what());
            }

            // Create and remember the option.
            OptionPtr opt(new Option(ipversion_ == 4 ? Option::V4 : Option::V6,
                                     code, bin));
            extra_opts_.insert(make_pair(code, opt));
            break;
        }
        case 'p':
            period_ = positiveInteger("value of test period:"
                                      " -p<value> must be a positive integer");
            break;

        case 'P':
            preload_ = nonNegativeInteger("number of preload packets:"
                                          " -P<value> must not be "
                                          "a negative integer");
            break;

        case 'r':
            try {
                rate_ = boost::lexical_cast<int>(optarg);
            } catch (const boost::bad_lexical_cast&) {
                isc_throw(InvalidParameter,
                          "value of rate: -r<value> must be zero or a positive integer");
            }
            break;

        case 'R':
            initClientsNum();
            break;

        case 's':
            seed_ = static_cast<unsigned int>
                (nonNegativeInteger("value of seed:"
                                    " -s <seed> must be non-negative integer"));
            seeded_ = seed_ > 0 ? true : false;
            break;

        case 'S':
            sid_offset_ = positiveInteger("value of server id offset:"
                                          " -S<value> must be a"
                                          " positive integer");
            break;

        case 't':
            report_delay_ = positiveInteger("value of report delay:"
                                            " -t<value> must be a"
                                            " positive integer");
            break;

        case 'T':
            if (template_file_.size() < 2) {
                sarg = nonEmptyString("template file name not specified,"
                                      " expected -T<filename>");
                template_file_.push_back(sarg);
            } else {
                isc_throw(isc::InvalidParameter,
                          "template files are already specified,"
                          " unexpected 3rd -T<filename> occurrence");
            }
            break;

        case 'v':
            version();
            return (true);

        case 'w':
            wrapped_ = nonEmptyString("command for wrapped mode:"
                                      " -w<command> must be specified");
            break;

        case 'x':
            diags_ = nonEmptyString("value of diagnostics selectors:"
                                    " -x<value> must be specified");
            break;

        case 'X':
            if (xid_offset_.size() < 2) {
                offset_arg = positiveInteger("value of transaction id:"
                                             " -X<value> must be a"
                                             " positive integer");
            } else {
                isc_throw(isc::InvalidParameter,
                          "transaction ids already specified,"
                          " unexpected 3rd -X<value> occurrence");
            }
            xid_offset_.push_back(offset_arg);
            break;

        case 'y':
            load_replies_from_file_ = true;
            break;

        case 'Y':
            save_replies_to_file_ = true;
            break;

        case 'z':
            renew_only_ = true;
            try {
                request_count_for_renews_ = boost::lexical_cast<int>(optarg);
            } catch (const boost::bad_lexical_cast&) {
                isc_throw(InvalidParameter,
                          "value of request count: -z<request-count> must be a positive integer");
            }
            break;

        case LONG_OPT_SCENARIO: {
            auto optarg_text = std::string(optarg);
            if (optarg_text == "basic") {
                scenario_ = Scenario::BASIC;
            } else if (optarg_text == "avalanche") {
                scenario_ = Scenario::AVALANCHE;
            } else {
                isc_throw(InvalidParameter, "scenario value '" << optarg << "' is wrong - should be 'basic' or 'avalanche'");
            }
            break;
        }
        default:
            isc_throw(isc::InvalidParameter, "wrong command line option");
        }
    }

    // If the IP version was not specified in the
    // command line, assume IPv4.
    if (ipversion_ == 0) {
        ipversion_ = 4;
    }

    // If template packet files specified for both DISCOVER/SOLICIT
    // and REQUEST/REPLY exchanges make sure we have transaction id
    // and random duid offsets for both exchanges. We will duplicate
    // value specified as -X<value> and -R<value> for second
    // exchange if user did not specified otherwise.
    if (template_file_.size() > 1) {
        if (xid_offset_.size() == 1) {
            xid_offset_.push_back(xid_offset_[0]);
        }
        if (rnd_offset_.size() == 1) {
            rnd_offset_.push_back(rnd_offset_[0]);
        }
    }

    // Get server argument
    // NoteFF02::1:2 and FF02::1:3 are defined in RFC 8415 as
    // All_DHCP_Relay_Agents_and_Servers and All_DHCP_Servers
    // addresses
    check(optind < argc -1, "extra arguments?");
    if (optind == argc - 1) {
        server_name_ = argv[optind];
        stream << " " << server_name_;
        // Decode special cases
        if ((ipversion_ == 4) && (server_name_.compare("all") == 0)) {
            broadcast_ = true;
            // Use broadcast address as server name.
            server_name_ = DHCP_IPV4_BROADCAST_ADDRESS;
        } else if ((ipversion_ == 6) && (server_name_.compare("all") == 0)) {
            server_name_ = ALL_DHCP_RELAY_AGENTS_AND_SERVERS;
        } else if ((ipversion_ == 6) &&
                   (server_name_.compare("servers") == 0)) {
            server_name_ = ALL_DHCP_SERVERS;
        }
    }
    if (!getCleanReport()) {
        if (print_cmd_line) {
            std::cout << "Running: " << stream.str() << std::endl;
        }

        if (scenario_ == Scenario::BASIC) {
            std::cout << "Scenario: basic." << std::endl;
        } else if (scenario_ == Scenario::AVALANCHE) {
            std::cout << "Scenario: avalanche." << std::endl;
        }

        if (!isSingleThreaded()) {
            std::cout << "Multi-thread mode enabled." << std::endl;
        }
    }

    // Handle the local '-l' address/interface
    if (!localname_.empty()) {
        if (server_name_.empty()) {
            if (is_interface_ && (ipversion_ == 4)) {
                broadcast_ = true;
                server_name_ = DHCP_IPV4_BROADCAST_ADDRESS;
            } else if (is_interface_ && (ipversion_ == 6)) {
                server_name_ = ALL_DHCP_RELAY_AGENTS_AND_SERVERS;
            }
        }
    }
    if (server_name_.empty()) {
        isc_throw(InvalidParameter,
                  "without an interface, server is required");
    }

    // If DUID is not specified from command line we need to
    // generate one.
    if (duid_template_.empty()) {
        generateDuidTemplate();
    }
    return (false);
}

void
CommandOptions::initClientsNum() {
    const std::string errmsg =
        "value of -R <value> must be non-negative integer";

    try {
        // Declare clients_num as as 64-bit signed value to
        // be able to detect negative values provided
        // by user. We would not detect negative values
        // if we casted directly to unsigned value.
        long long clients_num = boost::lexical_cast<long long>(optarg);
        check(clients_num < 0, errmsg);
        clients_num_ = boost::lexical_cast<uint32_t>(optarg);
    } catch (const boost::bad_lexical_cast&) {
        isc_throw(isc::InvalidParameter, errmsg);
    }
}

void
CommandOptions::initIsInterface() {
    is_interface_ = false;
    if (!localname_.empty()) {
        dhcp::IfaceMgr& iface_mgr = dhcp::IfaceMgr::instance();
        if (iface_mgr.getIface(localname_) != NULL)  {
            is_interface_ = true;
        }
    }
}

void
CommandOptions::decodeBase(const std::string& base) {
    std::string b(base);
    boost::algorithm::to_lower(b);

    // Currently we only support mac and duid
    if ((b.substr(0, 4) == "mac=") || (b.substr(0, 6) == "ether=")) {
        decodeMacBase(b);
    } else if (b.substr(0, 5) == "duid=") {
        decodeDuid(b);
    } else {
        isc_throw(isc::InvalidParameter,
                  "base value not provided as -b<value>,"
                  " expected -b mac=<mac> or -b duid=<duid>");
    }
}

void
CommandOptions::decodeMacBase(const std::string& base) {
    // Strip string from mac=
    size_t found = base.find('=');
    static const char* errmsg = "expected -b<base> format for"
        " mac address is -b mac=00::0C::01::02::03::04 or"
        " -b mac=00:0C:01:02:03:04";
    check(found == std::string::npos, errmsg);

    // Decode mac address to vector of uint8_t
    std::istringstream s1(base.substr(found + 1));
    std::string token;
    mac_template_.clear();
    // Get pieces of MAC address separated with : (or even ::)
    while (std::getline(s1, token, ':')) {
        // Convert token to byte value using std::istringstream
        if (token.length() > 0) {
            unsigned int ui = 0;
            try {
                // Do actual conversion
                ui = convertHexString(token);
            } catch (const isc::InvalidParameter&) {
                isc_throw(isc::InvalidParameter,
                          "invalid characters in MAC provided");

            }
            // If conversion succeeded store byte value
            mac_template_.push_back(ui);
        }
    }
    // MAC address must consist of 6 octets, otherwise it is invalid
    check(mac_template_.size() != 6, errmsg);
}

void
CommandOptions::decodeDuid(const std::string& base) {
    // Strip argument from duid=
    std::vector<uint8_t> duid_template;
    size_t found = base.find('=');
    check(found == std::string::npos, "expected -b<base>"
          " format for duid is -b duid=<duid>");
    std::string b = base.substr(found + 1);

    // DUID must have even number of digits and must not be longer than 64 bytes
    check(b.length() & 1, "odd number of hexadecimal digits in duid");
    check(b.length() > 128, "duid too large");
    check(b.length() == 0, "no duid specified");

    // Turn pairs of hexadecimal digits into vector of octets
    for (size_t i = 0; i < b.length(); i += 2) {
        unsigned int ui = 0;
        try {
            // Do actual conversion
            ui = convertHexString(b.substr(i, 2));
        } catch (const isc::InvalidParameter&) {
            isc_throw(isc::InvalidParameter,
                      "invalid characters in DUID provided,"
                      " expected hex digits");
        }
        duid_template.push_back(static_cast<uint8_t>(ui));
    }
    // @todo Get rid of this limitation when we manage add support
    // for DUIDs other than LLT. Shorter DUIDs may be useful for
    // server testing purposes.
    check(duid_template.size() < 6, "DUID must be at least 6 octets long");
    // Assign the new duid only if successfully generated.
    std::swap(duid_template, duid_template_);
}

void
CommandOptions::generateDuidTemplate() {
    using namespace boost::posix_time;
    // Duid template will be most likely generated only once but
    // it is ok if it is called more then once so we simply
    //  regenerate it and discard previous value.
    duid_template_.clear();
    const uint8_t duid_template_len = 14;
    duid_template_.resize(duid_template_len);
    // The first four octets consist of DUID LLT and hardware type.
    duid_template_[0] = static_cast<uint8_t>(static_cast<uint16_t>(isc::dhcp::DUID::DUID_LLT) >> 8);
    duid_template_[1] = static_cast<uint8_t>(static_cast<uint16_t>(isc::dhcp::DUID::DUID_LLT) & 0xff);
    duid_template_[2] = HWTYPE_ETHERNET >> 8;
    duid_template_[3] = HWTYPE_ETHERNET & 0xff;

    // As described in RFC 8415: 'the time value is the time
    // that the DUID is generated represented in seconds
    // since midnight (UTC), January 1, 2000, modulo 2^32.'
    ptime now = microsec_clock::universal_time();
    ptime duid_epoch(from_iso_string("20000101T000000"));
    time_period period(duid_epoch, now);
    uint32_t duration_sec = htonl(period.length().total_seconds());
    memcpy(&duid_template_[4], &duration_sec, 4);

    // Set link layer address (6 octets). This value may be
    // randomized before sending a packet to simulate different
    // clients.
    memcpy(&duid_template_[8], &mac_template_[0], 6);
}

uint8_t
CommandOptions::convertHexString(const std::string& text) const {
    unsigned int ui = 0;
    // First, check if we are dealing with hexadecimal digits only
    for (size_t i = 0; i < text.length(); ++i) {
        if (!std::isxdigit(text[i])) {
            isc_throw(isc::InvalidParameter,
                      "The following digit: " << text[i] << " in "
                      << text << "is not hexadecimal");
        }
    }
    // If we are here, we have valid string to convert to octet
    std::istringstream text_stream(text);
    text_stream >> std::hex >> ui >> std::dec;
    // Check if for some reason we have overflow - this should never happen!
    if (ui > 0xFF) {
        isc_throw(isc::InvalidParameter, "Can't convert more than"
                  " two hex digits to byte");
    }
    return ui;
}

bool CommandOptions::validateIP(const std::string& line) {
    try {
        asiolink::IOAddress ip_address_ = isc::asiolink::IOAddress(line);
        // let's silence not used warning
        (void) ip_address_;
    } catch (const isc::asiolink::IOError& e) {
        return (true);
    }
    giaddr_list_.push_back(line);
    multi_subnet_ = true;
    return (false);
}

void CommandOptions::loadGiaddr() {
    std::string line;
    std::ifstream infile(giaddr_list_file_.c_str());
    size_t cnt = 0;
    while (std::getline(infile, line)) {
        cnt++;
        stringstream tmp;
        tmp << "invalid address in line: "<< cnt;
        check(validateIP(line), tmp.str());
    }
}

void CommandOptions::loadMacs() {
    std::string line;
    std::ifstream infile(mac_list_file_.c_str());
    size_t cnt = 0;
    while (std::getline(infile, line)) {
        cnt++;
        stringstream tmp;
        tmp << "invalid mac in input line " << cnt;
        // Let's print more meaningful error that contains line with error.
        check(decodeMacString(line), tmp.str());
    }
}

bool CommandOptions::decodeMacString(const std::string& line) {
  // decode mac string into a vector of uint8_t returns true in case of error.
  std::istringstream s(line);
  std::string token;
  std::vector<uint8_t> mac;
  while(std::getline(s, token, ':')) {
    // Convert token to byte value using std::istringstream
    if (token.length() > 0) {
      unsigned int ui = 0;
      try {
        // Do actual conversion
        ui = convertHexString(token);
      } catch (const isc::InvalidParameter&) {
        return (true);
      }
      // If conversion succeeded store byte value
      mac.push_back(ui);
    }
  }
  mac_list_.push_back(mac);
  return (false);
}

void
CommandOptions::validate() {
    check((getIpVersion() != 4) && (isBroadcast() != 0),
          "-B is not compatible with IPv6 (-6)");
    check((getIpVersion() != 6) && (isRapidCommit() != 0),
          "-6 (IPv6) must be set to use -c");
    check(getIpVersion() == 4 && isUseRelayedV6(),
          "Can't use -4 with -A, it's a V6 only option.");
    check((getIpVersion() != 6) && (getReleaseRate() != 0),
          "-F<release-rate> may be used with -6 (IPv6) only");
    check((getExchangeMode() == DO_SA) && (getNumRequests().size() > 1),
          "second -n<num-request> is not compatible with -i");
    check((getIpVersion() == 4) && !getLeaseType().is(LeaseType::ADDRESS),
          "-6 option must be used if lease type other than '-e address-only'"
          " is specified");
    check(!getTemplateFiles().empty() &&
          !getLeaseType().is(LeaseType::ADDRESS),
          "template files may be only used with '-e address-only'");
    check((getExchangeMode() == DO_SA) && (getDropTime()[1] != 1.),
          "second -d<drop-time> is not compatible with -i");
    check((getExchangeMode() == DO_SA) &&
          ((getMaxDrop().size() > 1) || (getMaxDropPercentage().size() > 1)),
          "second -D<max-drop> is not compatible with -i");
    check((getExchangeMode() == DO_SA) && (isUseFirst()),
          "-1 is not compatible with -i");
    check((getExchangeMode() == DO_SA) && (getTemplateFiles().size() > 1),
          "second -T<template-file> is not compatible with -i");
    check((getExchangeMode() == DO_SA) && (getTransactionIdOffset().size() > 1),
          "second -X<xid-offset> is not compatible with -i");
    check((getExchangeMode() == DO_SA) && (getRandomOffset().size() > 1),
          "second -O<random-offset is not compatible with -i");
    check((getExchangeMode() == DO_SA) && (getElapsedTimeOffset() >= 0),
          "-E<time-offset> is not compatible with -i");
    check((getExchangeMode() == DO_SA) && (getServerIdOffset() >= 0),
          "-S<srvid-offset> is not compatible with -i");
    check((getExchangeMode() == DO_SA) && (getRequestedIpOffset() >= 0),
          "-I<ip-offset> is not compatible with -i");
    check((getExchangeMode() == DO_SA) && (getRenewRate() != 0),
          "-f<renew-rate> is not compatible with -i");
    check((getExchangeMode() == DO_SA) && (getReleaseRate() != 0),
          "-F<release-rate> is not compatible with -i");
    check((getExchangeMode() != DO_SA) && (isRapidCommit() != 0),
          "-i must be set to use -c");
    check((getRate() == 0) && (getReleaseRate() != 0),
          "Release rate specified as -F<release-rate> must not be specified"
          " when -r<rate> parameter is not specified");
    check((getTemplateFiles().size() < getTransactionIdOffset().size()),
          "-T<template-file> must be set to use -X<xid-offset>");
    check((getTemplateFiles().size() < getRandomOffset().size()),
          "-T<template-file> must be set to use -O<random-offset>");
    check((getTemplateFiles().size() < 2) && (getElapsedTimeOffset() >= 0),
          "second/request -T<template-file> must be set to use -E<time-offset>");
    check((getTemplateFiles().size() < 2) && (getServerIdOffset() >= 0),
          "second/request -T<template-file> must be set to "
          "use -S<srvid-offset>");
    check((getTemplateFiles().size() < 2) && (getRequestedIpOffset() >= 0),
          "second/request -T<template-file> must be set to "
          "use -I<ip-offset>");
    check((!getMacListFile().empty() && base_.size() > 0),
          "Can't use -b with -M option");

    auto nthreads = std::thread::hardware_concurrency();
    if (nthreads == 1 && isSingleThreaded() == false) {
        std::cout << "WARNING: Currently system can run only 1 thread in parallel." << std::endl
                  << "WARNING: Better results are achieved when run in single-threaded mode." << std::endl
                  << "WARNING: To switch use -g single option." << std::endl;
    } else if (nthreads > 1 && isSingleThreaded()) {
        std::cout << "WARNING: Currently system can run more than 1 thread in parallel." << std::endl
                  << "WARNING: Better results are achieved when run in multi-threaded mode." << std::endl
                  << "WARNING: To switch use -g multi option." << std::endl;
    }

    if (scenario_ == Scenario::AVALANCHE) {
        check(getClientsNum() <= 0,
              "in case of avalanche scenario number\nof clients must be specified"
              " using -R option explicitly");

        // in case of AVALANCHE drops ie. long responses should not be observed by perfdhcp
        double dt[2] = { 1000.0, 1000.0 };
        drop_time_.assign(dt, dt + 2);
        if (drop_time_set_) {
            std::cout << "INFO: in avalanche scenario drop time is ignored" << std::endl;
        }
    }
}

void
CommandOptions::check(bool condition, const std::string& errmsg) const {
    // The same could have been done with macro or just if statement but
    // we prefer functions to macros here
    std::ostringstream stream;
    stream << errmsg << "\n";
    if (condition) {
        isc_throw(isc::InvalidParameter, errmsg);
    }
}

int
CommandOptions::positiveInteger(const std::string& errmsg) const {
    try {
        int value = boost::lexical_cast<int>(optarg);
        check(value <= 0, errmsg);
        return (value);
    } catch (const boost::bad_lexical_cast&) {
        isc_throw(InvalidParameter, errmsg);
    }
}

int
CommandOptions::nonNegativeInteger(const std::string& errmsg) const {
    try {
        int value = boost::lexical_cast<int>(optarg);
        check(value < 0, errmsg);
        return (value);
    } catch (const boost::bad_lexical_cast&) {
        isc_throw(InvalidParameter, errmsg);
    }
}

std::string
CommandOptions::nonEmptyString(const std::string& errmsg) const {
    std::string sarg = optarg;
    if (sarg.length() == 0) {
        isc_throw(isc::InvalidParameter, errmsg);
    }
    return sarg;
}

void
CommandOptions::initLeaseType() {
    std::string lease_type_arg = optarg;
    lease_type_.fromCommandLine(lease_type_arg);
}

void
CommandOptions::printCommandLine() const {
    std::cout << "IPv" << static_cast<int>(ipversion_) << std::endl;
    if (exchange_mode_ == DO_SA) {
        if (ipversion_ == 4) {
            std::cout << "DISCOVER-OFFER only" << std::endl;
        } else {
            std::cout << "SOLICIT-ADVERTISE only" << std::endl;
        }
    }
    std::cout << "lease-type=" << getLeaseType().toText() << std::endl;
    if (rate_ != 0) {
        std::cout << "rate[1/s]=" << rate_ <<  std::endl;
    }
    if (getRenewRate() != 0) {
        std::cout << "renew-rate[1/s]=" << getRenewRate() << std::endl;
    }
    if (getReleaseRate() != 0) {
        std::cout << "release-rate[1/s]=" << getReleaseRate() << std::endl;
    }
    if (report_delay_ != 0) {
        std::cout << "report[s]=" << report_delay_ << std::endl;
    }
    if (clients_num_ != 0) {
        std::cout << "clients=" << clients_num_ << std::endl;
    }
    for (size_t i = 0; i < base_.size(); ++i) {
        std::cout << "base[" << i << "]=" << base_[i] <<  std::endl;
    }
    for (size_t i = 0; i < num_request_.size(); ++i) {
        std::cout << "num-request[" << i << "]=" << num_request_[i] << std::endl;
    }
    if (period_ != 0) {
        std::cout << "test-period=" << period_ << std::endl;
    }
    for (size_t i = 0; i < drop_time_.size(); ++i) {
        std::cout << "drop-time[" << i << "]=" << drop_time_[i] << std::endl;
    }
    for (size_t i = 0; i < max_drop_.size(); ++i) {
        std::cout << "max-drop{" << i << "]=" << max_drop_[i] << std::endl;
    }
    for (size_t i = 0; i < max_pdrop_.size(); ++i) {
        std::cout << "max-pdrop{" << i << "]=" << max_pdrop_[i] << std::endl;
    }
    if (preload_ != 0) {
        std::cout << "preload=" << preload_ <<  std::endl;
    }
    if (getLocalPort() != 0) {
        std::cout << "local-port=" << local_port_ <<  std::endl;
    }
    if (getRemotePort() != 0) {
        std::cout << "remote-port=" << remote_port_ <<  std::endl;
    }
    if (seeded_) {
        std::cout << "seed=" << seed_ << std::endl;
    }
    if (broadcast_) {
        std::cout << "broadcast" << std::endl;
    }
    if (rapid_commit_) {
        std::cout << "rapid-commit" << std::endl;
    }
    if (use_first_) {
        std::cout << "use-first" << std::endl;
    }
    if (!mac_list_file_.empty()) {
        std::cout << "mac-list-file=" << mac_list_file_ << std::endl;
    }
    for (size_t i = 0; i < template_file_.size(); ++i) {
        std::cout << "template-file[" << i << "]=" << template_file_[i] << std::endl;
    }
    for (size_t i = 0; i < xid_offset_.size(); ++i) {
        std::cout << "xid-offset[" << i << "]=" << xid_offset_[i] << std::endl;
    }
    if (elp_offset_ != 0) {
        std::cout << "elp-offset=" << elp_offset_ << std::endl;
    }
    for (size_t i = 0; i < rnd_offset_.size(); ++i) {
        std::cout << "rnd-offset[" << i << "]=" << rnd_offset_[i] << std::endl;
    }
    if (sid_offset_ != 0) {
        std::cout << "sid-offset=" << sid_offset_ << std::endl;
    }
    if (rip_offset_ != 0) {
        std::cout << "rip-offset=" << rip_offset_ << std::endl;
    }
    if (!diags_.empty()) {
        std::cout << "diagnostic-selectors=" << diags_ <<  std::endl;
    }
    if (!wrapped_.empty()) {
        std::cout << "wrapped=" << wrapped_ << std::endl;
    }
    if (!localname_.empty()) {
        if (is_interface_) {
            std::cout << "interface=" << localname_ << std::endl;
        } else {
            std::cout << "local-addr=" << localname_ << std::endl;
        }
    }
    if (!server_name_.empty()) {
        std::cout << "server=" << server_name_ << std::endl;
    }
    if (single_thread_mode_) {
        std::cout << "single-thread-mode" << std::endl;
    } else {
        std::cout << "multi-thread-mode" << std::endl;
    }
}

void
CommandOptions::usage() const {
    std::cout <<
        "perfdhcp [-huv] [-4|-6] [-A<encapsulation-level>] [-e<lease-type>]\n"
        "         [-r<rate>] [-f<renew-rate>]\n"
        "         [-F<release-rate>] [-t<report>] [-C<separator>] [-R<range>]\n"
        "         [-b<base>] [-n<num-request>] [-p<test-period>] [-d<drop-time>]\n"
        "         [-D<max-drop>] [-l<local-addr|interface>] [-P<preload>]\n"
        "         [-L<local-port>] [-N<remote-port>]\n"
        "         [-s<seed>] [-i] [-B] [-W<late-exit-delay>]\n"
        "         [-c] [-1] [-M<mac-list-file>] [-T<template-file>]\n"
        "         [-X<xid-offset>] [-O<random-offset] [-E<time-offset>]\n"
        "         [-S<srvid-offset>] [-I<ip-offset>] [-x<diagnostic-selector>]\n"
        "         [-w<wrapped>] [-z] [server]\n"
        "\n"
        "The [server] argument is the name/address of the DHCP server to\n"
        "contact.  For DHCPv4 operation, exchanges are initiated by\n"
        "transmitting a DHCP DISCOVER to this address.\n"
        "\n"
        "For DHCPv6 operation, exchanges are initiated by transmitting a DHCP\n"
        "SOLICIT to this address.  In the DHCPv6 case, the special name 'all'\n"
        "can be used to refer to All_DHCP_Relay_Agents_and_Servers (the\n"
        "multicast address FF02::1:2), or the special name 'servers' to refer\n"
        "to All_DHCP_Servers (the multicast address FF05::1:3).  The [server]\n"
        "argument is optional only in the case that -l is used to specify an\n"
        "interface, in which case [server] defaults to 'all'.\n"
        "\n"
        "The default is to perform a single 4-way exchange, effectively pinging\n"
        "the server.\n"
        "The -r option is used to set up a performance test, without\n"
        "it exchanges are initiated as fast as possible.\n"
        "The other scenario is an avalanche which is selected by\n"
        "--scenario avalanche. It first sends as many Discovery or Solicit\n"
        "messages as request in -R option then back off mechanism is used for\n"
        "each simulated client until all requests are answered. At the end\n"
        "time of whole scenario is reported.\n"
        "\n"
        "Options:\n"
        "-1: Take the server-ID option from the first received message.\n"
        "-4: DHCPv4 operation (default). This is incompatible with the -6 option.\n"
        "-6: DHCPv6 operation. This is incompatible with the -4 option.\n"
        "-u: Enable checking address uniqueness. Lease valid lifetime\n"
        "    should not be shorter than test duration.\n"
        "-b<base>: The base mac, duid, IP, etc, used to simulate different\n"
        "    clients.  This can be specified multiple times, each instance is\n"
        "    in the <type>=<value> form, for instance:\n"
        "    (and default) mac=00:0c:01:02:03:04.\n"
        "-d<drop-time>: Specify the time after which a request is treated as\n"
        "    having been lost.  The value is given in seconds and may contain a\n"
        "    fractional component.  The default is 1 second.\n"
        "-e<lease-type>: A type of lease being requested from the server. It\n"
        "    may be one of the following: address-only, prefix-only or\n"
        "    address-and-prefix. The address-only indicates that the regular\n"
        "    address (v4 or v6) will be requested. The prefix-only indicates\n"
        "    that the IPv6 prefix will be requested. The address-and-prefix\n"
        "    indicates that both IPv6 address and prefix will be requested.\n"
        "    The '-e prefix-only' and -'e address-and-prefix' must not be\n"
        "    used with -4.\n"
        "-E<time-offset>: Offset of the (DHCPv4) secs field / (DHCPv6)\n"
        "    elapsed-time option in the (second/request) template.\n"
        "    The value 0 disables it.\n"
        "-f<renew-rate>: Rate at which DHCPv4 or DHCPv6 renew requests are sent\n"
        "    to a server. This value is only valid when used in conjunction\n"
        "    with the exchange rate (given by -r<rate>).  Furthermore the sum of\n"
        "    this value and the release-rate (given by -F<rate) must be equal\n"
        "    to or less than the exchange rate.\n"
        "-g: Select thread mode: 'single' or 'multi'. In multi-thread mode packets\n"
        "    are received in separate thread. This allows better utilisation of CPUs.\n"
        "    If more than 1 CPU is present then multi-thread mode is the default,\n"
        "    otherwise single-thread is the default.\n"
        "-h: Print this help.\n"
        "-i: Do only the initial part of an exchange: DO or SA, depending on\n"
        "    whether -6 is given.\n"
        "-I<ip-offset>: Offset of the (DHCPv4) IP address in the requested-IP\n"
        "    option / (DHCPv6) IA_NA option in the (second/request) template.\n"
        "-J<giaddr-list-file>: Text file that include multiple addresses.\n"
        "    If provided perfdhcp will choose randomly one of addresses for each\n"
        "    exchange.\n"
        "-l<local-addr|interface>: For DHCPv4 operation, specify the local\n"
        "    hostname/address to use when communicating with the server.  By\n"
        "    default, the interface address through which traffic would\n"
        "    normally be routed to the server is used.\n"
        "    For DHCPv6 operation, specify the name of the network interface\n"
        "    via which exchanges are initiated.\n"
        "-L<local-port>: Specify the local port to use\n"
        "    (the value 0 means to use the default).\n"
        "-M<mac-list-file>: A text file containing a list of MAC addresses,\n"
        "   one per line. If provided, a MAC address will be chosen randomly\n"
        "   from this list for every new exchange. In the DHCPv6 case, MAC\n"
        "   addresses are used to generate DUID-LLs. This parameter must not be\n"
        "   used in conjunction with the -b parameter.\n"
        "-N<remote-port>: Specify the remote port to use\n"
        "    (the value 0 means to use the default).\n"
        "-O<random-offset>: Offset of the last octet to randomize in the template.\n"
        "-P<preload>: Initiate first <preload> exchanges back to back at startup.\n"
        "-r<rate>: Initiate <rate> DORA/SARR (or if -i is given, DO/SA)\n"
        "    exchanges per second.  A periodic report is generated showing the\n"
        "    number of exchanges which were not completed, as well as the\n"
        "    average response latency.  The program continues until\n"
        "    interrupted, at which point a final report is generated.\n"
        "-R<range>: Specify how many different clients are used. With 1\n"
        "    (the default), all requests seem to come from the same client.\n"
        "-s<seed>: Specify the seed for randomization, making it repeatable.\n"
        "--scenario <name>: where name is 'basic' (default) or 'avalanche'.\n"
        "-S<srvid-offset>: Offset of the server-ID option in the\n"
        "    (second/request) template.\n"
        "-T<template-file>: The name of a file containing the template to use\n"
        "    as a stream of hexadecimal digits.\n"
        "-v: Report the version number of this program.\n"
        "-W<time>: Specifies exit-wait-time parameter, that makes perfdhcp wait\n"
        "    for <time> us after an exit condition has been met to receive all\n"
        "    packets without sending any new packets. Expressed in microseconds.\n"
        "-w<wrapped>: Command to call with start/stop at the beginning/end of\n"
        "    the program.\n"
        "-x<diagnostic-selector>: Include extended diagnostics in the output.\n"
        "    <diagnostic-selector> is a string of single-keywords specifying\n"
        "    the operations for which verbose output is desired.  The selector\n"
        "    keyletters are:\n"
        "   * 'a': print the decoded command line arguments\n"
        "   * 'e': print the exit reason\n"
        "   * 'i': print rate processing details\n"
        "   * 's': print first server-id\n"
        "   * 't': when finished, print timers of all successful exchanges\n"
        "   * 'T': when finished, print templates\n"
        "-X<xid-offset>: Transaction ID (aka. xid) offset in the template.\n"
        "-z<request-count>: attempt to send as many renews as possible, send\n"
        "    only <request-count> solicits or discovers, howwever many are\n"
        "    needed to get credentials from their offer / advertise\n"
        "    counterparts for future and then send only renews\n"
        "\n"
        "DHCPv4 only options:\n"
        "-B: Force broadcast handling.\n"
        "\n"
        "DHCPv6 only options:\n"
        "-c: Add a rapid commit option (exchanges will be SA).\n"
        "-F<release-rate>: Rate at which IPv6 Release requests are sent to\n"
        "    a server. This value is only valid when used in conjunction with\n"
        "    the exchange rate (given by -r<rate>).  Furthermore the sum of\n"
        "    this value and the renew-rate (given by -f<rate) must be equal\n"
        "    to or less than the exchange rate.\n"
        "-A<encapsulation-level>: Specifies that relayed traffic must be\n"
        "    generated. The argument specifies the level of encapsulation, i.e.\n"
        "    how many relay agents are simulated. Currently the only supported\n"
        "    <encapsulation-level> value is 1, which means that the generated\n"
        "    traffic is an equivalent of the traffic passing through a single\n"
        "    relay agent.\n"
        "\n"
        "The remaining options are typically used in conjunction with -r:\n"
        "\n"
        "-D<max-drop>: Abort the test immediately if max-drop requests have\n"
        "    been dropped.  max-drop must be a positive integer. If max-drop\n"
        "    includes the suffix '%', it specifies a maximum percentage of\n"
        "    requests that may be dropped before abort. In this case, testing\n"
        "    of the threshold begins after 10 requests have been expected to\n"
        "    be received.\n"
        "-n<num-request>: Initiate <num-request> transactions.  No report is\n"
        "    generated until all transactions have been initiated/waited-for,\n"
        "    after which a report is generated and the program terminates.\n"
        "-p<test-period>: Send requests for the given test period, which is\n"
        "    specified in the same manner as -d.  This can be used as an\n"
        "    alternative to -n, or both options can be given, in which case the\n"
        "    testing is completed when either limit is reached.\n"
        "-t<report>: Delay in seconds between two periodic reports.\n"
        "-C<separator>: Output reduced, an argument is a separator for periodic\n"
        "    (-t) reports generated in easy parsable mode. Data output won't be\n"
        "    changed, remain identical as in -t option.\n"
        "\n"
        "Errors:\n"
        "- tooshort: received a too short message\n"
        "- orphans: received a message which doesn't match an exchange\n"
        "   (duplicate, late or not related)\n"
        "- locallimit: reached to local system limits when sending a message.\n"
        "\n"
        "Exit status:\n"
        "The exit status is:\n"
        "0 on complete success.\n"
        "1 for a general error.\n"
        "2 if an error is found in the command line arguments.\n"
        "3 if there are no general failures in operation, but one or more\n"
        "  exchanges are not successfully completed.\n";
}

void
CommandOptions::version() const {
    std::cout << "VERSION: " << VERSION << std::endl;
}


}  // namespace perfdhcp
}  // namespace isc

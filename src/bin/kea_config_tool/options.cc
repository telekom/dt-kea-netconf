// (C) 2020 Deutsche Telekom AG.
//
// Deutsche Telekom AG and all other contributors /
// copyright owners license this file to you under the Apache
// License, Version 2.0 (the "License"); you may not use this
// file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the License for the
// specific language governing permissions and limitations
// under the License.

#include <config.h>

#include <asiolink/io_address.h>
#include <kea_config_tool/controller.h>
#include <kea_config_tool/options.h>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include <iostream>

namespace po = boost::program_options;

using namespace std;

using isc::asiolink::IOAddress;
using isc::dhcp::DHCP_SPACE_UNKNOWN;
using isc::dhcp::DHCP_SPACE_V4;
using isc::dhcp::DHCP_SPACE_V6;

namespace isc {
namespace kea_config_tool {

CommandOptions::CommandOptions(std::string const& bin_name, std::ostream& out)
    : bin_name_(bin_name), out_(out) {
}

void CommandOptions::parse(int const argc, char* const argv[]) {
    initializeArgumentParser(argc, argv);

    if (count("dbinit")) {
        parseDbInit(argc, argv);
        return;
    }

    if (count("help")) {
        usage();
        exit(0);
    }

    if (count("version") || count("detailed-version")) {
        showVersion();
        exit(0);
    }

    sanityChecks();

    if (count("shard")) {
        parseShard();
    }

    if (count("master")) {
        parseMaster();
    }
}

boost::program_options::variable_value const&
    CommandOptions::operator[](std::string const& parameter) const {
    return options_[parameter];
}

int CommandOptions::count(std::string const& parameter) const {
    return options_.count(parameter);
}

boost::program_options::variable_value const&
CommandOptions::get(std::string const& parameter) const {
    return options_[parameter];
}

void CommandOptions::parseDbInit(int const argc, char* const argv[]) {
    kea_admin_parameters_ = "";
    bool found(false);
    bool first(true);
    for (int i = 0; i < argc; ++i) {
        string argument(argv[i]);
        if (argument == "--dbinit") {
            found = true;
        } else if (found) {
            if (first) {
                first = false;
            } else {
                kea_admin_parameters_ += " ";
            }
            kea_admin_parameters_ += argument;
        }
    }
}

void CommandOptions::parseMaster() {
    if (count("set-servers")) {
        if (count("list")) {
            vector<string> list = get("list").as<vector<string>>();
            for (string const& item : list) {
                item_list_.insert(item);
            }
        }
    } else if (count("get-servers")) {
        // Nothing to be done
    } else {
        isc_throw(InvalidUsage, "ambiguous operation: expected --set-servers or --get-servers");
    }
}

void CommandOptions::parseShard() {
    vector<string> list;
    if (count("list")) {
        list = get("list").as<vector<string>>();
    }
    if (count("get-subnets")) {
        string shard;
        for (string const& item : list) {
            if (item.empty()) {
                // Don't store empty shards and empty addresses.
                continue;
            }
            if (item.find(".") != string::npos || item.find(":") != string::npos) {
                if (shard.empty()) {
                    // Don't store non-empty addresses on an empty shard.
                    continue;
                }
                size_t const position(item.find("/"));
                if (position != string::npos) {
                    IOAddress address(IOAddress(item.substr(0, position)));
                    uint16_t prefix_length =
                        boost::lexical_cast<uint16_t>(item.substr(position + 1));
                    // Check for correctness.
                    if ((address.isV4() && prefix_length > 32) ||
                        (address.isV6() && prefix_length > 128)) {
                        isc_throw(BadValue, "Invalid prefix length " << prefix_length
                                                                     << " specified for subnet "
                                                                     << item);
                    }
                    subnet_list_[shard].push_back(item);
                }
            } else {
                shard = item;
                item_list_.insert(shard);
            }
        }
    } else if (count("set-config") || count("get-config") || count("add-subnets") ||
               count("del-subnets") || count("set-subnets")) {
        for (string const& item : list) {
            item_list_.insert(item);
        }
    } else {
        isc_throw(InvalidUsage, "ambiguous operation: expected --set-config, --get-config, "
                                "--add-subnets, --del-subnets, --set-subnets, --get-subnets");
    }
}

void CommandOptions::initializeArgumentParser(int const argc, char* const argv[]) {
    options_.clear();
    po::options_description dbinit;
    dbinit.add_options()("dbinit", "executes kea-admin passing all the following arguments to it");
    po::store(po::command_line_parser(argc, argv).options(dbinit).allow_unregistered().run(),
              options_);
    po::notify(options_);

    if (count("dbinit")) {
        // Special case, all other arguments are passed to kea-admin.
        return;
    }

    po::options_description description;
    description.add_options()  //
        ("help,h", "displays this help")  //
        ("debug,d", "enables debug mode")  //
        ("version,v", "displays version number")  //
        ("dry-run,t", "displays version number")  //
        ("detailed-version,V", "displays detailed version number")  //
        ("master", "handles server configuration data in the master database")  //
        ("shard", "handles server configuration data in the shard database")  //
        ("v4,4", "operate on DHCPv4 server configuration")  //
        ("v6,6", "operate on DHCPv6 server configuration")  //
        ("set-servers", "writes the provided servers configuration into master database")  //
        ("get-servers",
         "retrieves servers configuration from the master database and writes them locally")  //
        ("set-config", "writes the provided server configuration into shards")  //
        ("get-config", "retrieves server configuration from shards and writes it locally")  //
        ("add-subnets", "adds the provided subnet server configuration into shards")  //
        ("del-subnets", "deletes the provided subnet server configuration from shards")  //
        ("set-subnets", "updates the provided subnet server configuration into shards")  //
        ("get-subnets",
         "retrieves subnet server configuration from shards and writes it locally")  //
        ("credentials-file", po::value<string>(),
         "configuration file which provides the master database backend credentials.")  //
        ("shards-directory-path", po::value<string>(),
         "configuration file which provides the master database backend credentials.")  //
        ("list", po::value<vector<string>>(), "list of shards or subnets");

    po::positional_options_description positional;
    positional.add("credentials-file", 1);
    positional.add("shards-directory-path", 1);
    positional.add("list", -1);

    po::store(po::command_line_parser(argc, argv).options(description).positional(positional).run(),
              options_);
    po::notify(options_);
}

void CommandOptions::sanityChecks() {
    if (count("shard") + count("master") != 1) {
        isc_throw(InvalidUsage, "ambiguous mode: expected shard or master");
    }

    if (count("v4") + count("v6") != 1) {
        isc_throw(InvalidUsage, "ambiguous version: expected 4 or 6");
    }

    if (count("credentials-file") != 1) {
        isc_throw(InvalidUsage, "credentials-file is mandatory");
    }

    if (count("shards-directory-path") != 1) {
        isc_throw(InvalidUsage, "shards-directory-path is mandatory");
    }

    if (get("shards-directory-path").as<string>().empty()) {
        isc_throw(InvalidUsage, "shards-directory-path cannot be empty");
    }

    if (count("get-config") == 1 && count("list") == 0) {
        isc_throw(InvalidUsage, "list is mandatory with get-config");
    }

    if (count("get-subnets") == 1 && count("list") == 0) {
        isc_throw(InvalidUsage, "list is mandatory with get-subnets");
    }

    if (count("get-servers") == 1 && count("list") != 0) {
        isc_throw(InvalidUsage, "extraneous arguments");
    }
}

std::string CommandOptions::getVersion() {
    std::stringstream tmp;

    tmp << VERSION;
    if (options_.count("detailed-version")) {
        tmp << std::endl << EXTENDED_VERSION << std::endl;
    }

    return tmp.str();
}

void CommandOptions::showVersion() {
    out_ << getVersion() << std::endl;
}

void CommandOptions::usage() {
    if (count("shard")) {
        usageShard();
    } else if (count("master")) {
        usageMaster();
    } else {
        usageGeneric();
    }
}

void CommandOptions::usageGeneric() {
    cout << R"(    SYNOPSIS
        )"
         << bin_name_ << R"( [PARAMETERS...] COMMAND ARGUMENTS...

        kea-config-tool is a DHCP configuration tool used by system administrators. It provides a way
        of initializing and maintaining Kea server configuration when the server uses a database
        backend for leases, reservations and stateless configuration. The tool can be used to manage
        both DHCPv4 and DHCPv6 server configurations.

        Dependencies to the Kea server are statically linked so this is a standalone tool and no Kea
        libraries are required in order to run it.

    COMMAND (-h | --help for individual help)
        --dbinit:    a wrapper for kea-admin; it executes kea-admin and returns it's exit status.
        --master:    inserts or retrieve server configuration data into or from the master database.
        --shard:     inserts or retrieve server configuration data into or from the shard database.

    PARAMETERS
        -d | --debug:               enables debug mode with extra verbose data
        -h | --help:                displays this help and exits
        -v | --version:             displays version number and exits
        -V | --detailed-version:    displays detailed version number and exits
        -t | --dry-run:             does everything except run the Cassandra statements
)";
}

void CommandOptions::usageMaster() {
    cout << R"(    SYNOPSIS
        )"
         << bin_name_ << R"( [PARAMETERS...] --master ARGUMENTS...

    SCOPE
        --master: configures a master database

    USAGE
        --master --set-servers -4|-6 credentials-file input-shards-directory-path [list-of-shards]
        --master --get-servers -4|-6 credentials-file output-shards-directory-path

    ARGUMENTS
        --set-servers:                  writes the provided servers configuration into master database.
        --get-servers:                  retrieves servers configuration from the master database and writes them locally.
        --v4 | -4:                      indicates a DHCPv4 operation.
        --v6 | -6:                      indicates a DHCPv6 operation.
        credentials-file:               the configuration file which provides the database backend credentials
        input-shards-directory-path:    the path from the master configuration is loaded
        output-shards-directory-path:   the path where the master configuration will be stored
        list-of-shards:                 the space-separated shard names for which the command is applied; if empty, the command is applied for all found shards

    PARAMETERS
        -d | --debug:               enables debug mode with extra verbose data
        -h | --help:                displays this help and exits
        -v | --version:             displays version number and exits
        -V | --detailed-version:    displays detailed version number and exits
        -t | --dry-run:             does everything except run the Cassandra statements
)";
}

void CommandOptions::usageShard() {
    cout << R"(    SYNOPSIS
        )"
         << bin_name_ << R"( [PARAMETERS...] --shard ARGUMENTS...

    SCOPE
        --shard: configures a shard database

    USAGE
        --shard --set-config -4|-6 credentials-file input-shards-directory-path [list-of-shards]
        --shard --get-config -4|-6 credentials-file output-shards-directory-path list-of-shards
        --shard --add-subnets -4|-6 credentials-file input-shards-directory-path [list-of-shards]
        --shard --del-subnets -4|-6 credentials-file input-shards-directory-path [list-of-shards]
        --shard --set-subnets -4|-6 credentials-file input-shards-directory-path [list-of-shards]
        --shard --get-subnets -4|-6 credentials-file output-shards-directory-path list-of-subnets

    ARGUMENTS
        --set-config:                   writes the provided server configuration into shards
        --get-config:                   retrieves server configuration from shards and writes it locally
        --add-subnets:                  adds the provided subnet server configuration into shards
        --del-subnets:                  deletes the provided subnet server configuration from shards
        --set-subnets:                  updates the provided subnet server configuration into shards
        --get-subnets:                  retrieves subnet server configuration from shards and writes it locally
        --v4 | -4:                      indicates a DHCPv4 operation.
        --v6 | -6:                      indicates a DHCPv6 operation.
        credentials-file:               the configuration file which provides the database backend credentials
        input-shards-directory-path:    the path from the shards configuration is loaded
        output-shards-directory-path:   the path where the shards configuration will be stored
        list-of-shards:                 the space-separated shard names for which the command is applied; if empty, the command is applied for all found shards
        list-of-subnets:                the list of shard-subnet pairs in the form "shard1 1.2.3.4/16 2001:db8::/64 2001:db9::/64 shard2 3001:db8::/64" for which the command is applied

    PARAMETERS
        -d | --debug:               enables debug mode with extra verbose data
        -h | --help:                displays this help and exits
        -v | --version:             displays version number and exits
        -V | --detailed-version:    displays detailed version number and exits
        -t | --dry-run:             does everything except run the Cassandra statements
)";
}

}  // namespace kea_config_tool
}  // namespace isc

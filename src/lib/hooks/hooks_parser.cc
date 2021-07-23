// Copyright (C) 2017 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <cc/data.h>
#include <cc/dhcp_config_error.h>
#include <hooks/hooks_parser.h>
#include <util/strutil.h>

#include <vector>

using namespace std;
using namespace isc::data;
using namespace isc::hooks;
using namespace isc::dhcp;

namespace isc {
namespace hooks {

// @todo use the flat style, split into list and item

void HooksLibrariesParser::parse(HooksConfig& libraries, ElementPtr const& value) {
    // Initialize.
    libraries.clear();

    if (!value) {
        isc_throw(DhcpConfigError, "Tried to parse null hooks libraries");
    }

    // This is the new syntax.  Iterate through it and get each map.
    for (ElementPtr const& library_entry : value->listValue()) {
        // Is it a map?
        if (library_entry->getType() != Element::map) {
            isc_throw(DhcpConfigError, "hooks library configuration error:"
                                       " one or more entries in the hooks-libraries list is not"
                                       " a map ("
                                           << library_entry->getPosition() << ")");
        }

        // Iterate through each element in the map.  We check
        // whether we have found a library element.
        bool lib_found(false);
        string libname;
        string parameters_key;
        ElementPtr parameters;
        for (auto const& [key, value] : library_entry->mapValue()) {
            if (key == "library") {
                if (value->getType() != Element::string) {
                    isc_throw(DhcpConfigError, "hooks library configuration"
                                               " error: value of 'library' element is not a string"
                                               " giving the name of a hooks library ("
                                                   << value->getPosition() << ")");
                }
                continue;
            }

            if (key == "library-path") {
                if (value->getType() != Element::string) {
                    isc_throw(DhcpConfigError,
                              "hooks library configuration"
                              " error: value of 'library-path' element is not a string"
                              " giving the path to a hooks library ("
                                  << value->getPosition() << ")");
                }

                // Get the path of the library and add it to the list after
                // removing quotes.
                libname = value->stringValue();

                // Remove leading/trailing quotes and any leading/trailing
                // spaces.
                isc::util::str::erase_all(libname, "\"");
                libname = isc::util::str::trim(libname);
                if (libname.empty()) {
                    isc_throw(DhcpConfigError, "hooks library configuration"
                                               " error: value of 'library-path' element must not be"
                                               " blank ("
                                                   << value->getPosition() << ")");
                }

                // Note we have found the library path.
                lib_found = true;
                continue;
            }

            // If there are parameters, let's remember them.
            bool parameters_found_this_turn(false);
            for (string const& p : {"lawful-interception-parameters", "policy-engine-parameters",
                                    "parameters"}) {
                if (key == p) {
                    parameters_found_this_turn = true;
                    if (!parameters) {
                        parameters = value;
                        parameters_key = p;
                        break;
                    }
                }
            }
            if (!parameters_found_this_turn) {
                // For all other parameters we will throw.
                isc_throw(DhcpConfigError, "unknown hooks library parameter: "
                                               << key << "(" << library_entry->getPosition()
                                               << ")");
            }
        }

        if (!lib_found) {
            isc_throw(DhcpConfigError, "hooks library configuration error:"
                                       " one or more hooks-libraries elements are missing the"
                                       " path of the library"
                                           << " (" << library_entry->getPosition() << ")");
        }

        libraries.add(libname, parameters_key, parameters);
    }
}

}  // namespace hooks
}  // namespace isc

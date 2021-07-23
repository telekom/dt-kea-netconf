// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <exceptions/exceptions.h>
#include <netconf/netconf_controller.h>

#include <cstdlib>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

using isc::Exception;
using isc::netconf::NetconfController;
using isc::process::DControllerBasePtr;
using isc::process::InvalidUsage;
using isc::process::VersionMessage;

int main(int argc, char* argv[]) {
    int ret = EXIT_SUCCESS;

    // Launch the controller passing in command line arguments.
    // Exit program with the controller's return code.
    try {
        // Instantiate/fetch the application controller singleton.
        DControllerBasePtr& controller = NetconfController::instance();

        // 'false' value disables test mode.
        controller->launch(argc, argv, false);
    } catch (const VersionMessage& ex) {
        string msg(ex.what());
        if (!msg.empty()) {
            cout << msg << endl;
        }
    } catch (const InvalidUsage& ex) {
        string msg(ex.what());
        if (!msg.empty()) {
            cerr << msg << endl;
        }
        ret = EXIT_FAILURE;
    } catch (const Exception& ex) {
        cerr << "Service failed: " << ex.what() << endl;
        ret = EXIT_FAILURE;
    }

    return ret;
}

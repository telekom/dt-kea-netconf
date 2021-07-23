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

#include <kea_config_tool/controller.h>

using namespace isc;
using namespace isc::config;
using namespace isc::db;
using namespace isc::dhcp;
using namespace isc::log;
using namespace isc::data;

using namespace std;

using isc::admin::KeaAdmin;
using isc::hooks::InvalidHooksLibraries;

namespace isc {
namespace kea_config_tool {

Controller::Controller()
    : out_(cout), app_name_(APP_NAME), bin_name_(BIN_NAME), logger_(app_name_.c_str()),
      options_(bin_name_, out_) {
}

Controller::Controller(string const& bin_name, unordered_map<string, string> env)
    : out_(stream_), app_name_(APP_NAME), bin_name_(bin_name), logger_(app_name_.c_str()),
      options_(bin_name_, out_), env_(env) {
}

tuple<int, string> Controller::runExceptionSafe(string const& arguments) {
    // Set environment and make sure it is reverted back at the end.
    Env e(env_);

    // Do shell word expansion.
    string full_arguments = bin_name_ + " " + arguments;
    wordexp_t p;
    wordexp(full_arguments.c_str(), &p, 0);
    isc::util::Defer _([&] {
        // Free resources.
        wordfree(&p);

        // Retain output and clear stream_ for subsequent runs.
        stream_.str(string());
    });

    // Run.
    string output(stream_.str());
    return make_tuple(runExceptionSafe(p.we_wordc, p.we_wordv), output);
}

void Controller::run(string const& arguments) {
    // Set environment and make sure it is reverted back at the end.
    Env e(env_);

    // Do shell word expansion.
    string full_arguments = bin_name_ + " " + arguments;
    wordexp_t p;
    wordexp(full_arguments.c_str(), &p, 0);
    isc::util::Defer _([&] {
        // Free resources.
        wordfree(&p);
    });

    // Run.
    run(p.we_wordc, p.we_wordv);
}

int Controller::runExceptionSafe(int const argc, char* const argv[]) {
    string exception_what;
    exit_code_t return_code(EXIT_SUCCESSFUL);

    // Launch the controller passing in command line arguments.
    // Exit program with the controller's return code.
    try {
        run(argc, argv);
#ifdef TERASTREAM_FULL_TRANSACTIONS
    } catch (TransactionException const& exception) {
        exception_what = exception.what();
        return_code = EXIT_TRANSACTION_EXCEPTION;
#endif  // TERASTREAM_FULL_TRANSACTIONS
    } catch (TimestampHasChanged const& exception) {
        exception_what = exception.what();
        return_code = EXIT_TIMESTAMP_HAS_CHANGED;
    } catch (StatementNotApplied const& exception) {
        exception_what = exception.what();
        return_code = EXIT_TIMESTAMP_HAS_CHANGED;
    } catch (InvalidHooksLibraries const& exception) {
        exception_what = exception.what();
        return_code = EXIT_HOOKS_LIBRARY_LOAD_FAIL;
    } catch (InvalidUsage const& exception) {
        exception_what = exception.what();
        return_code = EXIT_INVALID_USAGE;
        options_.usage();
    } catch (boost::exception const& exception) {
        exception_what = boost::diagnostic_information(exception);
        return_code = EXIT_BOOST_EXCEPTION;
    } catch (exception const& exception) {
        exception_what = exception.what();
        return_code = EXIT_STD_EXCEPTION;
    } catch (...) {
        exception_what = "unknown exception";
        return_code = EXIT_UNKNOWN_EXCEPTION;
    }

    if (return_code == EXIT_SUCCESSFUL) {
        LOG_INFO(logger_, CONFIG_TOOL_SUCCESS);
        out_ << "kea-config-tool exited successfully." << endl;
    } else {
        LOG_ERROR(logger_, CONFIG_TOOL_FAIL)
            .arg(exception_what)
            .arg(magic_enum::enum_name(return_code))
            .arg(return_code);
        out_ << "ERROR: " << exception_what << ", exiting with code "
             << magic_enum::enum_name(return_code) << " (" << return_code << ")" << endl;
    }
    return return_code;
}

void Controller::run(int const argc, char* const argv[]) {
    try {
        try {
            options_.parse(argc, argv);
        } catch (boost::exception const& ex) {
            isc_throw(InvalidUsage, boost::diagnostic_information(ex));
        }

        if (options_.count("dbinit")) {
            KeaAdmin kea_admin;
            auto const& [exit_code, output](kea_admin.run(options_.kea_admin_parameters_));
            out_ << output << endl;
            if (exit_code != 0) {
                isc_throw(Unexpected, "kea-admin has exited with code " << to_string(exit_code));
            }
        } else if (options_.count("v4")) {
            VersionedController<DHCP_SPACE_V4, ConfigurationConstants::NOT_IETF>(*this).launch();
        } else if (options_.count("v6")) {
            VersionedController<DHCP_SPACE_V6, ConfigurationConstants::NOT_IETF>(*this).launch();
        }
    } catch (InvalidUsage const& ex) {
        if (ex.getShowAppHelp()) {
            options_.usage();
        }
        throw;
    }

    out_ << "Success!" << endl;
}

template <>
ElementPtr Controller::configureDhcpvServer<DHCP_SPACE_V4>(DhcpvSrv<DHCP_SPACE_V4>& server,
                                                           ElementPtr const& config_set,
                                                           bool const check_only /* = false */) {
    return configureDhcp4Server(server, config_set, check_only);
}

template <>
ElementPtr Controller::configureDhcpvServer<DHCP_SPACE_V6>(DhcpvSrv<DHCP_SPACE_V6>& server,
                                                           ElementPtr const& config_set,
                                                           bool const check_only /* = false */) {
    return configureDhcp6Server(server, config_set, check_only);
}

mutex Controller::mutex_;

}  // namespace kea_config_tool
}  // namespace isc

// Copyright (C) 2014-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <cc/command_interpreter.h>
#include <cc/data.h>
#include <cfgrpt/config_report.h>
#include <config/command_mgr.h>
#include <database/dbaccess_parser.h>
#include <dhcp/libdhcp++.h>
#include <dhcp4/ctrl_dhcp4_srv.h>
#include <dhcp4/dhcp4_log.h>
#include <dhcp4/dhcp4to6_ipc.h>
#include <dhcp4/json_config_parser.h>
#include <dhcp4/parser_context.h>
#include <dhcpsrv/cfg_db_access.h>
#include <dhcpsrv/cfg_multi_threading.h>
#include <dhcpsrv/cfgmgr.h>
#include <dhcpsrv/configuration_reader.h>
#include <dhcpsrv/db_type.h>
#include <dhcpsrv/subnet_mgr.h>
#include <dhcpsrv/subnet_mgr_factory.h>
#include <hooks/hooks.h>
#include <hooks/hooks_manager.h>
#include <stats/stats_mgr.h>
#include <util/magic_enum.hpp>
#include <util/multi_threading_mgr.h>

#include <signal.h>

#include <algorithm>
#include <memory>
#include <sstream>

using namespace isc::config;
using namespace isc::data;
using namespace isc::db;
using namespace isc::dhcp;
using namespace isc::hooks;
using namespace isc::stats;
using namespace isc::util;
using namespace std;
namespace ph = std::placeholders;

namespace {

/// Structure that holds registered hook indexes.
struct CtrlDhcp4Hooks {
    int hooks_index_dhcp4_srv_configured_;

    /// Constructor that registers hook points for the DHCPv4 server.
    CtrlDhcp4Hooks() {
        hooks_index_dhcp4_srv_configured_ = HooksManager::registerHook("dhcp4_srv_configured");
    }
};

// Declare a Hooks object. As this is outside any function or method, it
// will be instantiated (and the constructor run) when the module is loaded.
// As a result, the hook indexes will be defined before any method in this
// module is called.
CtrlDhcp4Hooks Hooks;

/// @brief Signals handler for DHCPv4 server.
///
/// This signal handler handles the following signals received by the DHCPv4
/// server process:
/// - SIGHUP - triggers server's dynamic reconfiguration.
/// - SIGTERM - triggers server's shut down.
/// - SIGINT - triggers server's shut down.
///
/// @param signo Signal number received.
void signalHandler(int signo) {
    // SIGHUP signals a request to reconfigure the server.
    if (signo == SIGHUP) {
        ControlledDhcpv4Srv::processCommand("config-reload",
                                            ElementPtr());
    } else if ((signo == SIGTERM) || (signo == SIGINT)) {
        ControlledDhcpv4Srv::processCommand("shutdown",
                                            ElementPtr());
    }
}

}

namespace isc {
namespace dhcp {

ControlledDhcpv4Srv* ControlledDhcpv4Srv::server_ = NULL;

void
ControlledDhcpv4Srv::init(const std::string& file_name) {
    // Keep the call timestamp.
    start_ = boost::posix_time::second_clock::universal_time();

    // Configure the server using JSON file.
    ElementPtr const& result(loadConfigFile(file_name));

    int rcode;
    ElementPtr comment(isc::config::parseAnswer(rcode, result));
    if (rcode != CONTROL_RESULT_SUCCESS) {
        string reason = comment ? comment->stringValue() :
            "no details available";
        isc_throw(isc::BadValue, reason);
    }

    // We don't need to call openActiveSockets() or startD2() as these
    // methods are called in processConfig() which is called by
    // processCommand("config-set", ...)

    // Set signal handlers. When the SIGHUP is received by the process
    // the server reconfiguration will be triggered. When SIGTERM or
    // SIGINT will be received, the server will start shutting down.
    signal_set_.reset(new isc::util::SignalSet(SIGINT, SIGHUP, SIGTERM));
    // Set the pointer to the handler function.
    signal_handler_ = signalHandler;
}

void ControlledDhcpv4Srv::cleanup() {
    // Nothing to do here. No need to disconnect from anything.
}

/// @brief Configure DHCPv4 server using the configuration file specified.
///
/// This function is used to both configure the DHCP server on its startup
/// and dynamically reconfigure the server when SIGHUP signal is received.
///
/// It fetches DHCPv4 server's configuration from the 'Dhcp4' section of
/// the JSON configuration file.
///
/// @param file_name Configuration file location.
/// @return status of the command
ElementPtr
ControlledDhcpv4Srv::loadConfigFile(const std::string& file_name) {
    // This is a configuration backend implementation that reads the
    // configuration from a JSON file.

    ElementPtr json;
    ElementPtr result;

    // Basic sanity check: file name must not be empty.
    try {
        if (file_name.empty()) {
            // Basic sanity check: file name must not be empty.
            isc_throw(isc::BadValue, "JSON configuration file not specified. Please "
                      "use -c command line option.");
        }

        // Read contents of the file and parse it as JSON
        Parser4Context parser;
        json = parser.parseFile(file_name, Parser4Context::PARSER_DHCP4);
        if (!json) {
            isc_throw(isc::BadValue, "no configuration found");
        }

        // Let's do sanity check before we call json->get() which
        // works only for map.
        if (json->getType() != Element::map) {
            isc_throw(isc::BadValue,
                      "Configuration file is expected to be a map, i.e., start with { and end with "
                      "} and contain at least an entry called 'Dhcp4' that itself is a map. "
                          << file_name << " is a valid JSON, but its top element is a "
                          << magic_enum::enum_name(json->getType()) << " instead of a "
                          << magic_enum::enum_name(Element::map)
                          << " Did you forget to add { } around your configuration?");
        }

        // Use parsed JSON structures to configure the server
        result = ControlledDhcpv4Srv::processCommand("config-set", json);
        if (!result) {
            // Undetermined status of the configuration. This should never
            // happen, but as the configureDhcp4Server returns a pointer, it is
            // theoretically possible that it will return NULL.
            isc_throw(isc::BadValue, "undefined result of "
                      "processCommand(\"config-set\", json)");
        }

        // Now check is the returned result is successful (rcode=0) or not
        // (see @ref isc::config::parseAnswer).
        int rcode;
        ElementPtr const& comment(isc::config::parseAnswer(rcode, result));
        if (rcode != CONTROL_RESULT_SUCCESS) {
            string reason = comment ? comment->stringValue() :
                "no details available";
            isc_throw(isc::BadValue, reason);
        }
    }  catch (const std::exception& ex) {
        // If configuration failed at any stage, we drop the staging
        // configuration and continue to use the previous one.
        CfgMgr::instance().rollback();

        LOG_ERROR(dhcp4_logger, DHCP4_CONFIG_LOAD_FAIL)
            .arg(file_name).arg(ex.what());
        isc_throw(isc::BadValue, "configuration error using file '"
                  << file_name << "': " << ex.what());
    }

    LOG_WARN(dhcp4_logger, DHCP4_MULTI_THREADING_INFO)
        .arg(MultiThreadingMgr::instance().getMode() ? "yes" : "no")
        .arg(MultiThreadingMgr::instance().getThreadPoolSize())
        .arg(MultiThreadingMgr::instance().getPacketQueueSize());

    return (result);
}

ElementPtr
ControlledDhcpv4Srv::commandShutdownHandler(const string&, ElementPtr args) {
    if (!ControlledDhcpv4Srv::getInstance()) {
        LOG_WARN(dhcp4_logger, DHCP4_NOT_RUNNING);
        return(createAnswer(CONTROL_RESULT_ERROR, "Shutdown failure."));
    }

    int exit_value = 0;
    if (args) {
        // @todo Should we go ahead and shutdown even if the args are invalid?
        if (args->getType() != Element::map) {
            return (createAnswer(CONTROL_RESULT_ERROR, "Argument must be a map"));
        }

        ElementPtr param = args->get("exit-value");
        if (param)  {
            if (param->getType() != Element::integer) {
                return (createAnswer(CONTROL_RESULT_ERROR,
                                     "parameter 'exit-value' is not an integer"));
            }

            exit_value = param->intValue();
        }
    }

    ControlledDhcpv4Srv::getInstance()->shutdownServer(exit_value);
    return (createAnswer(CONTROL_RESULT_SUCCESS, "Shutting down."));
}

ElementPtr
ControlledDhcpv4Srv::commandLibReloadHandler(const string&, ElementPtr) {
    // stop thread pool (if running)
    MultiThreadingCriticalSection cs;

    // Clear the packet queue.
    MultiThreadingMgr::instance().getThreadPool().reset();

    try {
        /// Get list of currently loaded libraries and reload them.
        HookLibsCollection loaded = HooksManager::getLibraryInfo();
        HooksManager::prepareUnloadLibraries();
        static_cast<void>(HooksManager::unloadLibraries());
        bool status = HooksManager::loadLibraries(loaded);
        if (!status) {
            isc_throw(Unexpected, "Failed to reload hooks libraries.");
        }
    } catch (const std::exception& ex) {
        LOG_ERROR(dhcp4_logger, DHCP4_HOOKS_LIBS_RELOAD_FAIL);
        ElementPtr const& answer(isc::config::createAnswer(
            CONTROL_RESULT_ERROR, "Failed to reload hooks libraries: " + string(ex.what())));
        return (answer);
    }
    ElementPtr const& answer(isc::config::createAnswer(CONTROL_RESULT_SUCCESS,
                             "Hooks libraries successfully reloaded."));
    return (answer);
}

ElementPtr
ControlledDhcpv4Srv::commandConfigReloadHandler(const string&,
                                                ElementPtr /*args*/) {
    // Get configuration file name.
    std::string file = ControlledDhcpv4Srv::getInstance()->getConfigFile();
    try {
        LOG_INFO(dhcp4_logger, DHCP4_DYNAMIC_RECONFIGURATION).arg(file);
        return (loadConfigFile(file));
    } catch (const std::exception& ex) {
        // Log the unsuccessful reconfiguration. The reason for failure
        // should be already logged. Don't rethrow an exception so as
        // the control channel perhaps keeps working.
        LOG_FATAL(dhcp4_logger, DHCP4_DYNAMIC_RECONFIGURATION_FAIL)
            .arg(file);
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "Config reload failed: " + string(ex.what())));
    }
}

ElementPtr
ControlledDhcpv4Srv::commandConfigGetHandler(const string&,
                                             ElementPtr /*args*/) {
    ElementPtr const& config(CfgMgr::instance().getCurrentCfg()->toElement());

    return (createAnswer(0, config));
}

ElementPtr
ControlledDhcpv4Srv::commandConfigWriteHandler(const string&, ElementPtr args) {
    string filename;

    if (args) {
        if (args->getType() != Element::map) {
            return (createAnswer(CONTROL_RESULT_ERROR, "Argument must be a map"));
        }
        ElementPtr const& filename_param(args->get("filename"));
        if (filename_param) {
            if (filename_param->getType() != Element::string) {
                return (createAnswer(CONTROL_RESULT_ERROR,
                                     "passed parameter 'filename' is not a string"));
            }
            filename = filename_param->stringValue();
        }
    }

    if (filename.empty()) {
        // filename parameter was not specified, so let's use whatever we remember
        // from the command-line
        filename = getConfigFile();
    }

    if (filename.empty()) {
        return (createAnswer(CONTROL_RESULT_ERROR, "Unable to determine filename."
                             "Please specify filename explicitly."));
    }

    // Ok, it's time to write the file.
    size_t size = 0;
    try {
        ElementPtr const& cfg(CfgMgr::instance().getCurrentCfg()->toElement());
        size = writeConfigFile(filename, cfg);
    } catch (const isc::Exception& ex) {
        return (createAnswer(CONTROL_RESULT_ERROR, string("Error during write-config:")
                             + ex.what()));
    }
    if (size == 0) {
        return (createAnswer(CONTROL_RESULT_ERROR, "Error writing configuration to "
                             + filename));
    }

    // Ok, it's time to return the successful response.
    ElementPtr params = Element::createMap();
    params->set("size", Element::create(static_cast<long long>(size)));
    params->set("filename", Element::create(filename));

    return (createAnswer(CONTROL_RESULT_SUCCESS, "Configuration written to "
                         + filename + " successful", params));
}

ElementPtr
ControlledDhcpv4Srv::commandConfigSetHandler(const string&,

                                             ElementPtr args) {
    const int status_code = CONTROL_RESULT_ERROR;
    ElementPtr dhcp4;
    string message;

    // Command arguments are expected to be:
    // { "Dhcp4": { ... } }
    if (!args) {
        message = "Missing mandatory 'arguments' parameter.";
    } else {
        dhcp4 = args->get("Dhcp4");
        if (!dhcp4) {
            message = "Missing mandatory 'Dhcp4' parameter.";
        } else if (dhcp4->getType() != Element::map) {
            message = "'Dhcp4' parameter expected to be a map.";
        }
    }

    // Check unsupported objects.
    if (message.empty()) {
        for (auto obj : args->mapValue()) {
            const string& obj_name = obj.first;
            if (obj_name != "Dhcp4") {
                LOG_ERROR(dhcp4_logger, DHCP4_CONFIG_UNSUPPORTED_OBJECT)
                    .arg(obj_name);
                if (message.empty()) {
                    message = "Unsupported '" + obj_name + "' parameter";
                } else {
                    message += " (and '" + obj_name + "')";
                }
            }
        }
        if (!message.empty()) {
            message += ".";
        }
    }

    if (!message.empty()) {
        // Something is amiss with arguments, return a failure response.
        ElementPtr const& result(isc::config::createAnswer(status_code,
                                                           message));
        return (result);
    }

    // stop thread pool (if running)
    MultiThreadingCriticalSection cs;

    ++CfgMgr::instance().config_set_command_received_count_;

    // Don't reconfigure control-socket & notifications in order to maintain communications.
    if (CfgMgr::instance().getCurrentCfg()->getControlSocketInfo()) {
        dhcp4->set("control-socket", CfgMgr::instance().getCurrentCfg()->getControlSocketInfo());
    }
    if (CfgMgr::instance().getCurrentCfg()->getNotificationsSocketInfo()) {
        dhcp4->set("notifications", CfgMgr::instance().getCurrentCfg()->getNotificationsSocketInfo());
    }

    // disable multi-threading (it will be applied by new configuration)
    // this must be done in order to properly handle MT to ST transition
    // when 'multi-threading' structure is missing from new config
    MultiThreadingMgr::instance().apply(false, 0, 0);

    // We are starting the configuration process so we should remove any
    // staging configuration that has been created during previous
    // configuration attempts.
    CfgMgr::instance().rollback();

    // Parse the logger configuration explicitly into the staging config.
    // Note this does not alter the current loggers, they remain in
    // effect until we apply the logging config below.  If no logging
    // is supplied logging will revert to default logging.
    Daemon::configureLogger(dhcp4, CfgMgr::instance().getStagingCfg());

    // Let's apply the new logging. We do it early, so we'll be able to print
    // out what exactly is wrong with the new config in case of problems.
    CfgMgr::instance().getStagingCfg()->applyLoggingCfg();

    // Now we configure the server proper.
    ElementPtr const& result(processConfig(dhcp4));

    // If the configuration parsed successfully, apply the new logger
    // configuration and the commit the new configuration.  We apply
    // the logging first in case there's a configuration failure.
    int rcode = 0;
    isc::config::parseAnswer(rcode, result);
    if (rcode == CONTROL_RESULT_SUCCESS) {
        CfgMgr::instance().getStagingCfg()->applyLoggingCfg();

        // Use new configuration.
        CfgMgr::instance().commit();
    } else {
        // Ok, we applied the logging from the upcoming configuration, but
        // there were problems with the config. As such, we need to back off
        // and revert to the previous logging configuration.
        CfgMgr::instance().getCurrentCfg()->applyLoggingCfg();

        if (CfgMgr::instance().getCurrentCfg()->getSequence() != 0) {
            // Not initial configuration so someone can believe we reverted
            // to the previous configuration. It is not the case so be clear
            // about this.
            LOG_FATAL(dhcp4_logger, DHCP4_CONFIG_UNRECOVERABLE_ERROR);
        }
    }

    return (result);
}

ElementPtr
ControlledDhcpv4Srv::commandConfigTestHandler(const string&,
                                              ElementPtr args) {
    const int status_code = CONTROL_RESULT_ERROR; // 1 indicates an error
    ElementPtr dhcp4;
    string message;

    // Command arguments are expected to be:
    // { "Dhcp4": { ... } }
    if (!args) {
        message = "Missing mandatory 'arguments' parameter.";
    } else {
        dhcp4 = args->get("Dhcp4");
        if (!dhcp4) {
            message = "Missing mandatory 'Dhcp4' parameter.";
        } else if (dhcp4->getType() != Element::map) {
            message = "'Dhcp4' parameter expected to be a map.";
        }
    }

    // Check unsupported objects.
    if (message.empty()) {
        for (auto obj : args->mapValue()) {
            const string& obj_name = obj.first;
            if (obj_name != "Dhcp4") {
                LOG_ERROR(dhcp4_logger, DHCP4_CONFIG_UNSUPPORTED_OBJECT)
                    .arg(obj_name);
                if (message.empty()) {
                    message = "Unsupported '" + obj_name + "' parameter";
                } else {
                    message += " (and '" + obj_name + "')";
                }
            }
        }
        if (!message.empty()) {
            message += ".";
        }
    }

    if (!message.empty()) {
        // Something is amiss with arguments, return a failure response.
        ElementPtr const& result(isc::config::createAnswer(status_code,
                                                           message));
        return (result);
    }

    // stop thread pool (if running)
    MultiThreadingCriticalSection cs;

    // We are starting the configuration process so we should remove any
    // staging configuration that has been created during previous
    // configuration attempts.
    CfgMgr::instance().rollback();

    // Now we check the server proper.
    return (checkConfig(dhcp4));
}

ElementPtr
ControlledDhcpv4Srv::commandDhcpDisableHandler(const std::string&,
                                               ElementPtr args) {
    std::ostringstream message;
    int64_t max_period = 0;

    // Parse arguments to see if the 'max-period' parameter has been specified.
    if (args) {
        // Arguments must be a map.
        if (args->getType() != Element::map) {
            message << "arguments for the 'dhcp-disable' command must be a map";

        } else {
            ElementPtr const& max_period_element(args->get("max-period"));
            // max-period is optional.
            if (max_period_element) {
                // It must be an integer, if specified.
                if (max_period_element->getType() != Element::integer) {
                    message << "'max-period' argument must be a number";

                } else {
                    // It must be positive integer.
                    max_period = max_period_element->intValue();
                    if (max_period <= 0) {
                        message << "'max-period' must be positive integer";
                    }

                    // The user specified that the DHCP service should resume not
                    // later than in max-period seconds. If the 'dhcp-enable' command
                    // is not sent, the DHCP service will resume automatically.
                    network_state_->delayedEnableAll(static_cast<unsigned>(max_period));
                }
            }
        }
    }

    // No error occurred, so let's disable the service.
    if (message.tellp() == 0) {
        network_state_->disableService();

        message << "DHCPv4 service disabled";
        if (max_period > 0) {
            message << " for " << max_period << " seconds";
        }
        // Success.
        return (config::createAnswer(CONTROL_RESULT_SUCCESS, message.str()));
    }

    // Failure.
    return (config::createAnswer(CONTROL_RESULT_ERROR, message.str()));
}

ElementPtr
ControlledDhcpv4Srv::commandDhcpEnableHandler(const std::string&, ElementPtr) {
    network_state_->enableService();
    return (config::createAnswer(CONTROL_RESULT_SUCCESS, "DHCP service successfully enabled"));
}

ElementPtr
ControlledDhcpv4Srv::commandVersionGetHandler(const string&, ElementPtr) {
    ElementPtr extended = Element::create(Dhcpv4Srv::getVersion(true));
    ElementPtr arguments = Element::createMap();
    arguments->set("extended", extended);
    ElementPtr const& answer(isc::config::createAnswer(CONTROL_RESULT_SUCCESS,
                                Dhcpv4Srv::getVersion(false),
                                arguments));
    return (answer);
}

ElementPtr
ControlledDhcpv4Srv::commandBuildReportHandler(const string&, ElementPtr) {
    ElementPtr answer =
        isc::config::createAnswer(CONTROL_RESULT_SUCCESS, isc::detail::getConfigReport());
    return (answer);
}

ElementPtr
ControlledDhcpv4Srv::commandLeasesReclaimHandler(const string&,
                                                 ElementPtr args) {
    int status_code = CONTROL_RESULT_ERROR;
    string message;

    // args must be { "remove": <bool> }
    if (!args) {
        message = "Missing mandatory 'remove' parameter.";
    } else {
        ElementPtr const& remove_name(args->get("remove"));
        if (!remove_name) {
            message = "Missing mandatory 'remove' parameter.";
        } else if (remove_name->getType() != Element::boolean) {
            message = "'remove' parameter expected to be a boolean.";
        } else {
            bool remove_lease = remove_name->boolValue();
            server_->alloc_engine_->reclaimExpiredLeases4(0, 0, remove_lease);
            status_code = 0;
            message = "Reclamation of expired leases is complete.";
        }
    }
    ElementPtr const& answer(isc::config::createAnswer(status_code, message));
    return (answer);
}

ElementPtr
ControlledDhcpv4Srv::commandServerTagGetHandler(const std::string&,
                                                ElementPtr) {
    const std::string& tag =
        CfgMgr::instance().getCurrentCfg()->getServerTag();
    ElementPtr response = Element::createMap();
    response->set("server-tag", Element::create(tag));

    return (createAnswer(CONTROL_RESULT_SUCCESS, response));
}

ElementPtr
ControlledDhcpv4Srv::commandConfigBackendPullHandler(const std::string&,
                                                     ElementPtr) {
    auto ctl_info = CfgMgr::instance().getCurrentCfg()->getConfigControlInfo();
    if (!ctl_info) {
        return (createAnswer(CONTROL_RESULT_EMPTY, "No config backend."));
    }

    // stop thread pool (if running)
    MultiThreadingCriticalSection cs;

    // Reschedule the periodic CB fetch.
    if (TimerMgr::instance()->isTimerRegistered("Dhcp4CBFetchTimer")) {
        TimerMgr::instance()->cancel("Dhcp4CBFetchTimer");
        TimerMgr::instance()->setup("Dhcp4CBFetchTimer");
    }

    // Code from cbFetchUpdates.
    // The configuration to use is the current one because this is called
    // after the configuration manager commit.
    try {
        auto srv_cfg = CfgMgr::instance().getCurrentCfg();
        auto mode = CBControlDHCPv4::FetchMode::FETCH_UPDATE;
        server_->getCBControl()->databaseConfigFetch(srv_cfg, mode);
    } catch (const std::exception& ex) {
        LOG_ERROR(dhcp4_logger, DHCP4_CB_ON_DEMAND_FETCH_UPDATES_FAIL)
            .arg(ex.what());
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "On demand configuration update failed: " +
                             string(ex.what())));
    }
    return (createAnswer(CONTROL_RESULT_SUCCESS,
                         "On demand configuration update successful."));
}

ElementPtr
ControlledDhcpv4Srv::commandStatusGetHandler(const string&,
                                             ElementPtr /*args*/) {
    ElementPtr status = Element::createMap();
    status->set("pid", Element::create(static_cast<int>(getpid())));

    auto now = boost::posix_time::second_clock::universal_time();
    // Sanity check: start_ is always initialized.
    if (!start_.is_not_a_date_time()) {
        auto uptime = now - start_;
        status->set("uptime", Element::create(uptime.total_seconds()));
    }

    auto last_commit = CfgMgr::instance().getCurrentCfg()->getLastCommitTime();
    if (!last_commit.is_not_a_date_time()) {
        auto reload = now - last_commit;
        status->set("reload", Element::create(reload.total_seconds()));
    }

    if (MultiThreadingMgr::instance().getMode()) {
        status->set("multi-threading-enabled", Element::create(true));
        status->set("thread-pool-size", Element::create(static_cast<int32_t>(
                        MultiThreadingMgr::instance().getThreadPoolSize())));
        status->set("packet-queue-size", Element::create(static_cast<int32_t>(
                        MultiThreadingMgr::instance().getPacketQueueSize())));
    } else {
        status->set("multi-threading-enabled", Element::create(false));
    }

    return (createAnswer(0, status));
}

ElementPtr
ControlledDhcpv4Srv::commandStatisticSetMaxSampleCountAllHandler(const string&,
                                                                 ElementPtr args) {
    StatsMgr& stats_mgr = StatsMgr::instance();
    ElementPtr answer = stats_mgr.statisticSetMaxSampleCountAllHandler(args);
    // Update the default parameter.
    long max_samples = stats_mgr.getMaxSampleCountDefault();
    CfgMgr::instance().getCurrentCfg()->addConfiguredGlobal(
        "statistic-default-sample-count", Element::create(max_samples));
    return (answer);
}

ElementPtr
ControlledDhcpv4Srv::commandStatisticSetMaxSampleAgeAllHandler(const string&,
                                                               ElementPtr args) {
    StatsMgr& stats_mgr = StatsMgr::instance();
    ElementPtr answer = stats_mgr.statisticSetMaxSampleAgeAllHandler(args);
    // Update the default parameter.
    auto duration = stats_mgr.getMaxSampleAgeDefault();
    long max_age = toSeconds(duration);
    CfgMgr::instance().getCurrentCfg()->addConfiguredGlobal(
        "statistic-default-sample-age", Element::create(max_age));
    return (answer);
}

ElementPtr
ControlledDhcpv4Srv::processCommand(const string& command,
                                    ElementPtr args) {
    string txt = args ? args->str() : "(none)";

    LOG_DEBUG(dhcp4_logger, DBG_DHCP4_COMMAND, DHCP4_COMMAND_RECEIVED)
              .arg(command).arg(txt);

    ControlledDhcpv4Srv* srv = ControlledDhcpv4Srv::getInstance();

    if (!srv) {
        ElementPtr no_srv = isc::config::createAnswer(CONTROL_RESULT_ERROR,
          "Server object not initialized, can't process command '" +
          command + "', arguments: '" + txt + "'.");
        return (no_srv);
    }

    try {
        if (command == "shutdown") {
            return (srv->commandShutdownHandler(command, args));

        } else if (command == "libreload") {
            return (srv->commandLibReloadHandler(command, args));

        } else if (command == "config-reload") {
            return (srv->commandConfigReloadHandler(command, args));

        } else if (command == "config-set") {
            return (srv->commandConfigSetHandler(command, args));

        } else if (command == "config-get") {
            return (srv->commandConfigGetHandler(command, args));

        } else if (command == "config-test") {
            return (srv->commandConfigTestHandler(command, args));

        } else if (command == "dhcp-disable") {
            return (srv->commandDhcpDisableHandler(command, args));

        } else if (command == "dhcp-enable") {
            return (srv->commandDhcpEnableHandler(command, args));

        } else if (command == "version-get") {
            return (srv->commandVersionGetHandler(command, args));

        } else if (command == "build-report") {
            return (srv->commandBuildReportHandler(command, args));

        } else if (command == "resource-get") {
            return (StatsMgr::instance().commandResourceGetHandler(command, args));

        } else if (command == "leases-reclaim") {
            return (srv->commandLeasesReclaimHandler(command, args));

        } else if (command == "config-write") {
            return (srv->commandConfigWriteHandler(command, args));

        } else if (command == "server-tag-get") {
            return (srv->commandServerTagGetHandler(command, args));

        } else if (command == "config-backend-pull") {
            return (srv->commandConfigBackendPullHandler(command, args));

        } else if (command == "status-get") {
            return (srv->commandStatusGetHandler(command, args));
        }
        return isc::config::createAnswer(CONTROL_RESULT_ERROR, "Unrecognized command: " +
                                         command);
    } catch (const Exception& ex) {
        return isc::config::createAnswer(CONTROL_RESULT_ERROR, "Error while processing command '" +
                                                                   command + "': " + ex.what() +
                                                                   ", params: '" + txt + "'");
    }
}

ElementPtr
ControlledDhcpv4Srv::processConfig(ElementPtr config) {
    LOG_DEBUG(dhcp4_logger, DBG_DHCP4_COMMAND, DHCP4_CONFIG_RECEIVED)
              .arg(config->str());

    using CM = ConfigurationManager<DHCP_SPACE_V4, ConfigurationConstants::NOT_IETF>;
    using CR = ConfigurationReader<DHCP_SPACE_V4, isc::dhcp::ConfigurationConstants::NOT_IETF>;

    CM::configureServerIDToStaging(config);

    CM::configureCredentialsToStaging(config);

    isc::data::ElementPtr const& candidate(CR::configureFromDatabase(config));
    if (candidate) {
        config = candidate;
    }

    ControlledDhcpv4Srv* srv = ControlledDhcpv4Srv::getInstance();

    // Single stream instance used in all error clauses
    std::ostringstream err;

    if (!srv) {
        err << "Server object not initialized, can't process config.";
        return isc::config::createAnswer(CONTROL_RESULT_ERROR, err.str());
    }

    isc::data::ElementPtr const& answer(configureDhcp4Server(*srv, config));

    // Check that configuration was successful. If not, do not reopen sockets
    // and don't bother with DDNS stuff.
    try {
        int rcode = 0;
        isc::config::parseAnswer(rcode, answer);
        if (rcode != 0) {
            return (answer);
        }
    } catch (const std::exception& ex) {
        err << "Failed to process configuration: " << ex.what();
        return isc::config::createAnswer(CONTROL_RESULT_ERROR, err.str());
    }

    // Re-open lease and host database with new parameters.
    try {
        DatabaseConnection::db_lost_callback =
            std::bind(&ControlledDhcpv4Srv::dbLostCallback, srv, ph::_1);
        CfgDbAccessPtr cfg_db = CfgMgr::instance().getStagingCfg()->getCfgDbAccess();
        cfg_db->setAppendedParameters("universe=4");
        cfg_db->createManagers();
    } catch (const std::exception& ex) {
        err << "Unable to open database: " << ex.what();
        return isc::config::createAnswer(CONTROL_RESULT_ERROR, err.str());
    }

    // Server will start DDNS communications if its enabled.
    try {
        srv->startD2();
    } catch (const std::exception& ex) {
        err << "Error starting DHCP_DDNS client after server reconfiguration: "
            << ex.what();
        return isc::config::createAnswer(CONTROL_RESULT_ERROR, err.str());
    }

    // Setup DHCPv4-over-DHCPv6 IPC
    try {
        Dhcp4to6Ipc::instance().open();
    } catch (const std::exception& ex) {
        std::ostringstream err;
        err << "error starting DHCPv4-over-DHCPv6 IPC "
               " after server reconfiguration: " << ex.what();
        return isc::config::createAnswer(CONTROL_RESULT_ERROR, err.str());
    }

    // Configure DHCP packet queueing
    try {
        ElementPtr qc;
        qc  = CfgMgr::instance().getStagingCfg()->getDHCPQueueControl();
        if (IfaceMgr::instance().configureDHCPPacketQueue(AF_INET, qc)) {
            LOG_INFO(dhcp4_logger, DHCP4_CONFIG_PACKET_QUEUE)
                     .arg(IfaceMgr::instance().getPacketQueue4()->getInfoStr());
        }

    } catch (const std::exception& ex) {
        err << "Error setting packet queue controls after server reconfiguration: "
            << ex.what();
        return (isc::config::createAnswer(1, err.str()));
    }

    // Configuration may change active interfaces. Therefore, we have to reopen
    // sockets according to new configuration. It is possible that this
    // operation will fail for some interfaces but the openSockets function
    // guards against exceptions and invokes a callback function to
    // log warnings. Since we allow that this fails for some interfaces there
    // is no need to rollback configuration if socket fails to open on any
    // of the interfaces.
    CfgMgr::instance().getStagingCfg()->getCfgIface()->
        openSockets(AF_INET, srv->getServerPort(),
                    getInstance()->useBroadcast());

    // Install the timers for handling leases reclamation.
    try {
        CfgMgr::instance().getStagingCfg()->getCfgExpiration()->
            setupTimers(&ControlledDhcpv4Srv::reclaimExpiredLeases,
                        &ControlledDhcpv4Srv::deleteExpiredReclaimedLeases,
                        server_);

    } catch (const std::exception& ex) {
        err << "unable to setup timers for periodically running the"
            " reclamation of the expired leases: "
            << ex.what() << ".";
        return (isc::config::createAnswer(CONTROL_RESULT_ERROR, err.str()));
    }

    // Setup config backend polling, if configured for it.
    auto ctl_info = CfgMgr::instance().getStagingCfg()->getConfigControlInfo();
    if (ctl_info) {
        long fetch_time = static_cast<long>(ctl_info->getConfigFetchWaitTime());
        // Only schedule the CB fetch timer if the fetch wait time is greater
        // than 0.
        if (fetch_time > 0) {
            // When we run unit tests, we want to use milliseconds unit for the
            // specified interval. Otherwise, we use seconds. Note that using
            // milliseconds as a unit in unit tests prevents us from waiting 1
            // second on more before the timer goes off. Instead, we wait one
            // millisecond which significantly reduces the test time.
            if (!server_->inTestMode()) {
                fetch_time = 1000 * fetch_time;
            }

            std::shared_ptr<unsigned> failure_count(new unsigned(0));
            TimerMgr::instance()->
                registerTimer("Dhcp4CBFetchTimer",
                              std::bind(&ControlledDhcpv4Srv::cbFetchUpdates,
                                        server_, CfgMgr::instance().getStagingCfg(),
                                        failure_count),
                              fetch_time,
                              asiolink::IntervalTimer::ONE_SHOT);
            TimerMgr::instance()->setup("Dhcp4CBFetchTimer");
        }
    }

    // Finally, we can commit runtime option definitions in libdhcp++. This is
    // exception free.
    LibDHCP::commitRuntimeOptionDefs();

    // This hook point notifies hooks libraries that the configuration of the
    // DHCPv4 server has completed. It provides the hook library with the pointer
    // to the common IO service object, new server configuration in the JSON
    // format and with the pointer to the configuration storage where the
    // parsed configuration is stored.
    if (HooksManager::calloutsPresent(Hooks.hooks_index_dhcp4_srv_configured_)) {
        CalloutHandlePtr callout_handle = HooksManager::createCalloutHandle();

        callout_handle->setArgument("io_context", srv->getIOService());
        callout_handle->setArgument("network_state", srv->getNetworkState());
        callout_handle->setArgument("json_config", config);
        callout_handle->setArgument("server_config", CfgMgr::instance().getStagingCfg());

        HooksManager::callCallouts(Hooks.hooks_index_dhcp4_srv_configured_,
                                   *callout_handle);

        // Ignore status code as none of them would have an effect on further
        // operation.
    }

    // Apply multi threading settings.
    // @note These settings are applied/updated only if no errors occur while
    // applying the new configuration.
    // @todo This should be fixed.
    try {
        CfgMultiThreading::apply(CfgMgr::instance().getStagingCfg()->getDHCPMultiThreading());
        if (MultiThreadingMgr::instance().getMode()) {
            LOG_FATAL(dhcp4_logger, DHCP4_MULTI_THREADING_WARNING);
        }
    } catch (const std::exception& ex) {
        err << "Error applying multi threading settings: "
            << ex.what();
        return (isc::config::createAnswer(CONTROL_RESULT_ERROR, err.str()));
    }

    return (answer);
}

ElementPtr
ControlledDhcpv4Srv::checkConfig(ElementPtr config) {

    LOG_DEBUG(dhcp4_logger, DBG_DHCP4_COMMAND, DHCP4_CONFIG_RECEIVED)
        .arg(config->str());

    ControlledDhcpv4Srv* srv = ControlledDhcpv4Srv::getInstance();

    // Single stream instance used in all error clauses
    std::ostringstream err;

    if (!srv) {
        err << "Server object not initialized, can't process config.";
        return (isc::config::createAnswer(CONTROL_RESULT_ERROR, err.str()));
    }

    return (configureDhcp4Server(*srv, config, true));
}

ControlledDhcpv4Srv::ControlledDhcpv4Srv(uint16_t server_port /*= DHCP4_SERVER_PORT*/,
                                         uint16_t client_port /*= 0*/)
    : Dhcpv4Srv(server_port, client_port), io_service_(),
      timer_mgr_(TimerMgr::instance()) {
    if (getInstance()) {
        isc_throw(InvalidOperation,
                  "There is another Dhcpv4Srv instance already.");
    }
    server_ = this; // remember this instance for later use in handlers

    // TimerMgr uses IO service to run asynchronous timers.
    TimerMgr::instance()->setIOService(getIOService());

    // CommandMgr uses IO service to run asynchronous socket operations.
    CommandMgr::instance().setIOService(getIOService());

    // These are the commands always supported by the DHCPv4 server.
    // Please keep the list in alphabetic order.
    CommandMgr::instance().registerCommand("build-report",
        std::bind(&ControlledDhcpv4Srv::commandBuildReportHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("resource-get",
        std::bind(&StatsMgr::commandResourceGetHandler, &StatsMgr::instance(), ph::_1, ph::_2));
    // Populate cache with a cpu usage value
    StatsMgr::instance().getCpuUsage();

    CommandMgr::instance().registerCommand("config-backend-pull",
        std::bind(&ControlledDhcpv4Srv::commandConfigBackendPullHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("config-get",
        std::bind(&ControlledDhcpv4Srv::commandConfigGetHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("config-reload",
        std::bind(&ControlledDhcpv4Srv::commandConfigReloadHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("config-set",
        std::bind(&ControlledDhcpv4Srv::commandConfigSetHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("config-test",
        std::bind(&ControlledDhcpv4Srv::commandConfigTestHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("config-write",
        std::bind(&ControlledDhcpv4Srv::commandConfigWriteHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("dhcp-enable",
        std::bind(&ControlledDhcpv4Srv::commandDhcpEnableHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("dhcp-disable",
        std::bind(&ControlledDhcpv4Srv::commandDhcpDisableHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("libreload",
        std::bind(&ControlledDhcpv4Srv::commandLibReloadHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("leases-reclaim",
        std::bind(&ControlledDhcpv4Srv::commandLeasesReclaimHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("server-tag-get",
        std::bind(&ControlledDhcpv4Srv::commandServerTagGetHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("shutdown",
        std::bind(&ControlledDhcpv4Srv::commandShutdownHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("status-get",
        std::bind(&ControlledDhcpv4Srv::commandStatusGetHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("version-get",
        std::bind(&ControlledDhcpv4Srv::commandVersionGetHandler, this, ph::_1, ph::_2));

    // Register statistic related commands
    CommandMgr::instance().registerCommand("statistic-get",
        std::bind(&StatsMgr::statisticGetHandler, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("statistic-reset",
        std::bind(&StatsMgr::statisticResetHandler, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("statistic-remove",
        std::bind(&StatsMgr::statisticRemoveHandler, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("statistic-get-all",
        std::bind(&StatsMgr::statisticGetAllHandler, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("statistic-reset-all",
        std::bind(&StatsMgr::statisticResetAllHandler, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("statistic-remove-all",
        std::bind(&StatsMgr::statisticRemoveAllHandler, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("statistic-sample-age-set",
        std::bind(&StatsMgr::statisticSetMaxSampleAgeHandler, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("statistic-sample-age-set-all",
        std::bind(&ControlledDhcpv4Srv::commandStatisticSetMaxSampleAgeAllHandler, this, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("statistic-sample-count-set",
        std::bind(&StatsMgr::statisticSetMaxSampleCountHandler, ph::_1, ph::_2));

    CommandMgr::instance().registerCommand("statistic-sample-count-set-all",
        std::bind(&ControlledDhcpv4Srv::commandStatisticSetMaxSampleCountAllHandler, this, ph::_1, ph::_2));
}

void ControlledDhcpv4Srv::shutdownServer(int exit_value) {
    setExitValue(exit_value);
    io_service_.stop();       // Stop ASIO transmissions
    shutdown();               // Initiate DHCPv4 shutdown procedure.
}

ControlledDhcpv4Srv::~ControlledDhcpv4Srv() {
    try {
        cleanup();

        // The closure captures either a shared pointer (memory leak)
        // or a raw pointer (pointing to a deleted object).
        DatabaseConnection::db_lost_callback = 0;

        timer_mgr_->unregisterTimers();

        // Close the command socket (if it exists).
        CommandMgr::instance().closeCommandSocket();

        // Deregister any registered commands (please keep in alphabetic order)
        CommandMgr::instance().deregisterCommand("build-report");
        CommandMgr::instance().deregisterCommand("resource-get");
        CommandMgr::instance().deregisterCommand("config-backend-pull");
        CommandMgr::instance().deregisterCommand("config-get");
        CommandMgr::instance().deregisterCommand("config-set");
        CommandMgr::instance().deregisterCommand("config-reload");
        CommandMgr::instance().deregisterCommand("config-set");
        CommandMgr::instance().deregisterCommand("config-test");
        CommandMgr::instance().deregisterCommand("config-write");
        CommandMgr::instance().deregisterCommand("dhcp-disable");
        CommandMgr::instance().deregisterCommand("dhcp-enable");
        CommandMgr::instance().deregisterCommand("leases-reclaim");
        CommandMgr::instance().deregisterCommand("libreload");
        CommandMgr::instance().deregisterCommand("server-tag-get");
        CommandMgr::instance().deregisterCommand("shutdown");
        CommandMgr::instance().deregisterCommand("statistic-get");
        CommandMgr::instance().deregisterCommand("statistic-get-all");
        CommandMgr::instance().deregisterCommand("statistic-remove");
        CommandMgr::instance().deregisterCommand("statistic-remove-all");
        CommandMgr::instance().deregisterCommand("statistic-reset");
        CommandMgr::instance().deregisterCommand("statistic-reset-all");
        CommandMgr::instance().deregisterCommand("statistic-sample-age-set");
        CommandMgr::instance().deregisterCommand("statistic-sample-age-set-all");
        CommandMgr::instance().deregisterCommand("statistic-sample-count-set");
        CommandMgr::instance().deregisterCommand("statistic-sample-count-set-all");
        CommandMgr::instance().deregisterCommand("status-get");
        CommandMgr::instance().deregisterCommand("version-get");

    } catch (...) {
        // Don't want to throw exceptions from the destructor. The server
        // is shutting down anyway.
        ;
    }

    server_ = NULL; // forget this instance. There should be no callback anymore
                    // at this stage anyway.
}

void ControlledDhcpv4Srv::sessionReader(void) {
    // Process one asio event. If there are more events, iface_mgr will call
    // this callback more than once.
    if (getInstance()) {
        getInstance()->io_service_.run_one();
    }
}

void
ControlledDhcpv4Srv::reclaimExpiredLeases(const size_t max_leases,
                                          const uint16_t timeout,
                                          const bool remove_lease,
                                          const uint16_t max_unwarned_cycles) {
    try {
        server_->alloc_engine_->reclaimExpiredLeases4(max_leases, timeout,
                                                      remove_lease,
                                                      max_unwarned_cycles);
    } catch (const std::exception& ex) {
        LOG_ERROR(dhcp4_logger, DHCP4_RECLAIM_EXPIRED_LEASES_FAIL)
            .arg(ex.what());
    }
    // We're using the ONE_SHOT timer so there is a need to re-schedule it.
    TimerMgr::instance()->setup(CfgExpiration::RECLAIM_EXPIRED_TIMER_NAME);
}

void
ControlledDhcpv4Srv::deleteExpiredReclaimedLeases(const uint32_t secs) {
    server_->alloc_engine_->deleteExpiredReclaimedLeases4(secs);
    // We're using the ONE_SHOT timer so there is a need to re-schedule it.
    TimerMgr::instance()->setup(CfgExpiration::FLUSH_RECLAIMED_TIMER_NAME);
}

void
ControlledDhcpv4Srv::dbReconnect(ReconnectCtlPtr db_reconnect_ctl) {
    bool reopened = false;

    // We lost at least one of them. Reopen all of them (lease, host, and CB databases).
    try {
        CfgDbAccessPtr cfg_db = CfgMgr::instance().getCurrentCfg()->getCfgDbAccess();
        cfg_db->createManagers();

        auto ctl_info = CfgMgr::instance().getCurrentCfg()->getConfigControlInfo();
        if (ctl_info) {
            auto srv_cfg = CfgMgr::instance().getCurrentCfg();
            server_->getCBControl()->databaseConfigConnect(srv_cfg);
        }

        reopened = true;
    } catch (const std::exception& ex) {
        LOG_ERROR(dhcp4_logger, DHCP4_DB_RECONNECT_ATTEMPT_FAILED).arg(ex.what());
    }

    if (reopened) {
        // Cancel the timer.
        if (TimerMgr::instance()->isTimerRegistered("Dhcp4DbReconnectTimer")) {
            TimerMgr::instance()->cancel("Dhcp4DbReconnectTimer");
        }

        // Set network state to service enabled
        network_state_->enableService();

        // Toss the reconnect control, we're done with it
        db_reconnect_ctl.reset();
    } else {
        if (!db_reconnect_ctl->checkRetries()) {
            // We're out of retries, log it and initiate shutdown.
            LOG_ERROR(dhcp4_logger, DHCP4_DB_RECONNECT_RETRIES_EXHAUSTED)
            .arg(db_reconnect_ctl->maxRetries());
            shutdownServer(EXIT_FAILURE);
            return;
        }

        LOG_INFO(dhcp4_logger, DHCP4_DB_RECONNECT_ATTEMPT_SCHEDULE)
                .arg(db_reconnect_ctl->maxRetries() - db_reconnect_ctl->retriesLeft() + 1)
                .arg(db_reconnect_ctl->maxRetries())
                .arg(db_reconnect_ctl->retryInterval());

        if (!TimerMgr::instance()->isTimerRegistered("Dhcp4DbReconnectTimer")) {
            TimerMgr::instance()->registerTimer("Dhcp4DbReconnectTimer",
                            std::bind(&ControlledDhcpv4Srv::dbReconnect, this,
                                      db_reconnect_ctl),
                            db_reconnect_ctl->retryInterval(),
                            asiolink::IntervalTimer::ONE_SHOT);
        }

        TimerMgr::instance()->setup("Dhcp4DbReconnectTimer");
    }
}

bool
ControlledDhcpv4Srv::dbLostCallback(ReconnectCtlPtr db_reconnect_ctl) {
    // Disable service until we recover
    network_state_->disableService();

    if (!db_reconnect_ctl) {
        // This shouldn't never happen
        LOG_ERROR(dhcp4_logger, DHCP4_DB_RECONNECT_NO_DB_CTL);
        return (false);
    }

    // If reconnect isn't enabled log it,
    // initiate a shutdown and return false.
    if (!db_reconnect_ctl->retriesLeft() ||
        !db_reconnect_ctl->retryInterval()) {
        LOG_INFO(dhcp4_logger, DHCP4_DB_RECONNECT_DISABLED)
            .arg(db_reconnect_ctl->retriesLeft())
            .arg(db_reconnect_ctl->retryInterval());
        shutdownServer(EXIT_FAILURE);
        return(false);
    }

    // Invoke reconnect method
    dbReconnect(db_reconnect_ctl);

    return(true);
}

void
ControlledDhcpv4Srv::cbFetchUpdates(const SrvConfigPtr& srv_cfg,
                                    std::shared_ptr<unsigned> failure_count) {
    // stop thread pool (if running)
    MultiThreadingCriticalSection cs;

    try {
        // Fetch any configuration backend updates since our last fetch.
        server_->getCBControl()->databaseConfigFetch(srv_cfg,
                                                     CBControlDHCPv4::FetchMode::FETCH_UPDATE);
        (*failure_count) = 0;

    } catch (const std::exception& ex) {
        LOG_ERROR(dhcp4_logger, DHCP4_CB_PERIODIC_FETCH_UPDATES_FAIL)
            .arg(ex.what());

        // We allow at most 10 consecutive failures after which we stop
        // making further attempts to fetch the configuration updates.
        // Let's return without re-scheduling the timer.
        if (++(*failure_count) > 10) {
            LOG_ERROR(dhcp4_logger,
                      DHCP4_CB_PERIODIC_FETCH_UPDATES_RETRIES_EXHAUSTED);
            return;
        }
    }

    // Reschedule the timer to fetch new updates or re-try if
    // the previous attempt resulted in an error.
    if (TimerMgr::instance()->isTimerRegistered("Dhcp4CBFetchTimer")) {
        TimerMgr::instance()->setup("Dhcp4CBFetchTimer");
    }
}

}  // namespace dhcp
}  // namespace isc

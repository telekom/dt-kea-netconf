// Copyright (C) 2018-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <asiolink/asio_wrapper.h>
#include <asiolink/io_address.h>
#include <asiolink/io_error.h>
#include <cc/command_interpreter.h>
#include <config/timeouts.h>
#include <netconf/netconf.h>
#include <netconf/netconf_controller.h>
#include <netconf/netconf_log.h>
#include <netconf/netconf_process.h>

#include <thread>

using namespace isc::asiolink;
using namespace isc::config;
using namespace isc::data;
using namespace isc::http;
using namespace isc::process;

using std::atomic;
using std::current_exception;
using std::exception;
using std::exception_ptr;
using std::thread;

namespace isc {
namespace netconf {

atomic<bool> NetconfProcess::shut_down(false);

NetconfProcess::NetconfProcess(const char* name, const asiolink::IOServicePtr& io_service)
    : DProcessBase(name, io_service, DCfgMgrBasePtr(new NetconfCfgMgr())) {
}

void NetconfProcess::init() {
    // CommandMgr uses IO service to run asynchronous socket operations.
    isc::config::CommandMgr::instance().setIOService(getIoService());
}

void NetconfProcess::run() {
    LOG_INFO(netconf_logger, NETCONF_STARTED).arg(VERSION);

    try {
        thread th([this]() {
            try {
                if (shouldShutdown()) {
                    return;
                }
                // Initialize sysrepo.
                agent_.initSysrepo();
                if (shouldShutdown()) {
                    return;
                }

                // Get the configuration manager.
                NetconfCfgMgrPtr cfg_mgr;
                if (shouldShutdown()) {
                    return;
                } else {
                    cfg_mgr = getNetconfCfgMgr();
                }

                // Call init.
                agent_.init(cfg_mgr);
            } catch (...) {
                // Should not happen but in case...
                exception_ptr eptr = current_exception();
                getIoService()->post([eptr]() {
                    if (eptr) {
                        rethrow_exception(eptr);
                    }
                });
            }
        });

        // Detach the thread.
        th.detach();

        // Let's process incoming data or expiring timers in a loop until
        // shutdown condition is detected.
        while (!shouldShutdown()) {
            runIO();
        }
        stopIOService();

    } catch (const exception& ex) {
        LOG_FATAL(netconf_logger, NETCONF_FAILED).arg(ex.what());
        try {
            stopIOService();
        } catch (...) {
            // Ignore double errors
        }
        isc_throw(DProcessBaseError, "Process run method failed: " << ex.what());
    }

    isc::config::CommandMgr::instance().closeCommandSocket();
    isc::config::CommandMgr::instance().deregisterAll();

    LOG_DEBUG(netconf_logger, isc::log::DBGLVL_START_SHUT, NETCONF_RUN_EXIT);
}

size_t NetconfProcess::runIO() {
    size_t cnt = getIoService()->get_io_service().poll();
    if (!cnt) {
        cnt = getIoService()->get_io_service().run_one();
    }
    return cnt;
}

isc::data::ElementPtr NetconfProcess::shutdown(isc::data::ElementPtr /*args*/) {
    shut_down = true;
    setShutdownFlag(true);
    return isc::config::createAnswer(0, "Netconf is shutting down");
}

isc::data::ElementPtr NetconfProcess::configure(isc::data::ElementPtr config_set, bool check_only) {
    ElementPtr answer = getCfgMgr()->simpleParseConfig(config_set, check_only);
    int rcode = 0;
    config::parseAnswer(rcode, answer);
    return answer;
}

NetconfCfgMgrPtr NetconfProcess::getNetconfCfgMgr() {
    return dynamic_pointer_cast<NetconfCfgMgr>(getCfgMgr());
}

}  // namespace netconf
}  // namespace isc

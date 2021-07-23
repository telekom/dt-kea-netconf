// File created from ../../../src/bin/netconf/netconf_messages.mes on Tue Dec 08 2020 21:14

#include <cstddef>
#include <log/message_types.h>
#include <log/message_initializer.h>

namespace isc {
namespace netconf {

extern const isc::log::MessageID NETCONF_BOOT_UPDATE_COMPLETED = "NETCONF_BOOT_UPDATE_COMPLETED";
extern const isc::log::MessageID NETCONF_CONFIG_CHANGED_DETAIL = "NETCONF_CONFIG_CHANGED_DETAIL";
extern const isc::log::MessageID NETCONF_CONFIG_CHANGE_EVENT = "NETCONF_CONFIG_CHANGE_EVENT";
extern const isc::log::MessageID NETCONF_CONFIG_CHECK_FAIL = "NETCONF_CONFIG_CHECK_FAIL";
extern const isc::log::MessageID NETCONF_CONFIG_FAIL = "NETCONF_CONFIG_FAIL";
extern const isc::log::MessageID NETCONF_DONE_CONFIG = "NETCONF_DONE_CONFIG";
extern const isc::log::MessageID NETCONF_DONE_CONFIG_COMPLETED = "NETCONF_DONE_CONFIG_COMPLETED";
extern const isc::log::MessageID NETCONF_DONE_CONFIG_FAILED = "NETCONF_DONE_CONFIG_FAILED";
extern const isc::log::MessageID NETCONF_DONE_CONFIG_STARTED = "NETCONF_DONE_CONFIG_STARTED";
extern const isc::log::MessageID NETCONF_ERROR = "NETCONF_ERROR";
extern const isc::log::MessageID NETCONF_FAILED = "NETCONF_FAILED";
extern const isc::log::MessageID NETCONF_GET_CONFIG = "NETCONF_GET_CONFIG";
extern const isc::log::MessageID NETCONF_GET_CONFIG_FAILED = "NETCONF_GET_CONFIG_FAILED";
extern const isc::log::MessageID NETCONF_GET_CONFIG_STARTED = "NETCONF_GET_CONFIG_STARTED";
extern const isc::log::MessageID NETCONF_GET_OPER_ITEM = "NETCONF_GET_OPER_ITEM";
extern const isc::log::MessageID NETCONF_GET_OPER_ITEM_FAILED = "NETCONF_GET_OPER_ITEM_FAILED";
extern const isc::log::MessageID NETCONF_GET_OPER_ITEM_SUCCESS = "NETCONF_GET_OPER_ITEM_SUCCESS";
extern const isc::log::MessageID NETCONF_GET_RPC_ITEM = "NETCONF_GET_RPC_ITEM";
extern const isc::log::MessageID NETCONF_GET_RPC_ITEM_FAILED = "NETCONF_GET_RPC_ITEM_FAILED";
extern const isc::log::MessageID NETCONF_GET_RPC_ITEM_SUCCESS = "NETCONF_GET_RPC_ITEM_SUCCESS";
extern const isc::log::MessageID NETCONF_INFO = "NETCONF_INFO";
extern const isc::log::MessageID NETCONF_INIT_CONFIG = "NETCONF_INIT_CONFIG";
extern const isc::log::MessageID NETCONF_INIT_CONFIG_COMPLETED = "NETCONF_INIT_CONFIG_COMPLETED";
extern const isc::log::MessageID NETCONF_INIT_CONFIG_FAILED = "NETCONF_INIT_CONFIG_FAILED";
extern const isc::log::MessageID NETCONF_INIT_CONFIG_STARTED = "NETCONF_INIT_CONFIG_STARTED";
extern const isc::log::MessageID NETCONF_MODULE_INSTALL = "NETCONF_MODULE_INSTALL";
extern const isc::log::MessageID NETCONF_MODULE_MISSING_ERR = "NETCONF_MODULE_MISSING_ERR";
extern const isc::log::MessageID NETCONF_MODULE_MISSING_WARN = "NETCONF_MODULE_MISSING_WARN";
extern const isc::log::MessageID NETCONF_MODULE_REVISION_ERR = "NETCONF_MODULE_REVISION_ERR";
extern const isc::log::MessageID NETCONF_MODULE_REVISION_WARN = "NETCONF_MODULE_REVISION_WARN";
extern const isc::log::MessageID NETCONF_RUN_EXIT = "NETCONF_RUN_EXIT";
extern const isc::log::MessageID NETCONF_SET_CONFIG = "NETCONF_SET_CONFIG";
extern const isc::log::MessageID NETCONF_SET_CONFIG_FAILED = "NETCONF_SET_CONFIG_FAILED";
extern const isc::log::MessageID NETCONF_SET_CONFIG_STARTED = "NETCONF_SET_CONFIG_STARTED";
extern const isc::log::MessageID NETCONF_STARTED = "NETCONF_STARTED";
extern const isc::log::MessageID NETCONF_SUBSCRIBE_CONFIG = "NETCONF_SUBSCRIBE_CONFIG";
extern const isc::log::MessageID NETCONF_SUBSCRIBE_CONFIG_FAILED = "NETCONF_SUBSCRIBE_CONFIG_FAILED";
extern const isc::log::MessageID NETCONF_SUBSCRIBE_NOTIFICATIONS = "NETCONF_SUBSCRIBE_NOTIFICATIONS";
extern const isc::log::MessageID NETCONF_SUBSCRIBE_NOTIFICATIONS_FAILED = "NETCONF_SUBSCRIBE_NOTIFICATIONS_FAILED";
extern const isc::log::MessageID NETCONF_SUBSCRIBE_OPER = "NETCONF_SUBSCRIBE_OPER";
extern const isc::log::MessageID NETCONF_SUBSCRIBE_OPER_FAILED = "NETCONF_SUBSCRIBE_OPER_FAILED";
extern const isc::log::MessageID NETCONF_SUBSCRIBE_RPC = "NETCONF_SUBSCRIBE_RPC";
extern const isc::log::MessageID NETCONF_SUBSCRIBE_RPC_FAILED = "NETCONF_SUBSCRIBE_RPC_FAILED";
extern const isc::log::MessageID NETCONF_UPDATE_CONFIG = "NETCONF_UPDATE_CONFIG";
extern const isc::log::MessageID NETCONF_UPDATE_CONFIG_COMPLETED = "NETCONF_UPDATE_CONFIG_COMPLETED";
extern const isc::log::MessageID NETCONF_UPDATE_CONFIG_FAILED = "NETCONF_UPDATE_CONFIG_FAILED";
extern const isc::log::MessageID NETCONF_UPDATE_CONFIG_REJECTED = "NETCONF_UPDATE_CONFIG_REJECTED";
extern const isc::log::MessageID NETCONF_UPDATE_CONFIG_STARTED = "NETCONF_UPDATE_CONFIG_STARTED";
extern const isc::log::MessageID NETCONF_UPDATE_INFO = "NETCONF_UPDATE_INFO";
extern const isc::log::MessageID NETCONF_UPDATE_SYNC = "NETCONF_UPDATE_SYNC";
extern const isc::log::MessageID NETCONF_WARNING = "NETCONF_WARNING";

} // namespace netconf
} // namespace isc

namespace {

const char* values[] = {
    "NETCONF_BOOT_UPDATE_COMPLETED", "Boot-update configuration completed for server %1",
    "NETCONF_CONFIG_CHANGED_DETAIL", "YANG configuration changed: %1",
    "NETCONF_CONFIG_CHANGE_EVENT", "Received YANG configuration change %1 event",
    "NETCONF_CONFIG_CHECK_FAIL", "Netconf configuration check failed: %1",
    "NETCONF_CONFIG_FAIL", "Netconf configuration failed: %1",
    "NETCONF_DONE_CONFIG", "updating configuration with %1 server: %2",
    "NETCONF_DONE_CONFIG_COMPLETED", "completed updating configuration for %1 server",
    "NETCONF_DONE_CONFIG_FAILED", "updating configuration with %1 server: %2",
    "NETCONF_DONE_CONFIG_STARTED", "started updating configuration for %1 server",
    "NETCONF_ERROR", "%1",
    "NETCONF_FAILED", "application experienced a fatal error: %1",
    "NETCONF_GET_CONFIG", "got configuration for %1 model: %2",
    "NETCONF_GET_CONFIG_FAILED", "getting configuration for %1 model failed: %2",
    "NETCONF_GET_CONFIG_STARTED", "getting configuration for %1 model",
    "NETCONF_GET_OPER_ITEM", "operational item requested at path: %1",
    "NETCONF_GET_OPER_ITEM_FAILED", "operational item requested failed, reason: %1",
    "NETCONF_GET_OPER_ITEM_SUCCESS", "operational request was successful for path: %1",
    "NETCONF_GET_RPC_ITEM", "RPC action requested at path: %1",
    "NETCONF_GET_RPC_ITEM_FAILED", "RPC action requested failed, reason: %1",
    "NETCONF_GET_RPC_ITEM_SUCCESS", "RPC action was successful for path: %1",
    "NETCONF_INFO", "%1",
    "NETCONF_INIT_CONFIG", "initialize configuration for %1 model: %2",
    "NETCONF_INIT_CONFIG_COMPLETED", "initializing configuration for %1 model succeeded",
    "NETCONF_INIT_CONFIG_FAILED", "initializing configuration for %1 model failed: %2",
    "NETCONF_INIT_CONFIG_STARTED", "initializing configuration for %1 model",
    "NETCONF_MODULE_INSTALL", "Sysrepo (un)installs a module: %1 (revision %2)",
    "NETCONF_MODULE_MISSING_ERR", "Missing essential module %1 in sysrepo",
    "NETCONF_MODULE_MISSING_WARN", "Missing module %1 in sysrepo",
    "NETCONF_MODULE_REVISION_ERR", "Essential module %1 doesn't have the right revision: expected %2, got %3",
    "NETCONF_MODULE_REVISION_WARN", "Module %1 doesn't have the right revision: expected %2, got %3",
    "NETCONF_RUN_EXIT", "application is exiting the event loop",
    "NETCONF_SET_CONFIG", "set configuration for %1 model: %2",
    "NETCONF_SET_CONFIG_FAILED", "setting configuration for %1 model failed: %2",
    "NETCONF_SET_CONFIG_STARTED", "setting configuration for %1 model",
    "NETCONF_STARTED", "Netconf (version %1) started",
    "NETCONF_SUBSCRIBE_CONFIG", "NetconfAgent::subscribeConfig(): subscribing to configuration changes for %1 with the %2 module",
    "NETCONF_SUBSCRIBE_CONFIG_FAILED", "NetconfAgent::subscribeConfig(): subscribing to configuration changes for %1 with the %2 module failed: %3",
    "NETCONF_SUBSCRIBE_NOTIFICATIONS", "NetconfAgent::subscribeNotifications(): subscribing to notifications for %1 with the %2 module",
    "NETCONF_SUBSCRIBE_NOTIFICATIONS_FAILED", "NetconfAgent::subscribeNotifications(): subscribing to notifications changes for %1 with the %2 module failed: %3",
    "NETCONF_SUBSCRIBE_OPER", "NetconfAgent::subscribeOperational(): subscribing operational data requests for %1 path with %2 module",
    "NETCONF_SUBSCRIBE_OPER_FAILED", "NetconfAgent::subscribeOperational(): subscribing operational data requests for %1 path with %2 module failed: %3",
    "NETCONF_SUBSCRIBE_RPC", "NetconfAgent::subscribeRpc(): subscribing to RPC for %1 with the %2 module",
    "NETCONF_SUBSCRIBE_RPC_FAILED", "NetconfAgent::subscribeRpc(): subscribing to RPC actions for %1 with the %2 module failed: %3",
    "NETCONF_UPDATE_CONFIG", "validating configuration with %1 server: %2",
    "NETCONF_UPDATE_CONFIG_COMPLETED", "completed validating configuration for %1 server",
    "NETCONF_UPDATE_CONFIG_FAILED", "validating configuration with %1 server got an error: %2",
    "NETCONF_UPDATE_CONFIG_REJECTED", "validating configuration with %1 server was rejected: %2",
    "NETCONF_UPDATE_CONFIG_STARTED", "started validating configuration for %1 server",
    "NETCONF_UPDATE_INFO", "%1",
    "NETCONF_UPDATE_SYNC", "synchronizing configuration with %1 model: %2",
    "NETCONF_WARNING", "%1",
    NULL
};

const isc::log::MessageInitializer initializer(values);

} // Anonymous namespace


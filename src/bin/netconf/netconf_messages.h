// File created from ../../../src/bin/netconf/netconf_messages.mes on Tue Dec 08 2020 21:14

#ifndef NETCONF_MESSAGES_H
#define NETCONF_MESSAGES_H

#include <log/message_types.h>

namespace isc {
namespace netconf {

extern const isc::log::MessageID NETCONF_BOOT_UPDATE_COMPLETED;
extern const isc::log::MessageID NETCONF_CONFIG_CHANGED_DETAIL;
extern const isc::log::MessageID NETCONF_CONFIG_CHANGE_EVENT;
extern const isc::log::MessageID NETCONF_CONFIG_CHECK_FAIL;
extern const isc::log::MessageID NETCONF_CONFIG_FAIL;
extern const isc::log::MessageID NETCONF_DONE_CONFIG;
extern const isc::log::MessageID NETCONF_DONE_CONFIG_COMPLETED;
extern const isc::log::MessageID NETCONF_DONE_CONFIG_FAILED;
extern const isc::log::MessageID NETCONF_DONE_CONFIG_STARTED;
extern const isc::log::MessageID NETCONF_ERROR;
extern const isc::log::MessageID NETCONF_FAILED;
extern const isc::log::MessageID NETCONF_GET_CONFIG;
extern const isc::log::MessageID NETCONF_GET_CONFIG_FAILED;
extern const isc::log::MessageID NETCONF_GET_CONFIG_STARTED;
extern const isc::log::MessageID NETCONF_GET_OPER_ITEM;
extern const isc::log::MessageID NETCONF_GET_OPER_ITEM_FAILED;
extern const isc::log::MessageID NETCONF_GET_OPER_ITEM_SUCCESS;
extern const isc::log::MessageID NETCONF_GET_RPC_ITEM;
extern const isc::log::MessageID NETCONF_GET_RPC_ITEM_FAILED;
extern const isc::log::MessageID NETCONF_GET_RPC_ITEM_SUCCESS;
extern const isc::log::MessageID NETCONF_INFO;
extern const isc::log::MessageID NETCONF_INIT_CONFIG;
extern const isc::log::MessageID NETCONF_INIT_CONFIG_COMPLETED;
extern const isc::log::MessageID NETCONF_INIT_CONFIG_FAILED;
extern const isc::log::MessageID NETCONF_INIT_CONFIG_STARTED;
extern const isc::log::MessageID NETCONF_MODULE_INSTALL;
extern const isc::log::MessageID NETCONF_MODULE_MISSING_ERR;
extern const isc::log::MessageID NETCONF_MODULE_MISSING_WARN;
extern const isc::log::MessageID NETCONF_MODULE_REVISION_ERR;
extern const isc::log::MessageID NETCONF_MODULE_REVISION_WARN;
extern const isc::log::MessageID NETCONF_RUN_EXIT;
extern const isc::log::MessageID NETCONF_SET_CONFIG;
extern const isc::log::MessageID NETCONF_SET_CONFIG_FAILED;
extern const isc::log::MessageID NETCONF_SET_CONFIG_STARTED;
extern const isc::log::MessageID NETCONF_STARTED;
extern const isc::log::MessageID NETCONF_SUBSCRIBE_CONFIG;
extern const isc::log::MessageID NETCONF_SUBSCRIBE_CONFIG_FAILED;
extern const isc::log::MessageID NETCONF_SUBSCRIBE_NOTIFICATIONS;
extern const isc::log::MessageID NETCONF_SUBSCRIBE_NOTIFICATIONS_FAILED;
extern const isc::log::MessageID NETCONF_SUBSCRIBE_OPER;
extern const isc::log::MessageID NETCONF_SUBSCRIBE_OPER_FAILED;
extern const isc::log::MessageID NETCONF_SUBSCRIBE_RPC;
extern const isc::log::MessageID NETCONF_SUBSCRIBE_RPC_FAILED;
extern const isc::log::MessageID NETCONF_UPDATE_CONFIG;
extern const isc::log::MessageID NETCONF_UPDATE_CONFIG_COMPLETED;
extern const isc::log::MessageID NETCONF_UPDATE_CONFIG_FAILED;
extern const isc::log::MessageID NETCONF_UPDATE_CONFIG_REJECTED;
extern const isc::log::MessageID NETCONF_UPDATE_CONFIG_STARTED;
extern const isc::log::MessageID NETCONF_UPDATE_INFO;
extern const isc::log::MessageID NETCONF_UPDATE_SYNC;
extern const isc::log::MessageID NETCONF_WARNING;

} // namespace netconf
} // namespace isc

#endif // NETCONF_MESSAGES_H

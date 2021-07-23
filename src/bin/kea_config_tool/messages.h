// File created from ../../../src/bin/kea_config_tool/messages.mes on Fri Sep 27 2019 17:38

#ifndef MESSAGES_H
#define MESSAGES_H

#include <log/message_types.h>

namespace isc {
namespace kea_config_tool {

extern const isc::log::MessageID CONFIG_TOOL_ADD_SUBNETS_CONFIG;
extern const isc::log::MessageID CONFIG_TOOL_DEL_SUBNETS_CONFIG;
extern const isc::log::MessageID CONFIG_TOOL_FAIL;
extern const isc::log::MessageID CONFIG_TOOL_GET_GENERIC_CONFIG;
extern const isc::log::MessageID CONFIG_TOOL_GET_JSON_CONFIG;
extern const isc::log::MessageID CONFIG_TOOL_GET_SHARDS_CONFIG;
extern const isc::log::MessageID CONFIG_TOOL_GET_SUBNETS_CONFIG;
extern const isc::log::MessageID CONFIG_TOOL_MASTER_GET_SERVERS;
extern const isc::log::MessageID CONFIG_TOOL_MASTER_UPDATE_SERVERS;
extern const isc::log::MessageID CONFIG_TOOL_SUCCESS;
extern const isc::log::MessageID CONFIG_TOOL_UPDATE_CONFIG;
extern const isc::log::MessageID CONFIG_TOOL_UPDATE_SHARDS_CONFIG;
extern const isc::log::MessageID CONFIG_TOOL_UPDATE_SUBNETS_CONFIG;

} // namespace kea_config_tool
} // namespace isc

#endif // MESSAGES_H

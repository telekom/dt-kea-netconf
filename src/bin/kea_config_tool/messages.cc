// File created from ../../../src/bin/kea_config_tool/messages.mes on Fri Sep 27 2019 17:38

#include <cstddef>
#include <log/message_types.h>
#include <log/message_initializer.h>

namespace isc {
namespace kea_config_tool {

extern const isc::log::MessageID CONFIG_TOOL_ADD_SUBNETS_CONFIG = "CONFIG_TOOL_ADD_SUBNETS_CONFIG";
extern const isc::log::MessageID CONFIG_TOOL_DEL_SUBNETS_CONFIG = "CONFIG_TOOL_DEL_SUBNETS_CONFIG";
extern const isc::log::MessageID CONFIG_TOOL_FAIL = "CONFIG_TOOL_FAIL";
extern const isc::log::MessageID CONFIG_TOOL_GET_GENERIC_CONFIG = "CONFIG_TOOL_GET_GENERIC_CONFIG";
extern const isc::log::MessageID CONFIG_TOOL_GET_JSON_CONFIG = "CONFIG_TOOL_GET_JSON_CONFIG";
extern const isc::log::MessageID CONFIG_TOOL_GET_SHARDS_CONFIG = "CONFIG_TOOL_GET_SHARDS_CONFIG";
extern const isc::log::MessageID CONFIG_TOOL_GET_SUBNETS_CONFIG = "CONFIG_TOOL_GET_SUBNETS_CONFIG";
extern const isc::log::MessageID CONFIG_TOOL_MASTER_GET_SERVERS = "CONFIG_TOOL_MASTER_GET_SERVERS";
extern const isc::log::MessageID CONFIG_TOOL_MASTER_UPDATE_SERVERS = "CONFIG_TOOL_MASTER_UPDATE_SERVERS";
extern const isc::log::MessageID CONFIG_TOOL_SUCCESS = "CONFIG_TOOL_SUCCESS";
extern const isc::log::MessageID CONFIG_TOOL_UPDATE_CONFIG = "CONFIG_TOOL_UPDATE_CONFIG";
extern const isc::log::MessageID CONFIG_TOOL_UPDATE_SHARDS_CONFIG = "CONFIG_TOOL_UPDATE_SHARDS_CONFIG";
extern const isc::log::MessageID CONFIG_TOOL_UPDATE_SUBNETS_CONFIG = "CONFIG_TOOL_UPDATE_SUBNETS_CONFIG";

} // namespace kea_config_tool
} // namespace isc

namespace {

const char* values[] = {
    "CONFIG_TOOL_ADD_SUBNETS_CONFIG", "Starting to update the configuration of the provided shards",
    "CONFIG_TOOL_DEL_SUBNETS_CONFIG", "Starting to update the configuration of the provided shards",
    "CONFIG_TOOL_FAIL", "ERROR: %1, exiting with code %2 (%3)",
    "CONFIG_TOOL_GET_GENERIC_CONFIG", "Starting to download the GENERIC configuration",
    "CONFIG_TOOL_GET_JSON_CONFIG", "Starting to download the JSON configuration",
    "CONFIG_TOOL_GET_SHARDS_CONFIG", "Starting to retrieve the configuration of the provided shards",
    "CONFIG_TOOL_GET_SUBNETS_CONFIG", "Starting to retrieve the configuration of the provided shards",
    "CONFIG_TOOL_MASTER_GET_SERVERS", "Starting to obtain the servers configuration from the master database",
    "CONFIG_TOOL_MASTER_UPDATE_SERVERS", "Starting to update the servers configuration on the master database",
    "CONFIG_TOOL_SUCCESS", "kea-config-tool exited succesfully.",
    "CONFIG_TOOL_UPDATE_CONFIG", "Starting to update the shard '%1' configuration into database",
    "CONFIG_TOOL_UPDATE_SHARDS_CONFIG", "Starting to update the configuration of the provided shards",
    "CONFIG_TOOL_UPDATE_SUBNETS_CONFIG", "Starting to update the configuration of the provided shards",
    NULL
};

const isc::log::MessageInitializer initializer(values);

} // Anonymous namespace


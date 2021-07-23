// File created from ../../../../src/hooks/dhcp/high_availability/ha_messages.mes on Mon Aug 17 2020 17:11

#include <cstddef>
#include <log/message_types.h>
#include <log/message_initializer.h>

namespace isc {
namespace ha {

extern const isc::log::MessageID HA_BUFFER4_RECEIVE_FAILED = "HA_BUFFER4_RECEIVE_FAILED";
extern const isc::log::MessageID HA_BUFFER4_RECEIVE_NOT_FOR_US = "HA_BUFFER4_RECEIVE_NOT_FOR_US";
extern const isc::log::MessageID HA_BUFFER4_RECEIVE_PACKET_OPTIONS_SKIPPED = "HA_BUFFER4_RECEIVE_PACKET_OPTIONS_SKIPPED";
extern const isc::log::MessageID HA_BUFFER4_RECEIVE_UNPACK_FAILED = "HA_BUFFER4_RECEIVE_UNPACK_FAILED";
extern const isc::log::MessageID HA_BUFFER6_RECEIVE_FAILED = "HA_BUFFER6_RECEIVE_FAILED";
extern const isc::log::MessageID HA_BUFFER6_RECEIVE_NOT_FOR_US = "HA_BUFFER6_RECEIVE_NOT_FOR_US";
extern const isc::log::MessageID HA_BUFFER6_RECEIVE_PACKET_OPTIONS_SKIPPED = "HA_BUFFER6_RECEIVE_PACKET_OPTIONS_SKIPPED";
extern const isc::log::MessageID HA_BUFFER6_RECEIVE_UNPACK_FAILED = "HA_BUFFER6_RECEIVE_UNPACK_FAILED";
extern const isc::log::MessageID HA_COMMAND_PROCESSED_FAILED = "HA_COMMAND_PROCESSED_FAILED";
extern const isc::log::MessageID HA_COMMUNICATION_INTERRUPTED = "HA_COMMUNICATION_INTERRUPTED";
extern const isc::log::MessageID HA_COMMUNICATION_INTERRUPTED_CLIENT4 = "HA_COMMUNICATION_INTERRUPTED_CLIENT4";
extern const isc::log::MessageID HA_COMMUNICATION_INTERRUPTED_CLIENT4_UNACKED = "HA_COMMUNICATION_INTERRUPTED_CLIENT4_UNACKED";
extern const isc::log::MessageID HA_COMMUNICATION_INTERRUPTED_CLIENT6 = "HA_COMMUNICATION_INTERRUPTED_CLIENT6";
extern const isc::log::MessageID HA_COMMUNICATION_INTERRUPTED_CLIENT6_UNACKED = "HA_COMMUNICATION_INTERRUPTED_CLIENT6_UNACKED";
extern const isc::log::MessageID HA_CONFIGURATION_FAILED = "HA_CONFIGURATION_FAILED";
extern const isc::log::MessageID HA_CONFIGURATION_SUCCESSFUL = "HA_CONFIGURATION_SUCCESSFUL";
extern const isc::log::MessageID HA_CONFIG_AUTO_FAILOVER_DISABLED = "HA_CONFIG_AUTO_FAILOVER_DISABLED";
extern const isc::log::MessageID HA_CONFIG_LEASE_SYNCING_DISABLED = "HA_CONFIG_LEASE_SYNCING_DISABLED";
extern const isc::log::MessageID HA_CONFIG_LEASE_SYNCING_DISABLED_REMINDER = "HA_CONFIG_LEASE_SYNCING_DISABLED_REMINDER";
extern const isc::log::MessageID HA_CONFIG_LEASE_UPDATES_AND_SYNCING_DIFFER = "HA_CONFIG_LEASE_UPDATES_AND_SYNCING_DIFFER";
extern const isc::log::MessageID HA_CONFIG_LEASE_UPDATES_DISABLED = "HA_CONFIG_LEASE_UPDATES_DISABLED";
extern const isc::log::MessageID HA_CONFIG_LEASE_UPDATES_DISABLED_REMINDER = "HA_CONFIG_LEASE_UPDATES_DISABLED_REMINDER";
extern const isc::log::MessageID HA_CONTINUE_HANDLER_FAILED = "HA_CONTINUE_HANDLER_FAILED";
extern const isc::log::MessageID HA_DEINIT_OK = "HA_DEINIT_OK";
extern const isc::log::MessageID HA_DHCP4_START_SERVICE_FAILED = "HA_DHCP4_START_SERVICE_FAILED";
extern const isc::log::MessageID HA_DHCP6_START_SERVICE_FAILED = "HA_DHCP6_START_SERVICE_FAILED";
extern const isc::log::MessageID HA_DHCP_DISABLE_COMMUNICATIONS_FAILED = "HA_DHCP_DISABLE_COMMUNICATIONS_FAILED";
extern const isc::log::MessageID HA_DHCP_DISABLE_FAILED = "HA_DHCP_DISABLE_FAILED";
extern const isc::log::MessageID HA_DHCP_ENABLE_COMMUNICATIONS_FAILED = "HA_DHCP_ENABLE_COMMUNICATIONS_FAILED";
extern const isc::log::MessageID HA_DHCP_ENABLE_FAILED = "HA_DHCP_ENABLE_FAILED";
extern const isc::log::MessageID HA_HEARTBEAT_COMMUNICATIONS_FAILED = "HA_HEARTBEAT_COMMUNICATIONS_FAILED";
extern const isc::log::MessageID HA_HEARTBEAT_FAILED = "HA_HEARTBEAT_FAILED";
extern const isc::log::MessageID HA_HEARTBEAT_HANDLER_FAILED = "HA_HEARTBEAT_HANDLER_FAILED";
extern const isc::log::MessageID HA_HIGH_CLOCK_SKEW = "HA_HIGH_CLOCK_SKEW";
extern const isc::log::MessageID HA_HIGH_CLOCK_SKEW_CAUSES_TERMINATION = "HA_HIGH_CLOCK_SKEW_CAUSES_TERMINATION";
extern const isc::log::MessageID HA_INIT_OK = "HA_INIT_OK";
extern const isc::log::MessageID HA_LEASES4_COMMITTED_FAILED = "HA_LEASES4_COMMITTED_FAILED";
extern const isc::log::MessageID HA_LEASES4_COMMITTED_NOTHING_TO_UPDATE = "HA_LEASES4_COMMITTED_NOTHING_TO_UPDATE";
extern const isc::log::MessageID HA_LEASES6_COMMITTED_FAILED = "HA_LEASES6_COMMITTED_FAILED";
extern const isc::log::MessageID HA_LEASES6_COMMITTED_NOTHING_TO_UPDATE = "HA_LEASES6_COMMITTED_NOTHING_TO_UPDATE";
extern const isc::log::MessageID HA_LEASES_SYNC_COMMUNICATIONS_FAILED = "HA_LEASES_SYNC_COMMUNICATIONS_FAILED";
extern const isc::log::MessageID HA_LEASES_SYNC_FAILED = "HA_LEASES_SYNC_FAILED";
extern const isc::log::MessageID HA_LEASES_SYNC_LEASE_PAGE_RECEIVED = "HA_LEASES_SYNC_LEASE_PAGE_RECEIVED";
extern const isc::log::MessageID HA_LEASE_SYNC_FAILED = "HA_LEASE_SYNC_FAILED";
extern const isc::log::MessageID HA_LEASE_SYNC_STALE_LEASE4_SKIP = "HA_LEASE_SYNC_STALE_LEASE4_SKIP";
extern const isc::log::MessageID HA_LEASE_SYNC_STALE_LEASE6_SKIP = "HA_LEASE_SYNC_STALE_LEASE6_SKIP";
extern const isc::log::MessageID HA_LEASE_UPDATES_DISABLED = "HA_LEASE_UPDATES_DISABLED";
extern const isc::log::MessageID HA_LEASE_UPDATES_ENABLED = "HA_LEASE_UPDATES_ENABLED";
extern const isc::log::MessageID HA_LEASE_UPDATE_COMMUNICATIONS_FAILED = "HA_LEASE_UPDATE_COMMUNICATIONS_FAILED";
extern const isc::log::MessageID HA_LEASE_UPDATE_CREATE_UPDATE_FAILED_ON_PEER = "HA_LEASE_UPDATE_CREATE_UPDATE_FAILED_ON_PEER";
extern const isc::log::MessageID HA_LEASE_UPDATE_DELETE_FAILED_ON_PEER = "HA_LEASE_UPDATE_DELETE_FAILED_ON_PEER";
extern const isc::log::MessageID HA_LEASE_UPDATE_FAILED = "HA_LEASE_UPDATE_FAILED";
extern const isc::log::MessageID HA_LOAD_BALANCING_DUID_MISSING = "HA_LOAD_BALANCING_DUID_MISSING";
extern const isc::log::MessageID HA_LOAD_BALANCING_IDENTIFIER_MISSING = "HA_LOAD_BALANCING_IDENTIFIER_MISSING";
extern const isc::log::MessageID HA_LOCAL_DHCP_DISABLE = "HA_LOCAL_DHCP_DISABLE";
extern const isc::log::MessageID HA_LOCAL_DHCP_ENABLE = "HA_LOCAL_DHCP_ENABLE";
extern const isc::log::MessageID HA_MAINTENANCE_CANCEL_HANDLER_FAILED = "HA_MAINTENANCE_CANCEL_HANDLER_FAILED";
extern const isc::log::MessageID HA_MAINTENANCE_NOTIFY_CANCEL_COMMUNICATIONS_FAILED = "HA_MAINTENANCE_NOTIFY_CANCEL_COMMUNICATIONS_FAILED";
extern const isc::log::MessageID HA_MAINTENANCE_NOTIFY_CANCEL_FAILED = "HA_MAINTENANCE_NOTIFY_CANCEL_FAILED";
extern const isc::log::MessageID HA_MAINTENANCE_NOTIFY_COMMUNICATIONS_FAILED = "HA_MAINTENANCE_NOTIFY_COMMUNICATIONS_FAILED";
extern const isc::log::MessageID HA_MAINTENANCE_NOTIFY_FAILED = "HA_MAINTENANCE_NOTIFY_FAILED";
extern const isc::log::MessageID HA_MAINTENANCE_NOTIFY_HANDLER_FAILED = "HA_MAINTENANCE_NOTIFY_HANDLER_FAILED";
extern const isc::log::MessageID HA_MAINTENANCE_SHUTDOWN_SAFE = "HA_MAINTENANCE_SHUTDOWN_SAFE";
extern const isc::log::MessageID HA_MAINTENANCE_STARTED = "HA_MAINTENANCE_STARTED";
extern const isc::log::MessageID HA_MAINTENANCE_STARTED_IN_PARTNER_DOWN = "HA_MAINTENANCE_STARTED_IN_PARTNER_DOWN";
extern const isc::log::MessageID HA_MAINTENANCE_START_HANDLER_FAILED = "HA_MAINTENANCE_START_HANDLER_FAILED";
extern const isc::log::MessageID HA_MISSING_CONFIGURATION = "HA_MISSING_CONFIGURATION";
extern const isc::log::MessageID HA_SCOPES_HANDLER_FAILED = "HA_SCOPES_HANDLER_FAILED";
extern const isc::log::MessageID HA_SERVICE_STARTED = "HA_SERVICE_STARTED";
extern const isc::log::MessageID HA_STATE_MACHINE_CONTINUED = "HA_STATE_MACHINE_CONTINUED";
extern const isc::log::MessageID HA_STATE_MACHINE_PAUSED = "HA_STATE_MACHINE_PAUSED";
extern const isc::log::MessageID HA_STATE_TRANSITION = "HA_STATE_TRANSITION";
extern const isc::log::MessageID HA_STATE_TRANSITION_PASSIVE_BACKUP = "HA_STATE_TRANSITION_PASSIVE_BACKUP";
extern const isc::log::MessageID HA_SYNC_FAILED = "HA_SYNC_FAILED";
extern const isc::log::MessageID HA_SYNC_HANDLER_FAILED = "HA_SYNC_HANDLER_FAILED";
extern const isc::log::MessageID HA_SYNC_START = "HA_SYNC_START";
extern const isc::log::MessageID HA_SYNC_SUCCESSFUL = "HA_SYNC_SUCCESSFUL";
extern const isc::log::MessageID HA_TERMINATED = "HA_TERMINATED";
extern const isc::log::MessageID HA_TERMINATED_RESTART_PARTNER = "HA_TERMINATED_RESTART_PARTNER";

} // namespace ha
} // namespace isc

namespace {

const char* values[] = {
    "HA_BUFFER4_RECEIVE_FAILED", "buffer4_receive callout failed: %1",
    "HA_BUFFER4_RECEIVE_NOT_FOR_US", "%1: dropping query to be processed by another server",
    "HA_BUFFER4_RECEIVE_PACKET_OPTIONS_SKIPPED", "an error upacking an option, caused subsequent options to be skipped: %1",
    "HA_BUFFER4_RECEIVE_UNPACK_FAILED", "failed to parse query from %1 to %2, received over interface %3, reason: %4",
    "HA_BUFFER6_RECEIVE_FAILED", "buffer6_receive callout failed: %1",
    "HA_BUFFER6_RECEIVE_NOT_FOR_US", "%1: dropping query to be processed by another server",
    "HA_BUFFER6_RECEIVE_PACKET_OPTIONS_SKIPPED", "an error upacking an option, caused subsequent options to be skipped: %1",
    "HA_BUFFER6_RECEIVE_UNPACK_FAILED", "failed to parse query from %1 to %2, received over interface %3, reason: %4",
    "HA_COMMAND_PROCESSED_FAILED", "command_processed callout failed: %1",
    "HA_COMMUNICATION_INTERRUPTED", "communication with %1 is interrupted",
    "HA_COMMUNICATION_INTERRUPTED_CLIENT4", "%1: new client attempting to get a lease from the partner",
    "HA_COMMUNICATION_INTERRUPTED_CLIENT4_UNACKED", "%1: partner server failed to respond, %2 clients unacked so far, %3 clients left before transitioning to the partner-down state",
    "HA_COMMUNICATION_INTERRUPTED_CLIENT6", "%1: new client attempting to get a lease from the partner",
    "HA_COMMUNICATION_INTERRUPTED_CLIENT6_UNACKED", "%1: partner server failed to respond, %2 clients unacked so far, %3 clients left before transitioning to the partner-down state",
    "HA_CONFIGURATION_FAILED", "failed to configure High Availability hooks library: %1",
    "HA_CONFIGURATION_SUCCESSFUL", "HA hook library has been successfully configured",
    "HA_CONFIG_AUTO_FAILOVER_DISABLED", "auto-failover disabled for %1",
    "HA_CONFIG_LEASE_SYNCING_DISABLED", "lease database synchronization between HA servers is disabled",
    "HA_CONFIG_LEASE_SYNCING_DISABLED_REMINDER", "bypassing SYNCING state because lease database synchronization is administratively disabled",
    "HA_CONFIG_LEASE_UPDATES_AND_SYNCING_DIFFER", "unusual configuration where \"send-lease-updates\": %1 and \"sync-leases\": %2",
    "HA_CONFIG_LEASE_UPDATES_DISABLED", "lease updates will not be generated",
    "HA_CONFIG_LEASE_UPDATES_DISABLED_REMINDER", "lease updates are administratively disabled and will not be generated while in %1 state",
    "HA_CONTINUE_HANDLER_FAILED", "ha-continue command failed: %1",
    "HA_DEINIT_OK", "unloading High Availability hooks library successful",
    "HA_DHCP4_START_SERVICE_FAILED", "failed to start DHCPv4 HA service in dhcp4_srv_configured callout: %1",
    "HA_DHCP6_START_SERVICE_FAILED", "failed to start DHCPv4 HA service in dhcp6_srv_configured callout: %1",
    "HA_DHCP_DISABLE_COMMUNICATIONS_FAILED", "failed to send request to disable DHCP service of %1: %2",
    "HA_DHCP_DISABLE_FAILED", "failed to disable DHCP service of %1: %2",
    "HA_DHCP_ENABLE_COMMUNICATIONS_FAILED", "failed to send request to enable DHCP service of %1: %2",
    "HA_DHCP_ENABLE_FAILED", "failed to enable DHCP service of %1: %2",
    "HA_HEARTBEAT_COMMUNICATIONS_FAILED", "failed to send heartbeat to %1: %2",
    "HA_HEARTBEAT_FAILED", "heartbeat to %1 failed: %2",
    "HA_HEARTBEAT_HANDLER_FAILED", "heartbeat command failed: %1",
    "HA_HIGH_CLOCK_SKEW", "%1, please synchronize clocks!",
    "HA_HIGH_CLOCK_SKEW_CAUSES_TERMINATION", "%1, causing HA service to terminate",
    "HA_INIT_OK", "loading High Availability hooks library successful",
    "HA_LEASES4_COMMITTED_FAILED", "leases4_committed callout failed: %1",
    "HA_LEASES4_COMMITTED_NOTHING_TO_UPDATE", "%1: leases4_committed callout was invoked without any leases",
    "HA_LEASES6_COMMITTED_FAILED", "leases6_committed callout failed: %1",
    "HA_LEASES6_COMMITTED_NOTHING_TO_UPDATE", "%1: leases6_committed callout was invoked without any leases",
    "HA_LEASES_SYNC_COMMUNICATIONS_FAILED", "failed to communicate with %1 while syncing leases: %2",
    "HA_LEASES_SYNC_FAILED", "failed to synchronize leases with %1: %2",
    "HA_LEASES_SYNC_LEASE_PAGE_RECEIVED", "received %1 leases from %2",
    "HA_LEASE_SYNC_FAILED", "synchronization failed for lease: %1, reason: %2",
    "HA_LEASE_SYNC_STALE_LEASE4_SKIP", "skipping stale lease %1 in subnet %2",
    "HA_LEASE_SYNC_STALE_LEASE6_SKIP", "skipping stale lease %1 in subnet %2",
    "HA_LEASE_UPDATES_DISABLED", "lease updates will not be sent to the partner while in %1 state",
    "HA_LEASE_UPDATES_ENABLED", "lease updates will be sent to the partner while in %1 state",
    "HA_LEASE_UPDATE_COMMUNICATIONS_FAILED", "%1: failed to communicate with %2: %3",
    "HA_LEASE_UPDATE_CREATE_UPDATE_FAILED_ON_PEER", "%1: failed to create or update the lease having type %2 for address %3, reason: %4",
    "HA_LEASE_UPDATE_DELETE_FAILED_ON_PEER", "%1: failed to delete the lease having type %2 for address %3, reason: %4",
    "HA_LEASE_UPDATE_FAILED", "%1: lease update to %2 failed: %3",
    "HA_LOAD_BALANCING_DUID_MISSING", "load balancing failed for the DHCPv6 message (transaction id: %1) because DUID is missing",
    "HA_LOAD_BALANCING_IDENTIFIER_MISSING", "load balancing failed for the DHCPv4 message (transaction id: %1) because HW address and client identifier are missing",
    "HA_LOCAL_DHCP_DISABLE", "local DHCP service is disabled while the %1 is in the %2 state",
    "HA_LOCAL_DHCP_ENABLE", "local DHCP service is enabled while the %1 is in the %2 state",
    "HA_MAINTENANCE_CANCEL_HANDLER_FAILED", "ha-maintenance-cancel command failed: %1",
    "HA_MAINTENANCE_NOTIFY_CANCEL_COMMUNICATIONS_FAILED", "failed to send ha-maintenance-notify to %1 in attempt to cancel its maintenance: %2",
    "HA_MAINTENANCE_NOTIFY_CANCEL_FAILED", "error returned while processing ha-maintenance-notify by %1 in attempt to cancel its maintenance: %2",
    "HA_MAINTENANCE_NOTIFY_COMMUNICATIONS_FAILED", "failed to send ha-maintenance-notify to %1: %2",
    "HA_MAINTENANCE_NOTIFY_FAILED", "error returned while processing ha-maintenance-notify by %1: %2",
    "HA_MAINTENANCE_NOTIFY_HANDLER_FAILED", "ha-maintenance-notify command failed: %1",
    "HA_MAINTENANCE_SHUTDOWN_SAFE", "the server can now be shutdown for maintenance as the partner has taken over the DHCP traffic",
    "HA_MAINTENANCE_STARTED", "the server is now in the partner-in-maintenance state and the partner is in-maintenance state",
    "HA_MAINTENANCE_STARTED_IN_PARTNER_DOWN", "the server is now in the partner-down mode as a result of requested maintenance",
    "HA_MAINTENANCE_START_HANDLER_FAILED", "ha-maintenance-start command failed: %1",
    "HA_MISSING_CONFIGURATION", "high-availability parameter not specified for High Availability hooks library",
    "HA_SCOPES_HANDLER_FAILED", "ha-scopes command failed: %1",
    "HA_SERVICE_STARTED", "started high availability service in %1 mode as %2 server",
    "HA_STATE_MACHINE_CONTINUED", "state machine is un-paused",
    "HA_STATE_MACHINE_PAUSED", "state machine paused in state %1",
    "HA_STATE_TRANSITION", "server transitions from %1 to %2 state, partner state is %3",
    "HA_STATE_TRANSITION_PASSIVE_BACKUP", "server transitions from %1 to %2 state",
    "HA_SYNC_FAILED", "lease database synchronization with %1 failed: %2",
    "HA_SYNC_HANDLER_FAILED", "ha-sync command failed: %1",
    "HA_SYNC_START", "starting lease database synchronization with %1",
    "HA_SYNC_SUCCESSFUL", "lease database synchronization with %1 completed successfully in %2",
    "HA_TERMINATED", "HA service terminated due to an unrecoverable condition. Check previous error message(s), address the problem and restart!",
    "HA_TERMINATED_RESTART_PARTNER", "waiting for the partner in the TERMINATED state to be restarted",
    NULL
};

const isc::log::MessageInitializer initializer(values);

} // Anonymous namespace


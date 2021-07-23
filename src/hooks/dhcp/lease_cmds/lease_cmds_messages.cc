// File created from ../../../../src/hooks/dhcp/lease_cmds/lease_cmds_messages.mes on Mon Aug 17 2020 17:11

#include <cstddef>
#include <log/message_types.h>
#include <log/message_initializer.h>

extern const isc::log::MessageID LEASE_CMDS_ADD4 = "LEASE_CMDS_ADD4";
extern const isc::log::MessageID LEASE_CMDS_ADD4_FAILED = "LEASE_CMDS_ADD4_FAILED";
extern const isc::log::MessageID LEASE_CMDS_ADD6 = "LEASE_CMDS_ADD6";
extern const isc::log::MessageID LEASE_CMDS_ADD6_FAILED = "LEASE_CMDS_ADD6_FAILED";
extern const isc::log::MessageID LEASE_CMDS_DEINIT_FAILED = "LEASE_CMDS_DEINIT_FAILED";
extern const isc::log::MessageID LEASE_CMDS_DEINIT_OK = "LEASE_CMDS_DEINIT_OK";
extern const isc::log::MessageID LEASE_CMDS_DEL4 = "LEASE_CMDS_DEL4";
extern const isc::log::MessageID LEASE_CMDS_DEL4_FAILED = "LEASE_CMDS_DEL4_FAILED";
extern const isc::log::MessageID LEASE_CMDS_DEL6 = "LEASE_CMDS_DEL6";
extern const isc::log::MessageID LEASE_CMDS_DEL6_FAILED = "LEASE_CMDS_DEL6_FAILED";
extern const isc::log::MessageID LEASE_CMDS_INIT_FAILED = "LEASE_CMDS_INIT_FAILED";
extern const isc::log::MessageID LEASE_CMDS_INIT_OK = "LEASE_CMDS_INIT_OK";
extern const isc::log::MessageID LEASE_CMDS_RESEND_DDNS4 = "LEASE_CMDS_RESEND_DDNS4";
extern const isc::log::MessageID LEASE_CMDS_RESEND_DDNS4_FAILED = "LEASE_CMDS_RESEND_DDNS4_FAILED";
extern const isc::log::MessageID LEASE_CMDS_RESEND_DDNS6 = "LEASE_CMDS_RESEND_DDNS6";
extern const isc::log::MessageID LEASE_CMDS_RESEND_DDNS6_FAILED = "LEASE_CMDS_RESEND_DDNS6_FAILED";

namespace {

const char* values[] = {
    "LEASE_CMDS_ADD4", "lease4-add command successful (parameters: %1)",
    "LEASE_CMDS_ADD4_FAILED", "lease4-add command failed (parameters: %1, reason: %2)",
    "LEASE_CMDS_ADD6", "lease6-add command successful (parameters: %1)",
    "LEASE_CMDS_ADD6_FAILED", "Lease6-add command failed (parameters: %1, reason: %2)",
    "LEASE_CMDS_DEINIT_FAILED", "unloading Lease Commands hooks library failed: %1",
    "LEASE_CMDS_DEINIT_OK", "unloading Lease Commands hooks library successful",
    "LEASE_CMDS_DEL4", "lease4-del command successful (parameters: %1)",
    "LEASE_CMDS_DEL4_FAILED", "lease4-del command failed (parameters: %1, reason: %2)",
    "LEASE_CMDS_DEL6", "lease4-del command successful (parameters: %1)",
    "LEASE_CMDS_DEL6_FAILED", "lease6-del command failed (parameters: %1, reason: %2)",
    "LEASE_CMDS_INIT_FAILED", "loading Lease Commands hooks library failed: %1",
    "LEASE_CMDS_INIT_OK", "loading Lease Commands hooks library successful",
    "LEASE_CMDS_RESEND_DDNS4", "lease4-resend-ddns command successful: %1",
    "LEASE_CMDS_RESEND_DDNS4_FAILED", "lease4-resend-ddns command failed: %1",
    "LEASE_CMDS_RESEND_DDNS6", "lease6-resend-ddns command successful: %1",
    "LEASE_CMDS_RESEND_DDNS6_FAILED", "lease6-resend-ddns command failed: %1",
    NULL
};

const isc::log::MessageInitializer initializer(values);

} // Anonymous namespace


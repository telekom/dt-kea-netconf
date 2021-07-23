// File created from ../../../src/bin/admin/messages.mes on Thu Mar 14 2019 08:35

#include <cstddef>
#include <log/message_types.h>
#include <log/message_initializer.h>

namespace isc {
namespace admin {

extern const isc::log::MessageID KEA_ADMIN_RUN = "KEA_ADMIN_RUN";

} // namespace admin
} // namespace isc

namespace {

const char* values[] = {
    "KEA_ADMIN_RUN", "Running the kea-admin script...",
    NULL
};

const isc::log::MessageInitializer initializer(values);

} // Anonymous namespace


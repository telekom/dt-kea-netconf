// File created from ../../../src/lib/database/db_messages.mes on Mon Aug 17 2020 17:11

#ifndef DB_MESSAGES_H
#define DB_MESSAGES_H

#include <log/message_types.h>

namespace isc {
namespace db {

extern const isc::log::MessageID DATABASE_CQL_CONNECTION_BEGIN_TRANSACTION;
extern const isc::log::MessageID DATABASE_CQL_CONNECTION_COMMIT;
extern const isc::log::MessageID DATABASE_CQL_CONNECTION_ROLLBACK;
extern const isc::log::MessageID DATABASE_CQL_DEALLOC_ERROR;
extern const isc::log::MessageID DATABASE_CQL_TRANSACTION_MGR_COMMIT_TRANSACTION;
extern const isc::log::MessageID DATABASE_CQL_TRANSACTION_MGR_REGISTER_TRANSACTION;
extern const isc::log::MessageID DATABASE_CQL_TRANSACTION_MGR_REGISTER_TRANSACTION_COMMIT;
extern const isc::log::MessageID DATABASE_CQL_TRANSACTION_MGR_REGISTER_TRANSACTION_ROLLBACK;
extern const isc::log::MessageID DATABASE_CQL_TRANSACTION_MGR_ROLLBACK_TRANSACTION;
extern const isc::log::MessageID DATABASE_CQL_TRANSACTION_MGR_START_TRANSACTION;
extern const isc::log::MessageID DATABASE_CQL_TRANSACTION_MGR_TRANSACTION_COMMIT;
extern const isc::log::MessageID DATABASE_CQL_TRANSACTION_MGR_TRANSACTION_NOT_FOUND;
extern const isc::log::MessageID DATABASE_CQL_TRANSACTION_MGR_TRANSACTION_ROLLBACK;
extern const isc::log::MessageID DATABASE_CQL_TRANSACTION_MGR_TRANSACTION_START;
extern const isc::log::MessageID DATABASE_INVALID_ACCESS;
extern const isc::log::MessageID DATABASE_MYSQL_COMMIT;
extern const isc::log::MessageID DATABASE_MYSQL_FATAL_ERROR;
extern const isc::log::MessageID DATABASE_MYSQL_ROLLBACK;
extern const isc::log::MessageID DATABASE_MYSQL_START_TRANSACTION;
extern const isc::log::MessageID DATABASE_PGSQL_COMMIT;
extern const isc::log::MessageID DATABASE_PGSQL_DEALLOC_ERROR;
extern const isc::log::MessageID DATABASE_PGSQL_FATAL_ERROR;
extern const isc::log::MessageID DATABASE_PGSQL_ROLLBACK;
extern const isc::log::MessageID DATABASE_PGSQL_START_TRANSACTION;
extern const isc::log::MessageID DATABASE_TO_JSON_ERROR;

} // namespace db
} // namespace isc

#endif // DB_MESSAGES_H

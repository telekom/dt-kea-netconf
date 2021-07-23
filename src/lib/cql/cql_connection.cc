// Copyright (C) 2018-2020 Internet Systems Consortium, Inc. ("ISC")
// Copyright (C) 2020 Deutsche Telekom AG.
//
// Authors: Razvan Becheriu <razvan.becheriu@qualitance.com>
//          Andrei Pavel <andrei.pavel@qualitance.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//                 https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <config.h>

#include <cql/cql_connection.h>
#include <cql/cql_exchange.h>
#include <cql/cql_transaction.h>

#include <database/db_exceptions.h>
#include <database/db_log.h>
#include <util/func.h>
#include <util/log.h>

#include <boost/lexical_cast.hpp>

#include <syslog.h>

#include <fstream>
#include <string>

using isc::log::Spdlog;

namespace isc {
namespace db {

#ifdef TERASTREAM
void static syslog_log_callback(CassLogMessage const* message, void* /* data */) {
    syslog(LOG_INFO, "%u.%03u [%s] (%s:%d:%s): %s\n",
           static_cast<unsigned int>(message->time_ms / 1000),
           static_cast<unsigned int>(message->time_ms % 1000),
           cass_log_level_string(message->severity), message->file, message->line,
           message->function, message->message);
    if (DatabaseConnection::enable_all_traces_) {
        std::cout << static_cast<unsigned int>(message->time_ms / 1000) << "."
                  << static_cast<unsigned int>(message->time_ms % 1000) << " ["
                  << cass_log_level_string(message->severity) << "] (" << message->file << ":"
                  << message->line << ":" << message->function << "): " << message->message
                  << std::endl;
    }
}
#endif  //  TERASTREAM

CqlConnection::CqlConnection(ParameterMap const& parameters)
    : DatabaseConnection(parameters),
#ifdef TERASTREAM_FULL_TRANSACTIONS
      begin_index_(""), commit_index_(""), rollback_index_(""),
#endif  // TERASTREAM_FULL_TRANSACTIONS
      statements_(), cluster_(MakePtr(cass_cluster_new())), session_(MakePtr(cass_session_new())),
      consistency_(CASS_CONSISTENCY_QUORUM), serial_consistency_(CASS_CONSISTENCY_QUORUM),
      max_statement_tries_(1), force_consistency_(true) {
    openDatabase();
}

CassConsistency CqlConnection::parseConsistency(std::string const& value) {
    static std::map<std::string, CassConsistency> consistency_map{
        {"any", CASS_CONSISTENCY_ANY},
        {"one", CASS_CONSISTENCY_ONE},
        {"two", CASS_CONSISTENCY_TWO},
        {"three", CASS_CONSISTENCY_THREE},
        {"quorum", CASS_CONSISTENCY_QUORUM},
        {"all", CASS_CONSISTENCY_ALL},
        {"local-quorum", CASS_CONSISTENCY_LOCAL_QUORUM},
        {"each-quorum", CASS_CONSISTENCY_EACH_QUORUM},
        {"serial", CASS_CONSISTENCY_SERIAL},
        {"local-serial", CASS_CONSISTENCY_LOCAL_SERIAL},
        {"local-one", CASS_CONSISTENCY_LOCAL_ONE}};
    if (consistency_map.find(value) == consistency_map.end()) {
        return CASS_CONSISTENCY_UNKNOWN;
    }
    return consistency_map[value];
}

void CqlConnection::openDatabase() {
    CassError rc;
    // Set up the values of the parameters
    char const* contact_points = "127.0.0.1";
    std::string scontact_points;
    try {
        scontact_points = getParameter("contact-points");
        contact_points = scontact_points.c_str();
    } catch (...) {
        // No host. Fine, we'll use "127.0.0.1".
    }

    char const* port = NULL;
    std::string sport;
    try {
        sport = getParameter("port");
        port = sport.c_str();
    } catch (...) {
        // No port. Fine, we'll use the default "9042".
    }

    char const* user = NULL;
    std::string suser;
    try {
        suser = getParameter("user");
        user = suser.c_str();
    } catch (...) {
        // No user. Fine, we'll use NULL.
    }

    char const* password = NULL;
    std::string spassword;
    try {
        spassword = getParameter("password");
        password = spassword.c_str();
    } catch (...) {
        // No password. Fine, we'll use NULL.
    }

    char const* keyspace = "keatest";
    std::string skeyspace;
    try {
        skeyspace = getParameter("keyspace");
        keyspace = skeyspace.c_str();
    } catch (...) {
        // No keyspace name. Fine, we'll use "keatest".
    }

    char const* consistency = NULL;
    std::string sconsistency;
    try {
        sconsistency = getParameter("consistency");
        consistency = sconsistency.c_str();
    } catch (...) {
        // No consistency. Fine, we'll use "quorum".
    }

    char const* serial_consistency = NULL;
    std::string sserial_consistency;
    try {
        sserial_consistency = getParameter("serial-consistency");
        serial_consistency = sserial_consistency.c_str();
    } catch (...) {
        // No serial consistency. Fine, we'll use "serial".
    }

    char const* reconnect_wait_time = NULL;
    std::string sreconnect_wait_time;
    try {
        sreconnect_wait_time = getParameter("reconnect-wait-time");
        reconnect_wait_time = sreconnect_wait_time.c_str();
    } catch (...) {
        // No reconnect wait time. Fine, we'll use the default "2000".
    }

    char const* connect_timeout = NULL;
    std::string sconnect_timeout;
    try {
        sconnect_timeout = getParameter("connect-timeout");
        connect_timeout = sconnect_timeout.c_str();
    } catch (...) {
        // No connect timeout. Fine, we'll use the default "5000".
    }

    char const* request_timeout = NULL;
    std::string srequest_timeout;
    try {
        srequest_timeout = getParameter("request-timeout");
        request_timeout = srequest_timeout.c_str();
    } catch (...) {
        // No request timeout. Fine, we'll use the default "12000".
    }

    char const* tcp_keepalive = NULL;
    std::string stcp_keepalive;
    try {
        stcp_keepalive = getParameter("tcp-keepalive");
        tcp_keepalive = stcp_keepalive.c_str();
    } catch (...) {
        // No tcp-keepalive. Fine, we'll not use TCP keepalive.
    }

    std::string stcp_nodelay;
    try {
        stcp_nodelay = getParameter("tcp-nodelay");
    } catch (...) {
        // No tcp-nodelay. Fine, we'll use the default false.
    }

    char const* ssl_cert = NULL;
    std::string sssl_cert;
    try {
        sssl_cert = getParameter("ssl-cert");
        ssl_cert = sssl_cert.c_str();
    } catch (...) {
        // No ssl-cert. Fine, we'll disable the ssl verification.
    }

    char const* max_statement_tries = NULL;
    std::string smax_statement_tries;
    try {
        smax_statement_tries = getParameter("max-statement-tries");
        max_statement_tries = smax_statement_tries.c_str();
    } catch (...) {
        // No max statement tries. Fine, we'll use the default 1.
    }

#ifdef TERASTREAM
    cass_log_set_callback(syslog_log_callback, NULL);
    if (DatabaseConnection::enable_all_traces_) {
        cass_log_set_level(CASS_LOG_TRACE);
    }
#endif  // TERASTREAM

    cass_cluster_set_contact_points(cluster_.get(), contact_points);

    if (user && password) {
        cass_cluster_set_credentials(cluster_.get(), user, password);
    }

    if (port) {
        int32_t port_number;
        try {
            port_number = boost::lexical_cast<int32_t>(port);
            if (port_number < 1 || port_number > 65535) {
                isc_throw(DbOperationError, "CqlConnection::openDatabase(): "
                                            "port outside of range, expected "
                                            "1-65535, instead got "
                                                << port);
            }
        } catch (boost::bad_lexical_cast const& ex) {
            isc_throw(DbOperationError, "CqlConnection::openDatabase(): invalid "
                                        "port, expected castable to int, instead got "
                                        "\"" << port
                                             << "\", " << ex.what());
        }
        cass_cluster_set_port(cluster_.get(), port_number);
    }

    if (consistency) {
        CassConsistency desired_consistency = CqlConnection::parseConsistency(sconsistency);
        CassConsistency desired_serial_consistency = CASS_CONSISTENCY_UNKNOWN;
        if (serial_consistency) {
            desired_serial_consistency = CqlConnection::parseConsistency(sserial_consistency);
        }
        if (desired_consistency != CASS_CONSISTENCY_UNKNOWN) {
            setConsistency(true, desired_consistency, desired_serial_consistency);
        }
    }

    if (reconnect_wait_time) {
        int64_t reconnect_wait_time_number;
        try {
            reconnect_wait_time_number = boost::lexical_cast<int64_t>(reconnect_wait_time);
            if (reconnect_wait_time_number < 0) {
                isc_throw(DbOperationError, "CqlConnection::openDatabase(): invalid reconnect "
                                            "wait time, expected positive number, instead got "
                                                << reconnect_wait_time);
            }
        } catch (boost::bad_lexical_cast const& ex) {
            isc_throw(DbOperationError, "CqlConnection::openDatabase(): "
                                        "invalid reconnect wait time, expected "
                                        "castable to int, instead got \""
                                            << reconnect_wait_time << "\", " << ex.what());
        }
        cass_cluster_set_constant_reconnect(cluster_.get(), reconnect_wait_time_number);
    }

    if (connect_timeout) {
        int32_t connect_timeout_number;
        try {
            connect_timeout_number = boost::lexical_cast<int32_t>(connect_timeout);
            if (connect_timeout_number < 0) {
                isc_throw(DbOperationError, "CqlConnection::openDatabase(): "
                                            "invalid connect timeout, expected "
                                            "positive number, instead got "
                                                << connect_timeout);
            }
        } catch (boost::bad_lexical_cast const& ex) {
            isc_throw(DbOperationError, "CqlConnection::openDatabase(): invalid connect timeout, "
                                        "expected castable to int, instead got \""
                                            << connect_timeout << "\", " << ex.what());
        }
        cass_cluster_set_connect_timeout(cluster_.get(), connect_timeout_number);
    }

    if (request_timeout) {
        int32_t request_timeout_number;
        try {
            request_timeout_number = boost::lexical_cast<int32_t>(request_timeout);
            if (request_timeout_number < 0) {
                isc_throw(DbOperationError, "CqlConnection::openDatabase(): "
                                            "invalid request timeout, expected "
                                            "positive number, instead got "
                                                << request_timeout);
            }
        } catch (boost::bad_lexical_cast const& ex) {
            isc_throw(DbOperationError, "CqlConnection::openDatabase(): invalid request timeout, "
                                        "expected castable to int, instead got \""
                                            << request_timeout << "\", " << ex.what());
        }
        cass_cluster_set_request_timeout(cluster_.get(), request_timeout_number);
    }

    if (tcp_keepalive) {
        int32_t tcp_keepalive_number;
        try {
            tcp_keepalive_number = boost::lexical_cast<int32_t>(tcp_keepalive);
            if (tcp_keepalive_number < 0) {
                isc_throw(DbOperationError, "CqlConnection::openDatabase(): "
                                            "invalid TCP keepalive, expected "
                                            "positive number, instead got "
                                                << tcp_keepalive);
            }
        } catch (boost::bad_lexical_cast const& ex) {
            isc_throw(DbOperationError, "CqlConnection::openDatabase(): invalid TCP keepalive, "
                                        "expected castable to int, instead got \""
                                            << tcp_keepalive << "\", " << ex.what());
        }
        cass_cluster_set_tcp_keepalive(cluster_.get(), cass_true, tcp_keepalive_number);
    }

    if (stcp_nodelay == "true") {
        cass_cluster_set_tcp_nodelay(cluster_.get(), cass_true);
    }

    auto ssl = MakePtr(cass_ssl_new());

    if (ssl_cert) {
        std::ifstream ssl_cert_file(ssl_cert, std::ios::binary);
        if (!ssl_cert_file.is_open()) {
            isc_throw(DbOperationError, "CqlConnection::openDatabase(): "
                                        "invalid ssl certificate file: "
                                            << sssl_cert);
        }

        std::vector<char> cert((std::istreambuf_iterator<char>(ssl_cert_file)),
                               (std::istreambuf_iterator<char>()));
        ssl_cert_file.close();

        rc = cass_ssl_add_trusted_cert_n(ssl.get(), cert.data(), cert.size());
        if (rc != CASS_OK) {
            isc_throw(DbOperationError, "CqlConnection::openDatabase(): "
                                        "error loading ssl certificate file: "
                                            << sssl_cert);
        }

        cass_ssl_set_verify_flags(ssl.get(), CASS_SSL_VERIFY_PEER_CERT);
        cass_cluster_set_ssl(cluster_.get(), ssl.get());
    } else {
        cass_ssl_set_verify_flags(ssl.get(), CASS_SSL_VERIFY_NONE);
    }

    if (max_statement_tries) {
        int32_t max_statement_tries_number;
        try {
            max_statement_tries_number = boost::lexical_cast<int32_t>(max_statement_tries);
            if (max_statement_tries_number < 0) {
                isc_throw(DbOperationError, "CqlConnection::openDatabase(): invalid reconnect "
                                            "wait time, expected positive number, instead got "
                                                << max_statement_tries);
            }
        } catch (boost::bad_lexical_cast const& ex) {
            isc_throw(DbOperationError, "CqlConnection::openDatabase(): "
                                        "invalid reconnect wait time, expected "
                                        "castable to int, instead got \""
                                            << max_statement_tries << "\", " << ex.what());
        }
        max_statement_tries_ = max_statement_tries_number;
    }

    auto connect_future =
        MakePtr(cass_session_connect_keyspace(session_.get(), cluster_.get(), keyspace));
    Spdlog::debug("{}: Connecting to the database...", PRETTY_METHOD_NAME());
    cass_future_wait(connect_future.get());
    Spdlog::debug("{}: Connection established.", PRETTY_METHOD_NAME());
    const std::string error = checkFutureError(PRETTY_METHOD_NAME() + ": " + scontact_points +
                                                   " - cass_session_connect_keyspace() != CASS_OK",
                                               connect_future);
    rc = cass_future_error_code(connect_future.get());
    if (rc != CASS_OK) {
        isc_throw(DbOpenError, error);
    }
}

void CqlConnection::prepareStatements(StatementMap& statements) {
    for (auto& [statement_name, tagged_statement] : statements) {
        if (statements_.find(statement_name) != statements_.end()) {
            isc_throw(DbOperationError, "CqlConnection::prepareStatements(): "
                                        "duplicate statement with name "
                                            << statement_name);
        }

        CassFuturePtr future(
            MakePtr(cass_session_prepare(session_.get(), tagged_statement.text_.c_str())));
        cass_future_wait(future.get());
        std::string const& error(checkFutureError("CqlConnection::prepareStatements():"
                                                  " cass_session_prepare() != CASS_OK",
                                                  future, statement_name));
        CassError const& return_code(cass_future_error_code(future.get()));
        if (return_code != CASS_OK) {
            isc_throw(DbOperationError, error);
        }

        tagged_statement.prepared_statement_ = MakePtr(cass_future_get_prepared(future.get()));
        statements_.try_emplace(statement_name, tagged_statement);
    }
}

void CqlConnection::setConsistency(bool force,
                                   CassConsistency consistency,
                                   CassConsistency serial_consistency) {
    force_consistency_ = force;
    consistency_ = consistency;
    serial_consistency_ = serial_consistency;
}

void CqlConnection::startTransaction() {
    DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_CONNECTION_BEGIN_TRANSACTION);
#ifdef TERASTREAM_FULL_TRANSACTIONS
    bool result = CqlTransactionMgr::instance().startTransaction(this);
    if (!result) {
        isc_throw(DbOperationError, "unable to start transaction");
    }
#endif  // TERASTREAM_FULL_TRANSACTIONS
}

void CqlConnection::commit() {
    DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_CONNECTION_COMMIT);
#ifdef TERASTREAM_FULL_TRANSACTIONS
    bool result = CqlTransactionMgr::instance().commit(this);
    if (!result) {
        isc_throw(DbOperationError, "commit failed");
    }
#endif  // TERASTREAM_FULL_TRANSACTIONS
}

void CqlConnection::rollback() {
    DB_LOG_DEBUG(DB_DBG_TRACE_DETAIL, CQL_CONNECTION_ROLLBACK);
#ifdef TERASTREAM_FULL_TRANSACTIONS
    bool result = CqlTransactionMgr::instance().rollback(this);
    if (!result) {
        isc_throw(DbOperationError, "rollback failed");
    }
#endif  // TERASTREAM_FULL_TRANSACTIONS
}

#ifdef TERASTREAM_FULL_TRANSACTIONS
void CqlConnection::setTransactionOperations(StatementTag const& begin_index,
                                             StatementTag const& commit_index,
                                             StatementTag const& rollback_index) {
    begin_index_ = begin_index;
    commit_index_ = commit_index;
    rollback_index_ = rollback_index;
}

CassUuid CqlConnection::getTransactionID() const {
    return CqlTransactionMgr::instance().getTransactionID(this);
}
#endif  // TERASTREAM_FULL_TRANSACTIONS

std::string const CqlConnection::checkFutureError(std::string const& what,
                                                  CassFuturePtr& future,
                                                  StatementTag const& statement_tag /* = NULL */) {
    CassError cass_error = cass_future_error_code(future.get());
    char const* error_message;
    size_t error_message_size;
    cass_future_error_message(future.get(), &error_message, &error_message_size);

    std::stringstream stream;
    if (statement_tag.length() > 0) {
        // future is from cass_session_execute() call.
        stream << "Statement ";
        stream << statement_tag;
    } else {
        // future is from cass_session_*() call.
        stream << "Session action";
    }
    if (cass_error == CASS_OK) {
        stream << " executed successfully.";
    } else {
        stream << " failed, Kea error: " << what
               << ", Cassandra error code: " << cass_error_desc(cass_error)
               << ", Cassandra future error: " << error_message;
    }
    return stream.str();
}

}  // namespace db
}  // namespace isc

// (C) 2020 Deutsche Telekom AG.
//
// Deutsche Telekom AG and all other contributors /
// copyright owners license this file to you under the Apache
// License, Version 2.0 (the "License"); you may not use this
// file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the License for the
// specific language governing permissions and limitations
// under the License.

#include <config.h>

#include <cql/cql_memory_mgr.h>
#include <database/common.h>

namespace isc {
namespace db {

template <>
void Deleter<CassCluster>::operator()(CassCluster* c) {
    if (c) {
        cass_cluster_free(c);
    }
}

template <>
void Deleter<CassFuture>::operator()(CassFuture* f) {
    if (f) {
        cass_future_free(f);
    }
}

template <>
void Deleter<CassIterator>::operator()(CassIterator* i) {
    if (i) {
        cass_iterator_free(i);
    }
}

template <>
void Deleter<CassPrepared const>::operator()(CassPrepared const* p) {
    if (p) {
        cass_prepared_free(p);
    }
}

template <>
void Deleter<CassResult const>::operator()(CassResult const* r) {
    if (r) {
        cass_result_free(r);
    }
}

template <>
void Deleter<CassStatement>::operator()(CassStatement* s) {
    if (s) {
        cass_statement_free(s);
    }
}

template <>
void Deleter<CassSession>::operator()(CassSession* s) {
    if (s) {
        CassFuturePtr f(MakePtr(cass_session_close(s)));
        cass_future_wait(f.get());
        cass_session_free(s);
    }
}

template <>
void Deleter<CassSsl>::operator()(CassSsl* s) {
    if (s) {
        cass_ssl_free(s);
    }
}

template <>
void Deleter<CassUuidGen>::operator()(CassUuidGen* g) {
    if (g) {
        cass_uuid_gen_free(g);
    }
}

}  // namespace db
}  // namespace isc

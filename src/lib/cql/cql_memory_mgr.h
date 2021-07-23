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

#ifndef CQL_MEMORY_MGR_H
#define CQL_MEMORY_MGR_H

#include <cassandra.h>

#include <database/common.h>

#include <memory>

namespace isc {
namespace db {

using CassClusterPtr = Ptr<CassCluster>;
using CassFuturePtr = Ptr<CassFuture>;
using CassIteratorPtr = Ptr<CassIterator>;
using CassPreparedPtr = Ptr<CassPrepared const>;
using CassResultPtr = Ptr<CassResult const>;
using CassStatementPtr = Ptr<CassStatement>;
using CassSessionPtr = Ptr<CassSession>;
using CassSslPtr = Ptr<CassSsl>;
using CassUuidGenPtr = Ptr<CassUuidGen>;

template <>
void Deleter<CassCluster>::operator()(CassCluster* c);

template <>
void Deleter<CassFuture>::operator()(CassFuture* f);

template <>
void Deleter<CassIterator>::operator()(CassIterator* i);

template <>
void Deleter<CassPrepared const>::operator()(CassPrepared const* p);

template <>
void Deleter<CassResult const>::operator()(CassResult const* r);

template <>
void Deleter<CassStatement>::operator()(CassStatement* s);

template <>
void Deleter<CassSession>::operator()(CassSession* s);

template <>
void Deleter<CassSsl>::operator()(CassSsl* s);

template <>
void Deleter<CassUuidGen>::operator()(CassUuidGen* g);

}  // namespace db
}  // namespace isc

#endif  // CQL_MEMORY_MGR_H

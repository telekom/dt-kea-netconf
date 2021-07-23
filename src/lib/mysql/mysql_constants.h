// Copyright (C) 2018-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MYSQL_CONSTANTS_H
#define MYSQL_CONSTANTS_H

#include <mysql.h>

namespace isc {
namespace db {

/// @name MySQL constants.
///
//@{
/// @brief my_bools type for vectors.
/// @note vector<bool> is specialized into a bitset, so vector<char>
/// must be used instead
typedef char my_bools;

#ifdef HAVE_MYSQL_MY_BOOL
using my_bool = my_bool;

/// @brief MySQL false value.
const my_bool MLM_FALSE = 0;

/// @brief MySQL true value.
const my_bool MLM_TRUE = 1;

#else  // HAVE_MYSQL_MY_BOOL
/// @brief my_bool type in MySQL 8.x.
using my_bool = bool;

/// @brief MySQL false value.
const my_bool MLM_FALSE = false;

/// @brief MySQL true value.
const my_bool MLM_TRUE = true;
#endif  // HAVE_MYSQL_MY_BOOL

///@brief check for bool size
static_assert(sizeof(my_bool) == sizeof(char), "unsupported bool size");

/// @brief MySQL fetch success code.
const int MLM_MYSQL_FETCH_SUCCESS = 0;

/// @brief MySQL fetch failure code.
const int MLM_MYSQL_FETCH_FAILURE = 0;
//@}

/// @name Current database schema version values.
/// @{
const uint32_t MYSQL_SCHEMA_VERSION_MAJOR = 10;
const uint32_t MYSQL_SCHEMA_VERSION_MINOR = 0;

/// @brief Define MySQL backend versions.
/// @{
uint32_t constexpr MYSQL_CONFIG_SCHEMA_VERSION_MAJOR = 1;
uint32_t constexpr MYSQL_CONFIG_SCHEMA_VERSION_MINOR = 0;
/// @}


}  // namespace db
}  // end of namespace isc

#endif

// Copyright (C) 2016 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef HOOKS_LIBINFO_H
#define HOOKS_LIBINFO_H

#include <cc/data.h>

#include <string>
#include <utility>
#include <vector>

namespace isc {
namespace hooks {

/// @brief Entity that holds information about hook libraries and their
/// parameters.
///
/// The first parameter is a full filename with path to the library.
/// The second parameter is a map of parameters that configure the
/// library. There's always at least one parameter: "library", which
/// contains the library name.
enum HookLibInfoType { LIBRARY = 0, PARAMETERS_KEY = 1, PARAMETERS_VALUE = 2 };
typedef std::tuple<std::string, std::string, data::ElementPtr> HookLibInfo;

/// @brief A storage for information about hook libraries.
typedef std::vector<HookLibInfo> HookLibsCollection;

/// @brief Shared pointer to collection of hooks libraries.
typedef std::shared_ptr<HookLibsCollection> HookLibsCollectionPtr;

/// @brief Extracts library names from full library information structure
std::vector<std::string> extractNames(const HookLibsCollection& libinfo);

}  // namespace hooks
}  // namespace isc

#endif

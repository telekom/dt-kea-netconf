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

#ifndef NETCONF_CHANGES_H
#define NETCONF_CHANGES_H

#include <util/func.h>
#include <util/magic_enum.hpp>

#include <sysrepo-cpp/Session.hpp>

namespace isc {
namespace netconf {

struct ExtendedVal {
    ExtendedVal(sysrepo::S_Val const& value) : is_leaf_list_(false), value_(value) {
    }

    ExtendedVal(char const* const value,
                sr_type_t const type,
                std::string const& xpath,
                bool const is_leaf_list)
        : is_leaf_list_(is_leaf_list), value_(std::make_shared<sysrepo::Val>(value, type)) {
        value_->xpath_set(xpath.c_str());
    }

    ExtendedVal(libyang::S_Data_Node const& node,
                sr_type_t const type,
                std::string const& xpath,
                bool const is_leaf_or_leaf_list,
                bool const is_leaf_list)
        : is_leaf_list_(is_leaf_list) {
        if (is_leaf_or_leaf_list) {
#ifdef USE_LIBYANG_C_API_TO_DETECT_LEAF_LISTS
            value_ = std::make_shared<sysrepo::Val>(
                ((lyd_node_leaf_list*)node->C_lyd_node())->value_str, type);
#else  // USE_LIBYANG_C_API_TO_DETECT_LEAF_LISTS
            value_ = std::make_shared<sysrepo::Val>(libyang::Data_Node_Leaf_List(node).value_str(),
                                                    type);
#endif  // USE_LIBYANG_C_API_TO_DETECT_LEAF_LISTS
            value_->xpath_set(xpath.c_str());
        }
    }

    bool is_leaf_list_;
    sysrepo::S_Val value_;
};

using S_ExtendedVal = std::shared_ptr<ExtendedVal>;

// Global type declarations
#ifdef USE_TREE_CHANGES_TO_DETECT_LEAF_LISTS
using Change = sysrepo::S_Tree_Change;
#else  // USE_TREE_CHANGES_TO_DETECT_LEAF_LISTS
using Change = sysrepo::S_Change;
#endif  // USE_TREE_CHANGES_TO_DETECT_LEAF_LISTS
using ChangeCollection = std::vector<Change>;

template <typename change_t>
struct ChangesT {
    using change_collection_t = std::vector<change_t>;

    void clear() {
        changes_.clear();
    }

    bool empty() {
        return changes_.empty();
    }

    change_collection_t& get() {
        return changes_;
    }

    void add(change_t const& change) {
        changes_.push_back(change);
    }

    static sr_type_t convert(LY_DATA_TYPE const type) {
        switch (type) {
        case LY_TYPE_BINARY:
            return SR_BINARY_T;
        case LY_TYPE_BITS:
            return SR_BITS_T;
        case LY_TYPE_BOOL:
            return SR_BOOL_T;
        case LY_TYPE_DEC64:
            return SR_DECIMAL64_T;
        case LY_TYPE_EMPTY:
            return SR_LEAF_EMPTY_T;
        case LY_TYPE_ENUM:
            return SR_ENUM_T;
        case LY_TYPE_IDENT:
            return SR_IDENTITYREF_T;
        case LY_TYPE_INST:
            return SR_INSTANCEID_T;
        case LY_TYPE_STRING:
            return SR_STRING_T;
        case LY_TYPE_INT8:
            return SR_INT8_T;
        case LY_TYPE_UINT8:
            return SR_UINT8_T;
        case LY_TYPE_INT16:
            return SR_INT16_T;
        case LY_TYPE_UINT16:
            return SR_UINT16_T;
        case LY_TYPE_INT32:
            return SR_INT32_T;
        case LY_TYPE_UINT32:
            return SR_UINT32_T;
        case LY_TYPE_INT64:
            return SR_INT64_T;
        case LY_TYPE_UINT64:
            return SR_UINT64_T;
        case LY_TYPE_UNKNOWN:
            return SR_UNKNOWN_T;
        case LY_TYPE_DER:
            [[fallthrough]];
        case LY_TYPE_LEAFREF:
            [[fallthrough]];
        case LY_TYPE_UNION:
            [[fallthrough]];
        default:
            isc_throw(BadValue, PRETTY_FUNCTION_NAME() << ": " << magic_enum::enum_name(type));
        }

        isc_throw(BadValue, PRETTY_FUNCTION_NAME() << ": " << magic_enum::enum_name(type));
    }

    static constexpr sr_type_t convert(LYD_ANYDATA_VALUETYPE const /* type */) {
        return SR_UNKNOWN_T;
    }

    static change_t next_change(sysrepo::S_Session session, sysrepo::S_Iter_Change iterator);
    static S_ExtendedVal newValue(change_t const& change);
    static S_ExtendedVal oldValue(change_t const& change);
    static sr_type_t type(change_t const& change, bool const is_leaf_list);
    static std::string xpath(change_t const& change);

private:
    change_collection_t changes_;
};

// Global type declarations
using Changes = ChangesT<Change>;
using ChangesPtr = std::shared_ptr<Changes>;

// Viable specializations
using SysrepoChanges = ChangesT<sysrepo::S_Change>;
using TreeChanges = ChangesT<sysrepo::S_Tree_Change>;

}  // namespace netconf
}  // namespace isc

#endif  // NETCONF_CHANGES_H

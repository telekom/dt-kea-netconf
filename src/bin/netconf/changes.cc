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

#include <exceptions/exceptions.h>
#include <netconf/changes.h>
#include <util/log.h>
#include <yang/translator.h>

using std::string;

using sysrepo::S_Change;
using sysrepo::S_Iter_Change;
using sysrepo::S_Session;
using sysrepo::S_Tree_Change;
using sysrepo::S_Val;
using sysrepo::Val;

using isc::log::Spdlog;

namespace isc {
namespace netconf {

/// @{
/// @brief First wave of specializations
template <>
sr_type_t SysrepoChanges::type(S_Change const& change, bool const /* is_leaf_list */) {
    sr_change_oper_t const operation(change->oper());
    switch (operation) {
    case SR_OP_CREATED:
        [[fallthrough]];
    case SR_OP_MODIFIED:
        [[fallthrough]];
    case SR_OP_MOVED:
        return change->new_val()->type();
    case SR_OP_DELETED:
        return change->old_val()->type();
    default:
        isc_throw(Unexpected, PRETTY_FUNCTION_NAME() << ": stumbled upon unhandled operation "
                                                     << magic_enum::enum_name(operation));
    }
}

template <>
sr_type_t TreeChanges::type(S_Tree_Change const& change, bool const is_leaf_or_leaf_list) {
    if (is_leaf_or_leaf_list) {
#ifdef USE_LIBYANG_C_API_TO_DETECT_LEAF_LISTS
        return convert(((lyd_node_leaf_list*)change->node()->C_lyd_node())->value_type);
#else  // USE_LIBYANG_C_API_TO_DETECT_LEAF_LISTS
        return convert(libyang::Data_Node_Leaf_List(change->node()).value_type());
#endif  // USE_LIBYANG_C_API_TO_DETECT_LEAF_LISTS
    }
    return SR_UNKNOWN_T;
}

template <>
string SysrepoChanges::xpath(S_Change const& change) {
    sr_change_oper_t const operation(change->oper());
    switch (operation) {
    case SR_OP_CREATED:
        [[fallthrough]];
    case SR_OP_MODIFIED:
        [[fallthrough]];
    case SR_OP_MOVED:
        return change->new_val()->xpath();
    case SR_OP_DELETED:
        return change->old_val()->xpath();
    default:
        isc_throw(Unexpected, PRETTY_FUNCTION_NAME() << ": stumbled upon unhandled operation "
                                                     << magic_enum::enum_name(operation));
    }
}

template <>
string TreeChanges::xpath(S_Tree_Change const& change) {
    if (!change->node()) {
        isc_throw(Unexpected, PRETTY_FUNCTION_NAME() << ": !change->node()");
    }
    return change->node()->path();
}
/// @}

template <>
S_Change SysrepoChanges::next_change(S_Session session, S_Iter_Change iterator) {
    return session->get_change_next(iterator);
}

template <>
S_Tree_Change TreeChanges::next_change(S_Session session, S_Iter_Change iterator) {
    return session->get_change_tree_next(iterator);
}

template <>
S_ExtendedVal SysrepoChanges::newValue(S_Change const& change) {
    return make_shared<ExtendedVal>(change->new_val());
}

template <>
S_ExtendedVal TreeChanges::newValue(S_Tree_Change const& change) {
    if (!change->node()) {
        Spdlog::debug("{}: !change->node()", PRETTY_FUNCTION_NAME());
        return nullptr;
    }
    if (!change->node()->schema()) {
        Spdlog::debug("{}: !change->node()->schema()", PRETTY_FUNCTION_NAME());
        return nullptr;
    }
    if (!change->node()->schema()->nodetype()) {
        Spdlog::debug("{}: !change->node()->schema()->nodetype()", PRETTY_FUNCTION_NAME());
        return nullptr;
    }
    bool const is_leaf_or_leaf_list(change->node()->schema()->nodetype() == LYS_LEAF ||
                                    change->node()->schema()->nodetype() == LYS_LEAFLIST);
    bool const is_leaf_list(change->node()->schema()->nodetype() == LYS_LEAFLIST);
    try {
        return make_shared<ExtendedVal>(change->node(), type(change, is_leaf_or_leaf_list),
                                        xpath(change), is_leaf_or_leaf_list, is_leaf_list);
    } catch (BadValue const&) {
        return nullptr;
    }
}

template <>
S_ExtendedVal SysrepoChanges::oldValue(S_Change const& change) {
    return make_shared<ExtendedVal>(change->old_val());
}

template <>
S_ExtendedVal TreeChanges::oldValue(S_Tree_Change const& change) {
    if (!change->prev_value()) {
        Spdlog::debug("{}: !change->prev_value()", PRETTY_FUNCTION_NAME());
        return nullptr;
    }
    if (!change->node()) {
        Spdlog::debug("{}: !change->node()", PRETTY_FUNCTION_NAME());
        return nullptr;
    }
    if (!change->node()->schema()) {
        Spdlog::debug("{}: !change->node()->schema()", PRETTY_FUNCTION_NAME());
        return nullptr;
    }
    if (!change->node()->schema()->nodetype()) {
        Spdlog::debug("{}: !change->node()->schema()->nodetype()", PRETTY_FUNCTION_NAME());
        return nullptr;
    }
    bool const is_leaf_or_leaf_list(change->node()->schema()->nodetype() == LYS_LEAF ||
                                    change->node()->schema()->nodetype() == LYS_LEAFLIST);
    bool const is_leaf_list(change->node()->schema()->nodetype() == LYS_LEAFLIST);
    try {
        return make_shared<ExtendedVal>(change->prev_value(), type(change, is_leaf_or_leaf_list),
                                        xpath(change), is_leaf_list);
    } catch (BadValue const&) {
        return nullptr;
    }
}

}  // namespace netconf
}  // namespace isc

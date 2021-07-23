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

#include <util/func.h>
#include <util/log.h>
#include <yang/translator_universal.h>

using isc::data::Element;
using isc::data::ElementPtr;
using isc::data::MapElement;
using isc::log::Spdlog;
using std::string;

namespace isc {
namespace yang {

void TranslatorUniversal::del(string const& xpath /* = string() */) {
    if (!xpath.empty()) {
        session_->delete_item(xpath.c_str());
        return;
    }

    for (string const& root_node : rootNodes().at(model_)) {
        string const& path("/" + model_ + ":" + root_node);
        session_->delete_item(path.c_str());
    }
}

ElementPtr TranslatorUniversal::get(string const& xpath /* = string() */) {
    ElementPtr result(Element::createMap());

    if (xpath.empty()) {
        // For each node...
        for (string const& root_node : rootNodes().at(model_)) {
            string const path("/" + model_ + ":" + root_node);
            getInternal(result, path);
        }
    } else {
        getInternal(result, xpath);
    }

    // Remove empty lists and maps resulted from non-existing, but defaulted
    // nodes since the JSON parser complains about them and they don't hold any
    // configuration/functionality.
    result->removeEmptyRecursively();

    return result;
}

void TranslatorUniversal::set(ElementPtr const& config, string const& xpath /* = string() */) {
    if (!xpath.empty()) {
        config->xpath_ = xpath;
        setInternal(config, getRootNodeFromXpath(xpath));
        return;
    }

    config->xpath_ = "/" + model_ + ":";
    for (string const& root_node : rootNodes().at(model_)) {
        ElementPtr const& toplevel(config->get(root_node));
        if (!toplevel) {
            continue;
        }
        setInternal(toplevel, root_node);
    }
}

void TranslatorUniversal::getInternal(ElementPtr const& result, string const& xpath) {
    S_Data_Node toplevel(session_->get_data(xpath.c_str()));

    for (libyang::S_Data_Node& root : toplevel->tree_for()) {
        for (S_Data_Node const& n : root->tree_dfs()) {
            // Set it into result at it's reported xpath in Sysrepo.
            nodeIntoResult(result, n);
        }
    }
}

ElementPtr TranslatorUniversal::getLeafOrLeafList(S_Data_Node const& node) {
#ifdef USE_LIBYANG_C_API_TO_DETECT_LEAF_LISTS
    lyd_node_leaf_list* n((lyd_node_leaf_list*)node->C_lyd_node());
    LY_DATA_TYPE const type(n->value_type);
    string const value(n->value_str);
#else  // USE_LIBYANG_C_API_TO_DETECT_LEAF_LISTS
    libyang::Data_Node_Leaf_List n(node);
    LY_DATA_TYPE const type(n.value_type());
    string const value(n.value_str());
#endif  // USE_LIBYANG_C_API_TO_DETECT_LEAF_LISTS
    return Element::fromJSONFallbackToString(value);
}

string TranslatorUniversal::getRootNodeFromXpath(string const& xpath) {
    size_t position(xpath.find(":"));
    if (position == string::npos) {
        return string();
    }
    string root_node(xpath.substr(position + 1));
    position = root_node.find("/");
    if (position == string::npos) {
        return root_node;
    }
    return root_node.substr(0, position);
}

void TranslatorUniversal::nodeIntoResult(ElementPtr const& result, S_Data_Node const& node) {
    ElementPtr element;
    lys_nodetype const node_type(node->schema()->nodetype());
    string const& xpath(node->path());
    switch (node_type) {
    case LYS_LEAF:
        [[fallthrough]];
    case LYS_LEAFLIST:
        element = getLeafOrLeafList(node);

        // Log.
        Spdlog::debug("{}: {} {}", PRETTY_METHOD_NAME(), xpath, element->str());

        break;
    case LYS_CONTAINER:
        element = Element::createMap();
        break;
    case LYS_LIST:
        element = Element::createList();
        break;
    default:
        isc_throw(BadValue, PRETTY_METHOD_NAME() << ": " << magic_enum::enum_name(node_type));
        break;
    }
    if (element) {
        result->xpath(xpath, element, node_type == LYS_LEAFLIST);
    }
}

void TranslatorUniversal::setInternal(ElementPtr const& toplevel, string const& root_node) {
    populateXpaths(toplevel, root_node);

    // For itself and all it's descendants...
    Element::forEach(toplevel, [&](ElementPtr const& i) {
        // If it is a leaf...
        if (i->getType() != Element::list && i->getType() != Element::map) {
            // Get unquoted value.
            string const unquoted_value(i->toUnquotedString());

            // Log.
            Spdlog::debug("{}: {} {}", PRETTY_METHOD_NAME(), i->xpath_, unquoted_value);

            // Set it in Sysrepo.
            session_->set_item_str(i->xpath_.c_str(), unquoted_value.c_str());
        }
    });
}

}  // namespace yang
}  // namespace isc

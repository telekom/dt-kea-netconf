// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef ISC_TRANSLATOR_H
#define ISC_TRANSLATOR_H 1

#include <cc/data.h>
#include <spdlog/spdlog.h>
#include <util/func.h>
#include <util/log.h>
#include <util/strutil.h>
#include <yang/sysrepo_error.h>
#include <yang/yang_models.h>

#include <sysrepo-cpp/Session.hpp>

#include <set>

namespace isc {
namespace yang {

/// @brief Between YANG and JSON translator class for basic values.
struct TranslatorBasic {
    using Data_Node = libyang::Data_Node;
    using S_Connection = sysrepo::S_Connection;
    using S_Context = libyang::S_Context;
    using S_Data_Node = libyang::S_Data_Node;
    using S_Module = libyang::S_Module;
    using S_Schema_Node = libyang::S_Schema_Node;
    using S_Schema_Node_Leaf = libyang::S_Schema_Node_Leaf;
    using S_Schema_Node_List = libyang::S_Schema_Node_List;
    using S_Session = sysrepo::S_Session;
    using S_Set = libyang::S_Set;
    using S_Subscribe = sysrepo::S_Subscribe;
    using S_Val = sysrepo::S_Val;
    using S_Vals = sysrepo::S_Vals;
    using Schema_Node_Leaf = libyang::Schema_Node_Leaf;
    using Schema_Node_List = libyang::Schema_Node_List;

    /// @brief Constructor.
    ///
    /// @param session Sysrepo session.
    /// @param model Model name (used and shared by derived classes).
    TranslatorBasic(S_Session session, std::string const& model)
        : session_(session), model_(model) {
    }

    /// @brief Destructor.
    virtual ~TranslatorBasic() = default;

    void clearSkipValidation() {
        for (std::string const& root_node : rootNodes().at(model_)) {
            delItem("/" + model_ + ":" + root_node + "/skip-validation");
        }
    }

    /// @brief Translate basic value from YANG to JSON.
    ///
    /// @note Please don't use this outside tests.
    ///
    /// @param s_val The value.
    /// @return The Element representing the sysrepo value.
    /// @throw NotImplemented when the value type is not supported.
    static isc::data::ElementPtr value(S_Val s_val);

    /// @brief Get and translate basic value from YANG to JSON.
    ///
    /// @note Should be const as it is read only...
    ///
    /// @param xpath The xpath of the basic value.
    /// @return The Element representing the item at xpath or null
    /// when not found.
    /// @throw SysrepoError when sysrepo raises an error.
    /// @throw NotImplemented when the value type is not supported.
    isc::data::ElementPtr getItem(const std::string& xpath);

    /// @brief Get and translate a list of basic values from YANG to JSON.
    ///
    /// @param xpath The xpath of the list of basic values.
    /// @return The ListElement representing the leaf-list at xpath or
    /// null when not found.
    isc::data::ElementPtr getItems(const std::string& xpath);

    S_Vals getValuesFromItems(std::string const& xpath) {
        try {
            return session_->get_items(xpath.c_str());
        } catch (sysrepo::sysrepo_exception const& exception) {
            isc_throw(SysrepoError, "sysrepo error getting items: " << exception.what());
        }
    }

    template <typename T>
    isc::data::ElementPtr getList(std::string const& xpath,
                                  T& t,
                                  isc::data::ElementPtr (T::*f)(std::string const& xpath)) {
        isc::data::ElementPtr result;
        S_Vals values(getValuesFromItems(xpath));
        if (values) {
            for (size_t i(0); i < values->val_cnt(); ++i) {
                isc::data::ElementPtr x((t.*f)(values->val(i)->xpath()));
                if (x) {
                    if (!result) {
                        result = isc::data::Element::createList();
                    }
                    result->add(x);
                }
            }
        }
        return result;
    }

    /// @brief Translate basic value from JSON to YANG.
    ///
    /// @note Please don't use this outside tests.
    ///
    /// @param elem The JSON element.
    /// @param type The sysrepo type.
    static S_Val value(isc::data::ElementPtr elem, sr_type_t type);

    void setItem(std::string const& xpath, sysrepo::S_Val const& value);

    void setItem(sysrepo::S_Val const& value);

    /// @brief Translate and set basic value from JSON to YANG.
    ///
    /// @param xpath The xpath of the basic value.
    /// @param elem The JSON element.
    /// @param type The sysrepo type.
    void setItem(const std::string& xpath, isc::data::ElementPtr elem, sr_type_t type);

    void delItem(sysrepo::S_Val const& value);

    /// @brief Delete basic value from YANG.
    ///
    /// @param xpath The xpath of the basic value.
    void delItem(const std::string& xpath);

    /// @brief Retrieves an item and stores in the specified storage.
    ///
    /// This will attempt to retrieve an item and, if exists, will
    /// store it in the storage.
    ///
    /// @param storage ElementMap (result will be stored here)
    /// @param xpath xpath location (data will be extracted from sysrepo)
    /// @param name name of the parameter
    void checkAndGetLeaf(isc::data::ElementPtr const& storage,
                         const std::string& xpath,
                         const std::string& name);

    void checkAndGetLeafList(isc::data::ElementPtr const& storage,
                             const std::string& xpath,
                             const std::string& name);

    template <typename T>
    void checkAndGet(isc::data::ElementPtr const& to,
                     std::string const& xpath,
                     std::string const& name,
                     T& t,
                     isc::data::ElementPtr (T::*f)(std::string const& xpath)) {
        isc::data::ElementPtr const& x((t.*f)(xpath + "/" + name));
        if (x) {
            to->set(name, x);
        }
    }

    template <typename T>
    void checkAndGetList(isc::data::ElementPtr const& to,
                         std::string const& xpath,
                         std::string const& name,
                         T& t,
                         isc::data::ElementPtr (T::*f)(std::string const& xpath)) {
        isc::data::ElementPtr const& x(getList(xpath + "/" + name, t, f));
        if (x && !x->empty()) {
            to->set(name, x);
        }
    }

    void checkAndSetLeaf(isc::data::ElementPtr const& from,
                         std::string const& xpath,
                         std::string const& name,
                         sr_type_t const& type);

    void checkAndSyncLeaf(isc::data::ElementPtr const& from,
                          std::string const& xpath,
                          std::string const& name,
                          sr_type_t const& type);

    void checkAndSetLeafList(isc::data::ElementPtr const& from,
                             std::string const& xpath,
                             std::string const& name,
                             sr_type_t const& type);

    template <typename T>
    void
    checkAndSet(isc::data::ElementPtr const& from,
                std::string const& xpath,
                std::string const& name,
                T& t,
                void (T::*f)(std::string const& xpath, isc::data::ElementPtr elem, bool skip)) {
        isc::data::ElementPtr const& x(from->get(name));
        if (x) {
            (t.*f)(xpath + "/" + name, x, false);
        }
    }

    template <typename T>
    void
    checkAndSetList(isc::data::ElementPtr const& from,
                    std::string const& xpath,
                    std::string const& name,
                    std::string const& key,
                    T& t,
                    void (T::*f)(std::string const& xpath, isc::data::ElementPtr const& elem, bool const skip),
                    bool const force_creation = false) {
        isc::data::ElementPtr const& list(from->get(name));
        if (!list) {
            return;
        }

        std::vector<std::string> keys;
        std::string mutable_key(key);
        while (true) {
            std::size_t const current(mutable_key.find(' '));
            keys.push_back(mutable_key.substr(0, current));
            if (current == std::string::npos) {
                break;
            } else {
                mutable_key = mutable_key.substr(current + 1);
            }
        }

        for (isc::data::ElementPtr const& i : list->listValue()) {
            std::string xxpath(xpath + "/" + name);
            for (std::string const& kk : keys) {
                isc::data::ElementPtr const& k(i->get(kk));
                if (k) {
                    std::string const key_string(k->toUnquotedString());
                    xxpath += "[" + kk + "='" + key_string + "']";
                }
            }

            if (force_creation) {
                // Force creation for nodes that don't have any mandatory fields outside the key.
                setItem(xxpath, isc::data::Element::createMap(), SR_CONTAINER_T);
            } else {
                (t.*f)(xxpath, i, false);
            }
        }
    }

    /// @brief Use in @ref checkAndSetList() to force creation of containers.
    static bool constexpr FORCE_CREATION = true;

    /// @brief Get the root node of a module.
    static std::unordered_map<std::string, std::vector<std::string>> const& rootNodes() {
        static std::unordered_map<std::string, std::vector<std::string>> const _{
            {IETF_DHCPV6_SERVER, {"dhcpv6-server"}},
            {KEA_CTRL_AGENT, {"config"}},
            {KEA_DHCP_DDNS, {"config"}},
            {KEA_DHCP4_SERVER, {"Dhcp4"}},
            {KEA_DHCP6_SERVER, {"Dhcp6"}},
            {KEATEST_MODULE, {"container", "main", "kernel-modules", "presence-container"}},
        };
        return _;
    }

    /// @brief Populate xpaths for all descending elements.
    void populateXpaths(isc::data::ElementPtr const& root_node, std::string const& root_node_name) {
        // First, empty xpath for all elements.
        isc::data::Element::forEach(
            root_node, [](isc::data::ElementPtr const& i) { i->xpath_ = std::string(); });

        root_node->xpath_ = root_node->parent_->xpath_ + root_node_name;
        for (auto const& [key, value] : root_node->mapValue()) {
            value->xpath_ = root_node->xpath_ + "/" + key;
        }

        isc::data::Element::forEach(root_node, [&](isc::data::ElementPtr const& i) {
            reassessChildren(i);
            if (i->parent_) {
                if (i->parent_->getType() == isc::data::Element::list) {
                    dynamic_pointer_cast<isc::data::ListElement>(i->parent_)->setXpathToChild(i);
                } else if (i->parent_->getType() == isc::data::Element::map) {
                    dynamic_pointer_cast<isc::data::MapElement>(i->parent_)
                        ->setXpathFromParentAndChild(
                            std::bind(&TranslatorBasic::keys, this, std::placeholders::_1));
                }
            }
        });
    }

    /// @brief Set all direct children again in order to reset their parent.
    static void reassessChildren(isc::data::ElementPtr const& parent) {
        if (parent->getType() == isc::data::Element::list) {
            for (size_t i = 0; i < parent->size(); ++i) {
                isc::data::ElementPtr const& child(parent->get(i));
                parent->set(i, child);
            }
        } else if (parent->getType() == isc::data::Element::map) {
            for (size_t i = 0; i < parent->size(); ++i) {
                std::string const& key(parent->get(i)->stringValue());
                isc::data::ElementPtr const& child(parent->get(key));
                parent->set(key, child);
            }
        }
    }

    /// @brief Check if a certain xpath is a key to it's list parent. Uses @ref findPath() to
    /// actually check if the xpath is valid rather than pertaining to a key node. Hacky because it
    /// catches a pretty critical exception which could have resulted from some other erroneous
    /// behaviour, but it empirically works.
    bool isKey(std::string const& xpath) {
        S_Set set;

        set = findPath(xpath);
        if (!set) {
            return false;
        }

        for (S_Schema_Node const& schema_node : set->schema()) {
            if (schema_node->nodetype() != LYS_LEAF) {
                continue;
            }
            Schema_Node_Leaf schema_node_leaf(schema_node);
            return schema_node_leaf.is_key() != nullptr;
        }

        return false;
    }

    /// @brief Return a list of keys for a certain list node found at given xpath.
    std::vector<std::string> keys(std::string const& xpath) {
        std::vector<std::string> result;

        S_Set const set(findPath(xpath));
        if (!set) {
            return result;
        }

        for (S_Schema_Node const& schema_node : set->schema()) {
            if (schema_node->nodetype() != LYS_LIST) {
                continue;
            }
            Schema_Node_List schema_node_list(schema_node);
            for (S_Schema_Node_Leaf const& key_node : schema_node_list.keys()) {
                if (key_node->nodetype() != LYS_LEAF) {
                    continue;
                }
                result.push_back(key_node->name());
            }
        }

        return result;
    }

private:
    /// @brief Return schema node for given xpath.
    S_Set findPath(std::string const& xpath) const {
        S_Context const context(session_->get_context());
        S_Module const module(context->get_module(model_.c_str()));
        S_Schema_Node const schema_node(module->data());

        // Data_Node::find_path() throws std::runtime_error exception if xpath is invalid.
        try {
            return schema_node->find_path(xpath.c_str());
        } catch (std::runtime_error const& exception) {
            isc::log::Spdlog::debug("{}: {}", PRETTY_METHOD_NAME(), exception.what());
            return S_Set();
        }
    }

protected:
    /// @brief the Sysrepo session
    S_Session session_;

public:
    /// @brief the Sysrepo model
    std::string model_;
};

struct ElementCatalogue;

struct ElementCatalogue {
    static ElementCatalogue& instance() {
        static ElementCatalogue _;
        return _;
    }

    void clear() {
        container_.clear();
    }

    bool contains(std::string const& criteria, std::string const& key) {
        return container_[criteria].contains(key);
    }

    void exceptions() {
        std::stringstream s;
        s << PRETTY_METHOD_NAME() << ":" << std::endl;
        for (auto const& [criteria, exceptions] : container_) {
            for (auto const& [key, exception] : exceptions) {
                if (exception) {
                    s << exception->what() << std::endl;
                }
            }
        }
        isc_throw(BadValue, s.str());
    }

    void insert(std::string const& criteria,
                std::string const& key,
                std::shared_ptr<isc::Exception> const& exception) {
        spdlog::warn("{}: {}", PRETTY_METHOD_NAME(), exception->what());
        container_[criteria].insert_or_assign(key, exception);
    }

    std::shared_ptr<isc::Exception> exception(std::string const& criteria, std::string const& key) {
        if (contains(criteria, key)) {
            return container_[criteria][key];
        }
        return nullptr;
    }

private:
    ElementCatalogue() = default;

    /// @brief non-copyable
    /// @{
    ElementCatalogue(ElementCatalogue const&) = delete;
    ElementCatalogue& operator=(ElementCatalogue const&) = delete;
    /// @}

    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::shared_ptr<isc::Exception>>>
        container_;
};

}  // namespace yang
}  // namespace isc

#endif  // ISC_TRANSLATOR_H

// Copyright (C) 2010-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <cc/data.h>

#include <fstream>
#include <iomanip>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include <util/func.h>
#include <util/strutil.h>

using isc::util::str::endsWithOrAdd;

namespace {

const char* const WHITESPACE = " \b\f\n\r\t";

}  // namespace

namespace isc {
namespace data {

std::string Element::Position::str() const {
    std::ostringstream ss;
    ss << file_ << ":" << line_ << ":" << pos_;
    return (ss.str());
}

std::ostream& operator<<(std::ostream& out, const Element::Position& pos) {
    out << pos.str();
    return (out);
}

std::string Element::str() const {
    std::stringstream ss;
    toJSON(ss);
    return (ss.str());
}

std::string Element::toUnquotedString() const {
    std::string unquoted_value;
    if (getType() == isc::data::Element::string) {
        // Avoid quotes.
        unquoted_value = stringValue();
    } else {
        unquoted_value = str();
    }
    return unquoted_value;
}

std::string Element::toWire() const {
    std::stringstream ss;
    toJSON(ss);
    return (ss.str());
}

void Element::toWire(std::ostream& ss) const {
    toJSON(ss);
}

bool Element::getValue(int64_t&) const {
    return (false);
}

bool Element::getValue(double&) const {
    return (false);
}

bool Element::getValue(bool&) const {
    return (false);
}

bool Element::getValue(std::string&) const {
    return (false);
}

bool Element::getValue(std::vector<ElementPtr>&) const {
    return (false);
}

bool Element::getValue(std::map<std::string, ElementPtr>&) const {
    return (false);
}

bool Element::setValue(const long long int) {
    return (false);
}

bool Element::setValue(const double) {
    return (false);
}

bool Element::setValue(const bool) {
    return (false);
}

bool Element::setValue(const std::string&) {
    return (false);
}

bool Element::setValue(const std::vector<ElementPtr>&) {
    return (false);
}

bool Element::setValue(const std::map<std::string, ElementPtr>&) {
    return (false);
}

#ifndef USE_TREE_CHANGES_TO_DETECT_LEAF_LISTS
namespace {
static bool is_leaf_list_brute_force(std::string const& xpath) {
    static std::array<std::regex, 6> const leaf_list_xpaths{
        std::regex(
            "/+masters\\[.*\\]/+shards\\[.*\\]/+shard-config/+Dhcp[4|6]/+hooks-libraries\\[.*\\]/+"
            ".*parameters/+config/+classification-group/+encoded-options"),
        std::regex(
            "/+masters\\[.*\\]/+shards\\[.*\\]/+shard-config/+Dhcp[4|6]/+hooks-libraries\\[.*\\]/+"
            ".*parameters/+config/+lawful-interception-classes"),
        std::regex(
            "/+masters\\[.*\\]/+shards\\[.*\\]/+shard-config/+Dhcp[4|6]/+hooks-libraries\\[.*\\]/+"
            ".*parameters/+config/+library-classes"),
        std::regex(
            "/+masters\\[.*\\]/+shards\\[.*\\]/+shard-config/+Dhcp[4|6]/+hooks-libraries\\[.*\\]/+"
            ".*parameters/+config/+network-topology/+zones\\[.*\\]/+allocation-group/"
            "+groups\\[.*\\]/+"
            "encoded-options"),
        std::regex(
            "/+masters\\[.*\\]/+shards\\[.*\\]/+shard-config/+Dhcp[4|6]/+hooks-libraries\\[.*\\]/+"
            ".*parameters/+config/+network-topology/+zones\\[.*\\]/+routers\\[.*\\]/+ports\\[.*\\]/"
            "+"
            "user-ports\\[.*\\]/+subnets"),
        std::regex(
            "/+masters\\[.*\\]/+shards\\[.*\\]/+master-config\\[.*\\]/+server-config/+Dhcp[4|6]/+"
            "interfaces-config/+interfaces")};
    for (std::regex const& x : leaf_list_xpaths) {
        if (std::regex_search(xpath, x)) {
            return true;
        }
    }
    return false;
}
}  // namespace
#endif  // USE_TREE_CHANGES_TO_DETECT_LEAF_LISTS

size_t Element::xpath_leaf_list(std::string const& key, ElementPtr const& value) {
    // It is a leaf list.
    if (!contains(key)) {
        // The leaf list does not exist. Create it.
        set(key, Element::createList());
    }

    // The leaf list now exists. Check if the element exists.
    ElementPtr e(get(key));
    for (ElementPtr const& x : e->listValue()) {
        if (x->equals(*value)) {
            // It does. Job is done.
            return MAX;
        }
    }

    // It doesn't. Add it.
    e->add(value);
    return INSERTED;
}

/// @brief retrieves the Element found at given xpath.
ElementPtr Element::xpath(std::string const& path, bool const trim_prefixes /* = true */) {
    ElementPtr value;
    xpath(path, path, value, GET, /* is_leaf_list = */ false, trim_prefixes);
    return value;
}

/// @brief sets the Element at given xpath.
size_t Element::xpath(std::string const& path,
                      ElementPtr value,
                      bool const is_leaf_list /* = false */,
                      bool const trim_prefixes /* = true */) {
    return xpath(path, path, value, SET, is_leaf_list, trim_prefixes);
}

size_t Element::xpath(std::string path,
                      std::string const& full_xpath,
                      ElementPtr& v,
                      operation_t const& operation,
                      bool const is_leaf_list,
                      bool const trim_prefixes) {
    size_t constexpr KEY(0);
    size_t constexpr VALUE(1);

    // Remove multiple consecutive slash occurences.
    while (true) {
        std::size_t current(path.find("//"));
        if (current == std::string::npos) {
            break;
        } else {
            path = path.replace(current, 2, "/");
        }
    }

    // If xpath contains initial `/model:config`...
    size_t current;
    if (trim_prefixes) {
        current = path.find(':');
        if (current != std::string::npos) {
            std::string const removed_path(path.substr(0, current));
            std::string const candidate_path(path.substr(current + 1));
            // If not any other ':' was found i.e. inside a list predicate...
            if (removed_path.find('[') == std::string::npos &&
                std::count(removed_path.begin(), removed_path.end(), '/') < 2) {
                // Strip `model:` away and prefix a "/" so that the toplevel node is considered as
                // any other node.
                path = "/" + candidate_path;
            }
        }
    }

    // Get the next element to process.
    // For "/config/masters[name]/shards[name]/shard-config/subnet6[id]/option-data[code][space]":
    //      element == "config"
    //      next_element == "masters[name]"
    std::string element;
    size_t previous(0);
    while (true) {
        current = path.find('/', previous + 1);

        // Treat the case when the element ID contains '/'.
        size_t opening_bracket(path.find('['));
        size_t closing_bracket(path.find(']'));
        if (opening_bracket != std::string::npos && closing_bracket != std::string::npos &&
            opening_bracket < current && current < closing_bracket) {
            current = path.find(']', previous + 1);
            if (current != std::string::npos) {
                ++current;
            }
        }

        // If we are not parsing right after an ending bracket where full_xpath ends...
        if (!path.empty()) {
            // Delimit the element and update the path.
            if (current == std::string::npos) {
                element = path.substr(previous + 1);
                path = std::string();
            } else {
                element = path.substr(previous + 1, current - previous - 1);
                path = path.substr(current);
            }
        }

        // If element is empty...
        if (element.empty()) {
            // Skip however many empty elements ("///master//shard" == "/master/shard").
            previous = current;
        }

        // Else use the element.
        break;
    }

    // Check if the element to be inserted is a list or a leaf/map.
    size_t opening_parantheses_position(element.find('['));
    size_t closing_parantheses_position(element.find(']'));
    size_t equals_position(element.find('='));
    bool is_list(opening_parantheses_position != std::string::npos &&
                 closing_parantheses_position != std::string::npos &&
                 equals_position != std::string::npos);

    // Delimit the next element.
    previous = path.find('/');
    if (is_list) {
        current = path.find('/', closing_parantheses_position);
    } else if (previous != std::string::npos) {
        current = path.find('/', previous + 1);
    }
    std::string next_element;
    if (current == std::string::npos) {
        next_element = path.substr(previous + 1);
    } else {
        next_element = path.substr(previous + 1, current - previous - 1);
    }

    // If it is a list...
    if (is_list) {
        std::string const& element_name(element.substr(0, opening_parantheses_position));
        // Then gather all key-value pairs from the xpath.
        std::string stripped_element(element);
        std::vector<std::tuple<std::string, std::string>> key_value_collection;
        while (!stripped_element.empty()) {
            // Sanity check.
            if (opening_parantheses_position == std::string::npos ||
                closing_parantheses_position == std::string::npos ||
                equals_position == std::string::npos ||
                closing_parantheses_position < opening_parantheses_position ||
                equals_position < opening_parantheses_position ||
                closing_parantheses_position < equals_position) {
                isc_throw(BadValue, "ill-formed xpath element " << element);
            }

            // Add.
            std::string const& key(
                stripped_element.substr(opening_parantheses_position + 1,
                                        equals_position - opening_parantheses_position - 1));
            std::string const& value(stripped_element.substr(
                equals_position + 1, closing_parantheses_position - equals_position - 1));
            key_value_collection.push_back({key, value});

            // Advance to the next key-value pair.
            stripped_element = stripped_element.substr(closing_parantheses_position + 1);

            opening_parantheses_position = stripped_element.find('[');
            closing_parantheses_position = stripped_element.find(']');
            equals_position = stripped_element.find('=');
        }

        size_t earlier_effect(MAX);

        // If it doesn't contain the requested list child...
        if (!contains(element_name)) {
            // If it was meant to be deleted...
            if (!v) {
                // Job is done.
                return MAX;
            }

            // Otherwise create it.
            set(element_name, createList());
            earlier_effect = INSERTED;
        }

        // Get the list child.
        ElementPtr const& child(get(element_name));

        // For all the grandchildren...
        for (size_t i(0); i < child->size(); ++i) {
            // Get the grandchild.
            ElementPtr const& e(child->get(i));

            // For all the requested key-value pairs...
            bool del(false);
            bool found(true);
            for (auto const& [key, value] : key_value_collection) {
#ifdef LEAF_LISTS_HAVE_DOTTED_KEY_XPATHS
                if (key == ".") {
                    return xpath_leaf_list(element_name, v);
                }
#endif  // LEAF_LISTS_HAVE_DOTTED_KEY_XPATHS

#ifdef USE_TREE_CHANGES_TO_DETECT_LEAF_LISTS
                if (path.empty() && is_leaf_list) {
                    return xpath_leaf_list(element_name, v);
                }
#endif  // USE_TREE_CHANGES_TO_DETECT_LEAF_LISTS

                // If the grandchild doesn't contain the key...
                if (!e->contains(key)) {
                    // And if it was meant to be deleted or retrieved and if the next element is the
                    // ID...
                    if ((!v || operation == GET) && key == next_element) {
                        if (operation == GET) {
                            v = child;
                        }
                        // Then job is done.
                        return MAX;
                    }

                    // Otherwise it's not a match.
                    found = false;
                    break;
                }

                // Get the key.
                ElementPtr const& list_child(e->get(key));

                // Get the key's value.
                std::string const e_value(list_child->toUnquotedString());

                // If values are equal...
                std::string const& quoteless_value(value.substr(1, value.size() - 2));
                if (e_value == quoteless_value) {
                    // Then if it was meant to be deleted and if the next element is the key...
                    if (operation == SET && !v && key == next_element) {
                        // Mark this grandchild for deletion.
                        del = true;
                    }
                } else {
                    // Otherwise it's not a match.
                    found = false;
                    break;
                }
            }

            // If it's a match.
            if (found) {
                if (del) {
                    // Make child remove grandchild.
                    child->remove(i);
                    if (child->empty()) {
                        remove(element_name);
                    }
                    return DELETED;
                }

                if (operation == GET && next_element.empty()) {
                    v = e;
                    return MAX;
                }

                // If this is the end of the xpath...
                if (path.empty()) {
                    // Update the value. Since it's a list this means remove + add.
                    child->remove(i);
                    child->add(v);
                    return MODIFIED;
                }

                // Recurse.
                size_t const effect(e->xpath(path, full_xpath, v, operation, is_leaf_list,
                                             /* trim_prefixes = */ false));

                // When returning from recursion...

                // If we are in creation or retrieval mode...
                if (v || operation == GET) {
                    // Job is done.
                    return effect;
                }

                // Check for empty lists.
                if (e->empty()) {
                    remove(element_name);
                    return DELETED;
                }

                // We are in deletion mode, test if the element contains only keys.
                if (key_value_collection.size() < e->size()) {
                    // It doesn't.
                    return effect;
                }
                for (auto const& [key, _] : key_value_collection) {
                    if (!e->contains(key)) {
                        // It doesn't.
                        return effect;
                    }
                }

                // It does, so remove it.
                child->remove(i);
                if (child->empty()) {
                    remove(element_name);
                }
                return effect;
            }
        }

        // At this point, it wasn't found.

        // If we had to delete or retrieve it...
        if (!v) {
            // Then job is done.
            return MAX;
        }

        // If this is the end of the xpath...
        if (path.empty()) {
            // If it is this list that had to be added except for leaf-list which is currently
            // missing the underlying value...
            if (next_element.empty() && !is_leaf_list) {
                // Then job is done.
                return earlier_effect;
            }
            // Else add the value.
            child->add(v);
            return INSERTED;
        }

        // Else create the item and add the entry to it.
        ElementPtr const& e(createMap());
        size_t effect(MAX);
        for (auto const& key_value : key_value_collection) {
            std::string const& key(std::get<KEY>(key_value));
            std::string const& value(std::get<VALUE>(key_value));
            std::string const& quoteless_value(value.substr(1, value.size() - 2));
            e->set(key, Element::fromJSONFallbackToString(quoteless_value));
            if (next_element == key) {
                effect = INSERTED;
            }
        }
        child->add(e);
        if (effect == MAX) {
            return e->xpath(path, full_xpath, v, operation, is_leaf_list,
                            /* trim_prefixes = */ false);
        } else {
            return effect;
        }
    }

    // It is not a list.
    if (next_element.empty()) {
        // If child name is empty...
        if (element.empty()) {
            // Nothing to do. Operation was referring to this element which should also be the root
            // node. If request was to set it, great, job done. If request was to delete it, bad
            // luck. Delete a child of the root node.
            return MAX;
        }

        // It is a leaf or a leaf-list.

#ifdef USE_TREE_CHANGES_TO_DETECT_LEAF_LISTS
        if (is_leaf_list) {
#else  // USE_TREE_CHANGES_TO_DETECT_LEAF_LISTS
        if (is_leaf_list_brute_force(full_xpath)) {
#endif  // USE_TREE_CHANGES_TO_DETECT_LEAF_LISTS
            return xpath_leaf_list(element, v);
        }

        // If it actually contains the element...
        if (contains(element)) {
            // If it was meant to be retrieved...
            if (operation == GET) {
                v = get(element);
                return MAX;
                // If it was meant to be deleted...
            } else if (!v) {
                // Remove it.
                remove(element);
                return DELETED;
            }

            // It was meant to be set.
            size_t effect;
            if (*get(element) == *v) {
                return MAX;
            } else {
                set(element, v);
                return MODIFIED;
            }
            return effect;
        } else {
            if (v && operation == SET) {
                set(element, v);
                return INSERTED;
            }

            // Otherwise job is done.
            return MAX;
        }
    }

    // If it doesn't contain the requested child...
    if (!contains(element)) {
        // If it was meant to be deleted or retrieved...
        if (!v) {
            // Job is done.
            return MAX;
        }

        // Otherwise create it.
        set(element, createMap());
    }

    // Recurse.
    ElementPtr const& child(get(element));
    size_t effect(
        child->xpath(path, full_xpath, v, operation, is_leaf_list, /* trim_prefixes = */ false));

    // Check for empty lists.
    if (operation == SET && !v && child->empty()) {
        remove(element);
    }

    return effect;
}

void MapElement::setXpathFromParentAndChild(
    std::function<std::vector<std::string>(std::string const& xpath)> keys) {
    // Toplevel nodes don't need xpath populated, they have it populated elsewhere.
    if (!parent_) {
        return;
    }

    // If parent is not a list, xpath will be populated elsewhere.
    if (parent_->getType() != list) {
        xpath_ = endsWithOrAdd(parent_->xpath_, ":", "/") + key_;
        // Complete xpaths for all other children.
        for (auto const& [key, value] : mapValue()) {
            value->xpath_ = endsWithOrAdd(xpath_, ":", "/") + key;
        }
        return;
    }

    // If xpath is already populated, then job is done.
    if (!xpath_.empty()) {
        return;
    }

    // Find the key for this YANG list.
    std::vector<std::string> list_keys(keys(parent_->xpath_));
    if (list_keys.empty()) {
        isc_throw(Unexpected, PRETTY_METHOD_NAME() << ": no keys could be found for xpath: '"
                                                   << parent_->xpath_ << "'");
    }

    // Update xpath with all keys.
    std::string xpath_selector;
    for (std::string const& key : list_keys) {
        ElementPtr const& child(get(key));
        if (!child) {
            isc_throw(Unexpected, PRETTY_METHOD_NAME()
                                      << ": key '" << key << "' missing for node found at xpath '"
                                      << parent_->xpath_ << "'");
        }

        // Get unquoted value.
        std::string const unquoted_value(child->toUnquotedString());
        xpath_selector = xpath_selector + "[" + key + "='" + unquoted_value + "']";
    }
    xpath_ = parent_->xpath_ + xpath_selector;

    // Complete xpaths for all children.
    for (auto const& [key, value] : mapValue()) {
        value->xpath_ = endsWithOrAdd(xpath_, ":", "/") + key;
    }
}

void Element::removeEmptyRecursively() {
    if (type_ == list || type_ == map) {
        size_t s = size();
        for (int i = 0; i < s; ++i) {
            // Get child.
            ElementPtr child;
            std::string key;
            if (type_ == list) {
                child = get(i);
            } else if (type_ == map) {
                key = get(i)->stringValue();
                child = get(key);
            }

            // Makes no sense to continue for non-container children.
            if (child->getType() != list && child->getType() != map) {
                continue;
            }

            // Recurse if not empty.
            if (!child->empty()){
                child->removeEmptyRecursively();
            }

            // When returning from recursion, remove if empty.
            if (child->empty()) {
                remove(i);
                --i;
                --s;
            }
        }
    }
}

ElementPtr Element::get(const int) const {
    throwTypeError("get(int) called on non-list non-map Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

void Element::set(const size_t, ElementPtr) {
    throwTypeError("set(int, element) called on non-list Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

void Element::add(ElementPtr) {
    throwTypeError("add() called on non-list Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

void Element::remove(const int) {
    throwTypeError("remove(int) called on non-container Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

size_t Element::size() const {
    throwTypeError("size() called on non-list Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

bool Element::empty() const {
    throwTypeError("empty() called on non-list non-map Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

ElementPtr Element::get(const std::string&) const {
    throwTypeError("get(string) called on non-map Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

void Element::set(const std::string&, ElementPtr) {
    throwTypeError("set(name, element) called on non-map Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

void Element::remove(const std::string&) {
    throwTypeError("remove(string) called on non-map Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

bool Element::contains(const std::string&) const {
    throwTypeError("contains(string) called on non-map Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

bool Element::containsAll(std::vector<std::string> const&) const {
    throwTypeError("containsAll() called on non-map Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

bool Element::containsAny(std::vector<std::string> const&) const {
    throwTypeError("containsAll() called on non-map Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

ElementPtr Element::find(const std::string&) const {
    throwTypeError("find(string) called on non-map Element of type " +
                   std::string(magic_enum::enum_name(type_)));
}

bool Element::find(const std::string&, ElementPtr&) const {
    return (false);
}

namespace {

inline void throwJSONError(const std::string& error, const std::string& file, int line, int pos) {
    std::stringstream ss;
    ss << error << " in " + file + ":" << line << ":" << pos;
    isc_throw(JSONError, ss.str());
}

}  // namespace

std::ostream& operator<<(std::ostream& out, const Element& e) {
    return (out << e.str());
}

bool operator==(const Element& a, const Element& b) {
    return (a.equals(b));
}

bool operator!=(const Element& a, const Element& b) {
    return (!a.equals(b));
}

//
// factory functions
//
ElementPtr Element::create(Position const& pos) {
    return std::make_shared<NullElement>(pos);
}

ElementPtr Element::create(const int i, const Position& pos) {
    return (create(static_cast<long long int>(i), pos));
}

ElementPtr Element::create(const long int i, const Position& pos) {
    return (create(static_cast<long long int>(i), pos));
}

ElementPtr Element::create(const long long int i, const Position& pos) {
    return std::make_shared<IntElement>(static_cast<int64_t>(i), pos);
}

ElementPtr Element::create(const unsigned int i, const Position& pos) {
    return (create(static_cast<long long int>(i), pos));
}

ElementPtr Element::create(const unsigned long int i, const Position& pos) {
    return (create(static_cast<long long int>(i), pos));
}

ElementPtr Element::create(const unsigned long long int i, const Position& pos) {
    return std::make_shared<IntElement>(static_cast<int64_t>(i), pos);
}

ElementPtr Element::create(const double d, const Position& pos) {
    return std::make_shared<DoubleElement>(d, pos);
}

ElementPtr Element::create(const bool b, const Position& pos) {
    return std::make_shared<BoolElement>(b, pos);
}

ElementPtr Element::create(const std::string& s, const Position& pos) {
    return std::make_shared<StringElement>(s, pos);
}

ElementPtr Element::create(const char* s, const Position& pos) {
    return (create(std::string(s), pos));
}

ElementPtr Element::createList(const Position& pos) {
    return std::make_shared<ListElement>(pos);
}

ElementPtr Element::createMap(const Position& pos /* = ZERO_POSITION() */) {
    return std::make_shared<MapElement>(pos);
}

//
// helper functions for fromJSON factory
//
namespace {

bool charIn(const int c, const char* chars) {
    const size_t chars_len = std::strlen(chars);
    for (size_t i = 0; i < chars_len; ++i) {
        if (chars[i] == c) {
            return (true);
        }
    }
    return (false);
}

void skipChars(std::istream& in, const char* chars, int& line, int& pos) {
    int c = in.peek();
    while (charIn(c, chars) && c != EOF) {
        if (c == '\n') {
            ++line;
            pos = 1;
        } else {
            ++pos;
        }
        in.ignore();
        c = in.peek();
    }
}

// skip on the input stream to one of the characters in chars
// if another character is found this function throws JSONError
// unless that character is specified in the optional may_skip
//
// It returns the found character (as an int value).
int skipTo(std::istream& in,
           const std::string& file,
           int& line,
           int& pos,
           const char* chars,
           const char* may_skip = "") {
    int c = in.get();
    ++pos;
    while (c != EOF) {
        if (c == '\n') {
            pos = 1;
            ++line;
        }
        if (charIn(c, may_skip)) {
            c = in.get();
            ++pos;
        } else if (charIn(c, chars)) {
            while (charIn(in.peek(), may_skip)) {
                if (in.peek() == '\n') {
                    pos = 1;
                    ++line;
                } else {
                    ++pos;
                }
                in.ignore();
            }
            return (c);
        } else {
            throwJSONError(std::string("'") + std::string(1, c) + "' read, one of \"" + chars +
                               "\" expected",
                           file, line, pos);
        }
    }
    throwJSONError(std::string("EOF read, one of \"") + chars + "\" expected", file, line, pos);
    return (c);  // shouldn't reach here, but some compilers require it
}

// TODO: Should we check for all other official escapes here (and
// error on the rest)?
std::string
strFromStringstream(std::istream& in, const std::string& file, const int line, int& pos) {
    std::stringstream ss;
    int c = in.get();
    ++pos;
    if (c == '"') {
        c = in.get();
        ++pos;
    } else {
        throwJSONError("String expected", file, line, pos);
    }

    while (c != EOF && c != '"') {
        if (c == '\\') {
            // see the spec for allowed escape characters
            int d;
            switch (in.peek()) {
            case '"':
                c = '"';
                break;
            case '/':
                c = '/';
                break;
            case '\\':
                c = '\\';
                break;
            case 'b':
                c = '\b';
                break;
            case 'f':
                c = '\f';
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            case 'u':
                // skip first 0
                in.ignore();
                ++pos;
                c = in.peek();
                if (c != '0') {
                    throwJSONError("Unsupported unicode escape", file, line, pos);
                }
                // skip second 0
                in.ignore();
                ++pos;
                c = in.peek();
                if (c != '0') {
                    throwJSONError("Unsupported unicode escape", file, line, pos - 2);
                }
                // get first digit
                in.ignore();
                ++pos;
                d = in.peek();
                if ((d >= '0') && (d <= '9')) {
                    c = (d - '0') << 4;
                } else if ((d >= 'A') && (d <= 'F')) {
                    c = (d - 'A' + 10) << 4;
                } else if ((d >= 'a') && (d <= 'f')) {
                    c = (d - 'a' + 10) << 4;
                } else {
                    throwJSONError("Not hexadecimal in unicode escape", file, line, pos - 3);
                }
                // get second digit
                in.ignore();
                ++pos;
                d = in.peek();
                if ((d >= '0') && (d <= '9')) {
                    c |= d - '0';
                } else if ((d >= 'A') && (d <= 'F')) {
                    c |= d - 'A' + 10;
                } else if ((d >= 'a') && (d <= 'f')) {
                    c |= d - 'a' + 10;
                } else {
                    throwJSONError("Not hexadecimal in unicode escape", file, line, pos - 4);
                }
                break;
            default:
                throwJSONError("Bad escape", file, line, pos);
            }
            // drop the escaped char
            in.ignore();
            ++pos;
        }
        ss.put(c);
        c = in.get();
        ++pos;
    }
    if (c == EOF) {
        throwJSONError("Unterminated string", file, line, pos);
    }
    return (ss.str());
}

std::string wordFromStringstream(std::istream& in, int& pos) {
    std::stringstream ss;
    while (isalpha(in.peek())) {
        ss << (char)in.get();
    }
    pos += ss.str().size();
    return (ss.str());
}

std::string numberFromStringstream(std::istream& in, int& pos) {
    std::stringstream ss;
    while (isdigit(in.peek()) || in.peek() == '+' || in.peek() == '-' || in.peek() == '.' ||
           in.peek() == 'e' || in.peek() == 'E') {
        ss << (char)in.get();
    }
    pos += ss.str().size();
    return (ss.str());
}

// Should we change from IntElement and DoubleElement to NumberElement
// that can also hold an e value? (and have specific getters if the
// value is larger than an int can handle)
//
ElementPtr
fromStringstreamNumber(std::istream& in, const std::string& file, const int& line, int& pos) {
    // Remember position where the value starts. It will be set in the
    // Position structure of the Element to be created.
    const uint32_t start_pos = pos;
    // This will move the pos to the end of the value.
    const std::string number = numberFromStringstream(in, pos);

    if (number.find_first_of(".eE") < number.size()) {
        try {
            return (Element::create(boost::lexical_cast<double>(number),
                                    Element::Position(file, line, start_pos)));
        } catch (const boost::bad_lexical_cast&) {
            throwJSONError(std::string("Number overflow: ") + number, file, line, start_pos);
        }
    } else {
        try {
            return (Element::create(boost::lexical_cast<int64_t>(number),
                                    Element::Position(file, line, start_pos)));
        } catch (const boost::bad_lexical_cast&) {
            throwJSONError(std::string("Number overflow: ") + number, file, line, start_pos);
        }
    }
    return (ElementPtr());
}

ElementPtr
fromStringstreamBool(std::istream& in, const std::string& file, const int line, int& pos) {
    // Remember position where the value starts. It will be set in the
    // Position structure of the Element to be created.
    const uint32_t start_pos = pos;
    // This will move the pos to the end of the value.
    const std::string word = wordFromStringstream(in, pos);

    if (word == "true") {
        return (Element::create(true, Element::Position(file, line, start_pos)));
    } else if (word == "false") {
        return (Element::create(false, Element::Position(file, line, start_pos)));
    } else {
        throwJSONError(std::string("Bad boolean value: ") + word, file, line, start_pos);
    }
    return (ElementPtr());
}

ElementPtr
fromStringstreamNull(std::istream& in, const std::string& file, const int line, int& pos) {
    // Remember position where the value starts. It will be set in the
    // Position structure of the Element to be created.
    const uint32_t start_pos = pos;
    // This will move the pos to the end of the value.
    const std::string word = wordFromStringstream(in, pos);
    if (word == "null") {
        return (Element::create(Element::Position(file, line, start_pos)));
    } else {
        throwJSONError(std::string("Bad null value: ") + word, file, line, start_pos);
        return (ElementPtr());
    }
}

ElementPtr fromStringstreamString(std::istream& in, const std::string& file, int& line, int& pos) {
    // Remember position where the value starts. It will be set in the
    // Position structure of the Element to be created.
    const uint32_t start_pos = pos;
    // This will move the pos to the end of the value.
    const std::string string_value = strFromStringstream(in, file, line, pos);
    return (Element::create(string_value, Element::Position(file, line, start_pos)));
}

ElementPtr fromStringstreamList(std::istream& in, const std::string& file, int& line, int& pos) {
    int c = 0;
    ElementPtr list = Element::createList(Element::Position(file, line, pos));
    ElementPtr cur_list_element;

    skipChars(in, WHITESPACE, line, pos);
    while (c != EOF && c != ']') {
        if (in.peek() != ']') {
            cur_list_element = Element::fromJSON(in, file, line, pos);
            list->add(cur_list_element);
            c = skipTo(in, file, line, pos, ",]", WHITESPACE);
        } else {
            c = in.get();
            ++pos;
        }
    }
    return (list);
}

ElementPtr fromStringstreamMap(std::istream& in, const std::string& file, int& line, int& pos) {
    ElementPtr map = Element::createMap(Element::Position(file, line, pos));
    skipChars(in, WHITESPACE, line, pos);
    int c = in.peek();
    if (c == EOF) {
        throwJSONError(std::string("Unterminated map, <string> or } expected"), file, line, pos);
    } else if (c == '}') {
        // empty map, skip closing curly
        in.ignore();
    } else {
        while (c != EOF && c != '}') {
            std::string key = strFromStringstream(in, file, line, pos);

            skipTo(in, file, line, pos, ":", WHITESPACE);
            // skip the :

            ElementPtr value = Element::fromJSON(in, file, line, pos);
            map->set(key, value);

            c = skipTo(in, file, line, pos, ",}", WHITESPACE);
        }
    }
    return (map);
}

}  // namespace

std::string Element::typeToName(Element::type_t type) {
    switch (type) {
    case Element::integer:
        return (std::string("integer"));
    case Element::real:
        return (std::string("real"));
    case Element::boolean:
        return (std::string("boolean"));
    case Element::string:
        return (std::string("string"));
    case Element::list:
        return (std::string("list"));
    case Element::map:
        return (std::string("map"));
    case Element::null:
        return (std::string("null"));
    case Element::any:
        return (std::string("any"));
    default:
        return (std::string("unknown"));
    }
}

Element::type_t Element::nameToType(const std::string& type_name) {
    if (type_name == "integer") {
        return (Element::integer);
    } else if (type_name == "real") {
        return (Element::real);
    } else if (type_name == "boolean") {
        return (Element::boolean);
    } else if (type_name == "string") {
        return (Element::string);
    } else if (type_name == "list") {
        return (Element::list);
    } else if (type_name == "map") {
        return (Element::map);
    } else if (type_name == "named_set") {
        return (Element::map);
    } else if (type_name == "null") {
        return (Element::null);
    } else if (type_name == "any") {
        return (Element::any);
    } else {
        isc_throw(TypeError, type_name << " is not a valid type name");
    }
}

ElementPtr Element::fromJSON(std::istream& in, bool preproc) {

    int line = 1, pos = 1;
    std::stringstream filtered;
    if (preproc) {
        preprocess(in, filtered);
    }

    ElementPtr value = fromJSON(preproc ? filtered : in, "<istream>", line, pos);

    return (value);
}

ElementPtr Element::fromJSON(std::istream& in, const std::string& file_name, bool preproc) {
    int line = 1, pos = 1;
    std::stringstream filtered;
    if (preproc) {
        preprocess(in, filtered);
    }
    return (fromJSON(preproc ? filtered : in, file_name, line, pos));
}

ElementPtr Element::fromJSON(std::istream& in, const std::string& file, int& line, int& pos) {
    int c = 0;
    ElementPtr element;
    bool el_read = false;
    skipChars(in, WHITESPACE, line, pos);
    while (c != EOF && !el_read) {
        c = in.get();
        switch (c) {
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':
        case '-':
        case '+':
        case '.':
            in.putback(c);
            element = fromStringstreamNumber(in, file, line, pos);
            el_read = true;
            break;
        case 't':
        case 'f':
            in.putback(c);
            element = fromStringstreamBool(in, file, line, pos);
            el_read = true;
            break;
        case 'n':
            in.putback(c);
            element = fromStringstreamNull(in, file, line, pos);
            el_read = true;
            break;
        case '"':
            in.putback('"');
            element = fromStringstreamString(in, file, line, pos);
            el_read = true;
            break;
        case '[':
            ++pos;
            element = fromStringstreamList(in, file, line, pos);
            el_read = true;
            break;
        case '{':
            ++pos;
            element = fromStringstreamMap(in, file, line, pos);
            el_read = true;
            break;
        case EOF:
            break;
        default:
            throwJSONError(std::string("error: unexpected character ") + std::string(1, c), file,
                           line, pos);
            break;
        }
    }
    if (el_read) {
        return (element);
    } else {
        isc_throw(JSONError, "nothing read");
    }
}

ElementPtr Element::fromJSON(const std::string& in, bool preproc) {
    std::stringstream ss;
    ss << in;

    int line = 1, pos = 1;
    std::stringstream filtered;
    if (preproc) {
        preprocess(ss, filtered);
    }
    ElementPtr result(fromJSON(preproc ? filtered : ss, "<string>", line, pos));
    skipChars(ss, WHITESPACE, line, pos);
    // ss must now be at end
    if (ss.peek() != EOF) {
        throwJSONError("Extra data", "<string>", line, pos);
    }
    return result;
}

ElementPtr Element::fromJSONFallbackToString(std::string const& input,
                                             bool const preproc /* = false */) {
    ElementPtr result;
    try {
        result = Element::fromJSON(input, preproc);
    } catch (isc::data::JSONError const&) {
        // Fallback to string one single time.
        result = Element::fromJSON("\"" + input + "\"", preproc);
    }
    return result;
}

ElementPtr Element::fromJSONFile(const std::string& file_name, bool preproc) {
    // zero out the errno to be safe
    errno = 0;

    std::ifstream infile(file_name.c_str(), std::ios::in | std::ios::binary);
    if (!infile.is_open()) {
        const char* error = strerror(errno);
        isc_throw(InvalidOperation, "failed to read file '" << file_name << "': " << error);
    }

    return (fromJSON(infile, file_name, preproc));
}

ElementPtr Element::fromJSONFileExceptionSafe(std::string const& file_name, bool const preproc) {
    ElementPtr result;

    try {
        result = fromJSONFile(file_name, preproc);
    } catch (InvalidOperation const& exception) {
        result = Element::createMap();
    }

    return result;
}

// to JSON format

void IntElement::toJSON(std::ostream& ss) const {
    ss << intValue();
}

void DoubleElement::toJSON(std::ostream& ss) const {
    // The default output for doubles nicely drops off trailing
    // zeros, however this produces strings without decimal points
    // for whole number values.  When reparsed this will create
    // IntElements not DoubleElements.  Rather than used a fixed
    // precision, we'll just tack on an ".0" when the decimal point
    // is missing.
    std::ostringstream val_ss;
    val_ss << doubleValue();
    ss << val_ss.str();
    if (val_ss.str().find_first_of('.') == std::string::npos) {
        ss << ".0";
    }
}

void BoolElement::toJSON(std::ostream& ss) const {
    if (boolValue()) {
        ss << "true";
    } else {
        ss << "false";
    }
}

void NullElement::toJSON(std::ostream& ss) const {
    ss << "null";
}

void StringElement::toJSON(std::ostream& ss) const {
    ss << "\"";
    const std::string& str = stringValue();
    for (size_t i = 0; i < str.size(); ++i) {
        const char c = str[i];
        // Escape characters as defined in JSON spec
        // Note that we do not escape forward slash; this
        // is allowed, but not mandatory.
        switch (c) {
        case '"':
            ss << '\\' << c;
            break;
        case '\\':
            ss << '\\' << c;
            break;
        case '\b':
            ss << '\\' << 'b';
            break;
        case '\f':
            ss << '\\' << 'f';
            break;
        case '\n':
            ss << '\\' << 'n';
            break;
        case '\r':
            ss << '\\' << 'r';
            break;
        case '\t':
            ss << '\\' << 't';
            break;
        default:
            if (((c >= 0) && (c < 0x20)) || (c < 0) || (c >= 0x7f)) {
                std::ostringstream esc;
                esc << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                    << (static_cast<unsigned>(c) & 0xff);
                ss << esc.str();
            } else {
                ss << c;
            }
        }
    }
    ss << "\"";
}

void ListElement::toJSON(std::ostream& ss) const {
    ss << "[ ";

    const std::vector<ElementPtr>& v = listValue();
    for (std::vector<ElementPtr>::const_iterator it = v.begin(); it != v.end(); ++it) {
        if (it != v.begin()) {
            ss << ", ";
        }
        (*it)->toJSON(ss);
    }
    ss << " ]";
}

void MapElement::toJSON(std::ostream& ss) const {
    ss << "{ ";

    const std::map<std::string, ElementPtr>& m = mapValue();
    for (std::map<std::string, ElementPtr>::const_iterator it = m.begin(); it != m.end(); ++it) {
        if (it != m.begin()) {
            ss << ", ";
        }
        ss << "\"" << (*it).first << "\": ";
        if ((*it).second) {
            (*it).second->toJSON(ss);
        } else {
            ss << "None";
        }
    }
    ss << " }";
}

// throws when one of the types in the path (except the one
// we're looking for) is not a MapElement
// returns 0 if it could simply not be found
// should that also be an exception?
ElementPtr MapElement::find(const std::string& id) const {
    const size_t sep = id.find('/');
    if (sep == std::string::npos) {
        return (get(id));
    } else {
        ElementPtr ce = get(id.substr(0, sep));
        if (ce) {
            // ignore trailing slash
            if (sep + 1 != id.size()) {
                return (ce->find(id.substr(sep + 1)));
            } else {
                return (ce);
            }
        } else {
            return (ElementPtr());
        }
    }
}

ElementPtr Element::fromWire(const std::string& s) {
    std::stringstream ss;
    ss << s;
    int line = 0, pos = 0;
    return (fromJSON(ss, "<wire>", line, pos));
}

ElementPtr Element::fromWire(std::stringstream& in, int) {
    //
    // Check protocol version
    //
    // for (int i = 0 ; i < 4 ; ++i) {
    //    const unsigned char version_byte = get_byte(in);
    //    if (PROTOCOL_VERSION[i] != version_byte) {
    //        throw DecodeError("Protocol version incorrect");
    //    }
    //}
    // length -= 4;
    int line = 0, pos = 0;
    return (fromJSON(in, "<wire>", line, pos));
}

bool MapElement::find(const std::string& id, ElementPtr& t) const {
    try {
        ElementPtr p = find(id);
        if (p) {
            t = p;
            return (true);
        }
    } catch (const TypeError&) {
        // ignore
    }
    return (false);
}

bool IntElement::equals(const Element& other) const {
    return (other.getType() == Element::integer) && (i == other.intValue());
}

bool DoubleElement::equals(const Element& other) const {
    return (other.getType() == Element::real) && (fabs(d - other.doubleValue()) < 1e-14);
}

bool BoolElement::equals(const Element& other) const {
    return (other.getType() == Element::boolean) && (b == other.boolValue());
}

bool NullElement::equals(const Element& other) const {
    return (other.getType() == Element::null);
}

bool StringElement::equals(const Element& other) const {
    return (other.getType() == Element::string) && (s == other.stringValue());
}

bool ListElement::equals(const Element& other) const {
    if (other.getType() == Element::list) {
        const size_t s = size();
        if (s != other.size()) {
            return (false);
        }
        for (size_t i = 0; i < s; ++i) {
            if (!get(i)->equals(*other.get(i))) {
                return (false);
            }
        }
        return (true);
    } else {
        return (false);
    }
}

void ListElement::setXpathToChild(ElementPtr const& child) {
    Element::type_t const value_type(child->getType());
    bool const child_is_leaf(value_type != Element::list && value_type != Element::map);
    if (child_is_leaf) {
        child->xpath_ = xpath_ + "[.='" + child->str() + "']";
    }
}

bool MapElement::equals(const Element& other) const {
    if (other.getType() == Element::map) {
        if (size() != other.size()) {
            return (false);
        }
        for (auto kv : mapValue()) {
            auto key = kv.first;
            if (other.contains(key)) {
                if (!get(key)->equals(*other.get(key))) {
                    return (false);
                }
            } else {
                return (false);
            }
        }
        return (true);
    } else {
        return (false);
    }
}

ElementPtr removeIdentical(ElementPtr a, ElementPtr b) {
    ElementPtr result = Element::createMap();

    if (!b) {
        return (result);
    }

    if (a->getType() != Element::map || b->getType() != Element::map) {
        isc_throw(TypeError, "Non-map Elements passed to removeIdentical");
    }

    for (auto kv : a->mapValue()) {
        auto key = kv.first;
        if (!b->contains(key) || !a->get(key)->equals(*b->get(key))) {
            result->set(key, kv.second);
        }
    }

    return (result);
}

void merge(ElementPtr element, ElementPtr other) {
    if (element->getType() != Element::map || other->getType() != Element::map) {
        isc_throw(TypeError, "merge arguments not MapElements");
    }

    for (auto const& [key, value] : other->mapValue()) {
        if (value && value->getType() != Element::null) {
            element->set(key, value);
        } else if (element->contains(key)) {
            element->remove(key);
        }
    }
}

ElementPtr copy(ElementPtr from, int level) {
    if (!from) {
        isc_throw(BadValue, "copy got a null pointer");
    }
    int from_type = from->getType();
    if (from_type == Element::integer) {
        return Element::create(from->intValue());
    } else if (from_type == Element::real) {
        return Element::create(from->doubleValue());
    } else if (from_type == Element::boolean) {
        return Element::create(from->boolValue());
    } else if (from_type == Element::null) {
        return Element::create();
    } else if (from_type == Element::string) {
        return Element::create(from->stringValue());
    } else if (from_type == Element::list) {
        ElementPtr result = ElementPtr(new ListElement());
        for (auto elem : from->listValue()) {
            if (level == 0) {
                result->add(elem);
            } else {
                result->add(copy(elem, level - 1));
            }
        }
        return (result);
    } else if (from_type == Element::map) {
        ElementPtr result = ElementPtr(new MapElement());
        for (auto const& [key, value] : from->mapValue()) {
            if (level == 0) {
                result->set(key, value);
            } else {
                result->set(key, copy(value, level - 1));
            }
        }
        return (result);
    } else {
        isc_throw(BadValue, "copy got an element of type: " << from_type);
    }
}

bool isEquivalent(ElementPtr a, ElementPtr b, uint8_t level /* = 100 */) {
    // check looping forever on cycles
    if (!level) {
        isc_throw(BadValue, "isEquivalent got infinite recursion: "
                            "arguments include cycles");
    }
    if (!a || !b) {
        isc_throw(BadValue, "isEquivalent got a null pointer");
    }
    // check types
    if (a->getType() != b->getType()) {
        return (false);
    }
    if (a->getType() == Element::list) {
        // check empty
        if (a->empty()) {
            return (b->empty());
        }
        // check size
        if (a->size() != b->size()) {
            return (false);
        }

        // copy b into a list
        const size_t s = a->size();
        std::vector<ElementPtr> l;
        for (size_t i = 0; i < s; ++i) {
            l.push_back(b->get(i));
        }

        // iterate on a
        for (size_t i = 0; i < s; ++i) {
            ElementPtr item = a->get(i);
            // lookup this item in the list
            bool found = false;
            for (std::vector<ElementPtr>::iterator it = l.begin(); it != l.end(); ++it) {
                // if found in the list remove it
                if (isEquivalent(item, *it, level - 1)) {
                    found = true;
                    it = l.erase(it);
                    break;
                }
            }
            // if not found argument differs
            if (!found) {
                return (false);
            }
        }

        // sanity check: the list must be empty
        if (!l.empty()) {
            isc_throw(Unexpected, "isEquivalent internal error");
        }
        return (true);
    } else if (a->getType() == Element::map) {
        // check sizes
        if (a->size() != b->size()) {
            return (false);
        }
        // iterate on the first map
        for (auto kv : a->mapValue()) {
            // get the b value for the given keyword and recurse
            ElementPtr item = b->get(kv.first);
            if (!item || !isEquivalent(kv.second, item, level - 1)) {
                return (false);
            }
        }
        return (true);
    } else {
        return (a->equals(*b));
    }
}

/// @brief diff() retrieves differences between two Elements.
bool diff(ElementPtr a,
          ElementPtr b,
          std::vector<std::tuple<std::string, ElementPtr, ElementPtr>>& diffs,
          std::string const& xpath,
          uint8_t level /* = 128 */,
          bool track_changes /* = true */) {
    // Check if the depth limit has been reached.
    if (level == 0) {
        isc_throw(BadValue, "diff() got infinite recursion: arguments include cycles");
    }

    // For null pointers, add the other non-null pointer.
    if (!a && !b) {
        return true;
    }
    if (!a || !b) {
        if (track_changes) {
            diffs.push_back({xpath, a, b});
        }
        return false;
    }

    // Check types.
    if (a->getType() != b->getType()) {
        if (track_changes) {
            diffs.push_back({xpath, a, b});
        }
        return false;
    }

    bool result(true);
    if (a->getType() == Element::list) {
        // Copy `b` into a list.
        std::vector<ElementPtr> l;
        for (size_t i = 0; i < b->size(); ++i) {
            l.push_back(b->get(i));
        }

        // Iterate on `a`.
        for (size_t i = 0; i < a->size(); ++i) {
            ElementPtr item(a->get(i));

            // Lookup this item in the list.
            bool found(false);
            for (auto it = l.begin(); it != l.end(); ++it) {
                // If found in the list, remove it.
                if (diff(item, *it, diffs, xpath + "[]", level - 1, track_changes)) {
                    found = true;
                    l.erase(it);
                    break;
                }
            }

            // If not found, element is missing from `b`.
            if (!found) {
                if (track_changes) {
                    diffs.push_back({xpath + "[]", a->get(i), nullptr});
                }
                result = false;
            }
        }

        // All other elements in `l` are missing from `a`.
        for (ElementPtr bb : l) {
            if (track_changes) {
                diffs.push_back({xpath + "[]", nullptr, bb});
            }
            result = false;
        }
        return result;
    } else if (a->getType() == Element::map) {
        // Iterate on `a`.
        for (auto& aa : a->mapValue()) {
            // Get `b`'s value for the given keyword and recurse.
            result &= diff(aa.second, b->get(aa.first), diffs, xpath + "/" + aa.first, level - 1,
                           track_changes);
        }

        // Iterate on `b`.
        for (auto& bb : b->mapValue()) {
            // Check if the keyword exists.
            if (!a->contains(bb.first)) {
                if (track_changes) {
                    diffs.push_back({xpath + "/" + bb.first, nullptr, bb.second});
                }
                result = false;
            }
        }
        return result;
    } else {
        if (!a->equals(*b)) {
            if (track_changes) {
                diffs.push_back({xpath, a, b});
            }
            return false;
        }
        return true;
    }
}

std::string parseDiff(std::vector<std::tuple<std::string, ElementPtr, ElementPtr>> const& diffs) {
    std::ostringstream msg;
    for (auto const& d : diffs) {
        std::string const& xpath(std::get<0>(d));
        ElementPtr const& left(std::get<1>(d));
        ElementPtr const& right(std::get<2>(d));
        msg << "--- " + xpath + " ---\n";
        msg << "<-<-<-<-<-<-<\n";
        if (left) {
            msg << prettyPrint(left) << "\n";
        }
        msg << "=-=-=-=-=-=-=\n";
        if (right) {
            msg << prettyPrint(right) << "\n";
        }
        msg << ">->->->->->->\n\n";
    }
    return msg.str();
}

void prettyPrint(ElementPtr element, std::ostream& out, unsigned indent, unsigned step) {
    if (!element) {
        isc_throw(BadValue, "prettyPrint got a null pointer");
    }
    if (element->getType() == Element::list) {
        // empty list case
        if (element->empty()) {
            out << "[ ]";
            return;
        }

        // complex ? multiline : oneline
        if (!element->get(0)) {
            isc_throw(BadValue, "prettyPrint got a null pointer");
        }
        int first_type = element->get(0)->getType();
        bool complex = false;
        if ((first_type == Element::list) || (first_type == Element::map)) {
            complex = true;
        }
        std::string separator = complex ? ",\n" : ", ";

        // open the list
        out << "[" << (complex ? "\n" : " ");

        // iterate on items
        const auto& l = element->listValue();
        for (auto it = l.begin(); it != l.end(); ++it) {
            // add the separator if not the first item
            if (it != l.begin()) {
                out << separator;
            }
            // add indentation
            if (complex) {
                out << std::string(indent + step, ' ');
            }
            // recursive call
            prettyPrint(*it, out, indent + step, step);
        }

        // close the list
        if (complex) {
            out << "\n" << std::string(indent, ' ');
        } else {
            out << " ";
        }
        out << "]";
    } else if (element->getType() == Element::map) {
        // empty map case
        if (element->size() == 0) {
            out << "{ }";
            return;
        }

        // open the map
        out << "{\n";

        // iterate on keyword: value
        const auto& m = element->mapValue();
        bool first = true;
        for (auto it = m.begin(); it != m.end(); ++it) {
            // add the separator if not the first item
            if (first) {
                first = false;
            } else {
                out << ",\n";
            }
            // add indentation
            out << std::string(indent + step, ' ');
            // add keyword:
            out << "\"" << it->first << "\": ";
            // recursive call
            prettyPrint(it->second, out, indent + step, step);
        }

        // close the map
        out << "\n" << std::string(indent, ' ') << "}";
    } else {
        // not a list or a map
        element->toJSON(out);
    }
}

std::string prettyPrint(ElementPtr element, unsigned indent, unsigned step) {
    std::stringstream ss;
    prettyPrint(element, ss, indent, step);
    return (ss.str());
}

void Element::preprocess(std::istream& in, std::stringstream& out) {

    std::string line;

    while (std::getline(in, line)) {
        // If this is a comments line, replace it with empty line
        // (so the line numbers will still match
        if (!line.empty() && line[0] == '#') {
            line = "";
        }

        // getline() removes end line characters. Unfortunately, we need
        // it for getting the line numbers right (in case we report an
        // error.
        out << line;
        out << "\n";
    }
}

}  // namespace data
}  // namespace isc

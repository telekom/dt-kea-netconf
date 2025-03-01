// Copyright (C) 2010-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef ISC_DATA_H
#define ISC_DATA_H 1

#include <exceptions/exceptions.h>

#include <bitset>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

#include <stdint.h>

#include <util/magic_enum.hpp>

namespace isc {
namespace data {

using effect_t = std::bitset<4>;

static size_t constexpr DELETED = 0;
static size_t constexpr INSERTED = 1;
static size_t constexpr MODIFIED = 2;
static size_t constexpr KEY_IS_AFFECTED = 3;

static size_t constexpr MAX = std::numeric_limits<std::size_t>::max();

struct Effect {
    static std::string const convert(effect_t const& effect) {
        std::string output;
        for (size_t i = 0; i < effect.size(); ++i) {
            if (effect.test(i)) {
                output += '1';
            } else {
                output += '0';
            }
        }
        return output;
    }

    static effect_t const convert(std::string const& effect) {
        isc::data::effect_t output;
        size_t i(0);
        for (auto const& bit : effect) {
            if (bit == '1') {
                output.set(i);
            }
            ++i;
        }
        return output;
    }

    static std::string print(effect_t const& effect) {
        bool first(true);
        std::stringstream stream;
        if (effect.none()) {
            assign(stream, first, "NO_EFFECT");
        }
        if (effect.test(DELETED)) {
            assign(stream, first, "DELETED");
        }
        if (effect.test(INSERTED)) {
            assign(stream, first, "INSERTED");
        }
        if (effect.test(MODIFIED)) {
            assign(stream, first, "MODIFIED");
        }
        if (effect.test(KEY_IS_AFFECTED)) {
            assign(stream, first, "KEY_IS_AFFECTED");
        }
        return stream.str();
    }

private:
    /// @brief static methods only
    /// @{
    Effect() = delete;
    ~Effect() = delete;
    Effect(Effect const&) = delete;
    Effect& operator=(Effect const&) = delete;
    /// @}

    static void assign(std::stringstream& stream, bool& first, std::string const& output) {
        if (first) {
            first = false;
        } else {
            stream << ", ";
        }
        stream << output;
    }
};

struct Element;
// todo: describe the rationale behind ElementPtr?
typedef std::shared_ptr<Element> ElementPtr;

///
/// @brief A standard Data module exception that is thrown if a function
/// is called for an Element that has a wrong type (e.g. int_value on a
/// ListElement)
///
class TypeError : public isc::Exception {
public:
    TypeError(const char* file, size_t line, const char* what) : isc::Exception(file, line, what) {
    }
};

///
/// @brief A standard Data module exception that is thrown if a parse
/// error is encountered when constructing an Element from a string
///
// i'd like to use Exception here but we need one that is derived from
// runtime_error (as this one is directly based on external data, and
// i want to add some values to any static data string that is provided)
class JSONError : public isc::Exception {
public:
    JSONError(const char* file, size_t line, const char* what) : isc::Exception(file, line, what) {
    }
};

///
/// @brief The @c Element class represents a piece of data, used by
/// the command channel and configuration parts.
///
/// An @c Element can contain simple types (int, real, string, bool and
/// None), and composite types (list and string->element maps)
///
/// Elements should in calling functions usually be referenced through
/// an @c ElementPtr, which can be created using the factory functions
/// @c Element::create() and @c Element::fromJSON()
///
/// Notes to developers: Element is a base class, implemented by a
/// specific subclass for each type (IntElement, BoolElement, etc).
/// Element does define all functions for all types, and defaults to
/// raising a @c TypeError for functions that are not supported for
/// the type in question.
///
struct Element : std::enable_shared_from_this<Element> {
    /// @brief Represents the position of the data element within a
    /// configuration string.
    ///
    /// Position comprises a file name, line number and an offset within this
    /// line where the element value starts. For example, if the JSON string is
    ///
    /// @code
    /// { "foo": "some string",
    ///   "bar": 123 }
    /// \endcode
    ///
    /// the position of the element "bar" is: line_ = 2; pos_ = 9, because
    /// beginning of the value "123" is at offset 9 from the beginning of
    /// the second line, including whitespaces.
    ///
    /// Note that the @c Position structure is used as an argument to @c Element
    /// constructors and factory functions to avoid ambiguity and so that the
    /// uint32_t arguments holding line number and position within the line are
    /// not confused with the @c Element values passed to these functions.
    struct Position {
        std::string file_;  ///< File name.
        uint32_t line_;  ///< Line number.
        uint32_t pos_;  ///< Position within the line.

        /// @brief Default constructor.
        Position() : file_(""), line_(0), pos_(0) {
        }

        /// @brief Constructor.
        ///
        /// @param file File name.
        /// @param line Line number.
        /// @param pos Position within the line.
        Position(const std::string& file, const uint32_t line, const uint32_t pos)
            : file_(file), line_(line), pos_(pos) {
        }

        /// @brief Returns the position in the textual format.
        ///
        /// The returned position has the following format: file:line:pos.
        std::string str() const;
    };

    /// @brief Returns @c Position object with line_ and pos_ set to 0, and
    /// with an empty file name.
    ///
    /// The object containing two zeros is a default for most of the
    /// methods creating @c Element objects. The returned value is static
    /// so as it is not created everytime the function with the default
    /// position argument is called.
    static const Position& ZERO_POSITION() {
        static Position position("", 0, 0);
        return (position);
    }

    // any is a special type used in list specifications, specifying
    // that the elements can be of any type
    enum type_t { integer, real, boolean, null, string, list, map, any };

    // base class; make dtor virtual
    virtual ~Element() = default;

    /// @brief iterators
    /// @{
    template <typename result_t>
    struct iteratorT;
    template <typename result_t>
    struct iteratorT {
        using list_iterator_t = std::vector<ElementPtr>::const_iterator;
        using map_iterator_t = std::map<std::string, ElementPtr>::const_iterator;

        iteratorT(result_t const& e = nullptr, iteratorT* parent = nullptr)
            : e_(e), parent_(parent) {
        }

        result_t operator*() {
            return e_;
        }

        bool operator==(iteratorT& other) {
            return *e_ == **other;
        }

        bool operator!=(iteratorT& other) {
            return *e_ != **other;
        }

        /// @todo: requires `this` to be changed rather than returning an operator and maybe
        /// `iteratorT&` to be returned to be idiomatic to C++20.
        iteratorT operator++() = delete;
        iteratorT increment() {
            isc_throw(NotImplemented, "bad implementation");
            if (e_->type_ == list) {
                if (list_iterator_) {
                    ++(*list_iterator_);
                } else {
                    std::vector<ElementPtr> const& l(e_->listValue());
                    list_iterator_.reset(new list_iterator_t(l.begin()));
                    list_end_.reset(new list_iterator_t(l.end()));
                }

                if (*list_iterator_ == *list_end_) {
                    if (parent_) {
                        return ++(*parent_);
                    }

                    // end
                    return iteratorT();
                }
                ElementPtr value(**list_iterator_);
                return iteratorT(value.get(), this);
            } else if (e_->type_ == map) {
                if (map_iterator_) {
                    ++(*map_iterator_);
                } else {
                    std::map<std::string, ElementPtr> const& m(e_->mapValue());
                    map_iterator_.reset(new map_iterator_t(m.begin()));
                    map_end_.reset(new map_iterator_t(m.end()));
                }

                if (*map_iterator_ == *map_end_) {
                    if (parent_) {
                        return ++(*parent_);
                    }

                    // end
                    return iteratorT();
                }
                auto const& [key, value](**map_iterator_);
                return iteratorT(value.get(), this);
            }

            // leaf
            if (parent_) {
                return ++(*parent_);
            }

            // end
            return iteratorT();
        }

    private:
        ElementPtr e_;
        iteratorT* parent_;
        std::unique_ptr<list_iterator_t> list_iterator_;
        std::unique_ptr<list_iterator_t> list_end_;
        std::unique_ptr<map_iterator_t> map_iterator_;
        std::unique_ptr<map_iterator_t> map_end_;
    };

    using iterator = iteratorT<ElementPtr>;

    iterator begin() {
        return iterator(shared_from_this());
    }

    iterator end() {
        return iterator();
    }
    /// @}

    /// @brief retrieves the ElementPtr found at given xpath.
    ElementPtr xpath(std::string const& path, bool const trim_prefixes = true);

    /// @brief sets the ElementPtr at given xpath.
    size_t xpath(std::string const& path,
                 ElementPtr value,
                 bool const is_leaf_list = false,
                 bool const trim_prefixes = true);

    void checkAndSet(std::string const& destination_key, ElementPtr const& source) {
        if (source) {
            set(destination_key, source);
        }
    }

    void checkAndXpath(std::string const& destination_key, ElementPtr const& source) {
        if (source) {
            xpath(destination_key, source);
        }
    }

    void checkAndSet(std::string const& destination_key,
                     ElementPtr const& source,
                     std::string const& source_key) {
        ElementPtr const& value(source->get(source_key));
        if (value) {
            set(destination_key, value);
        }
    }

    void checkAndAddAllListElements(ElementPtr const& source, std::string const& source_key) {
        ElementPtr const& value(source->get(source_key));
        if (value) {
            for (auto const& elem : value->listValue()) {
                add(elem);
            }
        }
    }

    void xpathIfNotEmpty(std::string const& destination_xpath, ElementPtr const& source) {
        if (!source->empty()) {
            xpath(destination_xpath, source);
        }
    }

    void xpathCheckAndSet(std::string const& destination_key,
                          ElementPtr const& source,
                          std::string const& source_xpath) {
        ElementPtr const& value(source->xpath(source_xpath));
        if (value) {
            set(destination_key, value);
        }
    }

    void checkAndXpath(std::string const& destination_xpath,
                       ElementPtr const& source,
                       std::string const& source_key) {
        ElementPtr const& value(source->get(source_key));
        if (value) {
            xpath(destination_xpath, value);
        }
    }

    void xpathCheckAndXpath(std::string const& destination_xpath,
                            ElementPtr const& source,
                            std::string const& source_xpath) {
        ElementPtr const& value(source->xpath(source_xpath));
        if (value) {
            xpath(destination_xpath, value);
        }
    }

    /// @brief Remove all empty maps and lists from this Element and all it's descendants.
    void removeEmptyRecursively();

    /// @brief Iterate through all descendants in DFS order and call function on them.
    template <typename function_t>
    static void forEach(ElementPtr const& it, function_t&& function) {
        function(it);
        if (it->type_ == list) {
            for (size_t i = 0; i < it->size(); ++i) {
                ElementPtr const& child(it->get(i));
                forEach(child, function);
            }
        } else if (it->type_ == map) {
            for (size_t i = 0; i < it->size(); ++i) {
                std::string const& key(it->get(i)->stringValue());
                ElementPtr const& child(it->get(key));
                forEach(child, function);
            }
        }
    }

    /// @brief Iterate through all descendants in BFS order and call function on them.
    template <typename function_t>
    static void forEachBFS(ElementPtr const& it, function_t&& function) {
        std::queue<ElementPtr> q;
        forEachBFSInternal(it, function, q);
    }

    /// @return the type of this element
    type_t getType() const {
        return (type_);
    }

    /// @brief Returns position where the data element's value starts in a
    /// configuration string.
    ///
    /// @warning The returned reference is valid as long as the object which
    /// created it lives.
    Position& getPosition() {
        return (position_);
    }

    /// Returns a string representing the Element and all its
    /// child elements; note that this is different from stringValue(),
    /// which only returns the single value of a StringElement
    ///
    /// The resulting string will contain the Element in JSON format.
    ///
    /// @return std::string containing the string representation
    std::string str() const;

    std::string toUnquotedString() const;

    /// Returns the wireformat for the Element and all its child
    /// elements.
    ///
    /// @return std::string containing the element in wire format
    std::string toWire() const;
    void toWire(std::ostream& out) const;

    /// @brief Add the position to a TypeError message
    /// should be used in place of isc_throw(TypeError, error)
#define throwTypeError(error)                                                                      \
    {                                                                                              \
        std::string msg_ = error;                                                                  \
        if ((position_.file_ != "") || (position_.line_ != 0) || (position_.pos_ != 0)) {          \
            msg_ += " in (" + position_.str() + ")";                                               \
        }                                                                                          \
        isc_throw(TypeError, msg_);                                                                \
    }

    /// @name pure virtuals, every derived class must implement these

    /// @return true if the other ElementPtr has the same type and value
    virtual bool equals(const Element& other) const = 0;

    /// Converts the Element to JSON format and appends it to
    /// the given stringstream.
    virtual void toJSON(std::ostream& ss) const = 0;

    /// @name Type-specific getters
    ///
    /// @brief These functions only
    /// work on their corresponding Element type. For all other
    /// types, a TypeError is thrown.
    /// If you want an exception-safe getter method, use
    /// getValue() below
    //@{
    virtual int64_t intValue() const {
        throwTypeError("intValue() called on non-integer Element of type " +
                       std::string(magic_enum::enum_name(type_)));
    }
    virtual double doubleValue() const {
        throwTypeError("doubleValue() called on non-double Element of type " +
                       std::string(magic_enum::enum_name(type_)));
    }
    virtual bool boolValue() const {
        throwTypeError("boolValue() called on non-Bool Element of type " +
                       std::string(magic_enum::enum_name(type_)));
    }
    virtual std::string stringValue() const {
        throwTypeError("stringValue() called on non-string Element of type " +
                       std::string(magic_enum::enum_name(type_)));
    }
    virtual const std::vector<ElementPtr>& listValue() const {
        // replace with real exception or empty vector?
        throwTypeError("listValue() called on non-list Element of type " +
                       std::string(magic_enum::enum_name(type_)));
    }
    virtual const std::map<std::string, ElementPtr>& mapValue() const {
        // replace with real exception or empty map?
        throwTypeError("mapValue() called on non-map Element of type " +
                       std::string(magic_enum::enum_name(type_)));
    }
    //@}

    /// @name Exception-safe getters
    ///
    /// @brief The getValue() functions return false if the given reference
    /// is of another type than the element contains
    /// By default it always returns false; the derived classes
    /// override the function for their type, copying their
    /// data to the given reference and returning true
    ///
    //@{
    virtual bool getValue(int64_t& t) const;
    virtual bool getValue(double& t) const;
    virtual bool getValue(bool& t) const;
    virtual bool getValue(std::string& t) const;
    virtual bool getValue(std::vector<ElementPtr>& t) const;
    virtual bool getValue(std::map<std::string, ElementPtr>& t) const;
    //@}

    ///
    /// @name Exception-safe setters.
    ///
    /// @brief Return false if the Element is not
    /// the right type. Set the value and return true if the Elements
    /// is of the correct type
    ///
    /// Notes: Read notes of IntElement definition about the use of
    ///        long long int, long int and int.
    //@{
    virtual bool setValue(const long long int v);
    bool setValue(const long int i) {
        return (setValue(static_cast<long long int>(i)));
    }
    bool setValue(const int i) {
        return (setValue(static_cast<long long int>(i)));
    }
    virtual bool setValue(const double v);
    virtual bool setValue(const bool t);
    virtual bool setValue(const std::string& v);
    virtual bool setValue(const std::vector<ElementPtr>& v);
    virtual bool setValue(const std::map<std::string, ElementPtr>& v);
    //@}

    // Other functions for specific subtypes

    /// @name ListElement functions
    ///
    /// @brief If the Element on which these functions are called are not
    /// an instance of ListElement, a TypeError exception is thrown.
    //@{
    /// Returns the ElementPtr at the given index. If the index is out
    /// of bounds, this function throws an std::out_of_range exception.
    /// @param i The position of the ElementPtr to return
    virtual ElementPtr get(const int i) const;

    /// Sets the ElementPtr at the given index. If the index is out
    /// of bounds, this function throws an std::out_of_range exception.
    /// @param i The position of the ElementPtr to set
    /// @param element The ElementPtr to set at the position
    virtual void set(const size_t i, ElementPtr element);

    /// @brief Use the @ref create() methods to set directly.
    template <typename T>
    void set(const size_t i, T candidate) {
        set(i, create(candidate));
    }

    /// Adds an ElementPtr to the list
    /// @param element The ElementPtr to add
    virtual void add(ElementPtr element);

    /// Removes the element at the given position. If the index is out
    /// of nothing happens.
    /// @param i The index of the element to remove.
    virtual void remove(const int i);

    /// Returns the number of elements in the list.
    virtual size_t size() const;

    /// Return true if there are no elements in the list.
    virtual bool empty() const;
    //@}

    /// @name MapElement functions
    ///
    /// @brief If the Element on which these functions are called are not
    /// an instance of MapElement, a TypeError exception is thrown.
    //@{
    /// Returns the ElementPtr at the given key
    /// @param name The key of the Element to return
    /// @return The ElementPtr at the given key, or null if not present
    virtual ElementPtr get(const std::string& name) const;

    /// Sets the ElementPtr at the given key
    /// @param name The key of the Element to set
    /// @param element The ElementPtr to set at the given key.
    virtual void set(const std::string& name, ElementPtr element);

    /// @brief Use the @ref create() methods to set directly.
    template <typename T>
    void set(const std::string& name, T candidate) {
        set(name, Element::create(candidate));
    }

    /// Remove the ElementPtr at the given key
    /// @param name The key of the Element to remove
    virtual void remove(const std::string& name);

    /// Checks if there is data at the given key
    /// @param name The key of the Element checked for existence
    /// @return true if there is data at the key, false if not.
    virtual bool contains(const std::string& name) const;
    virtual bool containsAll(std::vector<std::string> const&) const;
    virtual bool containsAny(std::vector<std::string> const&) const;

    /// Recursively finds any data at the given identifier. The
    /// identifier is a /-separated list of names of nested maps, with
    /// the last name being the leaf that is returned.
    ///
    /// For instance, if you have a MapElement that contains another
    /// MapElement at the key "foo", and that second MapElement contains
    /// Another Element at key "bar", the identifier for that last
    /// element from the first is "foo/bar".
    ///
    /// @param identifier The identifier of the element to find
    /// @return The ElementPtr at the given identifier. Returns a
    /// null ElementPtr if it is not found, which can be checked with
    /// Element::is_null(ElementPtr e).
    virtual ElementPtr find(const std::string& identifier) const;

    /// See @c Element::find()
    /// @param identifier The identifier of the element to find
    /// @param t Reference to store the resulting ElementPtr, if found.
    /// @return true if the element was found, false if not.
    virtual bool find(const std::string& identifier, ElementPtr& t) const;
    //@}

    /// @name Factory functions

    // TODO: should we move all factory functions to a different class
    // so as not to burden the Element base with too many functions?
    // and/or perhaps even to a separate header?

    /// @name Direct factory functions
    /// @brief These functions simply wrap the given data directly
    /// in an Element object, and return a reference to it, in the form
    /// of an @c ElementPtr.
    /// These factory functions are exception-free (unless there is
    /// no memory available, in which case bad_alloc is raised by the
    /// underlying system).
    /// (Note that that is different from an NullElement, which
    /// represents an empty value, and is created with Element::create())
    ///
    /// Notes: Read notes of IntElement definition about the use of
    ///        long long int, long int and int.
    //@{
    static ElementPtr create(const Position& pos = ZERO_POSITION());
    static ElementPtr create(const int i, const Position& pos = ZERO_POSITION());
    static ElementPtr create(const long int i, const Position& pos = ZERO_POSITION());
    static ElementPtr create(const long long int i, const Position& pos = ZERO_POSITION());
    static ElementPtr create(const unsigned int i, const Position& pos = ZERO_POSITION());
    static ElementPtr create(const unsigned long int i, const Position& pos = ZERO_POSITION());
    static ElementPtr create(const unsigned long long int i, const Position& pos = ZERO_POSITION());
    static ElementPtr create(const double d, const Position& pos = ZERO_POSITION());
    static ElementPtr create(const bool b, const Position& pos = ZERO_POSITION());
    static ElementPtr create(const std::string& s, const Position& pos = ZERO_POSITION());
    // need both std:string and char *, since c++ will match
    // bool before std::string when you pass it a char *
    static ElementPtr create(const char* s, const Position& pos = ZERO_POSITION());

    /// @brief Creates an empty ListElement type ElementPtr.
    ///
    /// @param pos A structure holding position of the data element value
    /// in the configuration string. It is used for error logging purposes.
    static ElementPtr createList(const Position& pos = ZERO_POSITION());

    /// @brief Creates an empty MapElement type ElementPtr.
    ///
    /// @param pos A structure holding position of the data element value
    /// in the configuration string. It is used for error logging purposes.
    static ElementPtr createMap(const Position& pos = ZERO_POSITION());
    //@}

    /// @name Compound factory functions

    /// @brief These functions will parse the given string (JSON)
    /// representation  of a compound element. If there is a parse
    /// error, an exception of the type isc::data::JSONError is thrown.

    //@{
    /// Creates an Element from the given JSON string
    /// @param in The string to parse the element from
    /// @param preproc specified whether preprocessing (e.g. comment removal)
    ///                should be performed
    /// @return An ElementPtr that contains the element(s) specified
    /// in the given string.
    static ElementPtr fromJSON(const std::string& in, bool preproc = false);

    /// Creates an Element from the given input stream containing JSON
    /// formatted data.
    ///
    /// @param in The string to parse the element from
    /// @param preproc specified whether preprocessing (e.g. comment removal)
    ///                should be performed
    /// @throw JSONError
    /// @return An ElementPtr that contains the element(s) specified
    /// in the given input stream.
    static ElementPtr fromJSON(std::istream& in, bool preproc = false);

    /// Creates an Element from the given input stream containing JSON
    /// formatted data.
    ///
    /// @param in The string to parse the element from
    /// @param file_name specified input file name (used in error reporting)
    /// @param preproc specified whether preprocessing (e.g. comment removal)
    ///                should be performed
    /// @throw JSONError
    /// @return An ElementPtr that contains the element(s) specified
    /// in the given input stream.
    /// @throw JSONError
    static ElementPtr
    fromJSON(std::istream& in, const std::string& file_name, bool preproc = false);

    /// Creates an Element from the given input stream, where we keep
    /// track of the location in the stream for error reporting.
    ///
    /// @param in The string to parse the element from.
    /// @param file The input file name.
    /// @param line A reference to the int where the function keeps
    /// track of the current line.
    /// @param pos A reference to the int where the function keeps
    /// track of the current position within the current line.
    /// @throw JSONError
    /// @return An ElementPtr that contains the element(s) specified
    /// in the given input stream.
    // make this one private?
    /// @throw JSONError
    static ElementPtr fromJSON(std::istream& in, const std::string& file, int& line, int& pos);

    static ElementPtr fromJSONFallbackToString(std::string const& input,
                                               bool const preproc = false);

    /// Reads contents of specified file and interprets it as JSON.
    ///
    /// @param file_name name of the file to read
    /// @param preproc specified whether preprocessing (e.g. comment removal)
    ///                should be performed
    /// @return An ElementPtr that contains the element(s) specified
    /// if the given file.
    static ElementPtr fromJSONFile(const std::string& file_name, bool preproc = false);
    //@}

    static ElementPtr fromJSONFileExceptionSafe(std::string const& file_name,
                                                bool const preproc = false);

    /// @name Type name conversion functions

    /// Returns the name of the given type as a string
    ///
    /// @param type The type to return the name of
    /// @return The name of the type, or "unknown" if the type
    ///         is not known.
    static std::string typeToName(Element::type_t type);

    /// Converts the string to the corresponding type
    /// Throws a TypeError if the name is unknown.
    ///
    /// @param type_name The name to get the type of
    /// @return the corresponding type value
    static Element::type_t nameToType(const std::string& type_name);

    /// @brief input text preprocessor
    ///
    /// This method performs preprocessing of the input stream (which is
    /// expected to contain a text version of to be parsed JSON). For now the
    /// sole supported operation is bash-style (line starting with #) comment
    /// removal, but it will be extended later to cover more cases (C, C++ style
    /// comments, file inclusions, maybe macro replacements?).
    ///
    /// This method processes the whole input stream. It reads all contents of
    /// the input stream, filters the content and returns the result in a
    /// different stream.
    ///
    /// @param in input stream to be preprocessed
    /// @param out output stream (filtered content will be written here)
    static void preprocess(std::istream& in, std::stringstream& out);

    /// @name Wire format factory functions

    /// These function pparse the wireformat at the given stringstream
    /// (of the given length). If there is a parse error an exception
    /// of the type isc::cc::DecodeError is raised.

    //@{
    /// Creates an Element from the wire format in the given
    /// stringstream of the given length.
    /// Since the wire format is JSON, this is the same as
    /// fromJSON, and could be removed.
    ///
    /// @param in The input stringstream.
    /// @param length The length of the wireformat data in the stream
    /// @return ElementPtr with the data that is parsed.
    static ElementPtr fromWire(std::stringstream& in, int length);

    /// Creates an Element from the wire format in the given string
    /// Since the wire format is JSON, this is the same as
    /// fromJSON, and could be removed.
    ///
    /// @param s The input string
    /// @return ElementPtr with the data that is parsed.
    static ElementPtr fromWire(const std::string& s);
    //@}

    /// @brief parent
    ElementPtr parent_;

    /// @brief absolute xpath including model name
    std::string xpath_;

    /// @brief key through which this Element is reached from a parent map
    std::string key_;

protected:
    /// @brief Constructor.
    ///
    /// @param t Element type.
    /// @param pos Structure holding position of the value of the data element.
    /// It comprises the line number and the position within this line. The values
    /// held in this structure are used for error logging purposes.
    Element(type_t t, const Position& pos = ZERO_POSITION())
        : parent_(nullptr), type_(t), position_(pos) {
    }

private:
    enum operation_t { GET, SET };
    size_t xpath(std::string xpath,
                 std::string const& full_xpath,
                 ElementPtr& element,
                 operation_t const& operation,
                 bool const is_leaf_list,
                 bool const trim_prefixes);

    size_t xpath_leaf_list(std::string const& key, ElementPtr const& value);

    template <typename function_t>
    static void
    forEachBFSInternal(ElementPtr const& it, function_t&& function, std::queue<ElementPtr>& q) {
        // Visit.
        function(it);

        // Enqueue.
        if (it->type_ == list) {
            for (size_t i = 0; i < it->size(); ++i) {
                ElementPtr const& child(it->get(i));
                q.emplace(child);
            }
        } else if (it->type_ == map) {
            for (size_t i = 0; i < it->size(); ++i) {
                std::string const& key(it->get(i)->stringValue());
                ElementPtr const& child(it->get(key));
                q.emplace(child);
            }
        }

        // Dequeue.
        while (!q.empty()) {
            ElementPtr const e(q.front());
            q.pop();
            forEachBFSInternal(e, function, q);
        }
    }

    // technically the type could be omitted; is it useful?
    // should we remove it or replace it with a pure virtual
    // function getType?
    type_t type_;

    /// @brief Position of the element in the configuration string.
    Position position_;

    /// @brief non-copyable
    /// @{
    Element(Element const&) = delete;
    Element& operator=(Element const&) = delete;
    /// @}
};

/// Notes: IntElement type is changed to int64_t.
///        Due to C++ problems on overloading and automatic type conversion,
///          (C++ tries to convert integer type values and reference/pointer
///           if value types do not match exactly)
///        We decided the storage as int64_t,
///           three (long long, long, int) override function definitions
///           and cast int/long/long long to int64_t via long long.
///        Therefore, call by value methods (create, setValue) have three
///        (int,long,long long) definitions. Others use int64_t.
///
class IntElement : public Element {
    int64_t i;

private:
public:
    IntElement(int64_t v, const Position& pos = ZERO_POSITION()) : Element(integer, pos), i(v) {
    }
    int64_t intValue() const {
        return (i);
    }
    using Element::getValue;
    bool getValue(int64_t& t) const {
        t = i;
        return (true);
    }
    using Element::setValue;
    bool setValue(long long int v) {
        i = v;
        return (true);
    }
    void toJSON(std::ostream& ss) const;
    bool equals(const Element& other) const;
};

class DoubleElement : public Element {
    double d;

public:
    DoubleElement(double v, const Position& pos = ZERO_POSITION()) : Element(real, pos), d(v) {
    }
    double doubleValue() const {
        return (d);
    }
    using Element::getValue;
    bool getValue(double& t) const {
        t = d;
        return (true);
    }
    using Element::setValue;
    bool setValue(const double v) {
        d = v;
        return (true);
    }
    void toJSON(std::ostream& ss) const;
    bool equals(const Element& other) const;
};

class BoolElement : public Element {
    bool b;

public:
    BoolElement(const bool v, const Position& pos = ZERO_POSITION()) : Element(boolean, pos), b(v) {
    }
    bool boolValue() const {
        return (b);
    }
    using Element::getValue;
    bool getValue(bool& t) const {
        t = b;
        return (true);
    }
    using Element::setValue;
    bool setValue(const bool v) {
        b = v;
        return (true);
    }
    void toJSON(std::ostream& ss) const;
    bool equals(const Element& other) const;
};

class NullElement : public Element {
public:
    NullElement(const Position& pos = ZERO_POSITION()) : Element(null, pos) {
    }
    void toJSON(std::ostream& ss) const;
    bool equals(const Element& other) const;
};

class StringElement : public Element {
    std::string s;

public:
    StringElement(std::string v, const Position& pos = ZERO_POSITION())
        : Element(string, pos), s(v) {
    }
    std::string stringValue() const {
        return (s);
    }
    using Element::getValue;
    bool getValue(std::string& t) const {
        t = s;
        return (true);
    }
    using Element::setValue;
    bool setValue(const std::string& v) {
        s = v;
        return (true);
    }
    void toJSON(std::ostream& ss) const;
    bool equals(const Element& other) const;
};

class ListElement : public Element {
    std::vector<ElementPtr> l;

public:
    ListElement(const Position& pos = ZERO_POSITION()) : Element(list, pos) {
    }
    const std::vector<ElementPtr>& listValue() const {
        return (l);
    }
    using Element::getValue;
    bool getValue(std::vector<ElementPtr>& t) const {
        t = l;
        return (true);
    }
    using Element::setValue;
    bool setValue(const std::vector<ElementPtr>& v) {
        l = v;
        return (true);
    }
    using Element::get;
    ElementPtr get(int i) const {
        return (l.at(i));
    }
    using Element::set;

    void set(size_t i, ElementPtr e) {
        l.at(i)->parent_ = nullptr;
        e->parent_ = shared_from_this();
        l.at(i) = e;
    }

    void add(ElementPtr e) {
        e->parent_ = shared_from_this();
        l.push_back(e);
    }
    using Element::remove;
    void remove(int i) {
        std::vector<ElementPtr>::const_iterator const it(l.begin() + i);
        (*it)->parent_ = nullptr;
        l.erase(it);
    }
    void toJSON(std::ostream& ss) const;
    size_t size() const {
        return (l.size());
    }
    bool empty() const {
        return (l.empty());
    }
    bool equals(const Element& other) const;

    /// @brief Set xpath to child only if the current list is a leaf-list.
    void setXpathToChild(ElementPtr const& child);
};

class MapElement : public Element {
    std::map<std::string, ElementPtr> m;

public:
    MapElement(const Position& pos = ZERO_POSITION()) : Element(map, pos) {
    }
    // @todo should we have direct iterators instead of exposing the std::map
    // here?
    const std::map<std::string, ElementPtr>& mapValue() const {
        return (m);
    }
    using Element::getValue;
    bool getValue(std::map<std::string, ElementPtr>& t) const {
        t = m;
        return (true);
    }
    using Element::setValue;
    bool setValue(const std::map<std::string, ElementPtr>& v) {
        m = v;
        return (true);
    }
    using Element::get;
    /// @brief Careful, this returns key, not value as one might expect.
    ElementPtr get(int i) const {
        auto it(m.begin());
        std::advance(it, i);
        return std::make_shared<StringElement>(it->first);
    }
    ElementPtr get(const std::string& s) const {
        auto const& found(m.find(s));
        return (found != m.end() ? found->second : ElementPtr());
    }
    using Element::set;

    void set(std::string const& key, ElementPtr child) {
        if (child) {
            child->parent_ = shared_from_this();
            child->key_ = key;
        }
        m.insert_or_assign(key, child);
    }

    using Element::remove;
    void remove(const std::string& s) {
        m.at(s)->parent_ = nullptr;
        m.erase(s);
    }
    void remove(int i) {
        std::string const& key(get(i)->stringValue());
        remove(key);
    }
    bool contains(const std::string& s) const {
        return (m.find(s) != m.end());
    }
    bool containsAll(std::vector<std::string> const& v) const {
        for (std::string const& s : v) {
            if (!contains(s)) {
                return false;
            }
        }
        return true;
    }
    bool containsAny(std::vector<std::string> const& v) const {
        for (std::string const& s : v) {
            if (contains(s)) {
                return true;
            }
        }
        return false;
    }
    void toJSON(std::ostream& ss) const;

    // we should name the two finds better...
    // find the element at id; raises TypeError if one of the
    // elements at path except the one we're looking for is not a
    // mapelement.
    // returns an empty element if the item could not be found
    ElementPtr find(const std::string& id) const;

    // find the Element at 'id', and store the element pointer in t
    // returns true if found, or false if not found (either because
    // it doesn't exist or one of the elements in the path is not
    // a MapElement)
    bool find(const std::string& id, ElementPtr& t) const;

    /// @brief Returns number of stored elements
    ///
    /// @return number of elements.
    size_t size() const {
        return (m.size());
    }

    bool equals(const Element& other) const;

    bool empty() const {
        return (m.empty());
    }

    /// @brief Set xpath for current element and all direct children by taking into consideration
    /// information from the parent if it is a list and information from it's direct children which
    /// could be keys.
    void setXpathFromParentAndChild(
        std::function<std::vector<std::string>(std::string const& xpath)> keys);
};

/// @brief Create a new ElementPtr from the first ElementPtr, removing all
/// values that are equal in the second. Both ElementPtrs MUST be MapElements.
/// The returned ElementPtr will be a MapElement that only contains new and
/// changed values (for ModuleCCSession and configuration update handlers).
/// Raises a TypeError if a or b are not MapElements
ElementPtr removeIdentical(ElementPtr a, ElementPtr b);

/// @brief Merges the data from other into element.
/// (on the first level). Both elements must be
/// MapElements.
/// Every string,value pair in other is copied into element
/// (the ElementPtr of value is copied, this is not a new object)
/// Unless the value is a NullElement, in which case the
/// key is removed from element, rather than setting the value to
/// the given NullElement.
/// This way, we can remove values from for instance maps with
/// configuration data (which would then result in reverting back
/// to the default).
/// Raises a TypeError if either ElementPtr is not a MapElement
void merge(ElementPtr element, ElementPtr other);

/// @brief Copy the data up to a nesting level.
///
/// The copy is a deep copy so nothing is shared if it is not
/// under the given nesting level.
///
/// @param from the pointer to the element to copy
/// @param level nesting level (default is 100, 0 means shallow copy,
/// negative means outbound and perhaps looping forever).
/// @return a pointer to a fresh copy
/// @throw raises a BadValue is a null pointer occurs.
ElementPtr copy(ElementPtr from, int level = 100);

/// @brief Compares the data with other using unordered lists
///
/// This comparison function handles lists (JSON arrays) as
/// unordered multi sets (multi means an item can occurs more
/// than once as soon as it occurs the same number of times).
bool isEquivalent(ElementPtr a, ElementPtr b, uint8_t level = 100);

bool diff(ElementPtr a,
          ElementPtr b,
          std::vector<std::tuple<std::string, ElementPtr, ElementPtr>>& diffs,
          std::string const& xpath,
          uint8_t level = 128,
          bool track_changes = true);

std::string parseDiff(std::vector<std::tuple<std::string, ElementPtr, ElementPtr>> const& diffs);

/// @brief Pretty prints the data into stream.
///
/// This operator converts the @c ElementPtr into a string and
/// inserts it into the output stream @c out with an initial
/// indentation @c indent and add at each level @c step spaces.
/// For maps if there is a comment property it is printed first.
///
/// @param element A @c ElementPtr to pretty print
/// @param out A @c std::ostream on which the print operation is performed
/// @param indent An initial number of spaces to add each new line
/// @param step A number of spaces to add to indentation at a new level
void prettyPrint(ElementPtr element, std::ostream& out, unsigned indent = 0, unsigned step = 2);

/// @brief Pretty prints the data into string
///
/// This operator converts the @c ElementPtr into a string with
/// an initial indentation @c indent and add at each level @c step spaces.
/// For maps if there is a comment property it is printed first.
///
/// @param element A @c ElementPtr to pretty print
/// @param indent An initial number of spaces to add each new line
/// @param step A number of spaces to add to indentation at a new level
/// @return a string where element was pretty printed
std::string prettyPrint(ElementPtr element, unsigned indent = 0, unsigned step = 2);

///
/// @brief Insert Element::Position as a string into stream.
///
/// This operator converts the @c Element::Position into a string and
/// inserts it into the output stream @c out.
///
/// @param out A @c std::ostream object on which the insertion operation is
/// performed.
/// @param pos The @c Element::Position structure to insert.
/// @return A reference to the same @c std::ostream object referenced by
/// parameter @c out after the insertion operation.
std::ostream& operator<<(std::ostream& out, const Element::Position& pos);

///
/// @brief Insert the Element as a string into stream.
///
/// This method converts the @c ElementPtr into a string with
/// @c Element::str() and inserts it into the
/// output stream @c out.
///
/// This function overloads the global operator<< to behave as described in
/// ostream::operator<< but applied to @c ElementPtr objects.
///
/// @param out A @c std::ostream object on which the insertion operation is
/// performed.
/// @param e The @c ElementPtr object to insert.
/// @return A reference to the same @c std::ostream object referenced by
/// parameter @c out after the insertion operation.
std::ostream& operator<<(std::ostream& out, const Element& e);

bool operator==(const Element& a, const Element& b);
bool operator!=(const Element& a, const Element& b);
}  // namespace data
}  // namespace isc
#endif  // ISC_DATA_H

// Local Variables:
// mode: c++
// End:

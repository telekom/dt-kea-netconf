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

#ifndef TRANSLATOR_UNIVERSAL_H
#define TRANSLATOR_UNIVERSAL_H

#include <cc/data.h>
#include <yang/translator.h>

namespace isc {
namespace yang {

/// @brief Translates between YANG and JSON translator for entire models.
struct TranslatorUniversal : TranslatorBasic {
    /// @brief Constructor
    ///
    /// @param session Sysrepo session
    /// @param model model name
    TranslatorUniversal(S_Session session, std::string const& model)
        : TranslatorBasic(session, model) {
    }

    /// @brief Destructor
    ~TranslatorUniversal() = default;

    /// @brief Delete data a certain xpath or the entire YANG data if missing.
    void del(std::string const& xpath = std::string());

    /// @brief Get the entire YANG data into an @ref ElementPtr.
    isc::data::ElementPtr get(std::string const& xpath = std::string());

    /// @brief Set the entire YANG data from an ElementPtr.
    void set(isc::data::ElementPtr const& input, std::string const& xpath = std::string());

private:
    void getInternal(isc::data::ElementPtr const& result, std::string const& xpath);

    /// @brief Get the value of a leaf or a leaf-list child into an @ref ElementPtr.
    isc::data::ElementPtr getLeafOrLeafList(S_Data_Node const& node);

    std::string getRootNodeFromXpath(std::string const& xpath);

    /// @brief Set a single node into a toplevel @ref ElementPtr via it's reported xpath in Sysrepo.
    void nodeIntoResult(isc::data::ElementPtr const& result, S_Data_Node const& node);

    void setInternal(isc::data::ElementPtr const& toplevel, std::string const& root_node);
};

}  // namespace yang
}  // namespace isc

#endif  // TRANSLATOR_UNIVERSAL_H

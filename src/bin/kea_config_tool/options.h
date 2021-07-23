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

#ifndef KEA_CONFIG_TOOL_OPTIONS_H
#define KEA_CONFIG_TOOL_OPTIONS_H

#include <util/dhcp.h>
#include <util/types.h>

#include <set>
#include <string>
#include <unordered_map>

#include <boost/program_options.hpp>

namespace isc {
namespace kea_config_tool {

using subnet_list_t = std::unordered_map<std::string, std::vector<std::string>>;
using shard_set_t = std::set<std::string>;

/// @brief Object responsible for parsing the command-line and storing the
/// specified options
struct CommandOptions {
    CommandOptions(std::string const& bin_name, std::ostream& out);

    /// @brief Parses commands into members
    void parse(int const argc, char* const argv[]);

    /// @brief Access parameters
    boost::program_options::variable_value const& operator[](std::string const& parameter) const;

    /// @brief Count parameters
    int count(std::string const& parameter) const;

    /// @brief Print usage
    void usage();

    /// @brief returns Kea version.
    std::string getVersion();

    void showVersion();

    auto begin() {
        return options_.begin();
    }

    auto end() {
        return options_.end();
    }

    std::string kea_admin_parameters_;
    shard_set_t item_list_;
    subnet_list_t subnet_list_;

private:
    /// @brief non-copyable
    /// @{
    CommandOptions(CommandOptions const&) = delete;
    CommandOptions& operator=(CommandOptions const&) = delete;
    /// @}

    /// @brief Access parameters
    boost::program_options::variable_value const& get(std::string const& parameter) const;

    /// @brief parsers
    /// @{
    void parseDbInit(int const argc, char* const argv[]);
    void parseMaster();
    void parseShard();
    void initializeArgumentParser(int const argc, char* const argv[]);
    void sanityChecks();
    /// @}

    /// @brief Print usages.
    /// @{
    void usageGeneric();
    void usageMaster();
    void usageShard();
    /// @}

    std::string bin_name_;
    std::ostream& out_;
    boost::program_options::variables_map options_;
};

}  // namespace kea_config_tool
}  // namespace isc

#endif  // KEA_CONFIG_TOOL_OPTIONS_H

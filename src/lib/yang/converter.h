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

#ifndef YANG_CONVERTER_H
#define YANG_CONVERTER_H

#include <cc/data.h>
#include <util/func.h>
#include <util/functional.h>
#include <util/log.h>
#include <yang/converter-table.h>
#include <yang/yang_models.h>

#include <functional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace isc {
namespace yang {

enum YangFlavor {
    IETF,
    Kea,
};

/// @brief Identifiers for the two directions where conversion is possible:
///     - ietf-dhcpv6-server to kea-dhcp6-server-ietf-aware
///     - kea-dhcp6-server-ietf-aware to ietf-dhcpv6-server
/// @{
using convertion_direction_t = std::size_t;
static constexpr convertion_direction_t IETF_to_Kea = 0;
static constexpr convertion_direction_t Kea_to_IETF = 1;
/// @}

/// @brief High-level abstraction to be used in kea-netconf, converts between modules.
template <convertion_direction_t D>
struct Converter {
    /// @brief Asserts that the input to be given to the conversion with this direction is valid.
    static void ascertain(isc::data::ElementPtr const& config);

    /// @brief Convert a single node found at given xpath.
    static void convert(isc::data::ElementPtr const& output,
                        isc::data::ElementPtr const& input,
                        std::string const& source_xpath) {
        // What transformation needs to be applied and where?
        auto const& [destination_xpath, transformations](map().at(source_xpath));

        // Get source node.
        isc::data::ElementPtr e(input->xpath(source_xpath, /* trim_prefixes = */ false));

        // No source node - no conversion
        if (!e) {
            isc::log::Spdlog::debug("{}: no node found at xpath '{}'", PRETTY_FUNCTION_NAME(),
                                    source_xpath);
            return;
        }

        // Copy source node.
        e = isc::data::copy(e);

        // Apply transformations.
        bool should_copy(false);
        for (transform_t const t : transformations) {
            if (t == copy) {
                should_copy = true;
            }
            e = transform().at(t)(e, input, output);
        }

        // Set converted node.
        if (should_copy) {
            output->xpath(destination_xpath, e, /* is_leaf_list = */ false,
                          /* trim_prefixes = */ false);
        }
    }

    /// @brief Convert the entire data of a module.
    static void convertAll(isc::data::ElementPtr const& output,
                           isc::data::ElementPtr const& input) {
        ascertain(input);

        // For all xpaths...
        for (tuple_t const& t : Table::table()) {
            // Convert.
            convert(output, input, std::get<D>(t));
        }
    }

    static void convertAll(isc::data::ElementPtr& io, std::string const& model) {
        if (model != IETF_DHCPV6_SERVER) {
            return;
        }
        isc::data::ElementPtr const output(isc::data::Element::createMap());
        convertAll(output, io);
        io = output;
    }

    static constexpr YangFlavor sourceFlavor();

    static constexpr YangFlavor targetFlavor();

private:
    /// @brief Destination xpath and list of transformations, indexed by source xpath
    using map_t =
        std::unordered_map<std::string, std::tuple<std::string, std::vector<transform_t>>>;

    /// @brief Transformation functions indexed by transformation identifiers
    using transform_table_t =
        std::unordered_map<transform_t,
                           std::function<isc::data::ElementPtr(isc::data::ElementPtr const&,
                                                               isc::data::ElementPtr const&,
                                                               isc::data::ElementPtr const&)>>;

    /// @brief Transformation functions for the option-set node indexed by option name
    using option_set_table_t =
        std::unordered_map<std::string,
                           std::function<isc::data::ElementPtr(isc::data::ElementPtr const&)>>;

    /// @brief Destination xpath and list of transformations, indexed by source xpath
    static map_t& map() {
        static map_t& _([]() -> map_t& {
            static map_t m;
            for (tuple_t const& t : Table::table()) {
                m.emplace(std::get<D>(t),  // source xpath
                          std::make_tuple(std::get<((D + 1) % 2)>(t),  // destination xpath
                                          std::get<2>(t)  // transformation
                                          ));
            }
            return m;
        }());
        return _;
    }

    /// @brief Transformation functions indexed by transformation identifiers, functions can operate
    ///     on the input as it is copied from it's source.
    static transform_table_t& transform();

    /// @brief Transformation functions specific to option-sets indexed by option-set name,
    /// functions return option-data structure from the Kea model alog with option-def
    /// if the isRootNode is true
    static option_set_table_t& transformOptionSets();
};

template <>
constexpr YangFlavor Converter<IETF_to_Kea>::sourceFlavor() {
    return IETF;
}

template <>
constexpr YangFlavor Converter<Kea_to_IETF>::sourceFlavor() {
    return Kea;
}

template <>
constexpr YangFlavor Converter<IETF_to_Kea>::targetFlavor() {
    return Kea;
}

template <>
constexpr YangFlavor Converter<Kea_to_IETF>::targetFlavor() {
    return IETF;
}

template <YangFlavor Y>
struct CounterpartModel {
    static std::string get(std::string const& model);
};

}  // namespace yang
}  // namespace isc

#endif  // YANG_CONVERTER_H

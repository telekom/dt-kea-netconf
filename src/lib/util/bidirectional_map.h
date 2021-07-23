/* Boost.MultiIndex example of a bidirectional map.
 *
 * Copyright 2003-2009 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#ifndef UTIL_BIDIRECTIONAL_MAP_H
#define UTIL_BIDIRECTIONAL_MAP_H

namespace isc {
namespace util {

#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

/* The class template bidirectional_map wraps the specification
 * of a bidirectional map based on multi_index_container.
 */
template <typename one_t, typename two_t, typename one_tag, typename two_tag>
struct bidirectional_map {
    struct value_type {
        value_type(const one_t& first_, const two_t& second_) : first(first_), second(second_) {
        }

        one_t first;
        two_t second;
    };

    /* A bidirectional map can be simulated as a multi_index_container
     * of pairs of (FromType,ToType) with two unique indices, one
     * for each member of the pair.
     */
    using type = boost::multi_index_container<
        value_type,
        indexed_by<ordered_unique<tag<one_tag>, member<value_type, one_t, &value_type::first>>,
                   ordered_unique<tag<two_tag>, member<value_type, two_t, &value_type::second>>>>;
};

}  // namespace util
}  // namespace isc

#endif  // UTIL_BIDIRECTIONAL_MAP_H
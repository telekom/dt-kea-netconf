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

#ifndef UTIL_FUNCTIONAL_H
#define UTIL_FUNCTIONAL_H

#include <log/macros.h>
#include <util/log.h>

#include <spdlog/spdlog.h>

template <typename functor_t>
void logExceptions(std::string const& context_message,
                   functor_t functor) {
    try {
        functor();
    } catch (std::exception const& exception) {
        spdlog::error(context_message + ": " + exception.what());
    } catch (...) {
        spdlog::error(context_message + ": something went extremely wrong");
        abort();
    }
}

template <typename functor_t>
void logExceptions(isc::log::Logger& logger,
                   isc::log::MessageID const& error_message,
                   std::string const& context_message,
                   functor_t functor) {
    try {
        functor();
    } catch (std::exception const& exception) {
        LOG_ERROR(logger, error_message).arg(context_message + ": " + exception.what());
    } catch (...) {
        LOG_ERROR(logger, error_message).arg(context_message + ": something went extremely wrong");
        abort();
    }
}

template <typename functor_t>
std::function<void()> exceptionLoggedBind(std::string const& context_message,
                                          functor_t functor) {
    return [=] { logExceptions(context_message, functor); };
}

template <typename functor_t>
std::function<void()> exceptionLoggedBind(isc::log::Logger& logger,
                                          isc::log::MessageID const& error_message,
                                          std::string const& context_message,
                                          functor_t functor) {
    return [=, &logger] { logExceptions(logger, error_message, context_message, functor); };
}

template <typename T>
T orElse(T value, T else_value) {
    if (value) {
        return value;
    }
    return else_value;
}

#endif  // UTIL_FUNCTIONAL_H

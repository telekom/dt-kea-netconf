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

#ifndef KEA_LIB_DHCPSRV_NOTIFICATIONS_MANAGER_H
#define KEA_LIB_DHCPSRV_NOTIFICATIONS_MANAGER_H

#include <cc/command_interpreter.h>
#include <cc/data.h>
#include <config/command_mgr.h>
#include <database/timestamp_store.h>
#include <dhcpsrv/cfgmgr.h>
#include <util/dt_thread_pool.h>
#include <util/functional.h>

namespace isc {
namespace dhcp {

struct NotificationsManager {
    using timestamp_t = isc::db::timestamp_t;

    NotificationsManager(isc::data::ElementPtr const& socket_credentials =
                             CfgMgr::instance().getCurrentCfg()->getNotificationsSocketInfo()) {
        if (!socket_credentials) {
            return;
        }
        isc::data::ElementPtr const& socket_type_element(socket_credentials->get("socket-type"));
        if (socket_type_element) {
            std::string const& socket_type(socket_type_element->stringValue());
            if (socket_type != "unix") {
                isc_throw(BadValue, PRETTY_METHOD_NAME() << ": " << socket_type
                                                         << " not supported for notifications");
            }
        }

        // No socket-type is allowed.
        isc::data::ElementPtr const& socket_name_element(socket_credentials->get("socket-name"));
        if (socket_name_element) {
            socket_path_ = socket_name_element->stringValue();
        }
    }

    isc::data::ElementPtr await(timestamp_t const command_ID) {
        if (command_ID == 0) {
            // Send failed.
            return nullptr;
        }
        std::unique_lock<std::mutex> lock(mutex_);
        if (!responses_.contains(command_ID)) {
            cv_.wait(lock, [&] { return responses_.contains(command_ID); });
        }
        isc::data::ElementPtr const response(responses_.at(command_ID));
        responses_.erase(command_ID);
        return response;
    }

    bool check(timestamp_t const command_ID) {
        std::lock_guard<std::mutex> lock(mutex_);
        return responses_.contains(command_ID);
    }

    timestamp_t notify(std::string const& notification) {
        return notify(notification, "unused");
    }

    template <typename T>
    timestamp_t notify(std::string const& notification, T const& data) {
        if (socket_path_.empty()) {
            return 0;
        }

        // Send command asynchronously.
        timestamp_t const command_ID(isc::db::TimestampStore::now());
        std::thread(exceptionLoggedBind(PRETTY_METHOD_NAME(), [=, this] {
            isc::data::ElementPtr const& command(isc::data::Element::createMap());
            command->set(isc::config::CONTROL_COMMAND, notification);
            command->set(isc::config::CONTROL_ARGUMENTS, data);
            isc::data::ElementPtr const& response(
                isc::config::CommandMgr::sendCommand(socket_path_, command));
            std::lock_guard<std::mutex> lock(mutex_);
            responses_.insert_or_assign(command_ID, response);
            cv_.notify_all();
        })).detach();

        return command_ID;
    }

    isc::data::ElementPtr notifySynchronous(std::string const& notification) {
        return notifySynchronous(notification, "unused");
    }

    template <typename T>
    isc::data::ElementPtr notifySynchronous(std::string const& notification, T const& data) {
        isc::data::ElementPtr const& command(isc::data::Element::createMap());
        command->set(isc::config::CONTROL_COMMAND, notification);
        command->set(isc::config::CONTROL_ARGUMENTS, data);
        isc::data::ElementPtr const& response(
            isc::config::CommandMgr::sendCommand(socket_path_, command));
        return response;
    }

private:
    std::string socket_path_;
    std::unordered_map<timestamp_t, isc::data::ElementPtr> responses_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

}  // namespace dhcp
}  // namespace isc

#endif  // KEA_LIB_DHCPSRV_NOTIFICATIONS_MANAGER_H

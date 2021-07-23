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

#ifndef CONTROL_SOCKET_FACTORY_H
#define CONTROL_SOCKET_FACTORY_H

#include <memory>

#include <netconf/control_socket.h>
#include <netconf/http_control_socket.h>
#include <netconf/kea_config_tool_control.h>
#include <netconf/stdout_control_socket.h>
#include <netconf/unix_control_socket.h>
#include <yang/yang_models.h>

namespace isc {
namespace netconf {

/// @brief Factory function for control sockets.
///
/// @param ctrl_sock The control socket configuration.
/// @return A pointer to a control socket communication object.
/// @throw BadValue if called with null or an unknown type.
template <isc::dhcp::DhcpSpaceType D>
struct ControlSocketFactory {
    static ControlSocketBasePtr<D> create(CfgSubscription const& subscription,
                                          std::string const& model) {
        // Get the control socket's details.
        CfgControlSocketPtr const& control_socket(subscription.control_socket_);

        // Effective factory
        CfgControlSocket::Type const socket_type(control_socket->getType());
        switch (socket_type) {
        case CfgControlSocket::Type::HTTP:
            return std::make_shared<HttpControlSocket<D>>(control_socket);
        case CfgControlSocket::Type::KEA_CONFIG_TOOL:
            if (isc::yang::Is::Ietf(model)) {
                return std::make_shared<
                    KeaConfigToolControl<D, isc::dhcp::ConfigurationConstants::HAS_IETF>>(
                    control_socket);
            } else {
                return std::make_shared<
                    KeaConfigToolControl<D, isc::dhcp::ConfigurationConstants::NOT_IETF>>(
                    control_socket);
            }
        case CfgControlSocket::Type::STDOUT:
            return std::make_shared<StdoutControlSocket<D>>(control_socket);
        case CfgControlSocket::Type::UNIX:
            return std::make_shared<UnixControlSocket<D>>(control_socket);
        default:
            isc_throw(BadValue, PRETTY_FUNCTION_NAME()
                                    << ": unknown control socket type '"
                                    << magic_enum::enum_name(socket_type) << "'");
        }
    }
};

}  // namespace netconf
}  // namespace isc

#endif  // CONTROL_SOCKET_FACTORY_H

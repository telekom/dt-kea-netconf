// Copyright (C) 2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file http_control_socket.h
/// Contains declarations for HTTP control socket communication.

#ifndef HTTP_CONTROL_SOCKET_H
#define HTTP_CONTROL_SOCKET_H

#include <cc/command_interpreter.h>
#include <config/command_mgr.h>
#include <config/timeouts.h>
#include <http/client.h>
#include <http/post_request_json.h>
#include <http/response_json.h>

#include <netconf/control_socket.h>

namespace isc {
namespace netconf {

/// @brief Class for control socket communication over HTTP socket.
///
/// This class is the derived class for control socket communication
/// over HTTP sockets.
/// This class implements config-get, config-test, config-set and leases-get.
template <isc::dhcp::DhcpSpaceType D>
struct HttpControlSocket : ControlSocketBase<D> {
    /// @brief Constructor.
    ///
    /// @param ctrl_sock The control socket configuration.
    HttpControlSocket(CfgControlSocketPtr ctrl_sock) : ControlSocketBase<D>(ctrl_sock) {
    }

    /// @brief Get configuration.
    ///
    /// Call config-get over the control socket.
    ///
    /// @return The JSON element answer of config-get.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr
    configGet([[maybe_unused]] ChangeCollection const& collection,
              [[maybe_unused]] isc::data::ElementPtr const& sysrepo_config) override final {
        return sendCommand(isc::config::createCommand("config-get"));
    }

    /// @brief Set configuration.
    ///
    /// Call config-set over the control socket.
    ///
    /// @param config The configuration to set.
    /// @return The JSON element answer of config-set.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr
    configSet(isc::data::ElementPtr const& config,
              [[maybe_unused]] isc::data::ElementPtr const& db_config,
              [[maybe_unused]] isc::data::ElementPtr const& sysrepo_config,
              [[maybe_unused]] std::vector<change_result_t> const& change_results) override final {
        return sendCommand(isc::config::createCommand("config-set", config));
    }

    /// @brief Test configuration.
    ///
    /// Call config-test over the control socket.
    ///
    /// @param config The configuration to test.
    /// @return The JSON element answer of config-test.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr
    configTest(isc::data::ElementPtr const& config,
               [[maybe_unused]] isc::data::ElementPtr const& db_config,
               [[maybe_unused]] std::vector<change_result_t> const& change_results) override final {
        return sendCommand(isc::config::createCommand("config-test", config));
    }

    /// @brief Get leases.
    ///
    /// Call leases-get over the control socket.
    ///
    /// @param subnet_id The subnet-id for which leases are returned
    /// @return The JSON element answer of leases-get.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr leasesGet(uint32_t subnet_id) override final {
        isc::data::ElementPtr subnet_id_elem(isc::data::Element::createMap());
        subnet_id_elem->set("subnet-id", subnet_id);
        return sendCommand(isc::config::createCommand("leases-get", subnet_id_elem));
    }

    /// @brief Delete a lease.
    ///
    /// Call delete-lease over the control socket.
    ///
    /// @param type The lease type for the wanted lease
    /// @param addr The lease address for the wanted lease
    /// @return The JSON element answer of delete-lease.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr deleteLease(uint32_t type, std::string const& addr) override final {
        isc::data::ElementPtr address_elem(isc::data::Element::createMap());
        address_elem->set("type", type);
        address_elem->set("address", addr);
        return sendCommand(isc::config::createCommand("delete-lease", address_elem));
    }

    /// @brief Get statistic.
    ///
    /// Call statistic-get over the control socket.
    ///
    /// @param name Argument provided to the statistic-get command
    /// @return The JSON element answer of statistic-get.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr statisticGet(std::string name) override final {
        isc::data::ElementPtr name_elem(isc::data::Element::createMap());
        name_elem->set("name", name);
        return sendCommand(isc::config::createCommand("statistic-get", name_elem));
    }

    /// @brief Get resource.
    ///
    /// Call resource-get over the control socket.
    ///
    /// @param name Argument provided to the resource-get command
    /// @return The JSON element answer of resource-get.
    /// @throw ControlSocketError when a communication error occurs.
    isc::data::ElementPtr resourceGet(std::string resource) override final {
        isc::data::ElementPtr resource_elem(isc::data::Element::createMap());
        resource_elem->set("resource", resource);
        return sendCommand(isc::config::createCommand("resource-get", resource_elem));
    }

private:
    /// @brief Perform the actual communication.
    ///
    /// @todo Use persistent connections (vs. a new connection per call)
    /// as the HTTP library supports them.
    ///
    /// @param command The command to send.
    /// @return The answer.
    isc::data::ElementPtr sendCommand(isc::data::ElementPtr const& command) override final {
        isc::http::PostHttpRequestJsonPtr const& request(
            std::make_shared<isc::http::PostHttpRequestJson>(
                isc::http::HttpRequest::Method::HTTP_POST, "/", isc::http::HttpVersion(1, 1)));
        request->setBodyAsJson(command);
        try {
            request->finalize();
        } catch (const std::exception& ex) {
            isc_throw(isc::config::ControlSocketError, "failed to create request: " << ex.what());
        }

        isc::asiolink::IOServicePtr io_service(std::make_shared<isc::asiolink::IOService>());
        isc::http::HttpClient client(*io_service);
        boost::system::error_code received_ec;
        std::string receive_errmsg;
        isc::http::HttpResponseJsonPtr response(std::make_shared<isc::http::HttpResponseJson>());

        client.asyncSendRequest(
            ControlSocketBase<D>::getUrl(), request, response,
            [&io_service, &received_ec, &receive_errmsg](const boost::system::error_code& ec,
                                                         const isc::http::HttpResponsePtr&,
                                                         const std::string& errmsg) {
                // Capture error code and message.
                received_ec = ec;
                receive_errmsg = errmsg;
                // Got the IO service so stop IO service.
                // This causes to stop IO service when
                // all handlers have been invoked.
                io_service->stopWork();
            },
            isc::http::HttpClient::RequestTimeout(isc::config::TIMEOUT_AGENT_FORWARD_COMMAND));

        // Perform this synchronously.
        io_service->run();

        if (received_ec) {
            // Got an error code.
            isc_throw(isc::config::ControlSocketError,
                      "communication error (code): " << received_ec.message());
        }

        if (!receive_errmsg.empty()) {
            // Got an error message.
            isc_throw(isc::config::ControlSocketError,
                      "communication error (message): " << receive_errmsg);
        }

        if (!response) {
            // Failed to get the answer.
            isc_throw(isc::config::ControlSocketError, "empty response");
        }

        try {
            return response->getBodyAsJson();
        } catch (const std::exception& ex) {
            isc_throw(isc::config::ControlSocketError, "unparsable response: " << ex.what());
        }
    }
};

/// @brief Type definition for the pointer to the @c HttpControlSocket.
template <isc::dhcp::DhcpSpaceType D>
using HttpControlSocketPtr = std::shared_ptr<HttpControlSocket<D>>;

}  // namespace netconf
}  // namespace isc

#endif  // HTTP_CONTROL_SOCKET_H

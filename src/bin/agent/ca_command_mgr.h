// Copyright (C) 2017 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef CTRL_AGENT_COMMAND_MGR_H
#define CTRL_AGENT_COMMAND_MGR_H

#include <config/hooked_command_mgr.h>
#include <exceptions/exceptions.h>
#include <boost/noncopyable.hpp>

namespace isc {
namespace agent {

/// @brief Exception thrown when an error occurred during control command
/// forwarding.
class CommandForwardingError : public Exception {
public:
    CommandForwardingError(const char* file, size_t line, const char* what) :
        isc::Exception(file, line, what) { };
};

/// @brief Command Manager for Control Agent.
///
/// This is an implementation of the Command Manager within Control Agent.
/// In addition to the standard capabilities of the @ref HookedCommandMgr
/// it is also intended to forward commands to the respective Kea servers
/// when the command is not supported directly by the Control Agent.
///
/// The @ref CtrlAgentCommandMgr is implemented as a singleton. The commands
/// are registered using @c CtrlAgentCommandMgr::instance().registerCommand().
/// The @ref CtrlAgentResponseCreator uses the sole instance of the Command
/// Manager to handle incoming commands.
class CtrlAgentCommandMgr : public config::HookedCommandMgr,
                            public boost::noncopyable {
public:

    /// @brief Returns sole instance of the Command Manager.
    static CtrlAgentCommandMgr& instance();

    /// @brief Handles the command having a given name and arguments.
    ///
    /// This method extends the base implementation with the ability to forward
    /// commands to Kea servers.
    ///
    /// If the received command doesn't include 'service' parameter or this
    /// parameter is blank, the command is first handled by the attached hooks
    /// libraries, and if still unhandled, the Control Agent itself.
    ///
    /// If the non-blank 'service' parameter has been specified the hooks
    /// are executed. If the hooks process the command the result is returned
    /// to the controlling client. Otherwise, the command is forwarded to each
    /// Kea server listed in the 'service' parameter.
    ///
    /// @param cmd_name Command name.
    /// @param params Command arguments.
    /// @param original_cmd Original command being processed.
    ///
    /// @return Pointer to the const data element representing a list of
    /// responses to the command. If the command has been handled by the CA,
    /// this list includes one response.
    virtual isc::data::ElementPtr
    handleCommand(const std::string& cmd_name,
                  const isc::data::ElementPtr& params,
                  const isc::data::ElementPtr& original_cmd);

private:

    /// @brief Implements the logic for @ref CtrlAgentCommandMgr::handleCommand.
    ///
    /// All parameters are passed by value because they may be modified within
    /// the method.
    ///
    /// @param cmd_name Command name.
    /// @param params Command arguments.
    /// @param original_cmd Original command being processed.
    ///
    /// @return Pointer to the const data element representing a list of responses
    /// to the command or a single response (not wrapped in a list). The
    /// @ref CtrlAgentCommandMgr::handleCommand will wrap non-list value returned
    /// in a single element list.
    isc::data::ElementPtr
    handleCommandInternal(std::string cmd_name,
                          isc::data::ElementPtr params,
                          isc::data::ElementPtr original_cmd);

    /// @brief Tries to forward received control command to a specified server.
    ///
    /// @param service Contains name of the service where the command should be
    /// forwarded.
    /// @param cmd_name Command name.
    /// @param command Pointer to the object representing the forwarded command.
    ///
    /// @return Response to forwarded command.
    /// @throw CommandForwardingError when an error occurred during forwarding.
    isc::data::ElementPtr
    forwardCommand(const std::string& service, const std::string& cmd_name,
                   const isc::data::ElementPtr& command);

    /// @brief Private constructor.
    ///
    /// The instance should be created using @ref CtrlAgentCommandMgr::instance,
    /// thus the constructor is private.
    CtrlAgentCommandMgr();

};

} // end of namespace isc::agent
} // end of namespace isc

#endif

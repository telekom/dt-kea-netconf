// Copyright (C) 2009-2018 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <exceptions/exceptions.h>
#include <cc/command_interpreter.h>
#include <cc/data.h>
#include <string>
#include <set>

using namespace std;

using isc::data::Element;
using isc::data::ElementPtr;
using isc::data::JSONError;

namespace isc {
namespace config {

const char *CONTROL_COMMAND = "command";
const char *CONTROL_RESULT = "result";
const char *CONTROL_TEXT = "text";
const char *CONTROL_ARGUMENTS = "arguments";
const char *CONTROL_SERVICE = "service";

// Full version, with status, text and arguments
ElementPtr
createAnswer(const int status_code, const std::string& text,
             const ElementPtr& arg) {
    if (status_code != 0 && text.empty()) {
        isc_throw(CtrlChannelError, "Text has to be provided for status_code != 0");
    }

    ElementPtr answer = Element::createMap();
    ElementPtr result = Element::create(status_code);
    answer->set(CONTROL_RESULT, result);

    if (!text.empty()) {
        answer->set(CONTROL_TEXT, Element::create(text));
    }
    if (arg) {
        answer->set(CONTROL_ARGUMENTS, arg);
    }
    return (answer);
}

ElementPtr
createAnswer() {
    return (createAnswer(0, string(""), ElementPtr()));
}

ElementPtr
createAnswer(const int status_code, const std::string& text) {
    return (createAnswer(status_code, text, ElementPtr()));
}

ElementPtr
createAnswer(const int status_code, const ElementPtr& arg) {
    return (createAnswer(status_code, "", arg));
}

ElementPtr
parseAnswer(int &rcode, const ElementPtr& msg) {
    if (!msg) {
        isc_throw(CtrlChannelError, "No answer specified");
    }
    if (msg->getType() != Element::map) {
        isc_throw(CtrlChannelError,
                  "Invalid answer Element specified, expected map");
    }
    if (!msg->contains(CONTROL_RESULT)) {
        isc_throw(CtrlChannelError,
                  "Invalid answer specified, does not contain mandatory 'result'");
    }

    ElementPtr result = msg->get(CONTROL_RESULT);
    if (result->getType() != Element::integer) {
            isc_throw(CtrlChannelError,
                      "Result element in answer message is not a string");
    }

    rcode = result->intValue();

    // If there are arguments, return them.
    ElementPtr args = msg->get(CONTROL_ARGUMENTS);
    if (args) {
        return (args);
    }

    // There are no arguments, let's try to return just the text status
    return (msg->get(CONTROL_TEXT));
}

std::tuple<ElementPtr, control_result_t> parseAnswer(ElementPtr const& msg) {
    int status_code;
    ElementPtr const result(parseAnswer(status_code, msg));
    return std::make_tuple(result, control_result_t(status_code));
}

std::string
answerToText(const ElementPtr& msg) {
    if (!msg) {
        isc_throw(CtrlChannelError, "No answer specified");
    }
    if (msg->getType() != Element::map) {
        isc_throw(CtrlChannelError,
                  "Invalid answer Element specified, expected map");
    }
    if (!msg->contains(CONTROL_RESULT)) {
        isc_throw(CtrlChannelError,
                  "Invalid answer specified, does not contain mandatory 'result'");
    }

    ElementPtr result = msg->get(CONTROL_RESULT);
    if (result->getType() != Element::integer) {
            isc_throw(CtrlChannelError,
                      "Result element in answer message is not a string");
    }

    stringstream txt;
    int rcode = result->intValue();
    if (rcode == 0) {
        txt << "success(0)";
    } else {
        txt << "failure(" << rcode << ")";
    }

    // Was any text provided? If yes, include it.
    ElementPtr txt_elem = msg->get(CONTROL_TEXT);
    if (txt_elem) {
        txt << ", text=" << txt_elem->stringValue();
    }

    return (txt.str());
}

ElementPtr
createCommand(const std::string& command) {
    return (createCommand(command, ElementPtr(), ""));
}

ElementPtr
createCommand(const std::string& command, ElementPtr arg) {
    return (createCommand(command, arg, ""));
}

ElementPtr
createCommand(const std::string& command, const std::string& service) {
    return (createCommand(command, ElementPtr(), service));
}

ElementPtr
createCommand(const std::string& command,
              ElementPtr arg,
              const std::string& service) {
    ElementPtr query = Element::createMap();
    ElementPtr cmd = Element::create(command);
    query->set(CONTROL_COMMAND, cmd);
    if (arg) {
        query->set(CONTROL_ARGUMENTS, arg);
    }
    if (!service.empty()) {
        ElementPtr services = Element::createList();
        services->add(Element::create(service));
        query->set(CONTROL_SERVICE, services);
    }
    return (query);
}

std::string
parseCommand(ElementPtr& arg, ElementPtr command) {
    if (!command) {
        isc_throw(CtrlChannelError, "No command specified");
    }
    if (command->getType() != Element::map) {
        isc_throw(CtrlChannelError, "Invalid command Element specified, expected map");
    }
    if (!command->contains(CONTROL_COMMAND)) {
        isc_throw(CtrlChannelError,
                  "Invalid answer specified, does not contain mandatory 'command'");
    }

    // Make sure that all specified parameters are supported.
    auto command_params = command->mapValue();
    for (auto param : command_params) {
        if ((param.first != CONTROL_COMMAND) &&
            (param.first != CONTROL_ARGUMENTS) &&
            (param.first != CONTROL_SERVICE)) {
            isc_throw(CtrlChannelError, "Received command contains unsupported "
                      "parameter '" << param.first << "'");
        }
    }

    ElementPtr cmd = command->get(CONTROL_COMMAND);
    if (cmd->getType() != Element::string) {
        isc_throw(CtrlChannelError,
                  "'command' element in command message is not a string");
    }

    arg = command->get(CONTROL_ARGUMENTS);

    return (cmd->stringValue());
}

std::string
parseCommandWithArgs(ElementPtr& arg, ElementPtr command) {
    std::string command_name = parseCommand(arg, command);

    // This function requires arguments within the command.
    if (!arg) {
        isc_throw(CtrlChannelError,
                  "no arguments specified for the '" << command_name
                  << "' command");
    }

    // Arguments must be a map.
    if (arg->getType() != Element::map) {
        isc_throw(CtrlChannelError, "arguments specified for the '" << command_name
                  << "' command are not a map");
    }

    // At least one argument is required.
    if (arg->size() == 0) {
        isc_throw(CtrlChannelError, "arguments must not be empty for "
                  "the '" << command_name << "' command");
    }

    return (command_name);
}

ElementPtr
combineCommandsLists(const ElementPtr& response1,
                     const ElementPtr& response2) {
    // Usually when this method is called there should be two non-null
    // responses. If there is just a single response, return this
    // response.
    if (!response1 && response2) {
        return (response2);

    } else if (response1 && !response2) {
        return (response1);

    } else if (!response1 && !response2) {
        return (ElementPtr());

    } else {
        // Both responses are non-null so we need to combine the lists
        // of supported commands if the status codes are 0.
        int status_code = -1;
        ElementPtr args1 = parseAnswer(status_code, response1);
        if (status_code != 0) {
            return (response1);
        }

        ElementPtr args2 = parseAnswer(status_code, response2);
        if (status_code != 0) {
            return (response2);
        }

        const std::vector<ElementPtr> vec1 = args1->listValue();
        const std::vector<ElementPtr> vec2 = args2->listValue();

        // Storing command names in a set guarantees that the non-unique
        // command names are aggregated.
        std::set<std::string> combined_set;
        for (auto v = vec1.cbegin(); v != vec1.cend(); ++v) {
            combined_set.insert((*v)->stringValue());
        }
        for (auto v = vec2.cbegin(); v != vec2.cend(); ++v) {
            combined_set.insert((*v)->stringValue());
        }

        // Create a combined list of commands.
        ElementPtr combined_list = Element::createList();
        for (auto s = combined_set.cbegin(); s != combined_set.cend(); ++s) {
            combined_list->add(Element::create(*s));
        }
        return (createAnswer(CONTROL_RESULT_SUCCESS, combined_list));
    }
}

}
}

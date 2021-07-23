/* Copyright (C) 2018-2020 Internet Systems Consortium, Inc. ("ISC")

   This Source Code Form is subject to the terms of the Mozilla Public
   License, v. 2.0. If a copy of the MPL was not distributed with this
   file, You can obtain one at http://mozilla.org/MPL/2.0/. */

%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.0"
%defines
%define api.parser.class {NetconfParser}
%define api.prefix {netconf_}
%define api.token.constructor
%define api.value.type variant
%define api.namespace {isc::netconf}
%define parse.assert
%code requires
{
#include <string>
#include <cc/data.h>
#include <boost/lexical_cast.hpp>
#include <netconf/parser_context_decl.h>

using namespace isc::netconf;
using namespace isc::data;
using namespace std;
}
// The parsing context.
%param { isc::netconf::ParserContext& ctx }
%locations
%define parse.trace
%define parse.error verbose
%code
{
#include <netconf/parser_context.h>
}


%define api.token.prefix {TOKEN_}
// Tokens in an order which makes sense and related to the intented use.
// Actual regexps for tokens are defined in netconf_lexer.ll.
%token
  END  0  "end of file"
  COMMA ","
  COLON ":"
  LSQUARE_BRACKET "["
  RSQUARE_BRACKET "]"
  LCURLY_BRACKET "{"
  RCURLY_BRACKET "}"
  NULL_TYPE "null"

  NETCONF "Netconf"

  USER_CONTEXT "user-context"
  COMMENT "comment"

  CONFIGURATION "configuration"
  CONTROL_SOCKET "control-socket"
  HTTP "http"
  KEA_CONFIG_TOOL "kea-config-tool"
  MODEL "model"
  MODELS "models"
  NOTIFICATIONS "notifications"
  ON_BOOT_UPDATE_SOCKET "on-boot-update-socket"
  ON_BOOT_UPDATE_SYSREPO "on-boot-update-sysrepo"
  OPERATIONAL "operational"
  RPC "rpc"
  SOCKET_NAME "socket-name"
  SOCKET_TYPE "socket-type"
  SOCKET_URL "socket-url"
  STDOUT "stdout"
  SUBSCRIBE "subscribe"
  UNIX "unix"
  VALIDATE "validate"

  HOOKS_LIBRARIES "hooks-libraries"
  LIBRARY "library"
  PARAMETERS "parameters"

  LOGGERS "loggers"
  NAME "name"
  OUTPUT_OPTIONS "output_options"
  OUTPUT "output"
  DEBUGLEVEL "debuglevel"
  SEVERITY "severity"
  FLUSH "flush"
  MAXSIZE "maxsize"
  MAXVER "maxver"
  PATTERN "pattern"

  // Not real tokens, just a way to signal what the parser is expected to
  // parse. This define the starting point. It either can be full grammar
  // (START_NETCONF), part of the grammar related to Netconf (START_SUB_NETCONF)
  // or can be any valid JSON (START_JSON)
  START_JSON
  START_NETCONF
  START_SUB_NETCONF
;

%token <std::string> STRING "constant string"
%token <int64_t> INTEGER "integer"
%token <double> FLOAT "floating point"
%token <bool> BOOLEAN "boolean"

%type <ElementPtr> value
%type <ElementPtr> map_value
%type <ElementPtr> socket_type_value

%printer { yyoutput << $$; } <*>;

%%

// The whole grammar starts with a map, because the config file
// consists of Netconf, DhcpX, Logger and DhcpDdns entries in one big { }.
%start start;

// The starting token can be one of those listed below. Note these are
// "fake" tokens. They're produced by the lexer before any input text
// is parsed.
start: START_JSON      { ctx.ctx_ = ctx.NO_KEYWORDS; } json
     | START_NETCONF     { ctx.ctx_ = ctx.CONFIG; } netconf_syntax_map
     | START_SUB_NETCONF { ctx.ctx_ = ctx.NETCONF; } sub_netconf
     ;

// This rule defines a "shortcut". Instead of specifying the whole structure
// expected by full grammar, we can tell the parser to start from content of
// the Netconf. This is very useful for unit-testing, so we don't need
// to repeat the outer map and "Netconf" map. We can simply provide
// the contents of that map.
sub_netconf: LCURLY_BRACKET {
    // Parse the Netconf map
    ElementPtr m(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.push_back(m);
} global_params RCURLY_BRACKET {
    // parsing completed
};

// --- generic JSON parser -----------------------------------------------------

// json expression can be a value. What value means is defined below.
json: value {
    // Push back the JSON value on the stack
    ctx.stack_.push_back($1);
};

// Rules for value. This can be one of the primary types allowed in JSON.
value: INTEGER { $$ = ElementPtr(new IntElement($1, ctx.loc2pos(@1))); }
     | FLOAT { $$ = ElementPtr(new DoubleElement($1, ctx.loc2pos(@1))); }
     | BOOLEAN { $$ = ElementPtr(new BoolElement($1, ctx.loc2pos(@1))); }
     | STRING { $$ = ElementPtr(new StringElement($1, ctx.loc2pos(@1))); }
     | NULL_TYPE { $$ = ElementPtr(new NullElement(ctx.loc2pos(@1))); }
     | map { $$ = ctx.stack_.back(); ctx.stack_.pop_back(); }
     | list_generic { $$ = ctx.stack_.back(); ctx.stack_.pop_back(); }
     ;

// Rule for map. It will start with {, have some content and will end with }.
map: LCURLY_BRACKET {
    // This code is executed when we're about to start parsing
    // the content of the map
    ElementPtr m(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.push_back(m);
} map_content RCURLY_BRACKET {
    // map parsing completed. If we ever want to do any wrap up
    // (maybe some sanity checking), this would be the best place
    // for it.
};

map_value: map { $$ = ctx.stack_.back(); ctx.stack_.pop_back(); };

// Rule for map content. In some cases it is allowed to have an empty map,
// so we should say that explicitly. In most cases, though, there will
// be some actual content inside. That's defined by not_empty_map
map_content: %empty // empty map
           | not_empty_map
           ;

// Rule for content of the map. It can have one of two formats:
// 1) string: value
// 2) non_empty_map , string: value
// The first case covers a single entry, while the second case
// covers all longer lists recursively.
not_empty_map: STRING COLON value {
                  // map containing a single entry
                  ctx.stack_.back()->set($1, $3);
                  }
             | not_empty_map COMMA STRING COLON value {
                  // map consisting of a shorter map followed by
                  // comma and string:value
                  ctx.stack_.back()->set($3, $5);
                  }
             ;

list_generic: LSQUARE_BRACKET {
    ElementPtr l(new ListElement(ctx.loc2pos(@1)));
    ctx.stack_.push_back(l);
} list_content RSQUARE_BRACKET {
};

list_content: %empty // Empty list
            | not_empty_list
            ;

not_empty_list: value {
                  // List consisting of a single element.
                  ctx.stack_.back()->add($1);
                  }
              | not_empty_list COMMA value {
                  // List ending with , and a value.
                  ctx.stack_.back()->add($3);
                  }
              ;

// --- generic JSON parser ends here -------------------------------------------

// --- syntax checking parser starts here --------------------------------------

// Unknown keyword in a map. This clever rule can be added to any map
// if you want to have a nice expression printed when unknown (mistyped?)
// parameter is found.
unknown_map_entry: STRING COLON {
    const std::string& where = ctx.contextName();
    const std::string& keyword = $1;
    error(@1,
          "got unexpected keyword \"" + keyword + "\" in " + where + " map.");
};

// This defines the top-level { } that holds Netconf object.
netconf_syntax_map: LCURLY_BRACKET {
    // This code is executed when we're about to start parsing
    // the content of the map
    ElementPtr m(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.push_back(m);
} global_objects RCURLY_BRACKET {
    // map parsing completed. If we ever want to do any wrap up
    // (maybe some sanity checking), this would be the best place
    // for it.
};

// This represents top-level entries: Netconf.
global_objects: global_object
              | global_objects COMMA global_object
              ;

// This represents a single top level entry, e.g. Netconf.
global_object: netconf_object
             ;

// This define the Netconf object.
netconf_object: NETCONF {

    // Let's create a MapElement that will represent it, add it to the
    // top level map (that's already on the stack) and put the new map
    // on the stack as well, so child elements will be able to add
    // themselves to it.

    // Prevent against duplicate.
    ctx.unique("Netconf", ctx.loc2pos(@1));
    ElementPtr m(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->set("Netconf", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.NETCONF);
} COLON LCURLY_BRACKET global_params RCURLY_BRACKET {
    // Ok, we're done with parsing Netconf. Let's take the map
    // off the stack.
    ctx.stack_.pop_back();
    ctx.leave();
};

global_params: %empty
             | not_empty_global_params
             ;

not_empty_global_params: global_param
                       | not_empty_global_params COMMA global_param
                       ;

// These are the parameters that are allowed in the top-level for
// Netconf.
global_param: models
            | hooks_libraries
            | loggers
            | user_context
            | comment
            | unknown_map_entry
            ;

user_context: USER_CONTEXT {
    ctx.enter(ctx.NO_KEYWORDS);
} COLON map_value {
    ElementPtr parent = ctx.stack_.back();
    ElementPtr user_context = $4;
    ElementPtr old = parent->get("user-context");

    // Handle already existing user context
    if (old) {
        // Check if it was a comment or a duplicate
        if ((old->size() != 1) || !old->contains("comment")) {
            std::stringstream msg;
            msg << "duplicate user-context entries (previous at "
                << old->getPosition().str() << ")";
            error(@1, msg.str());
        }
        // Merge the comment
        user_context->set("comment", old->get("comment"));
    }

    // Set the user context
    parent->set("user-context", user_context);
    ctx.leave();
};

comment: COMMENT {
    ctx.enter(ctx.NO_KEYWORDS);
} COLON STRING {
    ElementPtr parent = ctx.stack_.back();
    ElementPtr user_context(new MapElement(ctx.loc2pos(@1)));
    ElementPtr comment(new StringElement($4, ctx.loc2pos(@4)));
    user_context->set("comment", comment);

    // Handle already existing user context
    ElementPtr old = parent->get("user-context");
    if (old) {
        // Check for duplicate comment
        if (old->contains("comment")) {
            std::stringstream msg;
            msg << "duplicate user-context/comment entries (previous at "
                << old->getPosition().str() << ")";
            error(@1, msg.str());
        }
        // Merge the user context in the comment
        merge(user_context, old);
    }

    // Set the user context
    parent->set("user-context", user_context);
    ctx.leave();
};

// --- hooks-libraries ---------------------------------------------------------
hooks_libraries: HOOKS_LIBRARIES {
    ElementPtr l(new ListElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->set("hooks-libraries", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.HOOKS_LIBRARIES);
} COLON LSQUARE_BRACKET hooks_libraries_list RSQUARE_BRACKET {
    ctx.stack_.pop_back();
    ctx.leave();
};

hooks_libraries_list: %empty
                    | not_empty_hooks_libraries_list
                    ;

not_empty_hooks_libraries_list: hooks_library
    | not_empty_hooks_libraries_list COMMA hooks_library
    ;

hooks_library: LCURLY_BRACKET {
    ElementPtr m(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
} hooks_params RCURLY_BRACKET {
    ctx.stack_.pop_back();
};

hooks_params: hooks_param
            | hooks_params COMMA hooks_param
            | unknown_map_entry
            ;

hooks_param: library
           | parameters
           ;

library: LIBRARY {
    ctx.enter(ctx.NO_KEYWORDS);
} COLON STRING {
    ElementPtr lib(new StringElement($4, ctx.loc2pos(@4)));
    ctx.stack_.back()->set("library", lib);
    ctx.leave();
};

parameters: PARAMETERS {
    ctx.enter(ctx.NO_KEYWORDS);
} COLON map_value {
    ctx.stack_.back()->set("parameters", $4);
    ctx.leave();
};

// --- hooks-libraries end here ------------------------------------------------

// --- models starts here ------------------------------------------------------
configuration: CONFIGURATION {
    ElementPtr m(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->set("configuration", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.CONFIGURATION);
} COLON LCURLY_BRACKET configuration_keys RCURLY_BRACKET {
    ctx.stack_.pop_back();
    ctx.leave();
};

configuration_keys: configuration_key
                  | configuration_keys COMMA configuration_key
                  ;

configuration_key: control_socket
                 | on_boot_update_socket
                 | on_boot_update_sysrepo
                 | subscribe
                 | validate
                 | unknown_map_entry
                 ;

control_socket: CONTROL_SOCKET {
    ElementPtr m(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->set("control-socket", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.CONTROL_SOCKET);
} COLON LCURLY_BRACKET control_socket_params RCURLY_BRACKET {
    ctx.stack_.pop_back();
    ctx.leave();
};

control_socket_params: control_socket_param
                     | control_socket_params COMMA control_socket_param
                     ;

control_socket_param: socket_type
                    | socket_name
                    | socket_url
                    | user_context
                    | comment
                    | unknown_map_entry
                    ;

model_entry: LCURLY_BRACKET {
    ElementPtr m(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
} model_params RCURLY_BRACKET {
    ctx.stack_.pop_back();
};

model_params : model_param
             | model_params COMMA model_param
             ;

model_param : comment
            | configuration
            | model
            | notifications
            | operational
            | rpc
            | user_context
            | unknown_map_entry
            ;

model: MODEL {
    ctx.enter(ctx.NO_KEYWORDS);
} COLON STRING {
    ElementPtr model(new StringElement($4, ctx.loc2pos(@4)));
    ctx.stack_.back()->set("model", model);
    ctx.leave();
};

models: MODELS {
    ElementPtr m(new ListElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->set("models", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.MODELS);
} COLON LSQUARE_BRACKET models_entries RSQUARE_BRACKET {
    ctx.stack_.pop_back();
    ctx.leave();
};

models_entries : model_entry
               | models_entries COMMA model_entry
               ;

notifications: NOTIFICATIONS {
    ElementPtr m(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->set("notifications", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.NOTIFICATIONS);
} COLON LCURLY_BRACKET notifications_keys RCURLY_BRACKET {
    ctx.stack_.pop_back();
    ctx.leave();
};

notifications_keys: notifications_key
                  | notifications_keys COMMA notifications_key
                  ;

notifications_key: control_socket
                 | subscribe
                 | unknown_map_entry
                 ;

operational: OPERATIONAL {
    ElementPtr m(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->set("operational", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.OPERATIONAL);
} COLON LCURLY_BRACKET operational_keys RCURLY_BRACKET {
    ctx.stack_.pop_back();
    ctx.leave();
};

operational_keys: operational_key
                | operational_keys COMMA operational_key
                ;

operational_key: control_socket
               | subscribe
               | unknown_map_entry
               ;

rpc: RPC {
    ElementPtr m(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->set("rpc", m);
    ctx.stack_.push_back(m);
    ctx.enter(ctx.RPC);
} COLON LCURLY_BRACKET rpc_keys RCURLY_BRACKET {
    ctx.stack_.pop_back();
    ctx.leave();
};

rpc_keys: rpc_key
        | rpc_keys COMMA rpc_key
        ;

rpc_key: control_socket
       | subscribe
       | unknown_map_entry
       ;


on_boot_update_socket: ON_BOOT_UPDATE_SOCKET COLON BOOLEAN {
    ElementPtr flag(new BoolElement($3, ctx.loc2pos(@3)));
    ctx.stack_.back()->set("on-boot-update-socket", flag);
};

on_boot_update_sysrepo: ON_BOOT_UPDATE_SYSREPO COLON BOOLEAN {
    ElementPtr flag(new BoolElement($3, ctx.loc2pos(@3)));
    ctx.stack_.back()->set("on-boot-update-sysrepo", flag);
};

socket_type: SOCKET_TYPE {
    ctx.enter(ctx.SOCKET_TYPE);
} COLON socket_type_value {
    ctx.stack_.back()->set("socket-type", $4);
    ctx.leave();
};

socket_type_value : UNIX { $$ = ElementPtr(new StringElement("unix", ctx.loc2pos(@1))); }
                  | KEA_CONFIG_TOOL { $$ = ElementPtr(new StringElement("kea-config-tool", ctx.loc2pos(@1))); }
                  | HTTP { $$ = ElementPtr(new StringElement("http", ctx.loc2pos(@1))); }
                  | STDOUT { $$ = ElementPtr(new StringElement("stdout", ctx.loc2pos(@1))); }
                  ;

socket_name: SOCKET_NAME {
    ctx.enter(ctx.NO_KEYWORDS);
} COLON STRING {
    ElementPtr name(new StringElement($4, ctx.loc2pos(@4)));
    ctx.stack_.back()->set("socket-name", name);
    ctx.leave();
};

socket_url: SOCKET_URL {
    ctx.enter(ctx.NO_KEYWORDS);
} COLON STRING {
    ElementPtr url(new StringElement($4, ctx.loc2pos(@4)));
    ctx.stack_.back()->set("socket-url", url);
    ctx.leave();
};

 subscribe: SUBSCRIBE COLON BOOLEAN {
     ElementPtr flag(new BoolElement($3, ctx.loc2pos(@3)));
     ctx.stack_.back()->set("subscribe", flag);
 };

 validate: VALIDATE COLON BOOLEAN {
     ElementPtr flag(new BoolElement($3, ctx.loc2pos(@3)));
     ctx.stack_.back()->set("validate", flag);
 };

// --- models end here ---------------------------------------------------------

// --- Loggers starts here -----------------------------------------------------

loggers: LOGGERS {
    ElementPtr l(new ListElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->set("loggers", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.LOGGERS);
}  COLON LSQUARE_BRACKET loggers_entries RSQUARE_BRACKET {
    ctx.stack_.pop_back();
    ctx.leave();
};

// These are the parameters allowed in loggers: either one logger
// entry or multiple entries separate by commas.
loggers_entries: logger_entry
               | loggers_entries COMMA logger_entry
               ;

// This defines a single entry defined in loggers.
logger_entry: LCURLY_BRACKET {
    ElementPtr l(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->add(l);
    ctx.stack_.push_back(l);
} logger_params RCURLY_BRACKET {
    ctx.stack_.pop_back();
};

logger_params: logger_param
             | logger_params COMMA logger_param
             ;

logger_param: name
            | output_options_list
            | debuglevel
            | severity
            | user_context
            | comment
            | unknown_map_entry
            ;

name: NAME {
    ctx.enter(ctx.NO_KEYWORDS);
} COLON STRING {
    ElementPtr name(new StringElement($4, ctx.loc2pos(@4)));
    ctx.stack_.back()->set("name", name);
    ctx.leave();
};

debuglevel: DEBUGLEVEL COLON INTEGER {
    ElementPtr dl(new IntElement($3, ctx.loc2pos(@3)));
    ctx.stack_.back()->set("debuglevel", dl);
};

severity: SEVERITY {
    ctx.enter(ctx.NO_KEYWORDS);
} COLON STRING {
    ElementPtr sev(new StringElement($4, ctx.loc2pos(@4)));
    ctx.stack_.back()->set("severity", sev);
    ctx.leave();
};

output_options_list: OUTPUT_OPTIONS {
    ElementPtr l(new ListElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->set("output_options", l);
    ctx.stack_.push_back(l);
    ctx.enter(ctx.OUTPUT_OPTIONS);
} COLON LSQUARE_BRACKET output_options_list_content RSQUARE_BRACKET {
    ctx.stack_.pop_back();
    ctx.leave();
};

output_options_list_content: output_entry
                           | output_options_list_content COMMA output_entry
                           ;

output_entry: LCURLY_BRACKET {
    ElementPtr m(new MapElement(ctx.loc2pos(@1)));
    ctx.stack_.back()->add(m);
    ctx.stack_.push_back(m);
} output_params_list RCURLY_BRACKET {
    ctx.stack_.pop_back();
};

output_params_list: output_params
             | output_params_list COMMA output_params
             ;

output_params: output
             | flush
             | maxsize
             | maxver
             | pattern
             ;

output: OUTPUT {
    ctx.enter(ctx.NO_KEYWORDS);
} COLON STRING {
    ElementPtr sev(new StringElement($4, ctx.loc2pos(@4)));
    ctx.stack_.back()->set("output", sev);
    ctx.leave();
};

flush: FLUSH COLON BOOLEAN {
    ElementPtr flush(new BoolElement($3, ctx.loc2pos(@3)));
    ctx.stack_.back()->set("flush", flush);
};

maxsize: MAXSIZE COLON INTEGER {
    ElementPtr maxsize(new IntElement($3, ctx.loc2pos(@3)));
    ctx.stack_.back()->set("maxsize", maxsize);
};

maxver: MAXVER COLON INTEGER {
    ElementPtr maxver(new IntElement($3, ctx.loc2pos(@3)));
    ctx.stack_.back()->set("maxver", maxver);
};

pattern: PATTERN {
    ctx.enter(ctx.NO_KEYWORDS);
} COLON STRING {
    ElementPtr sev(new StringElement($4, ctx.loc2pos(@4)));
    ctx.stack_.back()->set("pattern", sev);
    ctx.leave();
};

%%

void
isc::netconf::NetconfParser::error(const location_type& loc,
                               const std::string& what)
{
    ctx.error(loc, what);
}

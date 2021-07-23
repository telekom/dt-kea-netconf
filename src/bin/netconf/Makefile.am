SUBDIRS = . tests

AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib
AM_CPPFLAGS += -I$(top_srcdir)/src/bin -I$(top_builddir)/src/bin
AM_CPPFLAGS += -I$(top_srcdir)/src -I$(top_builddir)/src
AM_CPPFLAGS += $(BOOST_INCLUDES) $(SYSREPO_INCLUDEDIR)

if HAVE_MYSQL
AM_CPPFLAGS += $(MYSQL_CPPFLAGS)
endif
if HAVE_PGSQL
AM_CPPFLAGS += $(PGSQL_CPPFLAGS)
endif
if HAVE_CQL
AM_CPPFLAGS += $(CQL_CPPFLAGS)
endif

AM_CXXFLAGS = $(KEA_CXXFLAGS)

if USE_STATIC_LINK
AM_LDFLAGS = -static
endif

CLEANFILES  = *.gcno *.gcda

# convenience archive

noinst_LTLIBRARIES = libnetconf.la

libnetconf_la_SOURCES  =
libnetconf_la_SOURCES += changes.cc changes.h
libnetconf_la_SOURCES += configuration_callback.h
libnetconf_la_SOURCES += control_socket.h
libnetconf_la_SOURCES += http_control_socket.h
libnetconf_la_SOURCES += kea_config_tool_control.h
libnetconf_la_SOURCES += location.hh position.hh stack.hh
libnetconf_la_SOURCES += netconf.cc netconf.h
libnetconf_la_SOURCES += netconf_cfg_mgr.cc netconf_cfg_mgr.h
libnetconf_la_SOURCES += netconf_config.cc netconf_config.h
libnetconf_la_SOURCES += netconf_controller.cc netconf_controller.h
libnetconf_la_SOURCES += netconf_log.cc netconf_log.h
libnetconf_la_SOURCES += netconf_lexer.ll netconf_parser.yy
libnetconf_la_SOURCES += netconf_messages.h netconf_messages.cc
libnetconf_la_SOURCES += netconf_parser.cc netconf_parser.h
libnetconf_la_SOURCES += netconf_process.cc netconf_process.h
libnetconf_la_SOURCES += notifications_callback.h
libnetconf_la_SOURCES += operational_callback.cc operational_callback.h
libnetconf_la_SOURCES += parser_context.cc parser_context.h
libnetconf_la_SOURCES += parser_context_decl.h
libnetconf_la_SOURCES += rpc_callback.cc rpc_callback.h
libnetconf_la_SOURCES += simple_parser.cc simple_parser.h
libnetconf_la_SOURCES += stdout_control_socket.h
libnetconf_la_SOURCES += unix_control_socket.h

libnetconf_la_LIBADD  =
libnetconf_la_LIBADD += $(top_builddir)/src/bin/dhcp4/libdhcp4.la
libnetconf_la_LIBADD += $(top_builddir)/src/bin/dhcp6/libdhcp6.la
libnetconf_la_LIBADD += $(top_builddir)/src/bin/kea_config_tool/libkeaconfigtool.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/asiolink/libkea-asiolink.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/cc/libkea-cc.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/cfgrpt/libcfgrpt.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/config/libkea-cfgclient.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/dhcpsrv/libkea-dhcpsrv.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/exceptions/libkea-exceptions.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/hooks/libkea-hooks.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/http/libkea-http.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/log/libkea-log.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/process/libkea-process.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/util/libkea-util.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/yang/libkea-yang.la
libnetconf_la_LIBADD += $(top_builddir)/src/lib/stats/libkea-stats.la
libnetconf_la_LIBADD += $(LOG4CPLUS_LIBS) $(BOOST_LIBS) $(SYSREPO_LIBS)

EXTRA_DIST  = netconf_messages.mes
EXTRA_DIST += netconf_lexer.ll
EXTRA_DIST += netconf_parser.yy

sbin_PROGRAMS = kea-netconf

kea_netconf_SOURCES  = main.cc

kea_netconf_LDADD  = libnetconf.la
if HAVE_CQL
kea_netconf_LDADD += $(CQL_LIBS)
endif

kea_netconfdir = $(pkgdatadir)

# If we want to get rid of all generated messages files, we need to use
# make maintainer-clean. The proper way to introduce custom commands for
# that operation is to define maintainer-clean-local target. However,
# make maintainer-clean also removes Makefile, so running configure script
# is required.  To make it easy to rebuild messages without going through
# reconfigure, a new target messages-clean has been added.
maintainer-clean-local:
	rm -f netconf_messages.h netconf_messages.cc

# To regenerate messages files, one can do:
#
# make messages-clean
# make messages
#
# This is needed only when a .mes file is modified.
messages-clean: maintainer-clean-local

if GENERATE_MESSAGES

# Define rule to build logging source files from message file
messages: netconf_messages.h netconf_messages.cc
	@echo Message files regenerated

netconf_messages.h netconf_messages.cc: netconf_messages.mes
	$(top_builddir)/src/lib/log/compiler/kea-msg-compiler $(top_srcdir)/src/bin/netconf/netconf_messages.mes

else

messages netconf_messages.h netconf_messages.cc:
	@echo Messages generation disabled. Configure with --enable-generate-messages to enable it.

endif

if GENERATE_PARSER

# Generate parser first.
all-recursive: netconf_lexer.cc location.hh position.hh stack.hh netconf_parser.cc netconf_parser.h

parser: netconf_lexer.cc location.hh position.hh stack.hh netconf_parser.cc netconf_parser.h
	@echo "Flex/bison files regenerated"

# --- Flex/Bison stuff below --------------------------------------------------
# When debugging grammar issues, it's useful to add -v to bison parameters.
# bison will generate parser.output file that explains the whole grammar.
# It can be used to manually follow what's going on in the parser.
# This is especially useful if yydebug_ is set to 1 as that variable
# will cause parser to print out its internal state.
# Call flex with -s to check that the default rule can be suppressed
# Call bison with -W to get warnings like unmarked empty rules
# Note C++11 deprecated register still used by flex < 2.6.0
location.hh position.hh stack.hh netconf_parser.cc netconf_parser.h: netconf_parser.yy
	flock /tmp/kea-parsers $(YACC) --defines=netconf_parser.h --report=all --report-file=netconf_parser.report -o netconf_parser.cc netconf_parser.yy
	flock /tmp/kea-parsers $(YACC) --update --defines=netconf_parser.h --report=all --report-file=netconf_parser.report -o netconf_parser.cc netconf_parser.yy
	flock /tmp/kea-parsers $(YACC) --graph --defines=netconf_parser.h --report=all --report-file=netconf_parser.report -o netconf_parser.cc netconf_parser.yy

netconf_lexer.cc: netconf_lexer.ll
	$(LEX) --prefix netconf_ -o netconf_lexer.cc netconf_lexer.ll

else

parser location.hh position.hh stack.hh netconf_parser.cc netconf_parser.h netconf_lexer.cc:
	@echo Parser generation disabled. Configure with --enable-generate-parser to enable it.

endif

.PHONY: parser

SUBDIRS = . tests

AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib
AM_CPPFLAGS += -I$(top_srcdir)/src/bin -I$(top_builddir)/src/bin
AM_CPPFLAGS += $(BOOST_INCLUDES)

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

# If we want to get rid of all generated messages files, we need to use
# make maintainer-clean. The proper way to introduce custom commands for
# that operation is to define maintainer-clean-local target. However,
# make maintainer-clean also removes Makefile, so running configure script
# is required.  To make it easy to rebuild messages without going through
# reconfigure, a new target messages-clean has been added.
maintainer-clean-local:
	rm -f messages.cc messages.h

# To regenerate messages files, one can do:
#
# make messages-clean
# make messages
#
# This is needed only when a .mes file is modified.
messages-clean: maintainer-clean-local

if GENERATE_MESSAGES

# Define rule to build logging source files from message file
messages: messages.cc messages.h
	@echo Message files regenerated

messages.cc messages.h: messages.mes
	$(top_builddir)/src/lib/log/compiler/kea-msg-compiler $(top_srcdir)/src/bin/kea_config_tool/messages.mes

else

messages messages.cc messages.h:
	@echo Messages generation disabled. Configure with --enable-generate-messages to enable it.

endif

# convenience archive

noinst_LTLIBRARIES = libkeaconfigtool.la

libkeaconfigtool_la_SOURCES  =
libkeaconfigtool_la_SOURCES += options.cc options.h
libkeaconfigtool_la_SOURCES += controller.cc controller.h
libkeaconfigtool_la_SOURCES += messages.cc messages.h

libkeaconfigtool_la_CPPFLAGS = $(AM_CPPFLAGS)

libkeaconfigtool_la_CXXFLAGS = $(AM_CXXFLAGS)

libkeaconfigtool_la_LIBADD  =
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/bin/admin/libadmin.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/bin/dhcp4/libdhcp4.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/bin/dhcp6/libdhcp6.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/lib/asiolink/libkea-asiolink.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/lib/cc/libkea-cc.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/lib/cfgrpt/libcfgrpt.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/lib/config/libkea-cfgclient.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/lib/cryptolink/libkea-cryptolink.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/lib/dhcp/libkea-dhcp++.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/lib/dhcpsrv/libkea-dhcpsrv.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/lib/dns/libkea-dns++.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/lib/exceptions/libkea-exceptions.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/lib/log/libkea-log.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/lib/process/libkea-process.la
libkeaconfigtool_la_LIBADD += $(top_builddir)/src/lib/util/libkea-util.la
libkeaconfigtool_la_LIBADD += $(CRYPTO_LIBS)
libkeaconfigtool_la_LIBADD += $(BOOST_LIBS)
libkeaconfigtool_la_LIBADD += $(PTHREAD_LDFLAGS)

libkeaconfigtool_la_LDFLAGS  = $(AM_LDFLAGS) $(CRYPTO_LDFLAGS)

sbin_PROGRAMS = kea-config-tool
kea_config_tool_SOURCES = main.cc
kea_config_tool_CPPFLAGS = $(AM_CPPFLAGS)
kea_config_tool_CXXFLAGS = $(AM_CXXFLAGS)
kea_config_tool_LDADD  = libkeaconfigtool.la
kea_config_tool_LDFLAGS  = $(AM_LDFLAGS) $(CRYPTO_LDFLAGS)
if HAVE_CQL
kea_config_tool_LDFLAGS += $(CQL_LIBS)
endif

DISTCLEANFILES = $(man_MANS)
EXTRA_DIST = $(man_MANS) kea-config-tool.xml messages.mes

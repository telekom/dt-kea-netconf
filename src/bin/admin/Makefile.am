SUBDIRS = . tests

# Install kea-admin in sbin.
sbin_SCRIPTS  = kea-admin

AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib
AM_CPPFLAGS += -I$(top_srcdir)/src/bin -I$(top_builddir)/src/bin
AM_CPPFLAGS += $(BOOST_INCLUDES)

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
	$(top_builddir)/src/lib/log/compiler/kea-msg-compiler $(top_srcdir)/src/bin/admin/messages.mes

else

messages messages.cc messages.h:
	@echo Messages generation disabled. Configure with --enable-generate-messages to enable it.

endif

# convenience archive

noinst_LTLIBRARIES = libadmin.la

libadmin_la_SOURCES  =
libadmin_la_SOURCES += kea_admin.cc kea_admin.h
libadmin_la_SOURCES += messages.cc messages.h

libadmin_la_CPPFLAGS = $(AM_CPPFLAGS)
libadmin_la_CXXFLAGS = $(AM_CXXFLAGS)

libadmin_la_LIBADD  =
libadmin_la_LDFLAGS = $(AM_LDFLAGS)

adminscriptsdir = ${datarootdir}/${PACKAGE_NAME}/scripts
adminscripts_DATA = admin-utils.sh

DISTCLEANFILES = $(sbin_SCRIPTS)
EXTRA_DIST = kea-admin.xml admin-utils.sh messages.mes

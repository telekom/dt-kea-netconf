SUBDIRS = .

AM_CPPFLAGS  =
AM_CPPFLAGS += -I$(builddir)/.. -I$(srcdir)/..
AM_CPPFLAGS += -I$(top_builddir)/src/bin -I$(top_srcdir)/src/bin
AM_CPPFLAGS += -I$(top_builddir)/src/lib -I$(top_srcdir)/src/lib
AM_CPPFLAGS += $(BOOST_INCLUDES)

AM_CXXFLAGS = $(KEA_CXXFLAGS)

if USE_STATIC_LINK
AM_LDFLAGS = -static
endif

CLEANFILES = *.gcno *.gcda

TESTS_ENVIRONMENT = $(LIBTOOL) --mode=execute $(VALGRIND_COMMAND)

TESTS =

if HAVE_MYSQL
AM_CPPFLAGS += $(MYSQL_CPPFLAGS)
endif
if HAVE_PGSQL
AM_CPPFLAGS += $(PGSQL_CPPFLAGS)
endif
if HAVE_CQL
AM_CPPFLAGS += $(CQL_CPPFLAGS)
endif

if HAVE_GTEST
TESTS += run_tests

run_tests_SOURCES  =
run_tests_SOURCES += kea_config_tool_test.cc
run_tests_SOURCES += run_tests.cc

run_tests_CPPFLAGS = $(AM_CPPFLAGS) $(GTEST_INCLUDES)
run_tests_LDFLAGS  = $(AM_LDFLAGS) $(CRYPTO_LDFLAGS) $(GTEST_LDFLAGS)

run_tests_LDADD  =
run_tests_LDADD += $(top_builddir)/src/bin/dhcp4/libdhcp4.la
run_tests_LDADD += $(top_builddir)/src/bin/dhcp6/libdhcp6.la
run_tests_LDADD += $(top_builddir)/src/bin/kea_config_tool/libkeaconfigtool.la
run_tests_LDADD += $(top_builddir)/src/lib/cql/testutils/libcqltest.la
run_tests_LDADD += $(top_builddir)/src/lib/dhcpsrv/libkea-dhcpsrv.la
run_tests_LDADD += $(top_builddir)/src/lib/dhcpsrv/testutils/libdhcpsrvtest.la
run_tests_LDADD += $(top_builddir)/src/lib/exceptions/libkea-exceptions.la
run_tests_LDADD += $(top_builddir)/src/lib/util/libkea-util.la
run_tests_LDADD += $(top_builddir)/src/lib/util/unittests/libutil_unittests.la
run_tests_LDADD += $(CRYPTO_LIBS) $(BOOST_LIBS) $(GTEST_LDADD)
endif

noinst_PROGRAMS = $(TESTS)

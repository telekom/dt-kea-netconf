SUBDIRS = . testutils tests

AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib
AM_CPPFLAGS += $(BOOST_INCLUDES) $(CQL_CPPFLAGS)

AM_CXXFLAGS = $(KEA_CXXFLAGS)

CLEANFILES = *.gcno *.gcda

lib_LTLIBRARIES = libkea-cql.la
libkea_cql_la_SOURCES  = cql_connection.cc cql_connection.h
libkea_cql_la_SOURCES += cql_exchange.cc cql_exchange.h
libkea_cql_la_SOURCES += cql_lock.h
libkea_cql_la_SOURCES += cql_memory_mgr.cc cql_memory_mgr.h
libkea_cql_la_SOURCES += cql_transaction.cc cql_transaction.h

libkea_cql_la_LIBADD  = $(top_builddir)/src/lib/asiolink/libkea-asiolink.la
libkea_cql_la_LIBADD += $(top_builddir)/src/lib/database/libkea-database.la
libkea_cql_la_LIBADD += $(top_builddir)/src/lib/exceptions/libkea-exceptions.la
libkea_cql_la_LIBADD += $(top_builddir)/src/lib/log/libkea-log.la
libkea_cql_la_LIBADD += $(top_builddir)/src/lib/stats/libkea-stats.la
libkea_cql_la_LIBADD += $(top_builddir)/src/lib/util/libkea-util.la
libkea_cql_la_LIBADD += $(LOG4CPLUS_LIBS) $(BOOST_LIBS)

libkea_cql_la_LDFLAGS  = -no-undefined -version-info 3:0:2
libkea_cql_la_LDFLAGS += $(CQL_LIBS)

# Specify the headers for copying into the installation directory tree.
libkea_cql_includedir = $(pkgincludedir)/cql
libkea_cql_include_HEADERS = \
    cql_connection.h         \
    cql_exchange.h           \
    cql_memory_mgr.h         \
    cql_transaction.h

AM_CPPFLAGS  =
AM_CPPFLAGS += -I$(top_builddir)/src/lib
AM_CPPFLAGS += -I$(top_srcdir)/src/lib_LTLIBRARIES

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

CLEANFILES = *.gcno *.gcda

sbin_PROGRAMS = kea-gist-cassandra
kea_gist_cassandra_SOURCES = kea-gist.cc

kea_gist_cassandra_CXXFLAGS = $(AM_CXXFLAGS)
kea_gist_cassandra_CPPFLAGS = $(AM_CPPFLAGS)

kea_gist_cassandra_LDADD = $(LOG4CPLUS_LIBS) $(CRYPTO_LIBS) $(BOOST_LIBS)

kea_gist_cassandra_LDFLAGS  =
kea_gist_cassandra_LDFLAGS += $(CRYPTO_LDFLAGS)
if HAVE_MYSQL
kea_gist_cassandra_LDFLAGS += $(MYSQL_LIBS)
endif
if HAVE_PGSQL
kea_gist_cassandra_LDFLAGS += $(PGSQL_LIBS)
endif
if HAVE_CQL
kea_gist_cassandra_LDFLAGS += $(CQL_LIBS)
endif

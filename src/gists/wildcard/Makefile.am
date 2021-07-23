CLEANFILES = *.gcno *.gcda

sbin_PROGRAMS = kea-gist-wildcard

kea_gist_wildcard_SOURCES  =
kea_gist_wildcard_SOURCES += kea-gist.cc

kea_gist_wildcard_CXXFLAGS = $(KEA_CXXFLAGS)

kea_gist_wildcard_CPPFLAGS  =
kea_gist_wildcard_CPPFLAGS += -I$(top_builddir)/src/gists
kea_gist_wildcard_CPPFLAGS += -I$(top_builddir)/src/lib
kea_gist_wildcard_CPPFLAGS += $(BOOST_INCLUDES)
if HAVE_CQL
kea_gist_wildcard_CPPFLAGS += $(CQL_CPPFLAGS)
endif
if HAVE_MYSQL
kea_gist_wildcard_CPPFLAGS += $(MYSQL_CPPFLAGS)
endif
if HAVE_PGSQL
kea_gist_wildcard_CPPFLAGS += $(PGSQL_CPPFLAGS)
endif
if HAVE_SYSREPO
kea_gist_wildcard_CPPFLAGS += $(SYSREPO_CPPFLAGS)
endif

kea_gist_wildcard_LDADD  =
kea_gist_wildcard_LDADD += $(top_builddir)/src/lib/cc/libkea-cc.la
kea_gist_wildcard_LDADD += $(top_builddir)/src/lib/database/libkea-database.la
if HAVE_SYSREPO
kea_gist_wildcard_LDADD += $(top_builddir)/src/lib/yang/libkea-yang.la
if HAVE_GTEST
kea_gist_wildcard_LDADD += $(top_builddir)/src/lib/yang/testutils/libyangtest.la
endif
endif
kea_gist_wildcard_LDADD += $(top_builddir)/src/lib/hooks/libkea-hooks.la
kea_gist_wildcard_LDADD += $(LOG4CPLUS_LIBS) $(CRYPTO_LIBS) $(BOOST_LIBS)

kea_gist_wildcard_LDFLAGS  =
kea_gist_wildcard_LDFLAGS += $(CRYPTO_LDFLAGS)
if HAVE_CQL
kea_gist_wildcard_LDFLAGS += $(CQL_LIBS)
endif
if HAVE_MYSQL
kea_gist_wildcard_LDFLAGS += $(MYSQL_LIBS)
endif
if HAVE_PGSQL
kea_gist_wildcard_LDFLAGS += $(PGSQL_LIBS)
endif
if HAVE_SYSREPO
kea_gist_wildcard_LDFLAGS += $(SYSREPO_LIBS)
endif

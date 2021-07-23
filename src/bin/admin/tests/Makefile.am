SUBDIRS = data .

SHTESTS = memfile_tests.sh

if HAVE_MYSQL
SHTESTS += mysql_tests.sh
endif

if HAVE_PGSQL
SHTESTS += pgsql_tests.sh
endif

CQLTESTSCRIPTS =
if HAVE_CQL
SHTESTS += cql_tests.sh
CQLTESTSCRIPTS += dhcpdb_create_1.0.cql
CQLTESTSCRIPTS += dhcpdb_create_1.0.mysql
CQLTESTSCRIPTS += dhcpdb_create_1.0.pgsql
endif

noinst_SCRIPTS = $(SHTESTS)
noinst_SCRIPTS += $(CQLTESTSCRIPTS)

EXTRA_DIST  = dhcpdb_create_1.0.cql
EXTRA_DIST += dhcpdb_create_1.0.mysql
EXTRA_DIST += dhcpdb_create_1.0.pgsql

CLEANFILES = *.log
DISTCLEANFILES = memfile_tests.sh mysql_tests.sh pgsql_tests.sh cql_tests.sh

dhcpdb_create_1.0.cql: dhcpdb_create_1.0.cql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

dhcpdb_create_1.0.mysql: dhcpdb_create_1.0.mysql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

dhcpdb_create_1.0.pgsql: dhcpdb_create_1.0.pgsql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

# Execute all test scripts.
check-local:
	for shtest in $(SHTESTS) ; do \
	echo Running test: $$shtest ; \
	chmod +x $(abs_builddir)/$$shtest ; \
	${SHELL} $(abs_builddir)/$$shtest || exit ; \
	done


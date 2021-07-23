SUBDIRS = .

sqlscriptsdir = ${datarootdir}/${PACKAGE_NAME}/scripts/pgsql
sqlscripts_DATA  = dhcpdb_create.pgsql
sqlscripts_DATA += dhcpdb_drop.pgsql
sqlscripts_DATA += configdb_drop.pgsql
sqlscripts_DATA += masterdb_drop.pgsql
sqlscripts_DATA += configdb_create.pgsql
sqlscripts_DATA += config_upgrade_0.0_to_1.0.pgsql
sqlscripts_DATA += config_upgrade_0.0_to_1.0.sh
sqlscripts_DATA += config_upgrade_1.0_to_2.0.pgsql
sqlscripts_DATA += config_upgrade_1.0_to_2.0.sh
sqlscripts_DATA += masterdb_create.pgsql
sqlscripts_DATA += master_upgrade_0.0_to_1.0.pgsql
sqlscripts_DATA += master_upgrade_0.0_to_1.0.sh
sqlscripts_DATA += upgrade_1.0_to_2.0.sh
sqlscripts_DATA += upgrade_2.0_to_3.0.sh
sqlscripts_DATA += upgrade_3.0_to_3.1.sh
sqlscripts_DATA += upgrade_3.1_to_3.2.sh
sqlscripts_DATA += upgrade_3.2_to_3.3.sh
sqlscripts_DATA += upgrade_3.3_to_4.0.sh
sqlscripts_DATA += upgrade_4.0_to_5.0.sh
sqlscripts_DATA += upgrade_5.0_to_5.1.sh
sqlscripts_DATA += upgrade_5.1_to_6.0.sh
sqlscripts_DATA += upgrade_6.0_to_6.1.sh
sqlscripts_DATA += wipe_data.sh

DISTCLEANFILES  = config_upgrade_0.0_to_1.0.pgsql
DISTCLEANFILES += config_upgrade_0.0_to_1.0.sh
DISTCLEANFILES += config_upgrade_1.0_to_2.0.pgsql
DISTCLEANFILES += config_upgrade_1.0_to_2.0.sh
DISTCLEANFILES += master_upgrade_0.0_to_1.0.pgsql
DISTCLEANFILES += master_upgrade_0.0_to_1.0.sh
DISTCLEANFILES += upgrade_1.0_to_2.0.sh
DISTCLEANFILES += upgrade_2.0_to_3.0.sh
DISTCLEANFILES += upgrade_3.0_to_3.1.sh
DISTCLEANFILES += upgrade_3.1_to_3.2.sh
DISTCLEANFILES += upgrade_3.2_to_3.3.sh
DISTCLEANFILES += upgrade_3.3_to_4.0.sh
DISTCLEANFILES += upgrade_4.0_to_5.0.sh
DISTCLEANFILES += upgrade_5.0_to_5.1.sh
DISTCLEANFILES += upgrade_5.1_to_6.0.sh
DISTCLEANFILES += upgrade_6.0_to_6.1.sh
DISTCLEANFILES += wipe_data.sh

CLEANFILES = ${sqlscripts_DATA}
EXTRA_DIST = ${sqlscripts_DATA}

config_upgrade_0.0_to_1.0.pgsql: config_upgrade_0.0_to_1.0.pgsql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

config_upgrade_1.0_to_2.0.pgsql: config_upgrade_1.0_to_2.0.pgsql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

configdb_create.pgsql: configdb_create.pgsql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

dhcpdb_create.pgsql: dhcpdb_create.pgsql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

dhcpdb_drop.pgsql: dhcpdb_drop.pgsql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

configdb_drop.pgsql: configdb_drop.pgsql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

masterdb_drop.pgsql: masterdb_drop.pgsql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

masterdb_create.pgsql: masterdb_create.pgsql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

master_upgrade_0.0_to_1.0.pgsql: master_upgrade_0.0_to_1.0.pgsql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@


SUBDIRS = .

sqlscriptsdir = ${datarootdir}/${PACKAGE_NAME}/scripts/cql
sqlscripts_DATA  = config_upgrade_0.0_to_1.0.cql
sqlscripts_DATA += config_upgrade_0.0_to_1.0.sh
sqlscripts_DATA += config_upgrade_1.0_to_2.0.cql
sqlscripts_DATA += config_upgrade_1.0_to_2.0.sh
sqlscripts_DATA += configdb_create.cql
sqlscripts_DATA += dhcpdb_create.cql
sqlscripts_DATA += dhcpdb_drop.cql
sqlscripts_DATA += configdb_drop.cql
sqlscripts_DATA += masterdb_drop.cql
sqlscripts_DATA += masterdb_create.cql
sqlscripts_DATA += masterdb_create_config.sh
sqlscripts_DATA += master_upgrade_0.0_to_1.0.cql
sqlscripts_DATA += master_upgrade_0.0_to_1.0.sh
sqlscripts_DATA += upgrade_1.0_to_2.0.sh
sqlscripts_DATA += upgrade_2.0_to_3.0.sh
sqlscripts_DATA += upgrade_3.0_to_4.0.sh
sqlscripts_DATA += upgrade_4.0_to_5.0.sh
sqlscripts_DATA += wipe_data.sh

CLEANFILES = ${sqlscripts_DATA}
EXTRA_DIST = ${sqlscripts_DATA}

config_upgrade_0.0_to_1.0.cql: config_upgrade_0.0_to_1.0.cql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

config_upgrade_1.0_to_2.0.cql: config_upgrade_1.0_to_2.0.cql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

dhcpdb_create.cql: dhcpdb_create.cql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

configdb_create.cql: configdb_create.cql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

masterdb_create.cql: masterdb_create.cql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

dhcpdb_drop.cql: dhcpdb_drop.cql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

configdb_drop.cql: configdb_drop.cql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

masterdb_drop.cql: masterdb_drop.cql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

master_upgrade_0.0_to_1.0.cql: master_upgrade_0.0_to_1.0.cql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@


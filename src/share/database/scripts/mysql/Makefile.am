SUBDIRS = .

sqlscriptsdir = ${datarootdir}/${PACKAGE_NAME}/scripts/mysql
sqlscripts_DATA  = dhcpdb_create.mysql
sqlscripts_DATA += dhcpdb_drop.mysql
sqlscripts_DATA += configdb_drop.mysql
sqlscripts_DATA += masterdb_drop.mysql
sqlscripts_DATA += configdb_create.mysql
sqlscripts_DATA += config_upgrade_0.0_to_1.0.mysql
sqlscripts_DATA += config_upgrade_0.0_to_1.0.sh
sqlscripts_DATA += config_upgrade_1.0_to_2.0.mysql
sqlscripts_DATA += config_upgrade_1.0_to_2.0.sh
sqlscripts_DATA += masterdb_create.mysql
sqlscripts_DATA += master_upgrade_0.0_to_1.0.mysql
sqlscripts_DATA += master_upgrade_0.0_to_1.0.sh
sqlscripts_DATA += upgrade_1.0_to_2.0.sh
sqlscripts_DATA += upgrade_2.0_to_3.0.sh
sqlscripts_DATA += upgrade_3.0_to_4.0.sh
sqlscripts_DATA += upgrade_4.0_to_4.1.sh
sqlscripts_DATA += upgrade_4.1_to_5.0.sh
sqlscripts_DATA += upgrade_5.0_to_5.1.sh
sqlscripts_DATA += upgrade_5.1_to_5.2.sh
sqlscripts_DATA += upgrade_5.2_to_6.0.sh
sqlscripts_DATA += upgrade_6.0_to_7.0.sh
sqlscripts_DATA += upgrade_7.0_to_8.0.sh
sqlscripts_DATA += upgrade_8.0_to_8.1.sh
sqlscripts_DATA += upgrade_8.1_to_8.2.sh
sqlscripts_DATA += upgrade_8.2_to_9.0.sh
sqlscripts_DATA += upgrade_9.0_to_9.1.sh
sqlscripts_DATA += upgrade_9.1_to_9.2.sh
sqlscripts_DATA += upgrade_9.2_to_9.3.sh
sqlscripts_DATA += wipe_data.sh

DISTCLEANFILES  = config_upgrade_0.0_to_1.0.mysql
DISTCLEANFILES += config_upgrade_0.0_to_1.0.sh
DISTCLEANFILES += config_upgrade_1.0_to_2.0.mysql
DISTCLEANFILES += config_upgrade_1.0_to_2.0.sh
DISTCLEANFILES += master_upgrade_0.0_to_1.0.mysql
DISTCLEANFILES += master_upgrade_0.0_to_1.0.sh
DISTCLEANFILES += upgrade_1.0_to_2.0.sh
DISTCLEANFILES += upgrade_2.0_to_3.0.sh
DISTCLEANFILES += upgrade_3.0_to_4.0.sh
DISTCLEANFILES += upgrade_4.0_to_4.1.sh
DISTCLEANFILES += upgrade_4.1_to_5.0.sh
DISTCLEANFILES += upgrade_5.0_to_5.1.sh
DISTCLEANFILES += upgrade_5.1_to_5.2.sh
DISTCLEANFILES += upgrade_5.2_to_6.0.sh
DISTCLEANFILES += upgrade_6.0_to_7.0.sh
DISTCLEANFILES += upgrade_7.0_to_8.0.sh
DISTCLEANFILES += upgrade_8.1_to_8.2.sh
DISTCLEANFILES += upgrade_8.2_to_9.0.sh
DISTCLEANFILES += upgrade_9.0_to_9.1.sh
DISTCLEANFILES += upgrade_9.1_to_9.2.sh
DISTCLEANFILES += upgrade_9.2_to_9.3.sh
DISTCLEANFILES += wipe_data.sh

CLEANFILES = ${sqlscripts_DATA}
EXTRA_DIST = ${sqlscripts_DATA}

config_upgrade_0.0_to_1.0.mysql: config_upgrade_0.0_to_1.0.mysql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

config_upgrade_1.0_to_2.0.mysql: config_upgrade_1.0_to_2.0.mysql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

configdb_create.mysql: configdb_create.mysql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

dhcpdb_create.mysql: dhcpdb_create.mysql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

dhcpdb_drop.mysql: dhcpdb_drop.mysql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

configdb_drop.mysql: configdb_drop.mysql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

masterdb_drop.mysql: masterdb_drop.mysql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

masterdb_create.mysql: masterdb_create.mysql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@

master_upgrade_0.0_to_1.0.mysql: master_upgrade_0.0_to_1.0.mysql.in
	$(top_builddir)/tools/defines_resolver.sh $< $@


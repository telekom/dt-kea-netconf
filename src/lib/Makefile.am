# The following build order must be maintained.
SUBDIRS = exceptions util log cryptolink dns asiolink cc testutils database stats

if HAVE_CQL
SUBDIRS += cql
endif

if HAVE_MYSQL
SUBDIRS += mysql
endif

if HAVE_PGSQL
SUBDIRS += pgsql
endif

SUBDIRS += config_backend hooks dhcp config asiodns dhcp_ddns eval cfgrpt process dhcpsrv http

if HAVE_SYSREPO
SUBDIRS += yang
endif

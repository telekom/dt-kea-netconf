SUBDIRS = .

yangmodulesdir = ${datarootdir}/${PACKAGE_NAME}/yang/modules

yangmodules_list  =
yangmodules_list += dhcpv6-server-stats-augment.yang
yangmodules_list += ietf-dhcpv6-client.yang
yangmodules_list += ietf-dhcpv6-common.yang
yangmodules_list += ietf-dhcpv6-options-rfc3319-server.yang
yangmodules_list += ietf-dhcpv6-options-rfc3646-server.yang
yangmodules_list += ietf-dhcpv6-options-rfc3898-server.yang
yangmodules_list += ietf-dhcpv6-options-rfc4075-server.yang
yangmodules_list += ietf-dhcpv6-options-rfc4280-server.yang
yangmodules_list += ietf-dhcpv6-options-rfc4704-server.yang
yangmodules_list += ietf-dhcpv6-options-rfc4833-server.yang
yangmodules_list += ietf-dhcpv6-options-rfc5908-server.yang
yangmodules_list += ietf-dhcpv6-options-rfc6603-server.yang
yangmodules_list += ietf-dhcpv6-options-rfc7341-server.yang
yangmodules_list += ietf-dhcpv6-options-rfc7598-server.yang
yangmodules_list += ietf-dhcpv6-options-rfc8415-server.yang
yangmodules_list += ietf-dhcpv6-relay.yang
yangmodules_list += ietf-dhcpv6-server.yang
yangmodules_list += ietf-inet-types.yang
yangmodules_list += ietf-interfaces.yang
yangmodules_list += ietf-netconf-acm.yang
yangmodules_list += ietf-yang-types.yang
yangmodules_list += kea-augments-to-ietf-dhcpv6-server.yang
yangmodules_list += kea-ctrl-agent.yang
yangmodules_list += kea-dhcp-ddns.yang
yangmodules_list += kea-dhcp-types.yang
yangmodules_list += kea-dhcp4-server.yang
yangmodules_list += kea-dhcp6-server.yang
yangmodules_list += kea-types.yang
yangmodules_list += keatest-module.yang

if HAVE_SYSREPO
yangmodules_DATA  = ${yangmodules_list}
endif

EXTRA_DIST = ${yangmodules_list}

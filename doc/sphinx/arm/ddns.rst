.. _dhcp-ddns-server:

********************
The DHCP-DDNS Server
********************

.. _dhcp-ddns-overview:

Overview
========

The DHCP-DDNS Server (kea-dhcp-ddns, known informally as D2) conducts
the client side of the Dynamic DNS protocol (DDNS, defined in `RFC
2136 <https://tools.ietf.org/html/rfc2136>`__) on behalf of the DHCPv4
and DHCPv6 servers (kea-dhcp4 and kea-dhcp6 respectively). The DHCP
servers construct DDNS update requests, known as Name Change Requests
(NCRs), based on DHCP lease change events and then post them to D2. D2
attempts to match each request to the appropriate DNS server(s) and
carries out the necessary conversation with those servers to update the
DNS data.

.. _dhcp-ddns-dns-server-selection:

DNS Server Selection
--------------------

In order to match a request to the appropriate DNS servers, D2 must have
a catalog of servers from which to select. In fact, D2 has two such
catalogs, one for forward DNS and one for reverse DNS; these catalogs
are referred to as DDNS Domain Lists. Each list consists of one or more
named DDNS Domains. Further, each DDNS Domain has a list of one or more
DNS servers that publish the DNS data for that domain.

When conducting forward domain matching, D2 compares the fully qualified
domain name (FQDN) in the request against the name of each Forward DDNS
Domain in its catalog. The domain whose name matches the longest portion
of the FQDN is considered the best match. For example, if the FQDN is
"myhost.sample.example.com.", and there are two forward domains in the
catalog, "sample.example.com." and "example.com.", the former is
regarded as the best match. In some cases, it may not be possible to
find a suitable match. Given the same two forward domains there would be
no match for the FQDN, "bogus.net", so the request would be rejected.
Finally, if there are no Forward DDNS Domains defined, D2 simply
disregards the forward update portion of requests.

When conducting reverse domain matching, D2 constructs a reverse FQDN
from the lease address in the request and compares that against the name
of each Reverse DDNS Domain. Again, the domain whose name matches the
longest portion of the FQDN is considered the best match. For instance,
if the lease address is "172.16.1.40" and there are two reverse domains
in the catalog, "1.16.172.in-addr.arpa." and "16.172.in-addr.arpa", the
former is the best match. As with forward matching, it may not find a
suitable match. Given the same two domains, there would be no match for
the lease address, "192.168.1.50", and the request would be rejected.
Finally, if there are no Reverse DDNS Domains defined, D2 simply
disregards the reverse update portion of requests.

.. _dhcp-ddns-conflict-resolution:

Conflict Resolution
-------------------

D2 implements the conflict resolution strategy prescribed by `RFC
4703 <https://tools.ietf.org/html/rfc4703>`__. Conflict resolution is
intended to prevent different clients from mapping to the same FQDN at
the same time. To make this possible, the RFC requires that forward DNS
entries for a given FQDN must be accompanied by a DHCID resource record
(RR). This record contains a client identifier that uniquely identifies
the client to whom the name belongs. Furthermore, any DNS updater that
wishes to update or remove existing forward entries for an FQDN may only
do so if their client matches that of the DHCID RR.

In other words, the DHCID RR maps an FQDN to the client to whom it
belongs, and thereafter changes to that mapping should only be done by
or at the behest of that client.

Currently, conflict detection is always performed.

.. _dhcp-ddns-dual-stack:

Dual-Stack Environments
-----------------------

`RFC 4703, section
5.2, <https://tools.ietf.org/html/rfc4703#section-5.2>`__ describes
issues that may arise with dual-stack clients. These are clients that
wish to have have both IPv4 and IPv6 mappings for the same FQDN. For
this to work properly, the clients are required to embed their IPv6 DUID
within their IPv4 client identifier option, as described in `RFC
4703 <https://tools.ietf.org/html/rfc4361>`__. In this way, DNS updates
for both IPv4 and IPv6 can be managed under the same DHCID RR. Kea does not
currently support this feature.

.. _dhcp-ddns-server-start-stop:

Starting and Stopping the DHCP-DDNS Server
==========================================

``kea-dhcp-ddns`` is the Kea DHCP-DDNS server and, due to the nature of
DDNS, it runs alongside either the DHCPv4 or DHCPv6 component (or both).
Like other parts of Kea, it is a separate binary that can be run on its
own or through ``keactrl`` (see :ref:`keactrl`). In normal
operation, controlling ``kea-dhcp-ddns`` with ``keactrl`` is
recommended; however, it is also possible to run the DHCP-DDNS server
directly. It accepts the following command-line switches:

-  ``-c file`` - specifies the configuration file. This is the only
   mandatory switch.

-  ``-d`` - specifies whether the server logging should be switched to
   debug/verbose mode. In verbose mode, the logging severity and
   debuglevel specified in the configuration file are ignored and
   "debug" severity and the maximum debuglevel (99) are assumed. The
   flag is convenient for temporarily switching the server into maximum
   verbosity, e.g. when debugging.

-  ``-v`` - displays the Kea version and exits.

-  ``-W`` - displays the Kea configuration report and exits. The report
   is a copy of the ``config.report`` file produced by ``./configure``;
   it is embedded in the executable binary.

-  ``-t file`` - specifies the configuration file to be tested.
   Kea-dhcp-ddns will attempt to load it and will conduct sanity checks.
   Note that certain checks are possible only while running the actual
   server. The actual status is reported with an exit code (0 =
   configuration looks ok, 1 = error encountered). Kea prints out log
   messages to standard output and errors to standard error when testing
   the configuration.

The ``config.report`` may also be accessed more directly, via the
following command. The binary ``path`` may be found in the install
directory or in the ``.libs`` subdirectory in the source tree. For
example: ``kea/src/bin/d2/.libs/kea-dhcp-ddns``.

::

   strings path/kea-dhcp-ddns | sed -n 's/;;;; //p'

Upon startup, the module will load its configuration and begin listening
for NCRs based on that configuration.

During startup, the server will attempt to create a PID file of the form:
[**runstatedir**]/[**conf name**].kea-dhcp-ddns.pid where:

-  ``runstatedir`` - is the value as passed into the build configure
   script; it defaults to "/usr/local/var/run". Note that this value may be
   overridden at runtime by setting the environment variable
   KEA_PIDFILE_DIR. This is intended primarily for testing purposes.

-  ``conf name`` - is the configuration file name used to start the server,
   minus all preceding paths and the file extension. For example, given
   a pathname of "/usr/local/etc/kea/myconf.txt", the portion used would
   be "myconf".

If the file already exists and contains the PID of a live process, the
server will issue a DHCP_DDNS_ALREADY_RUNNING log message and exit. It
is possible, though unlikely, that the file is a remnant of a system
crash and the process to which the PID belongs is unrelated to Kea. In
such a case it is necessary to manually delete the PID file.

.. _d2-configuration:

Configuring the DHCP-DDNS Server
================================

Before starting the ``kea-dhcp-ddns`` module for the first time, a
configuration file must be created. The following default configuration
is a template that can be customized to individual requirements.

::

   "DhcpDdns": {
       "ip-address": "127.0.0.1",
       "port": 53001,
       "dns-server-timeout": 100,
       "ncr-protocol": "UDP",
       "ncr-format": "JSON",
       "tsig-keys": [ ],
       "forward-ddns": {
           "ddns-domains": [ ]
       },
       "reverse-ddns": {
           "ddns-domains": [ ]
       }
   }

The configuration can be divided into the following sections, each of
which is described below:

-  *Global Server Parameters* - define values which control connectivity and
   global server behavior.

-  *Control Socket* - defines the Control Socket type and name.

-  *TSIG Key Info* - defines the TSIG keys used for secure traffic with
   DNS servers.

-  *Forward DDNS* - defines the catalog of Forward DDNS Domains.

-  *Reverse DDNS* - defines the catalog of Forward DDNS Domains.

.. _d2-server-parameter-config:

Global Server Parameters
------------------------

-  ``ip-address`` - the IP address on which D2 listens for requests. The
   default is the local loopback interface at address 127.0.0.1.
   Either an IPv4 or IPv6 address may be specified.

-  ``port`` - the port on which D2 listens for requests. The default value
   is 53001.

-  ``dns-server-timeout`` - the maximum amount of time, in milliseconds,
   that D2 will wait for a response from a DNS server to a single DNS
   update message.

-  ``ncr-protocol`` - the socket protocol to use when sending requests to
   D2. Currently only UDP is supported.

-  ``ncr-format`` - the packet format to use when sending requests to D2.
   Currently only JSON format is supported.

D2 must listen for change requests on a known address and port. By
default it listens at 127.0.0.1 on port 53001. The following example
illustrates how to change D2's global parameters so it will listen at
192.168.1.10 port 900:

::

   "DhcpDdns": {
       "ip-address": "192.168.1.10",
       "port": 900,
       ...
       }
   }

..

.. warning::

   It is possible for a malicious attacker to send bogus
   NameChangeRequests to the DHCP-DDNS server. Addresses other than the
   IPv4 or IPv6 loopback addresses (127.0.0.1 or ::1) should only be
   used for testing purposes; note that local users may still
   communicate with the DHCP-DDNS server.

.. note::

   If the ip-address and port are changed, the corresponding values in
   the DHCP servers' "dhcp-ddns" configuration section must be changed.

.. _d2-ctrl-channel:

Management API for the D2 Server
--------------------------------

The management API allows the issuing of specific management commands,
such as configuration retrieval or shutdown. For more details, see
:ref:`ctrl-channel`. Currently, the only supported communication
channel type is UNIX stream socket. By default there are no sockets
open; to instruct Kea to open a socket, the following entry in the
configuration file can be used:

::

   "DhcpDdns": {
       "control-socket": {
           "socket-type": "unix",
           "socket-name": "/path/to/the/unix/socket"
       },
       ...
   }

The length of the path specified by the ``socket-name`` parameter is
restricted by the maximum length for the UNIX socket name on the
operating system, i.e. the size of the ``sun_path`` field in the
``sockaddr_un`` structure, decreased by 1. This value varies on
different operating systems between 91 and 107 characters. Typical
values are 107 on Linux and 103 on FreeBSD.

Communication over the control channel is conducted using JSON structures.
See the `Control Channel section in the Kea Developer's
Guide <https://jenkins.isc.org/job/Kea_doc/doxygen/d2/d96/ctrlSocket.html>`__
for more details.

The D2 server supports the following operational commands:

-  build-report
-  config-get
-  config-reload
-  config-set
-  config-test
-  config-write
-  list-commands
-  shutdown
-  status-get
-  version-get

The ``shutdown`` command supports extra ``type`` argument which controls the
way the D2 server cleans up on exit.
The supported shutdown types are:

-  ``normal`` - Stops the queue manager and finishes all current transactions
   before exiting. This is the default.

-  ``drain_first`` - Stops the queue manager but continues processing requests
   from the queue until it is empty.

-  ``now`` - Exits immediately.

An example command may look like this:

::

   {
       "command": "shutdown"
       "arguments": {
           "exit-value": 3,
           "type": "drain_first"
       }
   }

.. _d2-tsig-key-list-config:

TSIG Key List
-------------

A DDNS protocol exchange can be conducted with or without TSIG (defined
in `RFC 2845 <https://tools.ietf/org/html/rfc2845>`__). This
configuration section allows the administrator to define the set of TSIG
keys that may be used in such exchanges.

To use TSIG when updating entries in a DNS domain, a key must be defined
in the TSIG Key list and referenced by name in that domain's
configuration entry. When D2 matches a change request to a domain, it
checks whether the domain has a TSIG key associated with it. If so, D2
uses that key to sign DNS update messages sent to and verify
responses received from the domain's DNS server(s). For each TSIG key
required by the DNS servers that D2 will be working with, there must be
a corresponding TSIG key in the TSIG Key list.

As one might gather from the name, the tsig-key section of the D2
configuration lists the TSIG keys. Each entry describes a TSIG key used
by one or more DNS servers to authenticate requests and sign responses.
Every entry in the list has three parameters:

-  ``name`` - is a unique text label used to identify this key within the
   list. This value is used to specify which key (if any) should be used
   when updating a specific domain. As long as the name is unique its
   content is arbitrary, although for clarity and ease of maintenance it
   is recommended that it match the name used on the DNS server(s). This
   field cannot be blank.

-  ``algorithm`` - specifies which hashing algorithm should be used with
   this key. This value must specify the same algorithm used for the key
   on the DNS server(s). The supported algorithms are listed below:

   -  HMAC-MD5
   -  HMAC-SHA1
   -  HMAC-SHA224
   -  HMAC-SHA256
   -  HMAC-SHA384
   -  HMAC-SHA512

   This value is not case-sensitive.

-  ``digest-bits`` - is used to specify the minimum truncated length in
   bits. The default value 0 means truncation is forbidden; non-zero
   values must be an integral number of octets, and be greater than both
   80 and half of the full length. (Note that in BIND 9 this parameter
   is appended after a dash to the algorithm name.)

-  ``secret`` - is used to specify the shared secret key code for this
   key. This value is case-sensitive and must exactly match the value
   specified on the DNS server(s). It is a base64-encoded text value.

As an example, suppose that a domain D2 will be updating is maintained
by a BIND 9 DNS server, which requires dynamic updates to be secured
with TSIG. Suppose further that the entry for the TSIG key in BIND 9's
named.conf file looks like this:

::

      :
      key "key.four.example.com." {
          algorithm hmac-sha224;
          secret "bZEG7Ow8OgAUPfLWV3aAUQ==";
      };
      :

By default, the TSIG Key list is empty:

::

   "DhcpDdns": {
      "tsig-keys": [ ],
      ...
   }

We must extend the list with a new key:

::

   "DhcpDdns": {
       "tsig-keys": [
           {
               "name": "key.four.example.com.",
               "algorithm": "HMAC-SHA224",
               "secret": "bZEG7Ow8OgAUPfLWV3aAUQ=="
           }
       ],
       ...
   }

These steps would be repeated for each TSIG key needed. Note that the
same TSIG key can be used with more than one domain.

.. _d2-forward-ddns-config:

Forward DDNS
------------

The Forward DDNS section is used to configure D2's forward update
behavior. Currently it contains a single parameter, the catalog of
Forward DDNS Domains, which is a list of structures.

::

   "DhcpDdns": {
       "forward-ddns": {
           "ddns-domains": [ ]
       },
       ...
   }

By default, this list is empty, which will cause the server to ignore
the forward update portions of requests.

.. _add-forward-ddns-domain:

Adding Forward DDNS Domains
~~~~~~~~~~~~~~~~~~~~~~~~~~~

A Forward DDNS Domain maps a forward DNS zone to a set of DNS servers
which maintain the forward DNS data (i.e. name-to-address mapping) for
that zone. Each zone served needs one Forward DDNS Domain. It may very
well be that some or all of the zones are maintained by the same
servers, but one DDNS Domain is still needed for each zone. Remember that
matching a request to the appropriate server(s) is done by zone and a
DDNS Domain only defines a single zone.

This section describes how to add Forward DDNS Domains; repeat these
steps for each Forward DDNS Domain desired. Each Forward DDNS Domain has
the following parameters:

-  ``name`` - the fully qualified domain name (or zone) that this DDNS
   Domain can update. This value is compared against the request FQDN
   during forward matching. It must be unique within the catalog.

-  ``key-name`` - if TSIG is used with this domain's servers, this value
   should be the name of the key from the TSIG Key list. If the
   value is blank (the default), TSIG will not be used in DDNS
   conversations with this domain's servers.

-  ``dns-servers`` - a list of one or more DNS servers which can conduct
   the server side of the DDNS protocol for this domain. The servers are
   used in a first-to-last preference; in other words, when D2 begins to
   process a request for this domain, it will pick the first server in
   this list and attempt to communicate with it. If that attempt fails,
   D2 will move to next one in the list and so on until either it
   is successful or the list is exhausted.

To create a new Forward DDNS Domain, add a new domain element and set
its parameters:

::

   "DhcpDdns": {
       "forward-ddns": {
           "ddns-domains": [
               {
                   "name": "other.example.com.",
                   "key-name": "",
                   "dns-servers": [
                   ]
               }
           ]
       }
   }

It is possible to add a domain without any servers; however, if that
domain matches a request, the request will fail. To make the domain
useful, at least one DNS server must be added to it.

.. _add-forward-dns-servers:

Adding Forward DNS Servers
^^^^^^^^^^^^^^^^^^^^^^^^^^

This section describes how to add DNS servers to a Forward DDNS Domain.
Repeat these instructions as needed for all the servers in each domain.

Forward DNS Server entries represent actual DNS servers which support
the server side of the DDNS protocol. Each Forward DNS Server has the
following parameters:

-  ``hostname`` - the resolvable host name of the DNS server; this
   parameter is not yet implemented.

-  ``ip-address`` - the IP address at which the server listens for DDNS
   requests. This may be either an IPv4 or an IPv6 address.

-  ``port`` - the port on which the server listens for DDNS requests. It
   defaults to the standard DNS service port of 53.

To create a new Forward DNS Server, a new server element must be added to
the domain and its parameters filled in. If, for example, the service is
running at "172.88.99.10", set the Forward DNS Server as follows:

::

   "DhcpDdns": {
       "forward-ddns": {
           "ddns-domains": [
               {
                   "name": "other.example.com.",
                   "key-name": "",
                   "dns-servers": [
                       {
                           "hostname": "",
                           "ip-address": "172.88.99.10",
                           "port": 53
                       }
                   ]
               }
           ]
       }
   }

..

.. note::

   Since "hostname" is not yet supported, the parameter "ip-address"
   must be set to the address of the DNS server.

.. _d2-reverse-ddns-config:

Reverse DDNS
------------

The Reverse DDNS section is used to configure D2's reverse update
behavior, and the concepts are the same as for the forward DDNS section.
Currently it contains a single parameter, the catalog of Reverse DDNS
Domains, which is a list of structures.

::

   "DhcpDdns": {
       "reverse-ddns": {
           "ddns-domains": [ ]
       }
       ...
   }

By default, this list is empty, which will cause the server to ignore
the reverse update portions of requests.

.. _add-reverse-ddns-domain:

Adding Reverse DDNS Domains
~~~~~~~~~~~~~~~~~~~~~~~~~~~

A Reverse DDNS Domain maps a reverse DNS zone to a set of DNS servers
which maintain the reverse DNS data (address-to-name mapping) for that
zone. Each zone served needs one Reverse DDNS Domain. It may very well
be that some or all of the zones are maintained by the same servers, but
one DDNS Domain entry is still needed for each zone. Remember that
matching a request to the appropriate server(s) is done by zone and a
DDNS Domain only defines a single zone.

This section describes how to add Reverse DDNS Domains; repeat these
steps for each Reverse DDNS Domain desired. Each Reverse DDNS Domain has
the following parameters:

-  ``name`` - the fully qualified reverse zone that this DDNS domain can
   update. This is the value used during reverse matching, which will
   compare it with a reversed version of the request's lease address.
   The zone name should follow the appropriate standards; for example,
   to support the IPv4 subnet 172.16.1, the name should be
   "1.16.172.in-addr.arpa.". Similarly, to support an IPv6 subnet of
   2001:db8:1, the name should be "1.0.0.0.8.B.D.0.1.0.0.2.ip6.arpa."
   Whatever the name, it must be unique within the catalog.

-  ``key-name`` - if TSIG is used with this domain's servers,
   this value should be the name of the key from the TSIG Key List. If
   the value is blank (the default), TSIG will not be used in DDNS
   conversations with this domain's servers. Currently this value is not
   used as TSIG has not been implemented.

-  ``dns-servers`` - a list of one or more DNS servers which can conduct
   the server side of the DDNS protocol for this domain. Currently, the
   servers are used in a first-to-last preference; in other words, when
   D2 begins to process a request for this domain, it will pick the
   first server in this list and attempt to communicate with it. If that
   attempt fails, D2 will move to the next one in the list and so on
   until either it is successful or the list is exhausted.

To create a new Reverse DDNS Domain, a new domain element must be added
and its parameters set. For example, to support subnet 2001:db8:1::, the
following configuration could be used:

::

   "DhcpDdns": {
       "reverse-ddns": {
           "ddns-domains": [
               {
                   "name": "1.0.0.0.8.B.D.0.1.0.0.2.ip6.arpa.",
                   "key-name": "",
                   "dns-servers": [
                   ]
               }
           ]
       }
   }

It is possible to add a domain without any servers; however, if that
domain matches a request, the request will fail. To make the domain
useful, at least one DNS server must be added to it.

.. _add-reverse-dns-servers:

Adding Reverse DNS Servers
^^^^^^^^^^^^^^^^^^^^^^^^^^

This section describes how to add DNS servers to a Reverse DDNS Domain.
Repeat these instructions as needed for all the servers in each domain.

Reverse DNS Server entries represent actual DNS servers which support
the server side of the DDNS protocol. Each Reverse DNS Server has the
following parameters:

-  ``hostname`` - the resolvable host name of the DNS server; this value
   is currently ignored.

-  ``ip-address`` - the IP address at which the server listens for DDNS
   requests.

-  ``port`` - the port on which the server listens for DDNS requests. It
   defaults to the standard DNS service port of 53.

To create a new reverse DNS Server, a new server
element must be added to the domain and its parameters filled in. If, for example, the
service is running at "172.88.99.10", then set it as follows:

::

   "DhcpDdns": {
       "reverse-ddns": {
           "ddns-domains": [
               {
                   "name": "1.0.0.0.8.B.D.0.1.0.0.2.ip6.arpa.",
                   "key-name": "",
                   "dns-servers": [
                       {
                           "hostname": "",
                           "ip-address": "172.88.99.10",
                           "port": 53
                       }
                   ]
               }
           ]
       }
   }

..

.. note::

   Since "hostname" is not yet supported, the parameter "ip-address"
   must be set to the address of the DNS server.

.. _d2-user-contexts:

User Contexts in DDNS
---------------------

.. note::

   User contexts were designed for hook libraries, which are not yet
   supported for DHCP-DDNS server configuration.

See :ref:`user-context` for additional background regarding the user
context idea.

User contexts can be specified on global scope, DDNS domain, DNS server,
TSIG key, and loggers. One other useful usage is the ability to store
comments or descriptions; the parser translates a "comment" entry into a
user context with the entry, which allows a comment to be attached
inside the configuration itself.

.. _d2-example-config:

Example DHCP-DDNS Server Configuration
--------------------------------------

This section provides a sample DHCP-DDNS server configuration, based on
a small example network. Let's suppose our example network has three
domains, each with their own subnet.

.. table:: Our Example Network

   +------------------+-----------------+-----------------+-----------------+
   | Domain           | Subnet          | Forward DNS     | Reverse DNS     |
   |                  |                 | Servers         | Servers         |
   +==================+=================+=================+=================+
   | four.example.com | 192.0.2.0/24    | 172.16.1.5,     | 172.16.1.5,     |
   |                  |                 | 172.16.2.5      | 172.16.2.5      |
   +------------------+-----------------+-----------------+-----------------+
   | six.example.com  | 2001:db8:1::/64 | 3001:1::50      | 3001:1::51      |
   +------------------+-----------------+-----------------+-----------------+
   | example.com      | 192.0.0.0/16    | 172.16.2.5      | 172.16.2.5      |
   +------------------+-----------------+-----------------+-----------------+

We need to construct three Forward DDNS Domains:

.. table:: Forward DDNS Domains Needed

   +----+-------------------+------------------------+
   | #  | DDNS Domain Name  | DNS Servers            |
   +====+===================+========================+
   | 1. | four.example.com. | 172.16.1.5, 172.16.2.5 |
   +----+-------------------+------------------------+
   | 2. | six.example.com.  | 3001:1::50             |
   +----+-------------------+------------------------+
   | 3. | example.com.      | 172.16.2.5             |
   +----+-------------------+------------------------+

As discussed earlier, FQDN-to-domain matching is based on the longest
match. The FQDN, "myhost.four.example.com.", will match the first domain
("four.example.com") while "admin.example.com." will match the third
domain ("example.com"). The FQDN, "other.example.net.", will fail to
match any domain and is rejected.

The following example configuration specifies the Forward DDNS Domains.

::

   "DhcpDdns": {
       "comment": "example configuration: forward part",
       "forward-ddns": {
           "ddns-domains": [
               {
                   "name": "four.example.com.",
                   "key-name": "",
                   "dns-servers": [
                       { "ip-address": "172.16.1.5" },
                       { "ip-address": "172.16.2.5" }
                   ]
               },
               {
                   "name": "six.example.com.",
                   "key-name": "",
                   "dns-servers": [
                       { "ip-address": "2001:db8::1" }
                   ]
               },
               {
                   "name": "example.com.",
                   "key-name": "",
                   "dns-servers": [
                       { "ip-address": "172.16.2.5" }
                   ],
                   "user-context": { "backup": false }
               },

           ]
       }
   }

Similarly, we need to construct the three Reverse DDNS Domains:

.. table:: Reverse DDNS Domains Needed

   +----+-----------------------------------+------------------------+
   | #  | DDNS Domain Name                  | DNS Servers            |
   +====+===================================+========================+
   | 1. | 2.0.192.in-addr.arpa.             | 172.16.1.5, 172.16.2.5 |
   +----+-----------------------------------+------------------------+
   | 2. | 1.0.0.0.8.d.b.0.1.0.0.2.ip6.arpa. | 3001:1::50             |
   +----+-----------------------------------+------------------------+
   | 3. | 0.182.in-addr.arpa.               | 172.16.2.5             |
   +----+-----------------------------------+------------------------+

An address of "192.0.2.150" will match the first domain,
"2001:db8:1::10" will match the second domain, and "192.0.50.77" the
third domain.

These Reverse DDNS Domains are specified as follows:

::

   "DhcpDdns": {
       "comment": "example configuration: reverse part",
       "reverse-ddns": {
           "ddns-domains": [
               {
                   "name": "2.0.192.in-addr.arpa.",
                   "key-name": "",
                   "dns-servers": [
                       { "ip-address": "172.16.1.5" },
                       { "ip-address": "172.16.2.5" }
                   ]
               }
               {
                   "name": "1.0.0.0.8.B.D.0.1.0.0.2.ip6.arpa.",
                   "key-name": "",
                   "dns-servers": [
                       { "ip-address": "2001:db8::1" }
                   ]
               }
               {
                   "name": "0.192.in-addr.arpa.",
                   "key-name": "",
                   "dns-servers": [
                       { "ip-address": "172.16.2.5" }
                   ]
               }
           ]
       }
   }

DHCP-DDNS Server Limitations
============================

The following are the current limitations of the DHCP-DDNS Server.

-  Requests received from the DHCP servers are placed in a queue until
   they are processed. Currently, all queued requests are lost if the
   server shuts down.

Supported Standards
===================

The following RFCs are supported by the DHCP-DDNS server:

- *Secret Key Transaction Authentication for DNS (TSIG)*, `RFC 2845
  <https://tools.ietf.org/html/rfc2845>`__: All DNS Update packets sent and
  received by DHCP-DDNS server can be protected by TSIG signatures.

- *Dynamic Updates in the Domain Name System (DNS UPDATE)*, `RFC 2136
  <https://tools.ietf.org/html/rfc2136>`__: The whole DNS Update mechanism is
  supported.

- *Resolution of Fully Qualified Domain Name (FQDN) Conflicts among Dynamic Host
  Configuration Protocol (DHCP) Clients*, `RFC 4703
  <https://tools.ietf.org/html/rfc4703>`__: The DHCP-DDNS takes care of the
  conflict resolution. This capability is used by DHCPv4 and DHCPv6 servers.

- *A DNS Resource Record (RR) for Encoding Dynamic Host Configuration Protocol
  (DHCP) Information (DHCID RR)*, `RFC 4701
  <https://tools.ietf.org/html/rfc4701>`__: The DHCP-DDNS server uses the DHCID
  records.

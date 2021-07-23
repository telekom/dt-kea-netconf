.. _logging:

*******
Logging
*******

Logging Configuration
=====================

During its operation Kea may produce many messages. They differ in
severity (some are more important than others) and source (different
components, like hooks, produce different messages). It is useful to
understand which log messages are critical and which are not, and to
configure logging appropriately. For example, debug-level messages
can be safely ignored in a typical deployment. They are, however, very
useful when debugging a problem.

The logging system in Kea is configured through the loggers entry in the
server section of your configuration file. In previous Kea releases this
entry was in an independent Logging section; this was still supported
for backward compatibility until Kea 1.7.9 included.

Loggers
-------

Within Kea, a message is logged through an entity called a "logger."
Different components log messages through different loggers, and each
logger can be configured independently of the others. Some components,
in particular the DHCP server processes, may use multiple loggers to log
messages pertaining to different logical functions of the component. For
example, the DHCPv4 server uses one logger for messages about packet
reception and transmission, another logger for messages related to lease
allocation, and so on. Some of the libraries used by the Kea server,
such as libdhcpsrv, use their own loggers.

Users implementing hooks libraries (code attached to the server at
runtime) are responsible for creating the loggers used by those
libraries. Such loggers should have unique names, different from the
logger names used by Kea. In this way the messages produced by the hooks
library can be distinguished from messages issued by the core Kea code.
Unique names also allow the loggers to be configured independently of
loggers used by Kea. Whenever it makes sense, a hooks library can use
multiple loggers to log messages pertaining to different logical parts
of the library.

In the server section of a configuration file the
configuration for zero or more loggers (including loggers used by the
proprietary hooks libraries) can be specified. If there are no loggers specified, the
code will use default values; these cause Kea to log messages of INFO
severity or greater to standard output. There is a small time window
after Kea has been started but before it has read its configuration;
logging in this short period can be controlled using environment
variables. For details, see :ref:`logging-during-startup`.

The three main elements of a logger configuration are: ``name`` (the
component that is generating the messages), ``severity`` (what to log),
and ``output_commands`` (where to log). There is also a ``debuglevel``
element, which is only relevant if debug-level logging has been
selected.

The name (string) Logger
~~~~~~~~~~~~~~~~~~~~~~~~

Each logger in the system has a name: that of the component binary file
using it to log messages. For instance, to configure logging
for the DHCPv4 server, add an entry for a logger named “kea-dhcp4”.
This configuration will then be used by the loggers in the DHCPv4
server and all the libraries used by it, unless a library defines its
own logger and there is a specific logger configuration that applies to
that logger.

When tracking down an issue with the server's operation, use of DEBUG
logging is required to obtain the verbose output needed for problem
diagnosis. However, the high verbosity is likely to overwhelm the
logging system in cases where the server is processing high-volume
traffic. To mitigate this problem, Kea can use multiple loggers, for
different functional parts of the server, that can each be configured
independently. If the user is reasonably confident that a problem
originates in a specific function of the server, or that the problem is
related to a specific type of operation, they may enable high verbosity
only for the relevant logger, thereby limiting the debug messages to the
required minimum.

The loggers are associated with a particular library or binary of Kea.
However, each library or binary may (and usually does) include multiple
loggers. For example, the DHCPv4 server binary contains separate loggers
for packet parsing, dropped packets, callouts, etc.

The loggers form a hierarchy. For each program in Kea, there is a "root"
logger, named after the program (e.g. the root logger for kea-dhcp, the
DHCPv4 server) is named kea-dhcp4. All other loggers are children of
this logger and are named accordingly, e.g. the allocation engine in the
DHCPv4 server logs messages using a logger called
kea-dhcp4.alloc-engine.

This relationship is important, as each child logger derives its default
configuration from its parent root logger. In the typical case, the root
logger configuration is the only logging configuration specified in the
configuration file and so applies to all loggers. If an entry is made
for a given logger, any attributes specified override those of the root
logger, whereas any not specified are inherited from it.

To illustrate this, suppose we are using the DHCPv4 server with the
root logger “kea-dhcp4” logging at the INFO level. In order to enable
DEBUG verbosity for DHCPv4 packet drops, we must create a configuration
entry for the logger called “kea-dhcp4.bad-packets” and specify severity
DEBUG for this logger. All other configuration parameters may be omitted
for this logger if the logger should use the default values specified in
the root logger's configuration.

If there are multiple logger specifications in the configuration that
might match a particular logger, the specification with the more
specific logger name takes precedence. For example, if there are entries
for both “kea-dhcp4” and “kea-dhcp4.dhcpsrv”, the main DHCPv4 server
program — and all libraries it uses other than the dhcpsrv library
(libdhcpsrv) — will log messages according to the configuration in the
first entry (“kea-dhcp4”). Messages generated by the dhcpsrv library
will be logged according to the configuration set by the second entry.

Currently defined loggers are defined in the following table. The
"Software Package" column of this table specifies whether the particular
loggers belong to the core Kea code (open source Kea binaries and
libraries), or hooks libraries (open source or premium).

.. tabularcolumns:: |p{0.2\linewidth}|p{0.2\linewidth}|p{0.6\linewidth}|

.. table:: List of Loggers Supported by Kea Servers and Hooks Libraries Shipped With Kea and Premium Packages
   :class: longtable
   :widths: 20 20 60

   +----------------------------------+------------------------+--------------------------------+
   | Logger Name                      | Software Package       | Description                    |
   +==================================+========================+================================+
   | ``kea-ctrl-agent``               | core                   | The root logger for            |
   |                                  |                        | the Control Agent              |
   |                                  |                        | exposing the RESTful           |
   |                                  |                        | control API. All               |
   |                                  |                        | components used by             |
   |                                  |                        | the Control Agent              |
   |                                  |                        | inherit the settings           |
   |                                  |                        | from this logger.              |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-ctrl-agent.http``          | core                   | A logger which                 |
   |                                  |                        | outputs log messages           |
   |                                  |                        | related to receiving,          |
   |                                  |                        | parsing, and sending           |
   |                                  |                        | HTTP messages.                 |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4``                    | core                   | The root logger for            |
   |                                  |                        | the DHCPv4 server.             |
   |                                  |                        | All components used            |
   |                                  |                        | by the DHCPv4 server           |
   |                                  |                        | inherit the settings           |
   |                                  |                        | from this logger.              |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp6``                    | core                   | The root logger for            |
   |                                  |                        | the DHCPv6 server.             |
   |                                  |                        | All components used            |
   |                                  |                        | by the DHCPv6 server           |
   |                                  |                        | inherit the settings           |
   |                                  |                        | from this logger.              |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.alloc-engine``,      | core                   | Used by the lease              |
   | ``kea-dhcp6.alloc-engine``       |                        | allocation engine,             |
   |                                  |                        | which is responsible           |
   |                                  |                        | for managing leases            |
   |                                  |                        | in the lease                   |
   |                                  |                        | database, i.e.                 |
   |                                  |                        | creating, modifying,           |
   |                                  |                        | and removing DHCP              |
   |                                  |                        | leases as a result of          |
   |                                  |                        | processing messages            |
   |                                  |                        | from clients.                  |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.bad-packets``,       | core                   | Used by the DHCP               |
   | ``kea-dhcp6.bad-packets``        |                        | servers for logging            |
   |                                  |                        | inbound client                 |
   |                                  |                        | packets that were              |
   |                                  |                        | dropped or to which            |
   |                                  |                        | the server responded           |
   |                                  |                        | with a DHCPNAK. It             |
   |                                  |                        | allows administrators          |
   |                                  |                        | to configure a                 |
   |                                  |                        | separate log output            |
   |                                  |                        | that contains only             |
   |                                  |                        | packet drop and                |
   |                                  |                        | reject entries.                |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.bootp-hooks``        | libdhcp_bootp          | This logger is used to log     |
   |                                  | hook library           | messages related to the        |
   |                                  |                        | operation of the BOOTP hook    |
   |                                  |                        | library.                       |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.callouts``,          | core                   | Used to log messages           |
   | ``kea-dhcp6.callouts``           |                        | pertaining to the              |
   |                                  |                        | callouts registration          |
   |                                  |                        | and execution for the          |
   |                                  |                        | particular hook                |
   |                                  |                        | point.                         |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.commands``,          | core                   | Used to log messages           |
   | ``kea-dhcp6.commands``           |                        | relating to the                |
   |                                  |                        | handling of commands           |
   |                                  |                        | received by the DHCP           |
   |                                  |                        | server over the                |
   |                                  |                        | command channel.               |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.database``,          | core                   | Used to log messages           |
   | ``kea-dhcp6.database``           |                        | relating to general            |
   |                                  |                        | operations on the              |
   |                                  |                        | relational databases           |
   |                                  |                        | and Cassandra.                 |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.ddns``,              | core                   | Used by the DHCP               |
   | ``kea-dhcp6.ddns``               |                        | server to log                  |
   |                                  |                        | messages related to            |
   |                                  |                        | Client FQDN and                |
   |                                  |                        | Hostname option                |
   |                                  |                        | processing. It also            |
   |                                  |                        | includes log messages          |
   |                                  |                        | related to the                 |
   |                                  |                        | relevant DNS updates.          |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.dhcp4``              | core                   | Used by the DHCPv4             |
   |                                  |                        | server daemon to log           |
   |                                  |                        | basic operations.              |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.dhcpsrv``,           | core                   | The base loggers for           |
   | ``kea-dhcp6.dhcpsrv``            |                        | the libkea-dhcpsrv             |
   |                                  |                        | library.                       |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.eval``,              | core                   | Used to log messages           |
   | ``kea-dhcp6.eval``               |                        | relating to the                |
   |                                  |                        | client classification          |
   |                                  |                        | expression evaluation          |
   |                                  |                        | code.                          |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.host-cache-hooks``,  | libdhcp_host_cache     | This logger is used            |
   | ``kea-dhcp6.host-cache-hooks``   | premium hook library   | to log messages                |
   |                                  |                        | related to the                 |
   |                                  |                        | operation of the Host          |
   |                                  |                        | Cache hooks library.           |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.flex-id-hooks``,     | libdhcp_flex_id        | This logger is used            |
   | ``kea-dhcp6.flex-id-hooks``      | premium hook library   | to log messages                |
   |                                  |                        | related to the                 |
   |                                  |                        | operation of the               |
   |                                  |                        | Flexible Identifiers           |
   |                                  |                        | hooks library.                 |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.ha-hooks``,          | libdhcp_ha hook        | This logger is used            |
   | ``kea-dhcp6.ha-hooks``           | library                | to log messages                |
   |                                  |                        | related to the                 |
   |                                  |                        | operation of the High          |
   |                                  |                        | Availability hooks             |
   |                                  |                        | library.                       |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.hooks``,             | core                   | Used to log messages           |
   | ``kea-dhcp6.hooks``              |                        | related to the                 |
   |                                  |                        | management of hooks            |
   |                                  |                        | libraries, e.g.                |
   |                                  |                        | registration and               |
   |                                  |                        | deregistration of the          |
   |                                  |                        | libraries, and to the          |
   |                                  |                        | initialization of the          |
   |                                  |                        | callouts execution             |
   |                                  |                        | for various hook               |
   |                                  |                        | points within the              |
   |                                  |                        | DHCP server.                   |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.host-cmds-hooks``,   | libdhcp_host_cmds      | This logger is used            |
   | ``kea-dhcp6.host-cmds-hooks``    | premium hook library   | to log messages                |
   |                                  |                        | related to the                 |
   |                                  |                        | operation of the Host          |
   |                                  |                        | Commands hooks                 |
   |                                  |                        | library. In general,           |
   |                                  |                        | these will pertain to          |
   |                                  |                        | the loading and                |
   |                                  |                        | unloading of the               |
   |                                  |                        | library and the                |
   |                                  |                        | execution of commands          |
   |                                  |                        | by the library.                |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.hosts``,             | core                   | Used within the                |
   | ``kea-dhcp6.hosts``              |                        | libdhcpsrv, it logs            |
   |                                  |                        | messages related to            |
   |                                  |                        | the management of              |
   |                                  |                        | DHCP host                      |
   |                                  |                        | reservations, i.e.             |
   |                                  |                        | retrieving                     |
   |                                  |                        | reservations and               |
   |                                  |                        | adding new                     |
   |                                  |                        | reservations.                  |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.lease-cmds-hooks``,  | libdhcp_lease_cmds     | This logger is used            |
   | ``kea-dhcp6.lease-cmds-hooks``   | hook library           | to log messages                |
   |                                  |                        | related to the                 |
   |                                  |                        | operation of the               |
   |                                  |                        | Lease Commands hooks           |
   |                                  |                        | library. In general,           |
   |                                  |                        | these will pertain to          |
   |                                  |                        | the loading and                |
   |                                  |                        | unloading of the               |
   |                                  |                        | library and the                |
   |                                  |                        | execution of commands          |
   |                                  |                        | by the library.                |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.leases``,            | core                   | Used by the DHCP               |
   | ``kea-dhcp6.leases``             |                        | server to log                  |
   |                                  |                        | messages related to            |
   |                                  |                        | lease allocation. The          |
   |                                  |                        | messages include               |
   |                                  |                        | detailed information           |
   |                                  |                        | about the allocated            |
   |                                  |                        | or offered leases,             |
   |                                  |                        | errors during the              |
   |                                  |                        | lease allocation,              |
   |                                  |                        | etc.                           |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.legal-log-hooks``,   | libdhcp_legal_log      | This logger is used            |
   | ``kea-dhcp6.legal-log-hooks``    | premium hook library   | to log messages                |
   |                                  |                        | related to the                 |
   |                                  |                        | operation of the               |
   |                                  |                        | Forensic Logging               |
   |                                  |                        | hooks library.                 |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.options``,           | core                   | Used by the DHCP               |
   | ``kea-dhcp6.options``            |                        | server to log                  |
   |                                  |                        | messages related to            |
   |                                  |                        | the processing of              |
   |                                  |                        | options in the DHCP            |
   |                                  |                        | messages, i.e.                 |
   |                                  |                        | parsing options,               |
   |                                  |                        | encoding options into          |
   |                                  |                        | on-wire format, and            |
   |                                  |                        | packet classification          |
   |                                  |                        | using options                  |
   |                                  |                        | contained in the               |
   |                                  |                        | received packets.              |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.packets``,           | core                   | This logger is mostly          |
   | ``kea-dhcp6.packets``            |                        | used to log messages           |
   |                                  |                        | related to                     |
   |                                  |                        | transmission of the            |
   |                                  |                        | DHCP packets, i.e.             |
   |                                  |                        | packet reception and           |
   |                                  |                        | the sending of a               |
   |                                  |                        | response. Such                 |
   |                                  |                        | messages include               |
   |                                  |                        | information about the          |
   |                                  |                        | source and                     |
   |                                  |                        | destination IP                 |
   |                                  |                        | addresses and                  |
   |                                  |                        | interfaces used to             |
   |                                  |                        | transmit packets. The          |
   |                                  |                        | logger is also used            |
   |                                  |                        | to log messages                |
   |                                  |                        | related to subnet              |
   |                                  |                        | selection, as this             |
   |                                  |                        | selection is usually           |
   |                                  |                        | based on the IP                |
   |                                  |                        | addresses, relay               |
   |                                  |                        | addresses, and/or              |
   |                                  |                        | interface names,               |
   |                                  |                        | which can be                   |
   |                                  |                        | retrieved from the             |
   |                                  |                        | received packet even           |
   |                                  |                        | before the DHCP                |
   |                                  |                        | message carried in             |
   |                                  |                        | the packet is parsed.          |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.radius-hooks``,      | libdhcp_radius         | This logger is used            |
   | ``kea-dhcp6.radius-hooks``       | premium hook library   | to log messages                |
   |                                  |                        | related to the                 |
   |                                  |                        | operation of the               |
   |                                  |                        | RADIUS hooks library.          |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.stat-cmds-hooks``,   | libdhcp_stat_cmds      | This logger is used            |
   | ``kea-dhcp6.stat-cmds-hooks``    | hook library           | to log messages                |
   |                                  |                        | related to the                 |
   |                                  |                        | operation of the               |
   |                                  |                        | Statistics Commands            |
   |                                  |                        | hooks library. In              |
   |                                  |                        | general, these will            |
   |                                  |                        | pertain to loading             |
   |                                  |                        | and unloading the              |
   |                                  |                        | library and the                |
   |                                  |                        | execution of commands          |
   |                                  |                        | by the library.                |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.subnet-cmds-hooks``, | libdhcp_subnet_cmds    | This logger is used            |
   | ``kea-dhcp6.subnet-cmds-hooks``  | hook library           | to log messages                |
   |                                  |                        | related to the                 |
   |                                  |                        | operation of the               |
   |                                  |                        | Subnet Commands hooks          |
   |                                  |                        | library. In general,           |
   |                                  |                        | these will pertain to          |
   |                                  |                        | loading and unloading          |
   |                                  |                        | the library and the            |
   |                                  |                        | execution of commands          |
   |                                  |                        | by the library.                |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.mysql-cb-hooks``,    | libdhcp_mysql_cb_hooks | This logger is used            |
   | ``kea-dhcp6.mysql-cb-hooks``     | hook library           | to log messages                |
   |                                  |                        | related to the                 |
   |                                  |                        | operation of the               |
   |                                  |                        | MySQL Configuration            |
   |                                  |                        | Backend hooks                  |
   |                                  |                        | library.                       |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp-ddns``                | core                   | The root logger for            |
   |                                  |                        | the kea-dhcp-ddns              |
   |                                  |                        | daemon. All                    |
   |                                  |                        | components used by             |
   |                                  |                        | this daemon inherit            |
   |                                  |                        | the settings from              |
   |                                  |                        | this logger unless             |
   |                                  |                        | there are                      |
   |                                  |                        | configurations for             |
   |                                  |                        | more specialized               |
   |                                  |                        | loggers.                       |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp-ddns.dctl``           | core                   | The logger used by             |
   |                                  |                        | the kea-dhcp-ddns              |
   |                                  |                        | daemon for logging             |
   |                                  |                        | basic information              |
   |                                  |                        | about the process,             |
   |                                  |                        | received signals, and          |
   |                                  |                        | triggered                      |
   |                                  |                        | reconfigurations.              |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp-ddns.dhcpddns``       | core                   | The logger used by             |
   |                                  |                        | the kea-dhcp-ddns              |
   |                                  |                        | daemon for logging             |
   |                                  |                        | events related to              |
   |                                  |                        | DDNS operations.               |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp-ddns.dhcp-to-d2``     | core                   | Used by the                    |
   |                                  |                        | kea-dhcp-ddns daemon           |
   |                                  |                        | for logging                    |
   |                                  |                        | information about              |
   |                                  |                        | events dealing with            |
   |                                  |                        | receiving messages             |
   |                                  |                        | from the DHCP servers          |
   |                                  |                        | and adding them to             |
   |                                  |                        | the queue for                  |
   |                                  |                        | processing.                    |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp-ddns.d2-to-dns``      | core                   | Used by the                    |
   |                                  |                        | kea-dhcp-ddns daemon           |
   |                                  |                        | for logging                    |
   |                                  |                        | information about              |
   |                                  |                        | events dealing with            |
   |                                  |                        | sending and receiving          |
   |                                  |                        | messages to and from           |
   |                                  |                        | the DNS servers.               |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-netconf``                  | core                   | The root logger for            |
   |                                  |                        | the NETCONF agent.             |
   |                                  |                        | All components used            |
   |                                  |                        | by NETCONF inherit             |
   |                                  |                        | the settings from              |
   |                                  |                        | this logger if there           |
   |                                  |                        | is no specialized              |
   |                                  |                        | logger provided.               |
   +----------------------------------+------------------------+--------------------------------+
   | ``kea-dhcp4.lease-query-hooks``, | libdhcp_lease_query    | This logger is used            |
   | ``kea-dhcp6.lease-query-hooks``  | hook library           | to log messages                |
   |                                  |                        | related to the                 |
   |                                  |                        | operation of the               |
   |                                  |                        | Leasequery hooks library       |
   +----------------------------------+------------------------+--------------------------------+

Note that user-defined hook libraries should not use any of the loggers
mentioned above, but should instead define new loggers with names that
correspond to the libraries using them. Suppose that a user created
a library called “libdhcp-packet-capture” to dump packets received and
transmitted by the server to a file. An appropriate name for the
logger could be ``kea-dhcp4.packet-capture-hooks``. (Note that the hook
library implementer only specifies the second part of this name, i.e.
“packet-capture”. The first part is a root-logger name and is prepended
by the Kea logging system.) It is also important to note that since this
new logger is a child of a root logger, it inherits the configuration
from the root logger, something that can be overridden by an entry in
the configuration file.

The easiest way to find a logger name is to configure all logging to go
to a single destination and look there for specific logger names. See
:ref:`logging-message-format` for details.

The severity (string) Logger
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This specifies the category of messages logged. Each message is logged
with an associated severity, which may be one of the following (in
descending order of severity):

-  FATAL - associated with messages generated by a condition that is so
   serious that the server cannot continue executing.

-  ERROR - associated with messages generated by an error condition. The
   server will continue executing, but the results may not be as
   expected.

-  WARN - indicates an out-of-the-ordinary condition. However, the
   server will continue executing normally.

-  INFO - an informational message marking some event.

-  DEBUG - messages produced for debugging purposes.

When the severity of a logger is set to one of these values, it will
only log messages of that severity and above (e.g. setting the logging
severity to INFO will log INFO, WARN, ERROR, and FATAL messages). The
severity may also be set to NONE, in which case all messages from that
logger are inhibited.

.. note::

   The ``keactrl`` tool, described in :ref:`keactrl`, can be configured
   to start the servers in verbose mode. If this is the case, the
   settings of the logging severity in the configuration file will have
   no effect; the servers will use a logging severity of DEBUG
   regardless of the logging settings specified in the configuration
   file. To control severity via the configuration file,
   please make sure that the ``kea_verbose`` value is set to "no" within
   the ``keactrl`` configuration.

The debuglevel (integer) Logger
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When a logger's severity is set to DEBUG, this value specifies what
level of debug messages should be printed. It ranges from 0 (least
verbose) to 99 (most verbose). If severity for the logger is not DEBUG,
this value is ignored.

The output_options (list) Logger
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each logger can have zero or more ``output_options``. These specify
where log messages are sent and are explained in detail below.

The output (string) Option
^^^^^^^^^^^^^^^^^^^^^^^^^^

This value determines the type of output. There are several special
values allowed here: ``stdout`` (messages are printed on standard
output), ``stderr`` (messages are printed on stderr), ``syslog``
(messages are logged to syslog using the default name), ``syslog:name``
(messages are logged to syslog using a specified name). Any other value is
interpreted as a filename to which messages should be written.

The flush (true of false) Option
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Flush buffers after each log message. Doing this will reduce performance
but will ensure that if the program terminates abnormally, all messages
up to the point of termination are output. The default is "true".

The maxsize (integer) Option
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This option is only relevant when the destination is a file; this is the maximum size
in bytes that a log file may reach. When the maximum size is reached,
the file is renamed and a new file opened. For example, a ".1" is
appended to the name; if a ".1" file exists, it is renamed ".2", etc.
This is referred to as rotation.

The default value is 10240000 (10MB). The smallest value that can be
specified without disabling rotation is 204800. Any value less than
this, including 0, disables rotation.

.. note::

   Due to a limitation of the underlying logging library (log4cplus),
   rolling over the log files (from ".1" to ".2", etc) may show odd
   results; there can be multiple small files at the timing of rollover.
   This can happen when multiple processes try to roll over the
   files simultaneously. Version 1.1.0 of log4cplus solved this problem,
   so if this version or later of log4cplus is used to build Kea, the
   issue should not occur. Even for older versions, it is normally
   expected to happen rarely unless the log messages are produced very
   frequently by multiple different processes.

The maxver (integer) Option
^^^^^^^^^^^^^^^^^^^^^^^^^^^

This option is only relevant when the destination is a file and rotation is enabled
(i.e. maxsize is large enough). This is the maximum number of rotated
versions that will be kept. Once that number of files has been reached,
the oldest file, "log-name.maxver", will be discarded each time the log
rotates. In other words, at most there will be the active log file plus
maxver rotated files. The minimum and default value is 1.

The pattern (string) Option
^^^^^^^^^^^^^^^^^^^^^^^^^^^

This option can be used to specify the layout pattern of log messages for
a logger. Kea logging is implemented using the Log4Cplus library and whose
output formatting is based, conceptually, on the printf formatting from C
and is discussed in detail in the the next section
:ref:`logging-message-format`.

Each output type (stdout, file, or syslog) has a default ``pattern`` which
describes the content of its log messages. This parameter can be used to
specifiy your own pattern.  The pattern for each logger is governed
individually so each configured logger can have it's own pattern. Omitting
the ``pattern`` parameter or setting it to an empty string, "", will cause
Kea to use the default pattern for that logger's output type.

In addition to the log text itself, the default patterns used for ``stdout``
and files contain information such as date and time, logger level, and
process information.  The default pattern for ``syslog`` is limited primarily
to log level, source, and the log text.  This avoids duplicating information
which is usually supplied by syslog.

.. warning::
    You are strongly encouraged to test your pattern(s) on a local,
    non-production instance of Kea, running in the foreground and
    logging to ``stdout``.


.. _logging-message-format:

Logging Message Format
----------------------

As mentioned above, Kea log message content is controlled via a scheme similar
to the C language's printf formatting. The "pattern" used for each message is
described by a string containing one or more format components as part of a
text string.  In addition to the components the string may contain any other
arbitrary text you find useful.

The Log4Cplus documentation provides a concise discussion of the supported
components and formatting behavior and can be seen here:

    https://log4cplus.sourceforge.io/docs/html/classlog4cplus_1_1PatternLayout.html

It is probably easiest to understand this by examining the default pattern
for stdout and files (currently they are the same).  That pattern is shown
below:

::

    "%D{%Y-%m-%d %H:%M:%S.%q} %-5p [%c/%i.%t] %m\n";

and a typical log produced by this pattern would look somethng like this:

::

    2019-08-05 14:27:45.871 DEBUG [kea-dhcp4.dhcpsrv/8475.12345] DHCPSRV_TIMERMGR_START_TIMER starting timer: reclaim-expired-leases

That breaks down as like so:

  - ``%D{%Y-%m-%d %H:%M:%S.%q}``
    '%D' is the date and time in local time that the log message is generated,
    while everything between the curly braces, '{}' are date and time components.
    From the example log above this produces:
    ``2019-08-05 14:27:45.871``

  - ``%-5p``
    The severity of message, output as a minimum of five characters,
    using right-padding with spaces. In our example log: ``DEBUG``

  - ``%c``
    The log source. This includes two elements: the Kea process generating the
    message, in this case, ``kea-dhcp4``; and the component within the program
    from which the message originated, ``dhcpsrv`` (e.g.  the name of the
    library used by DHCP server implementations).

  - ``%i``
    The process ID. From the example log: ``8475``

  - ``%t``
    The thread ID. From the example log: ``12345``.
    Note the format of the thread ID is OS dependent: e.g. on some systems
    it is an address so is displayed in hexadecimal.

  - ``%m``
    The log message itself. Keg log messages all begin with a message
    identifier followed by arbitrary log text. Every message in Kea has
    a unique identifier, which can be used as an index to the
    `Kea Messages Manual <https://jenkins.isc.org/job/Kea_doc/messages/kea-messages.html>`__,
    where more information can be obtained.  In our example log above, the
    identifier is ``DHCPSRV_TIMERMGR_START_TIMER``.   The log text is typically
    a brief description detailing the condition that caused the message to be
    logged. In our example, the information logged,
    ``starting timer: reclaim-expired-leases``, explains that the timer for
    the expired lease reclamation cycle has been started.

.. Warning::

    Omitting ``%m`` will omit the log message text from your output making it
    rather useless. You should consider ``%m`` mandatory.

Finally, note that spacing between components, the square brackets around the
log source and PID, and the final carriage return '\n' are all literal text
specified as part of the pattern.

.. Warning::

    In order to ensure each log entry is a separate line, your patterns
    must end with an ``\n``.  There may be use cases where it is not desired
    so we do not enforce its inclusion.  Be aware that if you omit it from
    your pattern that to common text tools or displays, the log entries
    will run together in one long, endless "line".


The default for pattern for syslog output is as follows:

::

    "%-5p [%c.%t] %m\n";

You can see that it omits the date and time as well the process ID as this
information is typically output by syslog.  Note that Kea uses the pattern
to construct the text it sends to syslog (or any other destination). It has
no influence on the content syslog may add or formatting it may do.

Consult your OS documentation for "syslog" behavior as there are multiple
implementations.


Example Logger Configurations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this example we want to set the server logging to write to the
console using standard output.

::

   "Server": {
       "loggers": [
           {
               "name": "kea-dhcp4",
               "output_options": [
                   {
                       "output": "stdout"
                   }
               ],
               "severity": "WARN"
           }
       ]
   }

In this second example, we want to store debug log messages in a file
that is at most 2MB and keep up to eight copies of old logfiles. Once the
logfile grows to 2MB, it will be renamed and a new file will be created.

::

   "Server": {
       "loggers": [
           {
               "name": "kea-dhcp6",
               "output_options": [
                   {
                       "output": "/var/log/kea-debug.log",
                       "maxver": 8,
                       "maxsize": 204800,
                       "flush": true
                       "pattern": "%d{%j %H:%M:%S.%q} %c %m\n"
                   }
               ],
               "severity": "DEBUG",
               "debuglevel": 99
           }
      ]
   }

Notice that the above configuration uses a custom pattern which produces output like this:

::

    220 13:50:31.783 kea-dhcp4.dhcp4 DHCP4_STARTED Kea DHCPv4 server version 1.6.0-beta2-git started


.. _logging-during-startup:

Logging During Kea Startup
--------------------------

The logging configuration is specified in the configuration file.
However, when Kea starts, the configuration file is not read until partway into the
initialization process. Prior to that, the logging settings are set to
default values, although it is possible to modify some aspects of the
settings by means of environment variables. Note that in the absence of
any logging configuration in the configuration file, the settings of the
(possibly modified) default configuration will persist while the program
is running.

The following environment variables can be used to control the behavior
of logging during startup:

KEA_LOCKFILE_DIR

   Specifies a directory where the logging system should create its lock
   file. If not specified, it is prefix/var/run/kea, where "prefix"
   defaults to /usr/local. This variable must not end with a slash.
   There is one special value: "none", which instructs Kea not to create
   a lock file at all. This may cause issues if several processes log to
   the same file.

KEA_LOGGER_DESTINATION

   Specifies logging output. There are several special values:

   ``stdout``
   Log to standard output.

   ``stderr``
   Log to standard error.

   ``syslog[:fac]``
   Log via syslog. The optional fac (which is separated from the word
   "syslog" by a colon) specifies the facility to be used for the log
   messages. Unless specified, messages will be logged using the
   facility "local0".

   Any other value is treated as a name of the output file. If not
   specified otherwise, Kea will log to standard output.

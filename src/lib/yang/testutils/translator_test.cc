// Copyright (C) 2018-2019 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <boost/lexical_cast.hpp>
#include <sstream>
#include <yang/testutils/translator_test.h>

using namespace std;
using namespace isc::data;
using namespace sysrepo;

namespace isc {
namespace yang {
namespace test {

YangRepr::YangReprItem YangRepr::YangReprItem::get(const string& xpath, S_Session session) {
    string val_xpath = xpath;
    string value;
    sr_type_t type = SR_UNKNOWN_T;
    bool settable = true;
    try {
        S_Val s_val = session->get_item(xpath.c_str());
        if (!s_val) {
            isc_throw(BadValue, "YangReprItem failed at '" << xpath << "'");
        }
        val_xpath = string(s_val->xpath());
        type = s_val->type();
        ostringstream num_value;
        switch (type) {
        case SR_CONTAINER_T:
        case SR_CONTAINER_PRESENCE_T:
            settable = false;
            break;

        case SR_LIST_T:
            break;

        case SR_STRING_T:
            value = s_val->data()->get_string();
            break;

        case SR_BOOL_T:
            value = s_val->data()->get_bool() ? "true" : "false";
            break;

        case SR_UINT8_T:
            num_value << static_cast<unsigned>(s_val->data()->get_uint8());
            value = num_value.str();
            break;

        case SR_UINT16_T:
            num_value << s_val->data()->get_uint16();
            value = num_value.str();
            break;

        case SR_UINT32_T:
            num_value << s_val->data()->get_uint32();
            value = num_value.str();
            break;

        case SR_INT8_T:
            num_value << static_cast<unsigned>(s_val->data()->get_int8());
            value = num_value.str();
            break;

        case SR_INT16_T:
            num_value << s_val->data()->get_int16();
            value = num_value.str();
            break;

        case SR_INT32_T:
            num_value << s_val->data()->get_int32();
            value = num_value.str();
            break;

        case SR_DECIMAL64_T:
            num_value << s_val->data()->get_decimal64();
            value = num_value.str();
            break;

        case SR_IDENTITYREF_T:
            value = s_val->data()->get_identityref();
            break;

        case SR_ENUM_T:
            value = s_val->data()->get_enum();
            break;

        case SR_BINARY_T:
            value = s_val->data()->get_binary();
            break;

        default:
            isc_throw(NotImplemented, "YangReprItem called with unsupported type: " << type);
        }
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error in YangReprItem: " << ex.what());
    }
    return (YangReprItem(val_xpath, value, type, settable));
}

YangRepr::Tree YangRepr::get(S_Session session) const {
    Tree result;
    try {
        const string& xpath0 = "/" + model_ + ":*//.";
        TranslatorBasic tb(session, model_);
        /* TODO
        S_Iter_Value iter = tb.getIter(xpath0);
        for (;;) {
            const string& xpath = tb.getNext(iter);
            if (xpath.empty()) {
                break;
            }
            result.push_back(YangReprItem::get(xpath, session));
        }
        // TODO */
    } catch (const sysrepo_exception& ex) {
        isc_throw(SysrepoError, "sysrepo error in  YangRepr::getTree: " << ex.what());
    }
    return (result);
}

bool YangRepr::verify(const Tree& expected, S_Session session, ostream& errs) const {
    const Tree& got = get(session);
    for (size_t i = 0; (i < expected.size()) && (i < got.size()); ++i) {
        if (expected[i] == got[i]) {
            continue;
        }
        errs << "expected[" << i << "]: " << expected[i] << endl;
        errs << "got[" << i << "]: " << got[i] << endl;
        return (false);
    }
    if (expected.size() == got.size()) {
        return (true);
    }
    if (expected.size() > got.size()) {
        errs << "missings " << (expected.size() - got.size());
        errs << " beginning by:" << endl << expected[got.size()] << endl;
    } else {
        errs << "extras " << (got.size() - expected.size());
        errs << " beginning by:" << endl << got[expected.size()] << endl;
    }
    return (false);
}

void YangRepr::set(const Tree& tree, S_Session session) const {
    for (auto item : tree) {
        if (!item.settable_) {
            continue;
        }
        try {
            S_Val s_val;
            switch (item.type_) {
            case SR_CONTAINER_T:
            case SR_CONTAINER_PRESENCE_T:
                isc_throw(NotImplemented, "YangRepr::set called for a container");

            case SR_LIST_T:
                break;

            case SR_STRING_T:
            case SR_IDENTITYREF_T:
            case SR_ENUM_T:
            case SR_BINARY_T:
                s_val.reset(new Val(item.value_.c_str(), item.type_));
                break;

            case SR_BOOL_T:
                if (item.value_ == "true") {
                    s_val.reset(new Val(true, SR_BOOL_T));
                } else if (item.value_ == "false") {
                    s_val.reset(new Val(false, SR_BOOL_T));
                } else {
                    isc_throw(BadValue, "'" << item.value_ << "' not a bool");
                }
                break;

            case SR_UINT8_T:
                try {
                    uint8_t u8 = boost::lexical_cast<unsigned>(item.value_);
                    s_val.reset(new Val(u8));
                } catch (const boost::bad_lexical_cast&) {
                    isc_throw(BadValue, "'" << item.value_ << "' not an uint8");
                }
                break;

            case SR_UINT16_T:
                try {
                    uint16_t u16 = boost::lexical_cast<uint16_t>(item.value_);
                    s_val.reset(new Val(u16));
                } catch (const boost::bad_lexical_cast&) {
                    isc_throw(BadValue, "'" << item.value_ << "' not an uint16");
                }
                break;

            case SR_UINT32_T:
                try {
                    uint32_t u32 = boost::lexical_cast<uint32_t>(item.value_);
                    s_val.reset(new Val(u32));
                } catch (const boost::bad_lexical_cast&) {
                    isc_throw(BadValue, "'" << item.value_ << "' not an uint32");
                }
                break;

            case SR_INT8_T:
                try {
                    int8_t i8 = boost::lexical_cast<int>(item.value_);
                    s_val.reset(new Val(i8));
                } catch (const boost::bad_lexical_cast&) {
                    isc_throw(BadValue, "'" << item.value_ << "' not an int8");
                }
                break;

            case SR_INT16_T:
                try {
                    int16_t i16 = boost::lexical_cast<int16_t>(item.value_);
                    s_val.reset(new Val(i16));
                } catch (const boost::bad_lexical_cast&) {
                    isc_throw(BadValue, "'" << item.value_ << "' not an int16");
                }
                break;

            case SR_INT32_T:
                try {
                    int32_t i32 = boost::lexical_cast<int32_t>(item.value_);
                    s_val.reset(new Val(i32));
                } catch (const boost::bad_lexical_cast&) {
                    isc_throw(BadValue, "'" << item.value_ << "' not an int32");
                }
                break;

            case SR_DECIMAL64_T:
                try {
                    double d64 = boost::lexical_cast<double>(item.value_);
                    s_val.reset(new Val(d64));
                } catch (const boost::bad_lexical_cast&) {
                    isc_throw(BadValue, "'" << item.value_ << "' not a real");
                }
                break;

            default:
                isc_throw(NotImplemented,
                          "YangRepr::set called with unsupported type: " << item.type_);
            }
            session->set_item(item.xpath_.c_str(), s_val);
        } catch (const sysrepo_exception& ex) {
            isc_throw(SysrepoError,
                      "sysrepo error in YangRepr::set for " << item << ", error: " << ex.what());
        }
    }
}

bool YangRepr::validate(S_Session session, std::ostream& errs) const {
    try {
        // Try to validate. If it succeeds, then we're done here.
        session->validate();
        return (true);
    } catch (const std::exception& ex) {
        errs << "validate fails with " << ex.what() << endl;
    }
    try {
        /* TODO
        // If we get here, it means the validate() threw exceptions.
        S_Errors s_errors = session->get_last_errors();
        if (!s_errors) {

            // This is really weird. An exception was thrown, but
            // get_last_errors() didn't return anything. Maybe we're out of
            // memory or something?
            errs << "no errors" << endl;
            return (false);
        }
        size_t cnt = s_errors->error_cnt();
        errs << "got " << cnt << " errors" << endl;
        for (size_t i = 0; i < cnt; ++i) {
            S_Error s_error = s_errors->error(i);
            if (!s_error) {
                continue;
            }
            const char* xpath = s_error->xpath();
            const char* message = s_error->message();
            if (!xpath || !message) {
                continue;
            }
            // Bug in sysrepo returning message for xpath().
            if (xpath == message) {
                errs << message << endl;
            } else {
                errs << message << endl
                     << "At " << xpath << endl;
            }
        }
        // TODO */
    } catch (const std::exception& ex) {
        // Bug in sysrepo rethrowing the last error when trying to get it.
        errs << "double error " << ex.what();
    }
    return (false);
}

ostream& operator<<(ostream& os, sr_type_t type) {
    switch (type) {
    case SR_CONTAINER_T:
        os << "container";
        break;
    case SR_CONTAINER_PRESENCE_T:
        os << "container presence";
        break;
    case SR_LIST_T:
        os << "list";
        break;
    case SR_STRING_T:
        os << "string";
        break;
    case SR_BOOL_T:
        os << "bool";
        break;
    case SR_UINT8_T:
        os << "uint8";
        break;
    case SR_UINT16_T:
        os << "uint16";
        break;
    case SR_UINT32_T:
        os << "uint32";
        break;
    case SR_INT8_T:
        os << "int8";
        break;
    case SR_INT16_T:
        os << "int16";
        break;
    case SR_INT32_T:
        os << "int32";
        break;
    case SR_IDENTITYREF_T:
        os << "identity ref";
        break;
    case SR_ENUM_T:
        os << "enum";
        break;
    case SR_BINARY_T:
        os << "binary";
        break;
    case SR_LEAF_EMPTY_T:
        os << "leaf empty";
        break;
    case SR_BITS_T:
        os << "bits";
        break;
    case SR_DECIMAL64_T:
        os << "decimal64";
        break;
    case SR_INSTANCEID_T:
        os << "instance id";
        break;
    case SR_INT64_T:
        os << "int64";
        break;
    case SR_UINT64_T:
        os << "uint64";
        break;
    case SR_ANYXML_T:
        os << "any xml";
        break;
    case SR_ANYDATA_T:
        os << "any data";
        break;
#ifdef SR_UNION_T
    case SR_UNION_T:
        os << "union";
        break;
#endif
    default:
        os << type;
        break;
    }
    return (os);
}

ostream& operator<<(ostream& os, const YangRepr::YangReprItem& item) {
    os << item.xpath_ << " = (" << item.type_ << ") '" << item.value_ << "'";
    return (os);
}

ostream& operator<<(ostream& os, const YangRepr::Tree& tree) {
    for (auto item : tree) {
        os << item << endl;
    }
    return (os);
}

isc::data::ElementPtr YangMock::ietf_dhcpv6_server() {
    static isc::data::ElementPtr _(isc::data::Element::fromJSON(R"(
{
  "dhcpv6-server": {
    "network-ranges": {
      "network-range": [
        {
          "id": 0,
          "address-pools": {
            "address-pool": [
              {
                "pool-id": 0,
                "end-address": "2020::",
                "host-reservations": {
                  "host-reservation": [
                    {
                      "reserved-addr": "2020::",
                      "client-duid": "abcd",
                      "preferred-lifetime": 43200,
                      "prefix-reservation": [
                        {
                          "reserv-prefix": "2020::/64",
                          "reserv-prefix-id": 0,
                          "reserv-prefix-len": 0
                        }
                      ],
                      "rapid-commit": false,
                      "rebind-time": 32400,
                      "renew-time": 7200,
                      "valid-lifetime": 54000
                    }
                  ]
                },
                "max-address-count": 0,
                "pool-prefix": "2020::/64",
                "preferred-lifetime": 43200,
                "rapid-commit": false,
                "rebind-time": 32400,
                "renew-time": 7200,
                "start-address": "2020::",
                "valid-lifetime": 54000
              }
            ]
          },
          "description": "le network",
          "network-prefix": "2020::/64",
          "preferred-lifetime": 43200,
          "prefix-pools": {
            "prefix-pool": [
              {
                "host-reservations": {
                  "preferred-lifetime": 43200,
                  "prefix-reservation": [
                    {
                      "reserved-prefix": "2020::/64",
                      "client-duid": "abcd",
                      "reserv-prefix-len": 0
                    }
                  ],
                  "rapid-commit": false,
                  "rebind-time": 32400,
                  "renew-time": 7200,
                  "valid-lifetime": 54000
                },
                "max-pd-space-utilization": 0,
                "pool-id": 0,
                "preferred-lifetime": 43200,
                "prefix": "2020::/64",
                "prefix-length": 0,
                "rapid-commit": false,
                "rebind-time": 32400,
                "renew-time": 7200,
                "valid-lifetime": 54000
              }
            ]
          },
          "rapid-commit": false,
          "rebind-time": 32400,
          "release-count": 0,
          "renew-time": 7200,
          "valid-lifetime": 54000
        }
      ],
      "preferred-lifetime": 43200,
      "rapid-commit": false,
      "rebind-time": 32400,
      "renew-time": 7200,
      "valid-lifetime": 54000
    },
    "option-sets": {
      "option-set": [
        {
          "option-set-id": 0,
          "description": "le option set"
        }
      ]
    },
    "server-duid": {
      "data": "abcd",
      "type-code": 65535
    }
  }
}
)"));
    return copy(_);
}

isc::data::ElementPtr YangMock::kea_dhcp6_server() {
    static isc::data::ElementPtr _(isc::data::Element::fromJSON(R"(
{
  "config": {
    "client-classes": [
      {
        "name": "dt-device",
        "test": "vendor-class[2937].exists"
      }
    ],
    "decline-probation-period": 86400,
    "dhcp-queue-control": {
      "capacity": 500,
      "enable-queue": false,
      "queue-type": "kea-ring6"
    },
    "dhcp4o6-port": 6767,
    "expired-leases-processing": {
      "flush-reclaimed-timer-wait-time": 25,
      "hold-reclaimed-time": 3600,
      "max-reclaim-leases": 20,
      "max-reclaim-time": 50,
      "reclaim-timer-wait-time": 10,
      "unwarned-reclaim-cycles": 5
    },
    "hooks-libraries": [
      {
        "library": "/opt/dt-kea-libs/lib/libdt-kea-policy-engine-lib.so",
        "library-path": "/opt/dt-kea-libs/lib/libdt-kea-policy-engine-lib.so",
        "policy-engine-parameters": {
          "config": {
            "classification-group": {
              "encoded-options": [ 18, 37 ],
              "groups": [
                {
                  "class": "dt-...",
                  "tag": "DTAG...."
                },
                {
                  "class": "dt-p..",
                  "tag": "DTAGP..."
                },
                {
                  "class": "dt-.4.",
                  "tag": "DTAG.4.."
                },
                {
                  "class": "dt-p4.",
                  "tag": "DTAGP4.."
                },
                {
                  "class": "dt-..s",
                  "tag": "DTAG..S."
                },
                {
                  "class": "dt-p.s",
                  "tag": "DTAGP.S."
                },
                {
                  "class": "dt-.4s",
                  "tag": "DTAG.4S."
                },
                {
                  "class": "dt-p4s",
                  "tag": "DTAGP4S."
                }
              ],
              "tag": "DTAG"
            },
            "config-database": {
              "connect-timeout": 5000,
              "contact-points": "127.0.0.1",
              "keyspace": "kea_shard",
              "password": "keatest",
              "reconnect-wait-time": 5000,
              "request-timeout": 5000,
              "type": "cql",
              "user": "keatest"
            },
            "library-classes": [ "dt-device" ],
            "network-topology": {
              "zones": [
                {
                  "address-format-group": {
                    "groups": [
                      {
                        "mask": "ffff:f800:0000:0000:0000:0000:0000:0000",
                        "type": "zone"
                      },
                      {
                        "mask": "0000:0003:ffe0:0000:0000:0000:0000:0000",
                        "type": "router"
                      },
                      {
                        "mask": "0000:0000:001f:0000:0000:0000:0000:0000",
                        "type": "port"
                      },
                      {
                        "mask": "0000:0000:0000:ff00:0000:0000:0000:0000",
                        "type": "user-port"
                      }
                    ]
                  },
                  "allocation-group": {
                    "groups": [
                      {
                        "allocation": "default",
                        "tag": "_ANY",
                        "type": "any"
                      },
                      {
                        "allocation": "generated",
                        "bits": "0000:0020:0000:0000:0000:0000:0000:0000",
                        "signature": "any",
                        "tag": "IPTV",
                        "type": "iptv"
                      },
                      {
                        "allocation": "generated",
                        "bits": "0000:0030:0000:0000:0000:0000:0000:0000",
                        "signature": "any",
                        "tag": "VOIP",
                        "type": "voip"
                      }
                    ],
                    "mask": "0000:07fc:0000:0000:0000:0000:0000:0000"
                  },
                  "id": 262177
                },
                {
                  "address-format-group": {
                    "groups": [
                      {
                        "mask": "ffff:f800:0000:0000:0000:0000:0000:0000",
                        "type": "zone"
                      },
                      {
                        "mask": "0000:0003:ffe0:0000:0000:0000:0000:0000",
                        "type": "router"
                      },
                      {
                        "mask": "0000:0000:001f:0000:0000:0000:0000:0000",
                        "type": "port"
                      },
                      {
                        "mask": "0000:0000:0000:ff00:0000:0000:0000:0000",
                        "type": "user-port"
                      }
                    ]
                  },
                  "allocation-group": {
                    "groups": [
                      {
                        "allocation": "default",
                        "tag": "_ANY",
                        "type": "any"
                      },
                      {
                        "allocation": "generated",
                        "bits": "0000:0020:0000:0000:0000:0000:0000:0000",
                        "signature": "any",
                        "tag": "IPTV",
                        "type": "iptv"
                      },
                      {
                        "allocation": "generated",
                        "bits": "0000:0030:0000:0000:0000:0000:0000:0000",
                        "signature": "any",
                        "tag": "VOIP",
                        "type": "voip"
                      }
                    ],
                    "mask": "0000:07fc:0000:0000:0000:0000:0000:0000"
                  },
                  "id": 262243
                }
              ]
            }
          }
        }
      }
    ],
    "lease-database": {
      "connect-timeout": 5000,
      "contact-points": "127.0.0.1",
      "keyspace": "kea_shard",
      "password": "keatest",
      "reconnect-wait-time": 5000,
      "request-timeout": 5000,
      "type": "cql",
      "user": "keatest"
    },
    "option-data": [
      {
        "always-send": false,
        "code": 88,
        "csv-format": true,
        "data": "2003:1b0b:fffd:853::1155,2003:1b0b:fffd:853::1255",
        "name": "dhcp4o6-server-addr",
        "space": "dhcp6"
      },
      {
        "always-send": false,
        "code": 99,
        "csv-format": true,
        "data": "2003:1b0b:fffd:113::2056,2003:1b0b:fffe:113::2056",
        "name": "dhcpv4o6",
        "space": "dhcp6"
      },
      {
        "always-send": false,
        "code": 111,
        "csv-format": true,
        "data": "88, 96, 64",
        "name": "s46-priority",
        "space": "dhcp6"
      },
      {
        "always-send": false,
        "code": 64,
        "csv-format": true,
        "data": "aftr.lab.terastrm.net",
        "name": "aftr-name",
        "space": "dhcp6"
      },
      {
        "always-send": false,
        "code": 198,
        "csv-format": true,
        "data": "bn831x1-nccfcpe-1a-lab.lab.terastrm.net",
        "name": "netconf-fqdn",
        "space": "dhcp6"
      },
      {
        "always-send": false,
        "code": 31,
        "csv-format": true,
        "data": "2003:1b0b:fffa:ffff::fc21,2003:1b0b:fffa:ffff::fc22",
        "name": "sntp-servers",
        "space": "dhcp6"
      },
      {
        "always-send": false,
        "code": 21,
        "csv-format": true,
        "data": "sip1.lab.terastrm.net,sip2.lab.terastrm.net",
        "name": "sip-server-dns",
        "space": "dhcp6"
      },
      {
        "always-send": false,
        "code": 23,
        "csv-format": true,
        "data": "2003:1b0b:fffa:ffff::fc73,2003:1b0b:fffa:ffff::fc74",
        "name": "dns-servers",
        "space": "dhcp6"
      }
    ],
    "option-def": [
      {
        "array": true,
        "code": 99,
        "encapsulate": "",
        "name": "dhcpv4o6",
        "record-types": "",
        "space": "dhcp6",
        "type": "ipv6-address"
      },
      {
        "array": true,
        "code": 111,
        "encapsulate": "",
        "name": "s46-priority",
        "record-types": "",
        "space": "dhcp6",
        "type": "uint16"
      },
      {
        "array": true,
        "code": 198,
        "encapsulate": "",
        "name": "netconf-fqdn",
        "record-types": "",
        "space": "dhcp6",
        "type": "fqdn"
      }
    ],
    "preferred-lifetime": 120,
    "privacy-history-size": 5,
    "privacy-valid-lifetime": 10,
    "rebind-timer": 90,
    "renew-timer": 20,
    "reservation-mode": "all",
    "server-id": {
      "htype": 1,
      "identifier": "56847afe6699",
      "time": 512640186,
      "type": "LLT"
    },
    "server-tag": "",
    "valid-lifetime": 150
  }
}
)"));
    return copy(_);
}

isc::data::ElementPtr YangMock::keatest_module() {
    static isc::data::ElementPtr _(isc::data::Element::fromJSON(R"(
{
  "container": {
    "list": [
      {
        "key1": "lorem",
        "key2": "ipsum",
        "leaf": "dolor"
      }
    ]
  },
  "main": {
    "i8": -8,
    "i16": -16,
    "i32": -32,
    "i64": -64,
    "ui8": 8,
    "ui16": 16,
    "ui32": 32,
    "ui64": 64,
    "empty": "",
    "boolean": true,
    "string": "sit",
    "numbers": [128, 196]
  },
  "kernel-modules": {
    "kernel-module": [
      {
        "loaded": false,
        "location": "/lib/modules",
        "name": "consectetur"
      }
    ]
  },
  "presence-container": {
    "topleaf1": -1,
    "topleaf2": -2,
    "child1": {
      "child1-leaf": -3,
      "grandchild1": {
        "grandchild1-leaf": -4
      }
    },
    "child2": {
      "child2-leaf": -5,
      "grandchild2": {
        "grandchild2-leaf1": -6,
        "grandchild2-leaf2": -7,
        "grandchild2-leaf3": -8
      }
    }
  }
}
)"));
    return copy(_);
}

}  // namespace test
}  // namespace yang
}  // namespace isc

// (C) 2020 Deutsche Telekom AG.
//
// Deutsche Telekom AG and all other contributors /
// copyright owners license this file to you under the Apache
// License, Version 2.0 (the "License"); you may not use this
// file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the License for the
// specific language governing permissions and limitations
// under the License.

#include <config.h>

#include <admin/kea_admin.h>
#include <database/backend_selector.h>
#include <database/database_connection.h>
#include <database/testutils/schema.h>
#include <dhcpsrv/tests/test_utils.h>
#include <kea_config_tool/controller.h>
#include <util/command.h>
#include <util/dhcp.h>
#include <util/filesystem.h>

#include <gtest/gtest.h>

#include <unordered_map>

using namespace isc::db;
using namespace isc::db::test;
using namespace isc::dhcp;
using namespace isc::util;
using namespace isc::kea_config_tool;

using isc::InvalidParameter;
using isc::admin::KeaAdmin;
using std::array;
using std::cout;
using std::endl;
using std::get;
using std::map;
using std::runtime_error;
using std::shared_ptr;
using std::string;
using std::to_string;
using std::unordered_map;
using std::vector;

namespace {

static map<string, vector<vector<string>>> const ARGUMENTS_ = {
    {"--dbinit", {{}}},
    {"--shard",
     {{"--set-config", "--get-config", "--add-subnets", "--del-subnets", "--set-subnets",
       "--get-subnets"},
      {"-4", "-6"},
      {"_input_", "_output_", "[_list_]"},
      {"_unknown_"}}},
    {"--master",
     {{"--set-servers", "--get-servers"},
      {"-4", "-6"},
      {"_input_", "_output_", "[_list_]"},
      {"_unknown_"}}}};

static vector<string> const NONPOSITIONAL_ARGUMENTS_ = {
#ifdef TERASTREAM
    "-d",
#endif  // TERASTREAM
    "-h", "-v", "-V", "-t"};

template <DhcpSpaceType D>
string subnetValue(string const& prefix = "128", uint8_t const i = 1);

template <>
string subnetValue<DHCP_SPACE_V4>(string const& prefix, uint8_t const i) {
    return prefix + ".0.0." + to_string(i) + "/24";
}

template <>
string subnetValue<DHCP_SPACE_V6>(string const& prefix, uint8_t const i) {
    return "2" + prefix + ":db8:" + to_string(i) + "::/64";
}

template <DhcpSpaceType D>
inline string teeTimes();

template <>
inline string teeTimes<DHCP_SPACE_V4>() {
    return R"("t1-percent": 0.5,
        "t2-percent": 0.875,)";
}

template <>
string teeTimes<DHCP_SPACE_V6>() {
    return "";
}

template <DhcpSpaceType D>
string pool(string const& prefix = "128");

template <>
string pool<DHCP_SPACE_V4>(string const& prefix) {
    return prefix + ".0.0.16-" + prefix + ".0.0.128";
}

template <>
string pool<DHCP_SPACE_V6>(string const& prefix) {
    return "2" + prefix + ":db8:1::/80";
}

template <BackendSelector::Type B>
string databaseKey() {
    return "name";
}

template <>
string databaseKey<BackendSelector::Type::CQL>() {
    return "keyspace";
}

template <BackendSelector::Type B>
string databaseTypeValue() {
    return BackendSelector::TypeToString<B>();
}

template <>
string databaseTypeValue<BackendSelector::Type::PGSQL>() {
    return "postgresql";
}

template <BackendSelector::Type B>
inline string const hostKey() {
    return "host";
}

template <>
inline string const hostKey<BackendSelector::Type::CQL>() {
    return "contact-points";
}

template <BackendSelector::Type B, DhcpSpaceType D>
inline string const masterConfig() {
    return R"({
  "Dhcp)" + dhcpSpaceToString<D>() +
           R"(": {
    "configuration-type": "database",
    "instance-id": "kea-config-tool4",
    "master-database": {
      ")" + hostKey<B>() +
           R"(": "127.0.0.1",
      ")" + databaseKey<B>() +
           R"(": "master",
      "max-statement-tries": 1,
      "password": "keatest",
      "protocol": "tcp",
      "type": ")" +
           databaseTypeValue<B>() + R"(",
      "user": "keatest"
    }
  }
})";
}

template <DhcpSpaceType D>
inline string const emptyConfig() {
    return R"({
  "Dhcp)" + dhcpSpaceToString<D>() +
           R"(": {
  }
})";
}

template <DhcpSpaceType D>
inline string const preferredLifetime();

template <>
inline string const preferredLifetime<DHCP_SPACE_V4>() {
    return "";
}

template <>
inline string const preferredLifetime<DHCP_SPACE_V6>() {
    return R"("preferred-lifetime": 3600,)";
}

inline string const privacy() {
#ifdef TERASTREAM
    return R"("privacy-history-size": 0,
    "privacy-valid-lifetime": 0,)";
#else  // TERASTREAM
    return "";
#endif  // TERASTREAM
}

template <DhcpSpaceType D>
inline string const authoritative();

template <>
inline string const authoritative<DHCP_SPACE_V4>() {
    return R"("authoritative": false,)";
}

template <>
inline string const authoritative<DHCP_SPACE_V6>() {
    return "";
}

template <DhcpSpaceType D>
inline string const calculateTeeTimes();

template <>
inline string const calculateTeeTimes<DHCP_SPACE_V4>() {
    return R"("calculate-tee-times": false,)";
}

template <>
inline string const calculateTeeTimes<DHCP_SPACE_V6>() {
    return "";
}

template <DhcpSpaceType D>
inline string const bootFileName();

template <>
inline string const bootFileName<DHCP_SPACE_V4>() {
    return R"("boot-file-name": "",)";
}

template <>
inline string const bootFileName<DHCP_SPACE_V6>() {
    return "";
}

template <DhcpSpaceType D>
inline string const echoClientID();

template <>
inline string const echoClientID<DHCP_SPACE_V4>() {
    return R"("echo-client-id": true,)";
}

template <>
inline string const echoClientID<DHCP_SPACE_V6>() {
    return "";
}

template <DhcpSpaceType D>
inline string const matchClientID();

template <>
inline string const matchClientID<DHCP_SPACE_V4>() {
    return R"("match-client-id": true,)";
}

template <>
inline string const matchClientID<DHCP_SPACE_V6>() {
    return "";
}

template <DhcpSpaceType D>
inline string const nextServer();

template <>
inline string const nextServer<DHCP_SPACE_V4>() {
    return R"("next-server": "0.0.0.0",)";
}

template <>
inline string const nextServer<DHCP_SPACE_V6>() {
    return "";
}

template <DhcpSpaceType D>
inline string const serverHostname();

template <>
inline string const serverHostname<DHCP_SPACE_V4>() {
    return R"("server-hostname": "",)";
}

template <>
inline string const serverHostname<DHCP_SPACE_V6>() {
    return "";
}

template <DhcpSpaceType D>
inline string const pdPools();

template <>
inline string const pdPools<DHCP_SPACE_V4>() {
    return "";
}

template <>
inline string const pdPools<DHCP_SPACE_V6>() {
    return R"("pd-pools": [],)";
}

template <DhcpSpaceType D>
inline string const rapidCommit();

template <>
inline string const rapidCommit<DHCP_SPACE_V4>() {
    return "";
}

template <>
inline string const rapidCommit<DHCP_SPACE_V6>() {
    return R"("rapid-commit": false,)";
}

template <DhcpSpaceType D>
inline string const
subnet(string const& prefix = "128", uint8_t const count = 1, uint32_t const renew_timer = 900) {
    string result(R"(
    "subnet)" + dhcpSpaceToString<D>() +
                  R"(": [)");
    for (uint8_t i = count; i > 0; --i) {
        result += R"(
      {
        )" + matchClientID<D>() +
                  R"(
        )" + nextServer<D>() +
                  R"(
        "option-data": [],
        )" + pdPools<D>() +
                  R"(
        "pools": [
          {
            "option-data": [],
            "pool": ")" +
                  pool<D>(prefix) + R"("
          }
        ],
        )" + preferredLifetime<D>() +
                  R"(
        )" + rapidCommit<D>() +
                  R"(
        "rebind-timer": 1800,
        "relay": {
          "ip-addresses": []
        },
        "renew-timer": )" +
                  to_string(renew_timer) + R"(,
        "reservation-mode": "all",
        "subnet": ")" +
                  subnetValue<D>(prefix, i) + R"(",
        "valid-lifetime": 7200
      })";
        if (i != 1) {
            result += ",";
        }
    }
    result += R"(
    ],)";
    return result;
}

template <BackendSelector::Type B, DhcpSpaceType D>
inline string const config(string const& prefix = "128",
                           uint8_t const subnet_count = 1,
                           uint32_t const renew_timer = 900) {
    return R"({
  "Dhcp)" + dhcpSpaceToString<D>() +
           R"(": {
    )" + authoritative<D>() +
           R"(
    )" + calculateTeeTimes<D>() +
           R"(
    )" + bootFileName<D>() +
           R"(
    "decline-probation-period": 86400,
    "dhcp-queue-control": {
      "capacity": 500,
      "enable-queue": false,
      "queue-type": "kea-ring)" +
           dhcpSpaceToString<D>() + R"("
    },
    "dhcp4o6-port": 0,
    )" + echoClientID<D>() +
           R"(
    "expired-leases-processing": {
      "flush-reclaimed-timer-wait-time": 25,
      "hold-reclaimed-time": 3600,
      "max-reclaim-leases": 100,
      "max-reclaim-time": 250,
      "reclaim-timer-wait-time": 10,
      "unwarned-reclaim-cycles": 5
    },
    "lease-database": {
      ")" + hostKey<B>() +
           R"(": "127.0.0.1",
      ")" + databaseKey<B>() +
           R"(": "shard",
      "max-statement-tries": 1,
      "password": "keatest",
      "protocol": "tcp",
      "type": ")" +
           databaseTypeValue<B>() + R"(",
      "user": "keatest"
    },
    )" + matchClientID<D>() +
           R"(
    )" + nextServer<D>() +
           R"(
    )" + preferredLifetime<D>() +
           R"(
    )" + privacy() +
           R"(
    "rebind-timer": 1800,
    "renew-timer": 900,
    "reservation-mode": "all",
    )" + serverHostname<D>() +
           R"(
    "server-tag": "",
    )" + subnet<D>(prefix, subnet_count, renew_timer) +
           teeTimes<D>() +
           R"(
    "valid-lifetime" : 7200
  }
}
)";
}

template <BackendSelector::Type B>
inline string const config_database() {
    return R"( "config-database": {
    "connect-timeout": 5000,
      ")" + hostKey<B>() +
           R"(": "127.0.0.1",
      ")" + databaseKey<B>() +
           R"(": "shard",
    "max-statement-tries": 1,
    "password": "keatest",
    "port": )" +
           to_string(BackendSelector::port<B>()) + R"(,
    "protocol": "tcp",
    "reconnect-wait-time": 2000,
    "request-timeout": 12000,
    "tcp-keepalive": 1200,
    "tcp-nodelay": true,
    "type": ")" +
           databaseTypeValue<B>() + R"(",
    "user": "keatest"
  })";
}

template <BackendSelector::Type B, DhcpSpaceType D>
inline string const serversConfig() {
    return R"({
)" + config_database<B>() +
           R"(,
  "master-config": [
    {
      "instance-id": "kea-dhcp6-server",
      "server-config": {
        "Dhcp)" +
           dhcpSpaceToString<D>() +
           R"(": {
          "interfaces-config": {
            "interfaces": [
              "ens4/2001:db8:1::1"
            ]
          }
        },
        "Logging": {
          "loggers": [
            {
              "debuglevel": 99,
              "name": "kea-dhcp6",
              "output_options": [
                {
                  "output": "/var/log/kea-dhcp6.log"
                }
              ],
              "severity": "DEBUG"
            }
          ]
        }
      }
    }
  ]
})";
}

template <BackendSelector::Type B>
inline string const credentials() {
    return R"({
)" + config_database<B>() +
           R"(
})";
}

void doesContain(string outer, string inner) {
    size_t const position = outer.find(inner);
    if (position == string::npos) {
        cout << "Expected " << inner << ", found:" << endl << outer << endl;
    }
    EXPECT_NE(position, string::npos);
}

void doesntContain(string outer, string inner) {
    size_t const position = outer.find(inner);
    if (position != string::npos) {
        cout << "Expected no " << inner << ", found:" << endl << outer << endl;
    }
    EXPECT_EQ(position, string::npos);
}

string toString(vector<string> const& stack) {
    string arguments;
    bool first = true;
    for (string argument : stack) {
        if (first) {
            first = false;
        } else {
            arguments += " ";
        }
        arguments += argument;
    }
    return arguments;
}

vector<string> generateArguments() {
    vector<string> arguments;
    arguments.push_back(std::string());
    vector<string> stack;
    for (auto const& [first_argument, following_arguments] : ARGUMENTS_) {
        stack.push_back(first_argument);
        arguments.push_back(toString(stack));
        size_t i = 0;
        vector<size_t> j;

        // Make sure j has enough elements.
        j.resize(following_arguments.size(), 0);

        while (following_arguments.size() > 0 && following_arguments.at(0).size() > 0) {
            // Push the entire i column on the stack.
            while (i < following_arguments.size() && j.at(i) < following_arguments.at(i).size()) {
                if (i < following_arguments.size() - 1) {
                    if (j.at(i) < following_arguments.at(i).size()) {
                        // This is a generation.
                        stack.push_back(following_arguments.at(i).at(j.at(i)));
                        arguments.push_back(toString(stack));
                    }
                    ++i;
                } else {
                    break;
                }
            }

            // Push the entire j row on argument list.
            while (j.at(i) < following_arguments.at(i).size()) {
                // This is a generation.
                stack.push_back(following_arguments.at(i).at(j.at(i)));
                arguments.push_back(toString(stack));
                stack.pop_back();
                ++j.at(i);
            }

            // Backtrack to the next non-visited position.
            while (i != 0 && j.at(i) > following_arguments.at(i).size() - 1) {
                j.at(i) = 0;
                --i;
                stack.pop_back();
                ++j.at(i);
            }

            // If we have travelled the entire i row, finish with this row.
            if (i == 0 && j.at(0) >= following_arguments.at(0).size()) {
                break;
            }
        }
        stack.pop_back();
    }
    return arguments;
}

void checkForErrors(std::tuple<int, std::string> const& result) {
    string output = get<Command::OUTPUT>(result);
    int exit_code = get<Command::EXIT_CODE>(result);

    doesntContain(output, "ERROR");
    doesntContain(output, "Error");
    doesntContain(output, "error");

    EXPECT_EQ(exit_code, 0);
}

/// @brief Test fixture for testing Cassandra Subnet Manager
///
/// Opens the database prior to each test and closes it afterwards.
/// All pending transactions are deleted prior to closure.
template <BackendSelector::Type B, DhcpSpaceType D>
struct KeaConfigToolTest : ::testing::Test {
    KeaConfigToolTest()
        : master_config_file_(
              "/tmp/master_config_file_XXXXXX.ini", masterConfig<B, D>(), true, ".ini", false),
          initial_config_("/tmp/initial_config_XXXXXX", true, false),
          shard_directory_(initial_config_() + "/shard", false, false),
          config_json_(shard_directory_() + "/config.json", emptyConfig<D>(), false, "", false),
          servers_json_(
              shard_directory_() + "/servers.json", serversConfig<B, D>(), false, "", false),
          credentials_json_(
              shard_directory_() + "/credentials.json", credentials<B>(), false, "", false),
          kea_data_dir_("/tmp/kea-data-dir-XXXXXX", true, false),
          kea_lockfile_dir_("/tmp/kea-lockfile-dir-XXXXXX", true, false),
          kea_pidfile_dir_("/tmp/kea-pidfile-dir-XXXXXX", true, false),
          kea_config_tool_("../../kea-config-tool",  //
                           {{"KEA_DATA_DIR", kea_data_dir_()},  //
                            {"KEA_LOCKFILE_DIR", kea_lockfile_dir_()},  //
                            {"KEA_PIDFILE_DIR", kea_pidfile_dir_()}}) {
    }

    void createMasterAndShardSchemas() {
        SCOPED_TRACE("createMasterAndShardSchemas");
        EXPECT_EQ(chmod((Filesystem::getDirname() + "/../../../admin/kea-admin").c_str(),
                        S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH),
                  0);

        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--dbinit db-create " + BackendSelector::TypeToString<B>() + " -h 127.0.0.1 -sp " +
            to_string(BackendSelector::port<B>()) + " -d " + Filesystem::getDirname() +
            "/../../../../share/database/scripts "
            "-n master"));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--dbinit master-init " + BackendSelector::TypeToString<B>() + " -h 127.0.0.1 -sp " +
            to_string(BackendSelector::port<B>()) + " -d " + Filesystem::getDirname() +
            "/../../../../share/database/scripts "
            "-n master"));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--dbinit db-create " + BackendSelector::TypeToString<B>() + " -h 127.0.0.1 -sp " +
            to_string(BackendSelector::port<B>()) + " -d " + Filesystem::getDirname() +
            "/../../../../share/database/scripts "
            "-n shard"));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--dbinit db-init " + BackendSelector::TypeToString<B>() + " -h 127.0.0.1 -sp " +
            to_string(BackendSelector::port<B>()) + " -d " + Filesystem::getDirname() +
            "/../../../../share/database/scripts "
            "-n shard"));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--dbinit config-init " + BackendSelector::TypeToString<B>() + " -h 127.0.0.1 -sp " +
            to_string(BackendSelector::port<B>()) + " -d " + Filesystem::getDirname() +
            "/../../../../share/database/scripts "
            "-n shard"));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--master --set-servers -" + dhcpSpaceToString<D>() + " " + master_config_file_() +
            " " + initial_config_()));
    }

    void destroyMasterAndShardSchemas() {
        kea_config_tool_.runExceptionSafe(
            "--dbinit db-remove " + BackendSelector::TypeToString<B>() + " -h 127.0.0.1 -sp " +
            to_string(BackendSelector::port<B>()) + " -d " + Filesystem::getDirname() +
            "/../../../share/database/scripts "
            "-n master");
        kea_config_tool_.runExceptionSafe(
            "--dbinit db-remove " + BackendSelector::TypeToString<B>() + " -h 127.0.0.1 -sp " +
            to_string(BackendSelector::port<B>()) + " -d " + Filesystem::getDirname() +
            "/../../../share/database/scripts "
            "-n shard");
    }

    string md5() {
        return get<Command::OUTPUT>(
            Command::exec("find " + master_config_file_() + " " + initial_config_() +
                              " -type f | grep -v config.timestamp | sort -V | "
                              "uniq | xargs -I{} bash -c \"jq -S '. | del(.Dhcp4.subnet4[]?.id) | "
                              "del(.Dhcp6.subnet6[]?.id)' < {}\" | md5sum | cut -d ' ' -f 1",
                          false));
    }

    string cat(string const& filename) {
        return get<Command::OUTPUT>(
            Command::exec("jq -S . < " + initial_config_() + "/shard/" + filename, false));
    }

    string diff(string s1, string s2) {
        File f1("/tmp/kea-config-tool-test-f1", s1);
        File f2("/tmp/kea-config-tool-test-f2", s2);
        return get<Command::OUTPUT>(Command::exec("diff " + f1() + " " + f2(), false));
    }

    /// @brief Clears the database and opens connection to it.
    void SetUp() override final {
        // Ensure schema is the correct one.
        destroyMasterAndShardSchemas();
        createMasterAndShardSchemas();
    }

    /// @brief Cleans up the schema.
    void TearDown() override final {
        destroyMasterAndShardSchemas();
    }

    void shardSetConfigTest() {
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --set-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
    }

    void shardGetConfigTest() {
        auto const& [exit_code, output](kea_config_tool_.runExceptionSafe(
            "--shard --get-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard"));

        doesContain(output, "no entry found");
        EXPECT_EQ(exit_code, 4);
    }

    void shardAddSubnetTest() {
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --add-subnets -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
    }

    void shardDelSubnetTest() {
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --del-subnets -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
    }

    void shardSetSubnetTest() {
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --set-subnets -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
    }

    void shardGetSubnetTest() {
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --get-subnets -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard"));
    }

    void masterSetServersTest() {
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--master --set-servers -" + dhcpSpaceToString<D>() + " " + master_config_file_() +
            " " + initial_config_()));
    }

    void masterGetServersTest() {
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--master --get-servers -" + dhcpSpaceToString<D>() + " " + master_config_file_() +
            " " + initial_config_()));
    }

    void masterSetServersRoundtripTest() {
        servers_json_ << emptyConfig<D>();
        string hash = md5();
        servers_json_ << serversConfig<B, D>();
        string servers_json = cat("servers.json");
        string newHash = md5();
        EXPECT_NE(hash, newHash);
        hash = newHash;
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--master --set-servers -" + dhcpSpaceToString<D>() + " " + master_config_file_() +
            " " + initial_config_()));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--master --get-servers -" + dhcpSpaceToString<D>() + " " + master_config_file_() +
            " " + initial_config_()));
        EXPECT_EQ(hash, md5()) << diff(servers_json, cat("servers.json"));
    }

    void shardSetConfigRoundtripTest() {
        config_json_ << emptyConfig<D>();
        string hash = md5();
        config_json_ << config<B, D>();
        string config_json = cat("config.json");
        string newHash = md5();
        EXPECT_NE(hash, newHash);
        hash = newHash;
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --set-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --get-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard"));
        EXPECT_EQ(hash, md5()) << diff(config_json, cat("config.json"));
    }

    void shardAddSubnetRoundtripTest() {
        config_json_ << config<B, D>("128", 1, 900);
        string config_json = cat("config.json");
        string hash = md5();
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --set-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --get-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard"));
        EXPECT_EQ(hash, md5()) << diff(config_json, cat("config.json"));
        config_json_ << config<B, D>("129", 1, 901);
        EXPECT_NE(hash, md5());
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --add-subnets -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --get-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard"));
        EXPECT_NE(hash, md5());
    }

    void shardDelSubnetRoundtripTest() {
        config_json_ << config<B, D>();
        string config_json = cat("config.json");
        string hash = md5();
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --set-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --get-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard"));
        string newHash = md5();
        EXPECT_EQ(hash, newHash) << diff(config_json, cat("config.json"));
        hash = newHash;
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --del-subnets -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --get-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard"));
        EXPECT_NE(hash, md5());
    }

    void shardGetSubnetRoundtripTest() {
        config_json_ << config<B, D>();
        string config_json = cat("config.json");
        string hash = md5();
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --set-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --get-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard"));
        string newHash = md5();
        EXPECT_EQ(hash, newHash) << diff(config_json, cat("config.json"));
        hash = newHash;
        config_json_ << config<B, D>("128", 1, 901);
        config_json = cat("config.json");
        newHash = md5();
        EXPECT_NE(hash, newHash);
        hash = newHash;
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --set-subnets -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --get-subnets -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard " + subnetValue<D>()));
        EXPECT_EQ(hash, md5()) << diff(config_json, cat("config.json"));
    }

    void shardSetSubnetRoundtripTest() {
        config_json_ << config<B, D>();
        string config_json = cat("config.json");
        string hash = md5();
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --set-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --get-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard"));
        string newHash = md5();
        EXPECT_EQ(hash, newHash) << diff(config_json, cat("config.json"));
        hash = newHash;
        config_json_ << config<B, D>("128", 1, 901);
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --set-subnets -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --get-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard"));
        EXPECT_NE(hash, md5());
    }

    void shardSetConfigRepeatedTest() {
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --set-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --get-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard"));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --set-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_()));
        checkForErrors(kea_config_tool_.runExceptionSafe(
            "--shard --get-config -" + dhcpSpaceToString<D>() + " " + master_config_file_() + " " +
            initial_config_() + " shard"));
    }

    File master_config_file_;
    Dir initial_config_;
    Dir shard_directory_;
    File config_json_;
    File servers_json_;
    File credentials_json_;
    Dir kea_data_dir_;
    Dir kea_lockfile_dir_;
    Dir kea_pidfile_dir_;

    Controller kea_config_tool_;
};

struct KeaConfigToolNoBackendTest : ::testing::Test {
    KeaConfigToolNoBackendTest()
        : kea_data_dir_("/tmp/kea-data-dir-XXXXXX", true, false),
          kea_lockfile_dir_("/tmp/kea-lockfile-dir-XXXXXX", true, false),
          kea_pidfile_dir_("/tmp/kea-pidfile-dir-XXXXXX", true, false),
          kea_config_tool_("../../kea-config-tool",  //
                           {{"KEA_DATA_DIR", kea_data_dir_()},  //
                            {"KEA_LOCKFILE_DIR", kea_lockfile_dir_()},  //
                            {"KEA_PIDFILE_DIR", kea_pidfile_dir_()}}) {
    }

    Dir kea_data_dir_;
    Dir kea_lockfile_dir_;
    Dir kea_pidfile_dir_;

    Controller kea_config_tool_;
};

TEST_F(KeaConfigToolNoBackendTest, DISABLED_nonpositionalArguments) {
    vector<string> arguments_list = generateArguments();
    for (string const& np_argument : NONPOSITIONAL_ARGUMENTS_) {
        for (string const& arguments : arguments_list) {
            auto const& [exit_code, output](kea_config_tool_.runExceptionSafe(arguments + " " + np_argument));
            checkForErrors(std::make_tuple(exit_code, output));
            if (np_argument == "-h") {
                doesContain(output, "PARAMETERS");
            } else if (np_argument == "-v") {
                doesContain(output, "isc");
            } else if (np_argument == "-V") {
                doesContain(output, "git ");
            }
        }
    }
}

TEST_F(KeaConfigToolNoBackendTest, dbinit) {
    // TODO: Grep the 'Usage:' because it doesContain binary name which is different.
    auto const& [kea_config_tool_exit_code,
                 kea_config_tool_output](kea_config_tool_.runExceptionSafe("--dbinit -h"));
    KeaAdmin kea_admin;
    auto const& [kea_admin_exit_code, kea_admin_output](kea_admin.run("-h"));

    EXPECT_EQ(kea_config_tool_exit_code, kea_admin_exit_code);
    doesContain(kea_config_tool_output, kea_admin_output);
}

#ifdef HAVE_CQL

using KeaConfigToolTest_CQL_v4 = KeaConfigToolTest<BackendSelector::Type::CQL, DHCP_SPACE_V4>;
using KeaConfigToolTest_CQL_v6 = KeaConfigToolTest<BackendSelector::Type::CQL, DHCP_SPACE_V6>;

#define KEA_CONFIG_TOOL_CQL_TEST(test)                                                             \
    TEST_F(KeaConfigToolTest_CQL_v4, test) {                                                       \
        test();                                                                                    \
    }                                                                                              \
    TEST_F(KeaConfigToolTest_CQL_v6, test) {                                                       \
        test();                                                                                    \
    }

KEA_CONFIG_TOOL_CQL_TEST(masterGetServersTest)
KEA_CONFIG_TOOL_CQL_TEST(masterSetServersTest)
KEA_CONFIG_TOOL_CQL_TEST(shardAddSubnetTest)
KEA_CONFIG_TOOL_CQL_TEST(shardGetConfigTest)
KEA_CONFIG_TOOL_CQL_TEST(shardDelSubnetTest)
KEA_CONFIG_TOOL_CQL_TEST(shardSetConfigTest)
KEA_CONFIG_TOOL_CQL_TEST(masterSetServersRoundtripTest)
KEA_CONFIG_TOOL_CQL_TEST(shardAddSubnetRoundtripTest)
KEA_CONFIG_TOOL_CQL_TEST(shardDelSubnetRoundtripTest)
KEA_CONFIG_TOOL_CQL_TEST(shardGetSubnetRoundtripTest)
KEA_CONFIG_TOOL_CQL_TEST(shardSetConfigRoundtripTest)
KEA_CONFIG_TOOL_CQL_TEST(shardSetSubnetRoundtripTest)
KEA_CONFIG_TOOL_CQL_TEST(shardSetConfigRepeatedTest)

#undef KEA_CONFIG_TOOL_CQL_TEST

#endif  // HAVE_CQL

#ifdef HAVE_MYSQL

using KeaConfigToolTest_MySQL_v4 = KeaConfigToolTest<BackendSelector::Type::MYSQL, DHCP_SPACE_V4>;
using KeaConfigToolTest_MySQL_v6 = KeaConfigToolTest<BackendSelector::Type::MYSQL, DHCP_SPACE_V6>;

#define KEA_CONFIG_TOOL_MYSQL_TEST(test)                                                           \
    TEST_F(KeaConfigToolTest_MySQL_v4, test) {                                                     \
        test();                                                                                    \
    }                                                                                              \
    TEST_F(KeaConfigToolTest_MySQL_v6, test) {                                                     \
        test();                                                                                    \
    }

KEA_CONFIG_TOOL_MYSQL_TEST(masterGetServersTest)
KEA_CONFIG_TOOL_MYSQL_TEST(masterSetServersTest)
KEA_CONFIG_TOOL_MYSQL_TEST(shardAddSubnetTest)
KEA_CONFIG_TOOL_MYSQL_TEST(shardGetConfigTest)
KEA_CONFIG_TOOL_MYSQL_TEST(shardDelSubnetTest)
KEA_CONFIG_TOOL_MYSQL_TEST(shardSetConfigTest)
KEA_CONFIG_TOOL_MYSQL_TEST(masterSetServersRoundtripTest)
KEA_CONFIG_TOOL_MYSQL_TEST(shardAddSubnetRoundtripTest)
KEA_CONFIG_TOOL_MYSQL_TEST(shardDelSubnetRoundtripTest)
KEA_CONFIG_TOOL_MYSQL_TEST(shardGetSubnetRoundtripTest)
KEA_CONFIG_TOOL_MYSQL_TEST(shardSetConfigRoundtripTest)
KEA_CONFIG_TOOL_MYSQL_TEST(shardSetSubnetRoundtripTest)
KEA_CONFIG_TOOL_MYSQL_TEST(shardSetConfigRepeatedTest)

#undef KEA_CONFIG_TOOL_MYSQL_TEST

#endif  // HAVE_MYSQL

#ifdef HAVE_PGSQL

using KeaConfigToolTest_PgSQL_v4 = KeaConfigToolTest<BackendSelector::Type::PGSQL, DHCP_SPACE_V4>;
using KeaConfigToolTest_PgSQL_v6 = KeaConfigToolTest<BackendSelector::Type::PGSQL, DHCP_SPACE_V6>;

#define KEA_CONFIG_TOOL_PGSQL_TEST(test)                                                           \
    TEST_F(KeaConfigToolTest_PgSQL_v4, test) {                                                     \
        test();                                                                                    \
    }                                                                                              \
    TEST_F(KeaConfigToolTest_PgSQL_v6, test) {                                                     \
        test();                                                                                    \
    }

KEA_CONFIG_TOOL_PGSQL_TEST(masterGetServersTest)
KEA_CONFIG_TOOL_PGSQL_TEST(masterSetServersTest)
KEA_CONFIG_TOOL_PGSQL_TEST(shardAddSubnetTest)
KEA_CONFIG_TOOL_PGSQL_TEST(shardGetConfigTest)
KEA_CONFIG_TOOL_PGSQL_TEST(shardDelSubnetTest)
KEA_CONFIG_TOOL_PGSQL_TEST(shardSetConfigTest)
KEA_CONFIG_TOOL_PGSQL_TEST(masterSetServersRoundtripTest)
KEA_CONFIG_TOOL_PGSQL_TEST(shardAddSubnetRoundtripTest)
KEA_CONFIG_TOOL_PGSQL_TEST(shardDelSubnetRoundtripTest)
KEA_CONFIG_TOOL_PGSQL_TEST(shardGetSubnetRoundtripTest)
KEA_CONFIG_TOOL_PGSQL_TEST(shardSetConfigRoundtripTest)
KEA_CONFIG_TOOL_PGSQL_TEST(shardSetSubnetRoundtripTest)
KEA_CONFIG_TOOL_PGSQL_TEST(shardSetConfigRepeatedTest)

#undef KEA_CONFIG_TOOL_PGSQL_TEST

#endif  // HAVE_PGSQL

}  // namespace

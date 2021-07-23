// Copyright (C) 2018-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <netconf/netconf.h>
#include <netconf/netconf_process.h>
#include <netconf/parser_context.h>
#include <netconf/simple_parser.h>
#include <netconf/unix_control_socket.h>
#include <asiolink/asio_wrapper.h>
#include <asiolink/interval_timer.h>
#include <asiolink/io_service.h>
#include <cc/command_interpreter.h>
#include <yang/yang_models.h>
#include <yang/yang_revisions.h>
#include <yang/translator_config.h>
#include <yang/testutils/translator_test.h>
#include <testutils/log_utils.h>
#include <testutils/threaded_test.h>
#include <testutils/sandbox.h>
#include <gtest/gtest.h>
#include <sstream>
#include <thread>
#include <atomic>

#ifdef TODO
static_assert(false, "Unexpected. TODO is defined?");
#endif  // TODO

using namespace isc;
using namespace isc::netconf;
using namespace isc::asiolink;
using namespace isc::config;
using namespace isc::data;
using namespace isc::http;
using namespace isc::test;
using namespace isc::yang;
using namespace isc::yang::test;
using namespace std;
using namespace sysrepo;

namespace {

/// @brief Test unix socket file name.
const string TEST_SOCKET = "test-socket";

/// @brief Type definition for the pointer to Thread objects.
using ThreadPtr = std::shared_ptr<thread>;

/// @brief Test version of the NetconfAgent class.
class NakedNetconfAgent : public NetconfAgent {
public:
    /// @brief Constructor.
    NakedNetconfAgent() = default;

    /// @brief Destructor.
    virtual ~NakedNetconfAgent() = default;

    /// @brief Export protected methods and fields.
    /// @{
    using NetconfAgent::onBootUpdateSocket;
    using NetconfAgent::onBootUpdateSysrepo;
    using NetconfAgent::subscribeConfig;
    using NetconfAgent::subscribeOperational;

    using NetconfAgent::connection_;
    using NetconfAgent::initSysrepo;
    using NetconfAgent::modules_;
    using NetconfAgent::running_session_;
    using NetconfAgent::startup_session_;
    using NetconfAgent::subscriptions_;
    /// @}
};

/// @brief Type definition for the pointer to NakedNetconfAgent objects.
using NakedNetconfAgentPtr = std::shared_ptr<NakedNetconfAgent>;

/// @brief Clear YANG configuration.
///
/// @param agent The naked netconf agent (fr its startup datastore session).
void clearYang(NakedNetconfAgentPtr agent) {
    if (agent && (agent->startup_session_)) {
        string xpath = "/kea-dhcp4-server:config";
        EXPECT_NO_THROW(agent->startup_session_->delete_item(xpath.c_str()));
        EXPECT_NO_THROW(agent->startup_session_->apply_changes());
    }
}

/// @brief Test fixture class for netconf agent.
class NetconfAgentTest : public ThreadedTest {
public:
    isc::test::Sandbox sandbox;

    /// @brief Constructor.
    NetconfAgentTest()
        : ThreadedTest(),
          io_service_(new IOService()),
          agent_(new NakedNetconfAgent),
          requests_(),
          responses_() {
        NetconfProcess::shut_down = false;
        removeUnixSocketFile();
    }

    /// @brief Destructor.
    virtual ~NetconfAgentTest() {
        NetconfProcess::shut_down = true;
        if (thread_) {
            thread_->join();
            thread_.reset();
        }
        // io_service must be stopped after the thread returns,
        // otherwise the thread may never return if it is
        // waiting for the completion of some asynchronous tasks.
        io_service_->stop();
        io_service_.reset();
        if (agent_) {
            clearYang(agent_);
            agent_->clear();
        }
        agent_.reset();
        requests_.clear();
        responses_.clear();
        removeUnixSocketFile();
    }

    /// @brief Returns socket file path.
    ///
    /// If the KEA_SOCKET_TEST_DIR environment variable is specified, the
    /// socket file is created in the location pointed to by this variable.
    /// Otherwise, it is created in the build directory.
    string unixSocketFilePath() {
        std::string socket_path;
        const char* env = getenv("KEA_SOCKET_TEST_DIR");
        if (env) {
            socket_path = std::string(env) + "/test-socket";
        } else {
            socket_path = sandbox.join("test-socket");
        }
        return socket_path;
    }

    /// @brief Removes unix socket descriptor.
    void removeUnixSocketFile() {
        static_cast<void>(remove(unixSocketFilePath().c_str()));
    }

    /// @brief Create configuration of the control socket.
    ///
    /// @return a pointer to a control socket configuration.
    CfgControlSocketPtr createCfgControlSocket() {
        CfgControlSocketPtr cfg;
        cfg.reset(new CfgControlSocket(CfgControlSocket::Type::UNIX,
                                       unixSocketFilePath(),
                                       Url("http://127.0.0.1:8000/")));
        return cfg;
    }

    /// @brief Fake server (returns OK answer).
    void fakeServer();

    /// @brief IOService object.
    IOServicePtr io_service_;

    /// @brief Test netconf agent.
    NakedNetconfAgentPtr agent_;

    /// @brief Request list.
    vector<string> requests_;

    /// @brief Response list.
    vector<string> responses_;
};

/// @brief Special test fixture for logging tests.
class NetconfAgentLogTest : public dhcp::test::LogContentTest {
public:
    /// @brief Constructor.
    NetconfAgentLogTest()
        : io_service_(new IOService()),
          thread_(),
          agent_(new NakedNetconfAgent) {
        NetconfProcess::shut_down = false;
    }

    /// @brief Destructor.
    virtual ~NetconfAgentLogTest() {
        NetconfProcess::shut_down = true;
        // io_service must be stopped to make the thread to return.
        io_service_->stop();
        io_service_.reset();
        if (thread_) {
            thread_->join();
            thread_.reset();
        }
        if (agent_) {
            clearYang(agent_);
            agent_->clear();
        }
        agent_.reset();
    }

    /// @brief IOService object.
    IOServicePtr io_service_;

    /// @brief Pointer to server thread.
    ThreadPtr thread_;

    /// @brief Test netconf agent.
    NakedNetconfAgentPtr agent_;
};

/// @brief Fake server (returns OK answer).
void
NetconfAgentTest::fakeServer() {
    // Acceptor.
    boost::asio::local::stream_protocol::acceptor
        acceptor(io_service_->get_io_service());
    EXPECT_NO_THROW(acceptor.open());
    boost::asio::local::stream_protocol::endpoint
        endpoint(unixSocketFilePath());
    boost::asio::socket_base::reuse_address option(true);
    acceptor.set_option(option);
    EXPECT_NO_THROW(acceptor.bind(endpoint));
    EXPECT_NO_THROW(acceptor.listen());
    boost::asio::local::stream_protocol::socket
        socket(io_service_->get_io_service());

    // Ready.
    signalReady();

    // Timeout.
    bool timeout = false;
    IntervalTimer timer(*io_service_);
    timer.setup([&timeout]() {
            timeout = true;
            FAIL() << "timeout";
        }, 1500, IntervalTimer::ONE_SHOT);

    // Accept.
    boost::system::error_code ec;
    bool accepted = false;
    acceptor.async_accept(socket,
                          [&ec, &accepted]
                          (const boost::system::error_code& error) {
                              ec = error;
                              accepted = true;
                          });
    while (!accepted && !timeout) {
        io_service_->run_one();
    }
    ASSERT_FALSE(ec);

    // Receive command.
    string rbuf(1024, ' ');
    size_t received = 0;
    socket.async_receive(boost::asio::buffer(&rbuf[0], rbuf.size()),
                         [&ec, &received]
                         (const boost::system::error_code& error, size_t cnt) {
                             ec = error;
                             received = cnt;
                         });
    while (!received && !timeout) {
        io_service_->run_one();
    }
    ASSERT_FALSE(ec);
    rbuf.resize(received);
    requests_.push_back(rbuf);
    ElementPtr json;
    EXPECT_NO_THROW(json = Element::fromJSON(rbuf));
    EXPECT_TRUE(json);
    string command;
    ElementPtr config;
    if (json) {
        ElementPtr arg;
        EXPECT_NO_THROW(command = parseCommand(arg, json));
        if (command == "config-get") {
            config = Element::fromJSON("{ \"comment\": \"empty\" }");
        }
    }

    // Send answer.
    string sbuf = createAnswer(CONTROL_RESULT_SUCCESS, config)->str();
    responses_.push_back(sbuf);
    size_t sent = 0;
    socket.async_send(boost::asio::buffer(&sbuf[0], sbuf.size()),
                      [&ec, &sent]
                      (const boost::system::error_code& error, size_t cnt) {
                          ec = error;
                          sent = cnt;
                      });
    while (!sent && !timeout) {
        io_service_->run_one();
    }
    ASSERT_FALSE(ec);

    // Stop timer.
    timer.cancel();

    // Close socket and acceptor.
    if (socket.is_open()) {
        EXPECT_NO_THROW(socket.close());
    }
    EXPECT_NO_THROW(acceptor.close());
    // Removed the socket file so it can be called again immediately.
    removeUnixSocketFile();

    /// Finished.
    EXPECT_FALSE(timeout);
    EXPECT_TRUE(accepted);
    EXPECT_TRUE(received);
    EXPECT_TRUE(sent);
    EXPECT_EQ(sent, sbuf.size());

    // signalStopped can't be called here because of the 2 runs for update.
}

/// Verifies the initSysrepo method opens sysrepo connection and sessions.
TEST_F(NetconfAgentTest, initSysrepo) {
    EXPECT_NO_THROW(agent_->initSysrepo());
    EXPECT_TRUE(agent_->connection_);
    EXPECT_TRUE(agent_->startup_session_);
    EXPECT_TRUE(agent_->running_session_);
}

/// @brief Default change callback (print changes and return OK).
struct TestCallback {
    static int module_change([[maybe_unused]] S_Session sess,
                              char const* /* module_name */,
                              [[maybe_unused]] char const* xpath,
                              [[maybe_unused]] sr_event_t event,
                              uint32_t /* request_id */) {
#ifdef TODO
        agent_.logChanges(sess, "/kea-dhcp4-server:config");
#endif  // TODO
        finished = true;
        return SR_ERR_OK;
    }

    // To know when the callback was called.
    static atomic<bool> finished;
    static NetconfAgent agent_;
};

NetconfAgent TestCallback::agent_;
atomic<bool> TestCallback::finished(false);

/// Verifies the logChanges method handles correctly changes.
TEST_F(NetconfAgentLogTest, logChanges) {
    // Initial YANG configuration.
    const YRTree tree0 = {
        { "/kea-dhcp4-server:config", "", SR_CONTAINER_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.0.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.0.0/24']/id",
          "1", SR_UINT32_T, true },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']/id",
          "2", SR_UINT32_T, true }
    };
    // Load initial YANG configuration.
    ASSERT_NO_THROW(agent_->initSysrepo());
    YangRepr repr(KEA_DHCP4_SERVER);
    ASSERT_NO_THROW(repr.set(tree0, agent_->startup_session_));
    EXPECT_NO_THROW(agent_->startup_session_->apply_changes());

    // Subscribe configuration changes.
    S_Subscribe subs(new Subscribe(agent_->running_session_));
    TestCallback::finished = false;
    EXPECT_NO_THROW(subs->module_change_subscribe(KEA_DHCP4_SERVER.c_str(),
                                                  TestCallback::module_change, 0, 0,
                                                  SR_SUBSCR_DONE_ONLY));
    thread_.reset(new thread([this]() { io_service_->run(); }));

    // Change configuration (subnet #1 moved from 10.0.0.0/24 to 10.0.1/0/24).
    const YRTree tree1 = {
        { "/kea-dhcp4-server:config", "", SR_CONTAINER_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.1.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.1.0/24']/id",
          "1", SR_UINT32_T, true },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']/id",
          "2", SR_UINT32_T, true }
    };
    EXPECT_NO_THROW(repr.set(tree1, agent_->running_session_));
    EXPECT_NO_THROW(agent_->running_session_->apply_changes());

    // Check that the debug output was correct.
    addString("NETCONF_CONFIG_CHANGED_DETAIL YANG configuration changed: "
              "modified: "
              "/kea-dhcp4-server:config/subnet4[id='1']/subnet = "
              "10.0.0.0/24 => "
              "/kea-dhcp4-server:config/subnet4[id='1']/subnet = "
              "10.0.1.0/24");

    // logChanges is called in another thread so we can have to wait for it.
    while (!TestCallback::finished) {
        usleep(1000);
    }
    // Enable this for debugging.
    // logCheckVerbose(true);
    EXPECT_TRUE(checkFile());
}

/// Verifies the logChanges method handles correctly changes.
/// Instead of the simple modified of the previous test, now there will
/// deleted, created and moved.
TEST_F(NetconfAgentLogTest, logChanges2) {
    // Initial YANG configuration.
    const YRTree tree0 = {
        { "/kea-dhcp4-server:config", "", SR_CONTAINER_T, false },
        { "/kea-dhcp4-server:config/subnet4[id='1']", "", SR_LIST_T, true },
        { "/kea-dhcp4-server:config/subnet4[id='1']/id",
          "1", SR_UINT32_T, false },
        { "/kea-dhcp4-server:config/subnet4[id='1']/subnet",
          "10.0.0.0/24", SR_STRING_T, true },
        { "/kea-dhcp4-server:config/subnet4[id='2']", "", SR_LIST_T, true },
        { "/kea-dhcp4-server:config/subnet4[id='2']/id",
          "2", SR_UINT32_T, false },
        { "/kea-dhcp4-server:config/subnet4[id='2']/subnet",
          "10.0.2.0/24", SR_STRING_T, true }
    };
    // Load initial YANG configuration.
    ASSERT_NO_THROW(agent_->initSysrepo());
    YangRepr repr(KEA_DHCP4_SERVER);
    ASSERT_NO_THROW(repr.set(tree0, agent_->startup_session_));
    EXPECT_NO_THROW(agent_->startup_session_->apply_changes());

    // Subscribe configuration changes.
    S_Subscribe subs(new Subscribe(agent_->running_session_));
    TestCallback::finished = false;
    EXPECT_NO_THROW(subs->module_change_subscribe(KEA_DHCP4_SERVER.c_str(),
                                                  TestCallback::module_change, 0, 0,
                                                  SR_SUBSCR_DONE_ONLY));
    thread_.reset(new thread([this]() { io_service_->run(); }));

    // Change configuration (subnet #1 moved to #10).
    string xpath = "/kea-dhcp4-server:config/subnet4[id='1']";
    EXPECT_NO_THROW(agent_->running_session_->delete_item(xpath.c_str()));
    const YRTree tree1 = {
        { "/kea-dhcp4-server:config", "", SR_CONTAINER_T, false },
        { "/kea-dhcp4-server:config/subnet4[id='10']", "", SR_LIST_T, true },
        { "/kea-dhcp4-server:config/subnet4[id='10']/id",
          "10", SR_UINT32_T, false },
        { "/kea-dhcp4-server:config/subnet4[id='10']/subnet",
          "10.0.0.0/24", SR_STRING_T, true }, // The change is here!
        { "/kea-dhcp4-server:config/subnet4[id='2']", "", SR_LIST_T, true },
        { "/kea-dhcp4-server:config/subnet4[id='2']/id",
          "2", SR_UINT32_T, false },
        { "/kea-dhcp4-server:config/subnet4[id='2']/subnet",
          "10.0.2.0/24", SR_STRING_T, true }
    };
    EXPECT_NO_THROW(repr.set(tree1, agent_->running_session_));
    EXPECT_NO_THROW(agent_->running_session_->apply_changes());

    // Check that the debug output was correct.
    addString("NETCONF_CONFIG_CHANGED_DETAIL YANG configuration changed: "
              "deleted: "
              "/kea-dhcp4-server:config/subnet4[id='1']/id = 1");
    addString("NETCONF_CONFIG_CHANGED_DETAIL YANG configuration changed: "
              "deleted: "
              "/kea-dhcp4-server:config/subnet4[id='1']/subnet = "
              "10.0.0.0/24");
    addString("NETCONF_CONFIG_CHANGED_DETAIL YANG configuration changed: "
              "deleted: "
              "/kea-dhcp4-server:config/subnet4[id='1'] "
              "(list instance)");
    addString("NETCONF_CONFIG_CHANGED_DETAIL YANG configuration changed: "
              "created: "
              "/kea-dhcp4-server:config/subnet4[id='10'] "
              "(list instance)");
    addString("NETCONF_CONFIG_CHANGED_DETAIL YANG configuration changed: "
              "created: "
              "/kea-dhcp4-server:config/subnet4[id='10']/id = 10");
    addString("NETCONF_CONFIG_CHANGED_DETAIL YANG configuration changed: "
              "created: "
              "/kea-dhcp4-server:config/subnet4[id='10']/subnet = "
              "10.0.0.0/24");
    addString("NETCONF_CONFIG_CHANGED_DETAIL YANG configuration changed: "
              "moved: "
              "/kea-dhcp4-server:config/subnet4[id='10'] "
              "after /kea-dhcp4-server:config/subnet4[id='2']");

    // logChanges is called in another thread so we can have to wait for it.
    while (!TestCallback::finished) {
        usleep(1000);
    }
    // Enable this for debugging.
    // logCheckVerbose(true);
    EXPECT_TRUE(checkFile());
}

/// Verifies the bootUpdate method works as expected.
TEST_F(NetconfAgentTest, bootUpdate) {
    // Netconf configuration.
    string config_prefix = "{\n"
        "  \"Netconf\": {\n"
        "    \"managed-servers\": {\n"
        "      \"dhcp4\": {\n"
        "        \"control-socket\": {\n"
        "          \"socket-type\": \"unix\",\n"
        "          \"socket-name\": \"";
    string config_trailer = "\"\n"
        "        }\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}";
    string config = config_prefix + unixSocketFilePath() + config_trailer;
    NetconfConfigPtr ctx(new NetconfConfig());
    ElementPtr json;
    ParserContext parser_context;
    EXPECT_NO_THROW(json =
        parser_context.parseString(config, ParserContext::PARSER_NETCONF));
    ASSERT_TRUE(json);
    ASSERT_EQ(Element::map, json->getType());
    ElementPtr netconf_json = json->get("Netconf");
    ASSERT_TRUE(netconf_json);
    json = netconf_json;
    ASSERT_TRUE(json);
    NetconfSimpleParser::setAllDefaults(json);
    NetconfSimpleParser::deriveParameters(json);
    NetconfSimpleParser parser;
    EXPECT_NO_THROW(parser.parse(ctx, json, false));

    // Get service pair.
    CfgModelsListPtr servers_map = ctx->getModelConfigs();
    ASSERT_TRUE(servers_map);
    ASSERT_EQ(1, servers_map->size());
    CfgModelPtr model_config = *servers_map->begin();

    // Launch server.
    thread_.reset(new thread([this]() { fakeServer(); signalStopped(); }));

    // Wait until the server is listening.
    waitReady();

#ifdef TODO
    // Try onBootUpdateSocket().
    EXPECT_NO_THROW(agent_->onBootUpdateSocket(model_config));
#endif  // TODO

    // Wait server to be stopped.
    waitStopped();

    // Check request.
    ASSERT_EQ(1, requests_.size());
    const string& request_str = requests_[0];
    ElementPtr request;
    ASSERT_NO_THROW(request = Element::fromJSON(request_str));
    string expected_str = "{\n"
        "\"command\": \"config-get\"\n"
        "}";
    ElementPtr expected;
    ASSERT_NO_THROW(expected = Element::fromJSON(expected_str));
    EXPECT_TRUE(expected->equals(*request));
    // Alternative showing more for debugging...
    // EXPECT_EQ(prettyPrint(expected), prettyPrint(request));

    // Check response.
    ASSERT_EQ(1, responses_.size());
    const string& response_str = responses_[0];
    ElementPtr response;
    ASSERT_NO_THROW(response = Element::fromJSON(response_str));
    expected_str = "{\n"
        "\"result\": 0,\n"
        "\"arguments\": {\n"
        "    \"comment\": \"empty\"\n"
        "    }\n"
        "}";
    ASSERT_NO_THROW(expected = Element::fromJSON(expected_str));
    EXPECT_TRUE(expected->equals(*response));
}

/// Verifies the yangConfig method works as expected: apply YANG config
/// to the server.
TEST_F(NetconfAgentTest, yangConfig) {
    // YANG configuration.
    const YRTree tree = {
        { "/kea-dhcp4-server:config", "", SR_CONTAINER_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.0.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.0.0/24']/id",
          "1", SR_UINT32_T, true },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']/id",
          "2", SR_UINT32_T, true }
    };
    // Load YANG configuration.
    ASSERT_NO_THROW(agent_->initSysrepo());
    YangRepr repr(KEA_DHCP4_SERVER);
    ASSERT_NO_THROW(repr.set(tree, agent_->startup_session_));
    EXPECT_NO_THROW(agent_->startup_session_->apply_changes());

    // Netconf configuration.
    string config_prefix = "{\n"
        "  \"Netconf\": {\n"
        "    \"managed-servers\": {\n"
        "      \"dhcp4\": {\n"
        "        \"control-socket\": {\n"
        "          \"socket-type\": \"unix\",\n"
        "          \"socket-name\": \"";
    string config_trailer = "\"\n"
        "        }\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}";
    string config = config_prefix + unixSocketFilePath() + config_trailer;
    NetconfConfigPtr ctx(new NetconfConfig());
    ElementPtr json;
    ParserContext parser_context;
    EXPECT_NO_THROW(json =
        parser_context.parseString(config, ParserContext::PARSER_NETCONF));
    ASSERT_TRUE(json);
    ASSERT_EQ(Element::map, json->getType());
    ElementPtr netconf_json = json->get("Netconf");
    ASSERT_TRUE(netconf_json);
    json = netconf_json;
    ASSERT_TRUE(json);
    NetconfSimpleParser::setAllDefaults(json);
    NetconfSimpleParser::deriveParameters(json);
    NetconfSimpleParser parser;
    EXPECT_NO_THROW(parser.parse(ctx, json, false));

    // Get service pair.
    CfgModelsListPtr servers_map = ctx->getModelConfigs();
    ASSERT_TRUE(servers_map);
    ASSERT_EQ(1, servers_map->size());
    CfgModelPtr model_config = *servers_map->begin();

    // Launch server.
    thread_.reset(new thread([this]() { fakeServer(); signalStopped();}));

    // Wait until the server is listening.
    waitReady();

#ifdef TODO
    // Try onBootUpdateSocket().
    EXPECT_NO_THROW(agent_->onBootUpdateSocket(model_config));
#endif  // TODO

    // Wait server to be stopped.
    waitStopped();

    // Check request.
    ASSERT_EQ(1, requests_.size());
    const string& request_str = requests_[0];
    ElementPtr request;
    ASSERT_NO_THROW(request = Element::fromJSON(request_str));
    string expected_str = "{\n"
        "\"command\": \"config-set\",\n"
        "\"arguments\": {\n"
        "    \"config\": {\n"
        "        \"subnet4\": [\n"
        "            {\n"
        "                \"id\": 1,\n"
        "                \"subnet\": \"10.0.0.0/24\"\n"
        "            },\n"
        "            {\n"
        "                \"id\": 2,\n"
        "                \"subnet\": \"10.0.2.0/24\"\n"
        "            }\n"
        "        ]\n"
        "    }\n"
        "  }\n"
        "}";
    ElementPtr expected;
    ASSERT_NO_THROW(expected = Element::fromJSON(expected_str));
    EXPECT_TRUE(expected->equals(*request));

    // Check response.
    ASSERT_EQ(1, responses_.size());
    const string& response_str = responses_[0];
    ElementPtr response;
    ASSERT_NO_THROW(response = Element::fromJSON(response_str));
    expected_str = "{\n"
        "\"result\": 0\n"
        "}";
    ASSERT_NO_THROW(expected = Element::fromJSON(expected_str));
    EXPECT_TRUE(expected->equals(*response));
}

/// Verifies the subscribeConfig method works as expected.
TEST_F(NetconfAgentTest, subscribeConfig) {
    // Netconf configuration.
    string config_prefix = "{\n"
        "  \"Netconf\": {\n"
        "    \"managed-servers\": {\n"
        "      \"dhcp4\": {\n"
        "        \"control-socket\": {\n"
        "          \"socket-type\": \"unix\",\n"
        "          \"socket-name\": \"";
    string config_trailer = "\"\n"
        "        }\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}";
    string config = config_prefix + unixSocketFilePath() + config_trailer;
    NetconfConfigPtr ctx(new NetconfConfig());
    ElementPtr json;
    ParserContext parser_context;
    EXPECT_NO_THROW(json =
        parser_context.parseString(config, ParserContext::PARSER_NETCONF));
    ASSERT_TRUE(json);
    ASSERT_EQ(Element::map, json->getType());
    ElementPtr netconf_json = json->get("Netconf");
    ASSERT_TRUE(netconf_json);
    json = netconf_json;
    ASSERT_TRUE(json);
    NetconfSimpleParser::setAllDefaults(json);
    NetconfSimpleParser::deriveParameters(json);
    NetconfSimpleParser parser;
    EXPECT_NO_THROW(parser.parse(ctx, json, false));

    // Get service pair.
    CfgModelsListPtr servers_map = ctx->getModelConfigs();
    ASSERT_TRUE(servers_map);
    ASSERT_EQ(1, servers_map->size());
    CfgModelPtr model_config = *servers_map->begin();

    // Try subscribeConfig.
    EXPECT_EQ(0, agent_->subscriptions_.size());
    ASSERT_NO_THROW(agent_->initSysrepo());
    EXPECT_EQ(0, agent_->subscriptions_.size());
    EXPECT_NO_THROW(agent_->subscribeConfig<isc::dhcp::DHCP_SPACE_V4>(model_config));
    EXPECT_EQ(1, agent_->subscriptions_.size());

    /// Unsubscribe.
    EXPECT_NO_THROW(agent_->subscriptions_.clear());
}

/// Verifies the subscribeOperational method works as expected.
TEST_F(NetconfAgentTest, subscribeOperational) {
    // Netconf configuration.
    string config_prefix = "{\n"
        "  \"Netconf\": {\n"
        "    \"managed-servers\": {\n"
        "      \"dhcp6\": {\n"
        "        \"control-socket\": {\n"
        "          \"socket-type\": \"unix\",\n"
        "          \"socket-name\": \"";
    string config_trailer = "\"\n"
        "        }\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}";
    string config = config_prefix + unixSocketFilePath() + config_trailer;
    NetconfConfigPtr ctx(new NetconfConfig());
    ElementPtr json;
    ParserContext parser_context;
    EXPECT_NO_THROW(json =
        parser_context.parseString(config, ParserContext::PARSER_NETCONF));
    ASSERT_TRUE(json);
    ASSERT_EQ(Element::map, json->getType());
    ElementPtr netconf_json = json->get("Netconf");
    ASSERT_TRUE(netconf_json);
    json = netconf_json;
    ASSERT_TRUE(json);
    NetconfSimpleParser::setAllDefaults(json);
    NetconfSimpleParser::deriveParameters(json);
    NetconfSimpleParser parser;
    EXPECT_NO_THROW(parser.parse(ctx, json, false));

    // Get service pair.
    CfgModelsListPtr servers_map = ctx->getModelConfigs();
    ASSERT_TRUE(servers_map);
    ASSERT_EQ(1, servers_map->size());
    CfgModelPtr model_config = *servers_map->begin();

    // Try subscribeOperational.
    EXPECT_EQ(0, agent_->subscriptions_.size());
    ASSERT_NO_THROW(agent_->initSysrepo());
    EXPECT_EQ(0, agent_->subscriptions_.size());
    EXPECT_NO_THROW(agent_->subscribeOperational<isc::dhcp::DHCP_SPACE_V6>(model_config));
    EXPECT_EQ(1, agent_->subscriptions_.size());

    /// Unsubscribe.
    EXPECT_NO_THROW(agent_->subscriptions_.clear());
}

/// Verifies the update method works as expected: apply new YANG configuration
/// to the server. Note it is called by the subscription callback.
// TODO: fix this after the KEA-IETF converter is done
TEST_F(NetconfAgentTest, update) {
    // Initial YANG configuration.
    const YRTree tree0 = {
        { "/kea-dhcp4-server:config", "", SR_CONTAINER_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.0.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.0.0/24']/id",
          "1", SR_UINT32_T, true },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']/id",
          "2", SR_UINT32_T, true }
    };

    // Load initial YANG configuration.
    ASSERT_NO_THROW(agent_->initSysrepo());
    YangRepr repr(KEA_DHCP4_SERVER);
    ASSERT_NO_THROW(repr.set(tree0, agent_->startup_session_));
    EXPECT_NO_THROW(agent_->startup_session_->apply_changes());

    // Netconf configuration.
    // Set validate-changes to false to avoid validate() to be called.
    string config_prefix = "{\n"
        "  \"Netconf\": {\n"
        "    \"validate-changes\": false,\n"
        "    \"managed-servers\": {\n"
        "      \"dhcp4\": {\n"
        "        \"control-socket\": {\n"
        "          \"socket-type\": \"unix\",\n"
        "          \"socket-name\": \"";
    string config_trailer = "\"\n"
        "        }\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}";
    string config = config_prefix + unixSocketFilePath() + config_trailer;
    NetconfConfigPtr ctx(new NetconfConfig());
    ElementPtr json;
    ParserContext parser_context;
    EXPECT_NO_THROW(json =
        parser_context.parseString(config, ParserContext::PARSER_NETCONF));
    ASSERT_TRUE(json);
    ASSERT_EQ(Element::map, json->getType());
    ElementPtr netconf_json = json->get("Netconf");
    ASSERT_TRUE(netconf_json);
    json = netconf_json;
    ASSERT_TRUE(json);
    NetconfSimpleParser::setAllDefaults(json);
    NetconfSimpleParser::deriveParameters(json);
    NetconfSimpleParser parser;
    EXPECT_NO_THROW(parser.parse(ctx, json, false));

    // Get service pair.
    CfgModelsListPtr servers_map = ctx->getModelConfigs();
    ASSERT_TRUE(servers_map);
    ASSERT_EQ(1, servers_map->size());
    CfgModelPtr model_config = *servers_map->begin();

    // Subscribe YANG changes.
    EXPECT_EQ(0, agent_->subscriptions_.size());
    EXPECT_NO_THROW(agent_->subscribeConfig<isc::dhcp::DHCP_SPACE_V4>(model_config));
    EXPECT_EQ(1, agent_->subscriptions_.size());

    // Launch server.
    thread_.reset(new thread([this]() { fakeServer(); signalStopped(); }));

    // Wait until the server is listening.
    waitReady();

    // Change configuration (subnet #1 moved from 10.0.0.0/24 to 10.0.1.0/24).
    const YRTree tree1 = {
        { "/kea-dhcp4-server:config", "", SR_CONTAINER_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.1.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.1.0/24']/id",
          "1", SR_UINT32_T, true },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']/id",
          "2", SR_UINT32_T, true }
    };
    // TODO: implementation got changed, config callback should now be involved
    EXPECT_NO_THROW(repr.set(tree1, agent_->running_session_));

    // Wait server to be stopped.
    waitStopped();

    // Check request.
    ASSERT_EQ(1, requests_.size());
    const string& request_str = requests_[0];
    ElementPtr request;
    ASSERT_NO_THROW(request = Element::fromJSON(request_str));
    string expected_str = "{\n"
        "\"command\": \"config-set\",\n"
        "\"arguments\": {\n"
        "    \"config\": {\n"
        "        \"subnet4\": [\n"
        "            {\n"
        "                \"id\": 1,\n"
        "                \"subnet\": \"10.0.1.0/24\"\n"
        "            },\n"
        "            {\n"
        "                \"id\": 2,\n"
        "                \"subnet\": \"10.0.2.0/24\"\n"
        "            }\n"
        "        ]\n"
        "    }\n"
        "  }\n"
        "}";
    ElementPtr expected;
    ASSERT_NO_THROW(expected = Element::fromJSON(expected_str));
    EXPECT_TRUE(expected->equals(*request));

    // Check response.
    ASSERT_EQ(1, responses_.size());
    const string& response_str = responses_[0];
    ElementPtr response;
    ASSERT_NO_THROW(response = Element::fromJSON(response_str));
    expected_str = "{\n"
        "\"result\": 0\n"
        "}";
    ASSERT_NO_THROW(expected = Element::fromJSON(expected_str));
    EXPECT_TRUE(expected->equals(*response));
}

/// Verifies the validate method works as expected: test new YANG configuration
/// with the server. Note it is called by the subscription callback and
/// update is called after.
// TODO: this should be fixed after the KEA-IETF converter is done
TEST_F(NetconfAgentTest, validate) {
    // Initial YANG configuration.
    const YRTree tree0 = {
        { "/kea-dhcp4-server:config", "", SR_CONTAINER_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.0.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.0.0/24']/id",
          "1", SR_UINT32_T, true },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']/id",
          "2", SR_UINT32_T, true }
    };
    // Load initial YANG configuration.
    ASSERT_NO_THROW(agent_->initSysrepo());
    YangRepr repr(KEA_DHCP4_SERVER);
    ASSERT_NO_THROW(repr.set(tree0, agent_->startup_session_));
    EXPECT_NO_THROW(agent_->startup_session_->apply_changes());

    // Netconf configuration.
    string config_prefix = "{\n"
        "  \"Netconf\": {\n"
        "    \"managed-servers\": {\n"
        "      \"dhcp4\": {\n"
        "        \"control-socket\": {\n"
        "          \"socket-type\": \"unix\",\n"
        "          \"socket-name\": \"";
    string config_trailer = "\"\n"
        "        }\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}";
    string config = config_prefix + unixSocketFilePath() + config_trailer;
    NetconfConfigPtr ctx(new NetconfConfig());
    ElementPtr json;
    ParserContext parser_context;
    EXPECT_NO_THROW(json =
        parser_context.parseString(config, ParserContext::PARSER_NETCONF));
    ASSERT_TRUE(json);
    ASSERT_EQ(Element::map, json->getType());
    ElementPtr netconf_json = json->get("Netconf");
    ASSERT_TRUE(netconf_json);
    json = netconf_json;
    ASSERT_TRUE(json);
    NetconfSimpleParser::setAllDefaults(json);
    NetconfSimpleParser::deriveParameters(json);
    NetconfSimpleParser parser;
    EXPECT_NO_THROW(parser.parse(ctx, json, false));

    // Get service pair.
    CfgModelsListPtr servers_map = ctx->getModelConfigs();
    ASSERT_TRUE(servers_map);
    ASSERT_EQ(1, servers_map->size());
    CfgModelPtr model_config = *servers_map->begin();

    // Subscribe YANG changes.
    EXPECT_EQ(0, agent_->subscriptions_.size());
    EXPECT_NO_THROW(agent_->subscribeConfig<isc::dhcp::DHCP_SPACE_V4>(model_config));
    EXPECT_EQ(1, agent_->subscriptions_.size());

    // Launch server twice.
    thread_.reset(new thread([this]()
                             {
                                 fakeServer();
                                 fakeServer();
                                 signalStopped();
                             }));

    // Wait until the server is listening.
    waitReady();

    // Change configuration (subnet #1 moved from 10.0.0.0/24 to 10.0.1/0/24).
    const YRTree tree1 = {
        { "/kea-dhcp4-server:config", "", SR_CONTAINER_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.1.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.1.0/24']/id",
          "1", SR_UINT32_T, true },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.2.0/24']/id",
          "2", SR_UINT32_T, true }
    };
    EXPECT_NO_THROW(repr.set(tree1, agent_->running_session_));
    EXPECT_NO_THROW(agent_->running_session_->apply_changes());

    // Wait servers to be stopped.
    waitStopped();

    // Check requests.
    ASSERT_EQ(2, requests_.size());
    string request_str = requests_[0];
    ElementPtr request;
    ASSERT_NO_THROW(request = Element::fromJSON(request_str));
    string expected_str = "{\n"
        "\"command\": \"config-test\",\n"
        "\"arguments\": {\n"
        "    \"Dhcp4\": {\n"
        "        \"subnet4\": [\n"
        "            {\n"
        "                \"id\": 1,\n"
        "                \"subnet\": \"10.0.1.0/24\"\n"
        "            },\n"
        "            {\n"
        "                \"id\": 2,\n"
        "                \"subnet\": \"10.0.2.0/24\"\n"
        "            }\n"
        "        ]\n"
        "    }\n"
        "  }\n"
        "}";
    ElementPtr expected;
    ASSERT_NO_THROW(expected = Element::fromJSON(expected_str));
    EXPECT_TRUE(expected->equals(*request));

    request_str = requests_[1];
    ASSERT_NO_THROW(request = Element::fromJSON(request_str));
    expected_str = "{\n"
        "\"command\": \"config-set\",\n"
        "\"arguments\": {\n"
        "    \"Dhcp4\": {\n"
        "        \"subnet4\": [\n"
        "            {\n"
        "                \"id\": 1,\n"
        "                \"subnet\": \"10.0.1.0/24\"\n"
        "            },\n"
        "            {\n"
        "                \"id\": 2,\n"
        "                \"subnet\": \"10.0.2.0/24\"\n"
        "            }\n"
        "        ]\n"
        "    }\n"
        "  }\n"
        "}";
    ASSERT_NO_THROW(expected = Element::fromJSON(expected_str));
    EXPECT_TRUE(expected->equals(*request));

    // Check responses.
    ASSERT_EQ(2, responses_.size());
    string response_str = responses_[0];
    ElementPtr response;
    ASSERT_NO_THROW(response = Element::fromJSON(response_str));
    expected_str = "{\n"
        "\"result\": 0\n"
        "}";
    ASSERT_NO_THROW(expected = Element::fromJSON(expected_str));
    EXPECT_TRUE(expected->equals(*response));

    response_str = responses_[1];
    ASSERT_NO_THROW(response = Element::fromJSON(response_str));
    expected_str = "{\n"
        "\"result\": 0\n"
        "}";
    ASSERT_NO_THROW(expected = Element::fromJSON(expected_str));
    EXPECT_TRUE(expected->equals(*response));
}

/// Verifies what happens when the validate method returns an error.
TEST_F(NetconfAgentTest, noValidate) {
    // Initial YANG configuration.
    const YRTree tree0 = {
        { "/kea-dhcp4-server:config", "", SR_CONTAINER_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.1.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.1.0/24']/id",
          "1", SR_UINT32_T, true }
    };
    // Load initial YANG configuration.
    ASSERT_NO_THROW(agent_->initSysrepo());
    YangRepr repr(KEA_DHCP4_SERVER);
    ASSERT_NO_THROW(repr.set(tree0, agent_->startup_session_));
    EXPECT_NO_THROW(agent_->startup_session_->apply_changes());

    // Netconf configuration.
    string config_prefix = "{\n"
        "  \"Netconf\": {\n"
        "    \"managed-servers\": {\n"
        "      \"dhcp4\": {\n"
        "        \"control-socket\": {\n"
        "          \"socket-type\": \"unix\",\n"
        "          \"socket-name\": \"";
    string config_trailer = "\"\n"
        "        }\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}";
    string config = config_prefix + unixSocketFilePath() + config_trailer;
    NetconfConfigPtr ctx(new NetconfConfig());
    ElementPtr json;
    ParserContext parser_context;
    EXPECT_NO_THROW(json =
        parser_context.parseString(config, ParserContext::PARSER_NETCONF));
    ASSERT_TRUE(json);
    ASSERT_EQ(Element::map, json->getType());
    ElementPtr netconf_json = json->get("Netconf");
    ASSERT_TRUE(netconf_json);
    json = netconf_json;
    ASSERT_TRUE(json);
    NetconfSimpleParser::setAllDefaults(json);
    NetconfSimpleParser::deriveParameters(json);
    NetconfSimpleParser parser;
    EXPECT_NO_THROW(parser.parse(ctx, json, false));

    // Get service pair.
    CfgModelsListPtr servers_map = ctx->getModelConfigs();
    ASSERT_TRUE(servers_map);
    ASSERT_EQ(1, servers_map->size());
    CfgModelPtr model_config = *servers_map->begin();

    // Subscribe YANG changes.
    EXPECT_EQ(0, agent_->subscriptions_.size());
    EXPECT_NO_THROW(agent_->subscribeConfig<isc::dhcp::DHCP_SPACE_V4>(model_config));
    EXPECT_EQ(1, agent_->subscriptions_.size());

    // Change configuration (add invalid user context).
    const YRTree tree1 = {
        { "/kea-dhcp4-server:config", "", SR_CONTAINER_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.1.0/24']", "", SR_LIST_T, false },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.1.0/24']/id",
          "1", SR_UINT32_T, true },
        { "/kea-dhcp4-server:config/subnet4[subnet='10.0.1.0/24']/user-context",
          "BOGUS", SR_STRING_T, true }
    };
    EXPECT_NO_THROW(repr.set(tree1, agent_->running_session_));
    try {
        agent_->running_session_->apply_changes();
    } catch (const sysrepo_exception& ex) {
        auto err = agent_->running_session_->get_error();
        EXPECT_EQ(1, err->error_cnt());
        EXPECT_EQ("Validation failed", string(err->message(0)));
    } catch (const std::exception& ex) {
        ADD_FAILURE() << "unexpected exception: " << ex.what();
    } catch (...) {
        ADD_FAILURE() << "unexpected exception";
    }
}

TEST_F(NetconfAgentTest, getOperationalData) {
    // Initial YANG configuration.
    const YRTree tree0 = {
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "admin-credentials/type",
         "cql", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "admin-credentials/user",
         "keatest", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "admin-credentials/password",
         "keatest", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "admin-credentials/connect-timeout",
         "10000", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "admin-credentials/contact-points",
         "127.0.0.1", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "admin-credentials/keyspace",
         "kea_shard", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "admin-credentials/reconnect-wait-time",
         "10000", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "admin-credentials/request-timeout",
         "10000", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "shard-config"
         "/dhcpv6-server/network-ranges/network-range[network-range-id='1']/network-prefix",
         "fe80::/10", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "shard-config"
         "/dhcpv6-server/network-ranges/network-range[network-range-id='1']/description",
         "BOGUS", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "shard-config"
         "/dhcpv6-server/network-ranges/network-range[network-range-id='1']/address-pools/"
         "address-pool[pool-id='0']/pool-prefix",
         "fe80::/10", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "shard-config"
         "/dhcpv6-server/network-ranges/network-range[network-range-id='1']/address-pools/"
         "address-pool[pool-id='0']/start-address",
         "2020::", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "shard-config"
         "/dhcpv6-server/network-ranges/network-range[network-range-id='1']/address-pools/"
         "address-pool[pool-id='0']/end-address",
         "2020::", SR_STRING_T, true},
        {"/ietf-dhcpv6-distributed:config/masters[name='kea_master']/shards[name='kea_shard']/"
         "shard-config"
         "/dhcpv6-server/network-ranges/network-range[network-range-id='1']/address-pools/"
         "address-pool[pool-id='0']/max-address-count",
         "0", SR_UINT32_T, true}};

    // Load initial YANG configuration.
    ASSERT_NO_THROW(agent_->initSysrepo());
    YangRepr repr(IETF_DHCPV6_SERVER);
    ASSERT_NO_THROW(repr.set(tree0, agent_->startup_session_));
    EXPECT_NO_THROW(agent_->startup_session_->apply_changes());

    // Netconf configuration.
    // Set validate-changes and subscribe-changes to avoid module_change to be called.
    string config_prefix = "{\n"
                           "  \"Netconf\": {\n"
                           "    \"validate-changes\": false,\n"
                           "    \"subscribe-changes\": false,\n"
                           "    \"subscribe-operational\": true,\n"
                           "    \"managed-servers\": {\n"
                           "      \"dhcp6\": {\n"
                           "        \"control-socket\": {\n"
                           "          \"socket-type\": \"unix\",\n"
                           "          \"socket-name\": \"";
    string config_trailer = "\"\n"
                            "        },\n"
                            "       \"model\": \"ietf-dhcpv6-distributed\""
                            "      }\n"
                            "    }\n"
                            "  }\n"
                            "}";
    string config = config_prefix + unixSocketFilePath() + config_trailer;
    NetconfConfigPtr ctx(new NetconfConfig());
    ElementPtr json;
    ParserContext parser_context;
    EXPECT_NO_THROW(json = parser_context.parseString(config, ParserContext::PARSER_NETCONF));
    ASSERT_TRUE(json);
    ASSERT_EQ(Element::map, json->getType());
    ElementPtr netconf_json = json->get("Netconf");
    ASSERT_TRUE(netconf_json);
    json = netconf_json;
    ASSERT_TRUE(json);
    NetconfSimpleParser::setAllDefaults(json);
    NetconfSimpleParser::deriveParameters(json);
    NetconfSimpleParser parser;
    EXPECT_NO_THROW(parser.parse(ctx, json, false));

    // Get service pair.
    CfgModelsListPtr servers_map = ctx->getModelConfigs();
    ASSERT_TRUE(servers_map);
    ASSERT_EQ(1, servers_map->size());
    CfgModelPtr model_config = *servers_map->begin();

    // Subscribe YANG changes.
    EXPECT_EQ(0, agent_->subscriptions_.size());
    EXPECT_NO_THROW(agent_->subscribeOperational<isc::dhcp::DHCP_SPACE_V6>(model_config));
    EXPECT_EQ(1, agent_->subscriptions_.size());

    // Change configuration (subnet #1 moved from 10.0.0.0/24 to 10.0.1.0/24).
    const string operXpath("/ietf-dhcpv6-distributed:config/masters[name='kea_master']/"
                           "shards[name='kea_shard']/shard-config"
                           "/dhcpv6-server/network-ranges/network-range[network-range-id='1']/"
                           "address-pools/address-pool[pool-id='0']");

    S_Session oper_session_ = std::make_shared<Session>(agent_->connection_, SR_DS_OPERATIONAL);
    EXPECT_TRUE(oper_session_);
    // A subnet should be present in cassandra for this to work
    EXPECT_NO_THROW(oper_session_->get_items(operXpath.c_str()));
}

}

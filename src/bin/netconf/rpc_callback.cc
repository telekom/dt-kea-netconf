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

#include <dhcpsrv/lease_mgr_factory.h>
#include <netconf/control_socket_factory.h>
#include <netconf/netconf_log.h>
#include <netconf/rpc_callback.h>

using namespace std;

using isc::config::answerToText;
using isc::config::CONTROL_RESULT_SUCCESS;
using isc::config::parseAnswer;
using isc::data::ElementPtr;
using isc::dhcp::DHCP_SPACE_V6;
using isc::dhcp::Lease;

namespace isc {
namespace netconf {

RpcCallback::RpcCallback(CfgModelPtr const& model_config) : model_config_(model_config) {
}

int RpcCallback::rpc(sysrepo::S_Session session,
                     const char* op_path,
                     const sysrepo::S_Vals input,
                     sr_event_t /*event*/,
                     uint32_t /*request_id*/,
                     sysrepo::S_Vals_Holder output) {
    string path(op_path);
    LOG_INFO(netconf_logger, NETCONF_GET_RPC_ITEM)
        .arg(string("RPC action requested at path: ") + path);

    Lease::Type lease_type = Lease::TYPE_NA;
    if (path.find("delete-prefix-lease") != string::npos) {
        lease_type = Lease::TYPE_PD;
    } else if (path.find("delete-address-lease") == string::npos) {
        logErrorAndSetOutput(path, "unsupported RPC action requested", output);
        return SR_ERR_OK;
    }

    shared_ptr<ControlSocketInterface> comm;
    string const& model(model_config_->getModel());
    try {
        comm = ControlSocketFactory<DHCP_SPACE_V6>::create(model_config_->getRpc(), model);
    } catch (exception const& exception) {
        logErrorAndSetOutput(
            path, string("control socket creation failed with ") + exception.what(), output);
        return SR_ERR_OK;
    }

    // Obtain the lease by calling leases-get through the unix socket
    ElementPtr answer, leases_elem;
    int rcode;
    try {
        answer = comm->deleteLease(lease_type, input->val(0)->val_to_string());
        leases_elem = parseAnswer(rcode, answer);
    } catch (exception const& exception) {
        logErrorAndSetOutput(path, string("delete-lease command failed with ") + exception.what(),
                             output);
        return SR_ERR_OK;
    }
    if (rcode != CONTROL_RESULT_SUCCESS) {
        logErrorAndSetOutput(path, answerToText(answer), output);
        return SR_ERR_OK;
    }
    auto out_vals = output->allocate(1);
    out_vals->val(0)->set((path + "/return-message").c_str(), "Lease deleted successfully.",
                          SR_STRING_T);

    LOG_INFO(netconf_logger, NETCONF_GET_RPC_ITEM_SUCCESS)
        .arg(string("Lease deleted successfully: ") + input->val(0)->val_to_string());
    return SR_ERR_OK;
}

void RpcCallback::logErrorAndSetOutput(std::string const& path,
                                       std::string const& err,
                                       sysrepo::S_Vals_Holder const& output) {
    LOG_ERROR(netconf_logger, NETCONF_GET_RPC_ITEM_FAILED).arg(err);
    auto out_vals = output->allocate(1);
    out_vals->val(0)->set((path + "/return-message").c_str(), err.c_str(), SR_STRING_T);
}

}  // namespace netconf
}  // namespace isc

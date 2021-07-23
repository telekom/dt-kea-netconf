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

#ifndef RPC_CALLBACK_H
#define RPC_CALLBACK_H

#include <netconf/netconf_config.h>

#include <sysrepo-cpp/Session.hpp>

namespace isc {
namespace netconf {

struct RpcCallback {

    RpcCallback(CfgModelPtr const& model_config);

    int rpc(sysrepo::S_Session session,
            const char* op_path,
            const sysrepo::S_Vals input,
            sr_event_t event,
            uint32_t request_id,
            sysrepo::S_Vals_Holder output);

private:
    void logErrorAndSetOutput(std::string const& path,
                              std::string const& err,
                              sysrepo::S_Vals_Holder const& output);

    CfgModelPtr model_config_;
};

}  // namespace netconf
}  // namespace isc

#endif  // RPC_CALLBACK_H

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

#ifndef OPERATIONAL_CALLBACK_H
#define OPERATIONAL_CALLBACK_H

#include <sysrepo-cpp/Session.hpp>

namespace isc {
namespace netconf {

struct OperationalCallback {
    using ElementPtr = isc::data::ElementPtr;
    using Element = isc::data::Element;

    /// @brief Constructor.
    ///
    /// @param sysrepo_get_sess The session used for getting the config from sysrepo
    OperationalCallback(sysrepo::S_Session sysrepo_get_sess, CfgModelPtr const& model_config);

    /// @brief Operational get items callback.
    ///
    /// This callback is called by sysrepo when the user requests
    /// for operational data.
    ///
    /// @param session The running datastore session.
    /// @param module_name The module name.
    /// @param path The path used for subscription call
    /// @param request_xpath The path requested by the user
    /// @param parent Pointer to an existing parent of the requested nodes
    /// @return the sysrepo return code.
    template <isc::dhcp::DhcpSpaceType D>
    int setOperationalData(sysrepo::S_Session session,
                           const char* module_name,
                           const char* /* path */,
                           const char* request_xpath,
                           uint32_t /* request_id */,
                           libyang::S_Data_Node& parent);

    static ElementPtr getLeasesBySubnet(std::shared_ptr<ControlSocketInterface> comm,
                                        uint32_t subnetId);

    static ElementPtr getKEAConfig(std::shared_ptr<ControlSocketInterface> comm);

    template <isc::dhcp::DhcpSpaceType D>
    static std::string getResourcePrefix();

    static bool setXpath(sysrepo::S_Session const& session,
                         libyang::S_Data_Node& parent,
                         std::string const& node_xpath,
                         std::string const& value);

    /// @brief Internally used map that associates between IETF operational node
    /// and statistic name type used through the unix socket request
    /// The boolean value is true if the mean value should be calculated
    template <isc::dhcp::DhcpSpaceType D>
    static std::unordered_map<std::string, std::tuple<std::string, bool>> const& getStatisticMap();

    /// @brief Retrieves the configuration from sysrepo
    ///
    /// @param module_name The name of the module to retrieve configuration from
    /// @return @ref isc::data::ElementPtr tree with the data from the running datastore
    ElementPtr getSysrepoConfig(const char* module_name);

protected:
    //    template <isc::dhcp::DhcpSpaceType D>
    //    void doActiveDUID(sysrepo::S_Session const& session,
    //                      std::string const& request_xpath,
    //                      libyang::S_Data_Node& parent) {
    //        std::shared_ptr<ControlSocketInterface> const& comm(ControlSocketFactory<D>::create(
    //            model_config_->getOperational(), model_config_->getModel()));
    //
    //        isc::data::ElementPtr const& answer(
    //            comm->sendCommand(isc::config::createCommand("active-duid")));
    //        auto const& [DUID, status_code](isc::config::parseAnswer(answer));
    //        if (status_code != isc::config::CONTROL_RESULT_SUCCESS) {
    //            isc_throw(Unexpected, PRETTY_METHOD_NAME()
    //                                      << ": " << magic_enum::enum_name(status_code));
    //        }
    //    }

    /// @brief Sets the resource-stats operational nodes from the IETF Model
    ///

    /// @param parent Inherited from the main callback and is set by this function
    /// @param session Current session
    /// @param request_xpath The path where to set the statistics
    template <isc::dhcp::DhcpSpaceType D>
    void setAllResources(libyang::S_Data_Node& parent,
                         sysrepo::S_Session& session,
                         std::string request_xpath);

    /// @brief Sets the message-stats operational nodes from the IETF Model
    ///
    /// @param parent Inherited from the main callback and is set by this function
    /// @param session Current session
    /// @param request_xpath The path where to set the statistics
    template <isc::dhcp::DhcpSpaceType D>
    void setAllStatistics(libyang::S_Data_Node& parent,
                          sysrepo::S_Session& session,
                          std::string const& request_xpath);

    void prepareAndSetActiveLeases(libyang::S_Data_Node& parent,
                                   sysrepo::S_Session session,
                                   const char* module_name,
                                   std::string const& setXpath);

    /// @brief Sets the active-leases data by populating the parent node with the
    /// information provided by the sel_subnet and leases parameters
    ///
    /// @param parent Inherited from the main callback and is set by this function
    /// @param sel_subnet Pointer to the requested subnet data
    /// @param leases Vector of the requested leases filtered by subnet-id
    /// @param session Current session
    /// @param main_xpath Main path of the active-leases sub-tree
    /// @param pool_elem Pointer to the requested pool data
    /// @return Boolean value indicating if parent node has been set with the
    /// active-leases structure
    bool setActiveLeasesData(libyang::S_Data_Node& parent,
                             isc::dhcp::SubnetInfoPtr<isc::dhcp::DHCP_SPACE_V6>& sel_subnet,
                             isc::dhcp::Lease6Collection& leases,
                             sysrepo::S_Session const& session,
                             std::string const& main_xpath,
                             bool is_addr_pools_request,
                             ElementPtr const& pool_elem);

    /// @brief the running session
    ///
    /// Pointer to the running datastore session
    sysrepo::S_Session sysrepo_get_session_;

    CfgModelPtr model_config_;
};

}  // namespace netconf
}  // namespace isc

#endif  // OPERATIONAL_CALLBACK_H

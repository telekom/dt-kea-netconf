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

#ifndef CONFIGURATION_CALLBACK_H
#define CONFIGURATION_CALLBACK_H

#include <database/db_exceptions.h>
#include <kea_config_tool/controller.h>
#include <netconf/changes.h>
#include <netconf/control_socket.h>
#include <netconf/control_socket_factory.h>
#include <netconf/http_control_socket.h>
#include <netconf/netconf_cfg_mgr.h>
#include <netconf/netconf_log.h>
#include <netconf/stdout_control_socket.h>
#include <netconf/unix_control_socket.h>
#include <util/dt_thread_pool.h>
#include <yang/converter.h>
#include <yang/translator_universal.h>

#include <sysrepo-cpp/Session.hpp>

#include <memory>
#include <sstream>
#include <utility>

namespace isc {
namespace netconf {

/// @brief Module change subscription callback
template <isc::dhcp::DhcpSpaceType D>
struct ConfigurationCallback {
    ConfigurationCallback(CfgModelPtr const& model_config,
                          sysrepo::S_Session const& startup_session = nullptr)
        : changes_(std::make_shared<Changes>()), model_config_(model_config),
          startup_session_(startup_session), thread_pool_(1) {
    }

    template <typename functor_t>
    void asyncCommit(functor_t f, sr_datastore_t datastore) {
        thread_pool_.add(exceptionLoggedBind(
            netconf_logger, NETCONF_ERROR, PRETTY_METHOD_NAME(), [=, this]() mutable {
                sysrepo::S_Connection connection(std::make_shared<sysrepo::Connection>());
                sysrepo::S_Session session(
                    std::make_shared<sysrepo::Session>(connection, datastore));
                std::forward<functor_t>(f)(session);
                session->apply_changes();
            }));
    }

    template <isc::yang::convertion_direction_t C>
    static void convert(isc::data::ElementPtr& config, std::string const& model) {
        config = encloseIntoRootNode(
            config,
            isc::yang::CounterpartModel<isc::yang::Converter<C>::sourceFlavor()>::get(model));
        isc::yang::Converter<C>::convertAll(config, model);
        config = encloseIntoRootNode(
            config,
            isc::yang::CounterpartModel<isc::yang::Converter<C>::targetFlavor()>::get(model));
    }

    isc::data::ElementPtr discloseFromRootNode(isc::data::ElementPtr const& configuration,
                                               std::string const& model) {
        /// @todo only one toplevel node is supported for now
        std::string const& key(isc::yang::TranslatorUniversal::rootNodes().at(model).at(0));

        isc::data::ElementPtr const& underlying(configuration->get(key));
        if (underlying) {
            configuration->remove(key);
        } else {
            return underlying;
        }
        return underlying;
    }

    /// @brief Done
    ///
    /// @param sess The sysrepo running datastore session.
    /// @param model_config The service name and configuration pair.
    /// @return return code for sysrepo.
    int done(sysrepo::S_Session const& session) {
        std::string const& model(model_config_->getModel());
        isc::dhcp::DhcpSpaceType const dhcp_space(model_config_->getDhcpSpace());

        // Print an info that we're about to update the configuration.
        LOG_INFO(netconf_logger, NETCONF_DONE_CONFIG_STARTED)
            .arg(magic_enum::enum_name(dhcp_space));

        changes_->clear();

        try {
            // Retrieve the Sysrepo configuration.
            isc::yang::TranslatorUniversal translator(session, model);
            isc::data::ElementPtr config(translator.get());
            if (!config) {
                LOG_ERROR(netconf_logger, NETCONF_DONE_CONFIG_FAILED)
                    .arg(magic_enum::enum_name(dhcp_space))
                    .arg("YANG configuration for " + model + " is empty");
                return SR_ERR_OPERATION_FAILED;
            }

            LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_DONE_CONFIG)
                .arg(magic_enum::enum_name(dhcp_space))
                .arg("\n" + prettyPrint(config));

            // Push to startup also.
            if (startup_session_) {
                thread_pool_.add(exceptionLoggedBind(
                    netconf_logger, NETCONF_ERROR, PRETTY_METHOD_NAME(),
                    [=, this]() { startup_session_->copy_config(SR_DS_RUNNING, model.c_str()); }));
            }
        } catch (std::exception const& exception) {
            LOG_ERROR(netconf_logger, NETCONF_DONE_CONFIG_FAILED)
                .arg(magic_enum::enum_name(dhcp_space))
                .arg(exception.what());
            return SR_ERR_OPERATION_FAILED;
        }

        LOG_INFO(netconf_logger, NETCONF_DONE_CONFIG_COMPLETED)
            .arg(magic_enum::enum_name(dhcp_space));
        return SR_ERR_OK;
    }

    static isc::data::ElementPtr encloseIntoRootNode(isc::data::ElementPtr const& configuration,
                                                     std::string const& model) {
        /// @todo only one toplevel node is supported for now
        std::string const& root_node(isc::yang::TranslatorUniversal::rootNodes().at(model).at(0));

        if (configuration->get(root_node)) {
            return configuration;
        }

        isc::data::ElementPtr const& config_wrapper(isc::data::Element::createMap());
        config_wrapper->set(root_node, configuration);
        return config_wrapper;
    }

    int failure(std::string const& model,
                ControlSocketBasePtr<D> const& comm,
                sysrepo::S_Session const& session,
                isc::data::ElementPtr const& config,
                std::string const& reason) {
        LOG_ERROR(netconf_logger, NETCONF_UPDATE_CONFIG_FAILED).arg(model).arg(reason);
        changes_->clear();
        if (config) {
            // Pull all, because all might have not been pulled in `config` because of granular
            // change control.
            auto const& [comm_rcode, comm_config](comm->configGetParsed({}, nullptr));
            if (comm_rcode == isc::config::CONTROL_RESULT_SUCCESS) {
                pushDirectlyToSysrepo(model, comm_config, session, FRESH_PUSH);
            } else {
                LOG_WARN(netconf_logger, NETCONF_UPDATE_INFO)
                    .arg(PRETTY_METHOD_NAME() +
                         ": configGetParsed(): return_code == " + std::to_string(comm_rcode));
            }
        }
        return SR_ERR_VALIDATION_FAILED;
    }

    bool isBeingPushedDirectlyToSysrepo(isc::data::ElementPtr const& config) {
        isc::data::ElementPtr const& skip(config->get("skip-validation"));
        bool result(skip ? skip->boolValue() : false);
        if (skip) {
            config->remove("skip-validation");
        }
        return result;
    }

    void logChange(Change const& change, std::ostringstream const& msg) {
        changes_->add(change);
        std::string report(msg.str());
        isc::util::str::erase_all(report, '\n');
        LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_CONFIG_CHANGED_DETAIL)
            .arg(report);
    }

    /// @brief Log changes.
    ///
    /// Iterate on changes logging them. Sysrepo changes are an operation
    /// (created, modified, deleted or moved) with old and new values
    /// (cf sr_change_oper_e sysrepo documentation).
    ///
    /// @param sess The sysrepo running datastore session.
    /// @param model The model name.
    void logChanges(sysrepo::S_Session session, std::string const& xpath) {
        if (!changes_->empty()) {
            LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_INFO)
                .arg(
                    "changes are still recorded from a previous edit-config, not recording changes "
                    "this time");
            return;
        }

        sysrepo::S_Iter_Change const& iter(session->get_changes_iter(xpath.c_str()));
        if (!iter) {
            LOG_WARN(netconf_logger, NETCONF_WARNING).arg("no iterator");
            return;
        }

        while (true) {
            Change change;
            std::ostringstream msg;
            try {
                change = Changes::next_change(session, iter);
            } catch (const sysrepo::sysrepo_exception& exception) {
                msg << "get change iterator next failed: " << exception.what();
                LOG_WARN(netconf_logger, NETCONF_WARNING).arg(msg.str());
                return;
            }
            if (!change) {
                // End of changes, not an error.
                LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_INFO)
                    .arg("end of changes");
                return;
            }
            S_ExtendedVal extended_new_value(Changes::newValue(change));
            S_ExtendedVal extended_old_value(Changes::oldValue(change));
            sysrepo::S_Val new_val(extended_new_value ? extended_new_value->value_ : nullptr);
            sysrepo::S_Val old_val(extended_old_value ? extended_old_value->value_ : nullptr);
            switch (change->oper()) {
            case SR_OP_CREATED:
                if (!new_val) {
                    LOG_WARN(netconf_logger, NETCONF_WARNING)
                        .arg("created but without a new value");
                    break;
                }
                msg << "created: " << new_val->to_string();
                logChange(change, msg);
                break;

            case SR_OP_MODIFIED:
                if (!old_val || !new_val) {
                    LOG_WARN(netconf_logger, NETCONF_WARNING)
                        .arg("modified but without an old or new value");
                    break;
                }
                msg << "modified: " << old_val->to_string() << " => " << new_val->to_string();
                logChange(change, msg);
                break;

            case SR_OP_DELETED:
                if (!old_val) {
                    LOG_WARN(netconf_logger, NETCONF_WARNING)
                        .arg("deleted but without an old value");
                    break;
                }
                msg << "deleted: " << old_val->to_string();
                logChange(change, msg);
                break;

            case SR_OP_MOVED:
                if (!new_val) {
                    LOG_WARN(netconf_logger, NETCONF_WARNING).arg("moved but without a new value");
                    break;
                }
                msg << "moved: " << new_val->xpath();
                if (!old_val) {
                    msg << " first";
                } else {
                    msg << " after " << old_val->xpath();
                }
                logChange(change, msg);
                break;

            default:
                msg << "unknown operation (" << change->oper() << ")";
                LOG_WARN(netconf_logger, NETCONF_WARNING).arg(msg.str());
                break;
            }
        }
    }

    /// @brief Module configuration change callback.
    ///
    /// This callback is called by sysrepo when there is a change to
    /// module configuration.
    ///
    /// @param session The running datastore session.
    /// @param module_name The module name.
    /// @param event The event.
    /// @param private_ctx The private context.
    /// @return the sysrepo return code.
    int module_change([[maybe_unused]] sysrepo::S_Session session,
                      char const* /* module_name */,
                      [[maybe_unused]] char const* xpath,
                      sr_event_t event,
                      uint32_t /* request_id */) {
        LOG_INFO(netconf_logger, NETCONF_CONFIG_CHANGE_EVENT).arg(magic_enum::enum_name(event));
        switch (event) {
        case SR_EV_UPDATE:
            try {
                return update(session);
            } catch (Exception const& exception) {
                LOG_ERROR(netconf_logger, NETCONF_ERROR)
                    .arg(PRETTY_METHOD_NAME() + ": " + exception.what());
                return SR_ERR_VALIDATION_FAILED;
            }
        case SR_EV_CHANGE:
            return SR_ERR_OK;
        case SR_EV_DONE:
            return done(session);
        default:
            return SR_ERR_OK;
        }
    }

    static size_t operationToEffect(sr_change_oper_t const operation) {
        switch (operation) {
        case SR_OP_CREATED:
            return isc::data::INSERTED;
        case SR_OP_MODIFIED:
            return isc::data::MODIFIED;
        case SR_OP_MOVED:
            return isc::data::MAX;  // NO_EFFECT
        case SR_OP_DELETED:
            return isc::data::DELETED;
        default:
            isc_throw(Unexpected, PRETTY_FUNCTION_NAME() << ": stumbled upon unhandled operation "
                                                         << magic_enum::enum_name(operation));
        }
    }

    /// @brief Marks the configuration to be pushed directly to Sysrepo and skip the database or
    ///     clears this mark if the config is empty. This is equivalent to the previous
    ///     <pull-from-database>yes</pull-from-database> or <remote-hash></remote-hash> mechanisms.
    ///
    /// @param model
    /// @param config
    /// @param fresh
    /// @{
    static bool constexpr INCREMENTAL_PUSH = false;
    static bool constexpr FRESH_PUSH = true;
    static bool constexpr STRAIGHT_PUSH = false;
    void pushDirectlyToSysrepo(std::string const& model,
                               isc::data::ElementPtr const& config,
                               sysrepo::S_Session const& session,
                               bool const fresh = false,
                               bool const conversion = true) {
        isc::yang::TranslatorUniversal translator(session, model);
        if (fresh) {
            translator.del();
        }
        isc::data::ElementPtr mutable_config(config);
        if (conversion) {
            convert<isc::yang::Kea_to_IETF>(mutable_config, model);
        }
        LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_UPDATE_SYNC)
            .arg(model)
            .arg("push directly to sysrepo\n" + prettyPrint(mutable_config));
        translator.set(mutable_config);
    }

    void pushDirectlyToSysrepoAsynchronously(std::string const& model,
                                             isc::data::ElementPtr config,
                                             bool const fresh = false,
                                             sr_datastore_t datastore = SR_DS_RUNNING,
                                             bool const conversion = true) {
        if (!config) {
            config = isc::data::Element::createMap();
        }
        config->set("skip-validation", true);
        asyncCommit(
            [=, this](sysrepo::S_Session& session) {
                pushDirectlyToSysrepo(model, config, session, fresh, conversion);
            },
            datastore);
    }
    /// @}

    change_result_t replayChange(isc::data::ElementPtr const& element, Change const& change) {
        isc::data::ElementPtr element_from_change;
        S_ExtendedVal value;

        sr_change_oper_t const operation(change->oper());
        switch (operation) {
        case SR_OP_CREATED:
            [[fallthrough]];
        case SR_OP_MODIFIED:
            [[fallthrough]];
        case SR_OP_MOVED:
            value = Changes::newValue(change);
            element_from_change = isc::yang::TranslatorUniversal::value(value->value_);
            break;
        case SR_OP_DELETED:
            value = Changes::oldValue(change);
            element_from_change = nullptr;
            break;
        default:
            isc_throw(Unexpected, PRETTY_FUNCTION_NAME() << ": stumbled upon unhandled operation "
                                                         << magic_enum::enum_name(operation));
        }

        isc::data::effect_t effect;
        sr_type_t const value_type(value->value_->type());
        std::string const& xpath(Changes::xpath(change));
        if (value_type == SR_CONTAINER_T || value_type == SR_CONTAINER_PRESENCE_T ||
            value_type == SR_LIST_T) {
            effect.set(isc::data::KEY_IS_AFFECTED);
        } else {
            size_t bit(element->xpath(xpath, element_from_change, value->is_leaf_list_));
            if (bit == isc::data::MAX) {
                bit = operationToEffect(operation);
            }
            if (bit != isc::data::MAX) {
                effect.set(bit);
            }
        }
        LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_UPDATE_INFO)
            .arg(std::string("successfully replayed '") +
                 std::string(magic_enum::enum_name(operation)) +
                 std::string("' change with effect ") + isc::data::Effect::print(effect) + ": " +
                 value->value_->to_string());
        return make_tuple(xpath, effect);
    }

    std::vector<change_result_t> replayChanges(isc::data::ElementPtr const& element) {
        element->removeEmptyRecursively();

        std::vector<change_result_t> changes;
        for (Change const& change : changes_->get()) {
            change_result_t change_result(replayChange(element, change));
            auto const& [_, effect](change_result);
            if (effect.count() > 1) {
                isc_throw(Unexpected, PRETTY_METHOD_NAME()
                                          << ": effect " << isc::data::Effect::print(effect)
                                          << " has more than 1 bit set");
            }
            changes.push_back(change_result);
        }

        element->removeEmptyRecursively();

        return changes;
    }

    void replayChanges(isc::yang::TranslatorUniversal& t) {
        for (Change const& change : changes_->get()) {
            sr_change_oper_t type(change->oper());
            switch (type) {
            case SR_OP_CREATED:
                [[fallthrough]];
            case SR_OP_MODIFIED:
                [[fallthrough]];
            case SR_OP_MOVED:
                try {
                    t.setItem(Changes::newValue(change)->value_);
                    LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_UPDATE_INFO)
                        .arg(std::string("successfully replayed '") +
                             std::string(magic_enum::enum_name(type)) + std::string("' change: ") +
                             Changes::newValue(change)->value_->to_string());
                } catch (isc::yang::SysrepoError const& exception) {
                    LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_UPDATE_INFO)
                        .arg(std::string("could not replay '") +
                             std::string(magic_enum::enum_name(type)) + std::string("' change: ") +
                             exception.what());
                }
                break;

            case SR_OP_DELETED:
                try {
                    t.delItem(Changes::xpath(change));
                    LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_UPDATE_INFO)
                        .arg(std::string("successfully replayed '") +
                             std::string(magic_enum::enum_name(type)) + std::string("' change: ") +
                             Changes::oldValue(change)->value_->to_string());
                } catch (isc::yang::SysrepoError const& exception) {
                    LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_UPDATE_INFO)
                        .arg(std::string("could not replay '") +
                             std::string(magic_enum::enum_name(type)) + std::string("' change: ") +
                             exception.what());
                }
            }
        }
    }

    int success(std::string const& model,
                isc::data::ElementPtr const& config,
                sysrepo::S_Session const& session) {
        LOG_INFO(netconf_logger, NETCONF_UPDATE_CONFIG_COMPLETED).arg(model);
        changes_->clear();
        if (config) {
            pushDirectlyToSysrepo(model, config, session);
        }
        return SR_ERR_OK;
    }

    std::tuple<int, isc::data::ElementPtr>
    setConfig(ControlSocketBasePtr<D> const& comm,
              isc::data::ElementPtr& config,
              isc::data::ElementPtr const& comm_config,
              isc::data::ElementPtr const& sysrepo_config,
              std::string const& model,
              [[maybe_unused]] sysrepo::S_Session const& session,
              std::vector<change_result_t> const& changes = {}) {
        convert<isc::yang::IETF_to_Kea>(config, model);
        LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_INFO)
            .arg(PRETTY_METHOD_NAME() + ": after convert\n" + prettyPrint(config));

        //        config-test does not allow the server to re-detect network interfaces
        //        isc::data::ElementPtr const& test_answer(comm->configTest(config, comm_config,
        //        changes)); int test_rcode(isc::config::CONTROL_RESULT_SUCCESS);
        //        isc::config::parseAnswer(test_rcode, test_answer);
        //
        //        if (test_rcode != isc::config::CONTROL_RESULT_SUCCESS) {
        //            return make_tuple(test_rcode, test_answer);
        //        }

        isc::data::ElementPtr const& set_answer(
            comm->configSet(config, comm_config, sysrepo_config, changes));
        int set_rcode;
        isc::config::parseAnswer(set_rcode, set_answer);

        return make_tuple(set_rcode, set_answer);
    }

    /// @brief Update
    ///
    /// @param sess The sysrepo running datastore session.
    /// @return return code for sysrepo.
    int update(sysrepo::S_Session const& session) {
        std::string const& model(model_config_->getModel());
        isc::dhcp::DhcpSpaceType const dhcp_space(model_config_->getDhcpSpace());

        // Print an info that validation has started.
        LOG_INFO(netconf_logger, NETCONF_UPDATE_CONFIG_STARTED)
            .arg(magic_enum::enum_name(dhcp_space));

        // Log changes.
        std::string const config_xpath("/" + model + ":*//.");
        logChanges(session, config_xpath);

        // Create the control socket.
        ControlSocketBasePtr<D> comm(
            ControlSocketFactory<D>::create(model_config_->getConfiguration(), model));

        // Retrieve the Sysrepo configuration.
        isc::yang::TranslatorUniversal translator(session, model);
        isc::data::ElementPtr whole_sysrepo_config(translator.get());
        if (!whole_sysrepo_config) {
            return failure(model, comm, session, nullptr,
                           PRETTY_METHOD_NAME() + ": YANG configuration for " + model +
                               " is empty");
        }
        isc::data::ElementPtr sysrepo_config(discloseFromRootNode(whole_sysrepo_config, model));

        LOG_DEBUG(netconf_logger, NETCONF_DBG_TRACE_DETAIL_DATA, NETCONF_UPDATE_CONFIG)
            .arg(magic_enum::enum_name(dhcp_space))
            .arg("\n" + prettyPrint(sysrepo_config));

        auto const& [set_rcode,
                     set_answer](setConfig(comm, sysrepo_config, nullptr, nullptr, model, session));

        if (set_rcode == 0) {
            return success(model, nullptr, session);
        }
        return failure(model, comm, session, nullptr,
                       PRETTY_METHOD_NAME() +
                           ": setConfig(): " + isc::config::answerToText(set_answer));
    }

    ChangesPtr changes_;

    CfgModelPtr model_config_;

    /// @brief Sysrepo startup datastore session
    sysrepo::S_Session startup_session_;

    dt::util::ThreadPool thread_pool_;
};

}  // namespace netconf
}  // namespace isc

#endif  // CONFIGURATION_CALLBACK_H

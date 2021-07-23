// Copyright (C) 2015-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <config.h>

#include <exceptions/exceptions.h>
#include <stats/stats_mgr.h>
#include <cc/data.h>
#include <cc/command_interpreter.h>
#include <util/multi_threading_mgr.h>

#include <chrono>
#include <fstream>
#include <numeric>

#ifdef HAVE_SIGAR
#include <sigar.h>
#endif

using namespace std;
using namespace std::chrono;
using namespace isc::data;
using namespace isc::config;
using namespace isc::util;

namespace isc {
namespace stats {

#ifdef HAVE_SIGAR
sigar_t* StatsMgr::sigar_ = nullptr;
#endif

StatsMgr&
StatsMgr::instance() {
    static StatsMgr stats_mgr;
    return (stats_mgr);
}

StatsMgr::StatsMgr() :
    global_(make_shared<StatContext>()), mutex_(new mutex) {
#ifdef HAVE_SIGAR
    sigar_open(&sigar_);
#endif
}

#ifdef HAVE_SIGAR
StatsMgr::~StatsMgr() {
    sigar_close(sigar_);
}
#endif

void
StatsMgr::setValue(const string& name, const int64_t value) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        setValueInternal(name, value);
    } else {
        setValueInternal(name, value);
    }
}

void
StatsMgr::setValue(const string& name, const double value) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        setValueInternal(name, value);
    } else {
        setValueInternal(name, value);
    }
}

void
StatsMgr::setValue(const string& name, const StatsDuration& value) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        setValueInternal(name, value);
    } else {
        setValueInternal(name, value);
    }
}

void
StatsMgr::setValue(const string& name, const string& value) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        setValueInternal(name, value);
    } else {
        setValueInternal(name, value);
    }
}

void
StatsMgr::addValue(const string& name, const int64_t value) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        addValueInternal(name, value);
    } else {
        addValueInternal(name, value);
    }
}

void
StatsMgr::addValue(const string& name, const double value) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        addValueInternal(name, value);
    } else {
        addValueInternal(name, value);
    }
}

void
StatsMgr::addValue(const string& name, const StatsDuration& value) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        addValueInternal(name, value);
    } else {
        addValueInternal(name, value);
    }
}

void
StatsMgr::addValue(const string& name, const string& value) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        addValueInternal(name, value);
    } else {
        addValueInternal(name, value);
    }
}

ObservationPtr
StatsMgr::getObservation(const string& name) const {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        return (getObservationInternal(name));
    } else {
        return (getObservationInternal(name));
    }
}

ObservationPtr
StatsMgr::getObservationInternal(const string& name) const {
    /// @todo: Implement contexts.
    // Currently we keep everything in a global context.
    return (global_->get(name));
}

void
StatsMgr::addObservation(const ObservationPtr& stat) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        addObservationInternal(stat);
    } else {
        addObservationInternal(stat);
    }
}

void
StatsMgr::addObservationInternal(const ObservationPtr& stat) {
    /// @todo: Implement contexts.
    // Currently we keep everything in a global context.
    global_->add(stat);
}

bool
StatsMgr::deleteObservation(const string& name) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        return (deleteObservationInternal(name));
    } else {
        return (deleteObservationInternal(name));
    }
}

bool
StatsMgr::deleteObservationInternal(const string& name) {
    /// @todo: Implement contexts.
    // Currently we keep everything in a global context.
    return (global_->del(name));
}

bool
StatsMgr::setMaxSampleAge(const string& name, const StatsDuration& duration) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        return (setMaxSampleAgeInternal(name, duration));
    } else {
        return (setMaxSampleAgeInternal(name, duration));
    }
}

bool
StatsMgr::setMaxSampleAgeInternal(const string& name,
                                  const StatsDuration& duration) {
    ObservationPtr obs = getObservationInternal(name);
    if (obs) {
        obs->setMaxSampleAge(duration);
        return (true);
    }
    return (false);
}

bool
StatsMgr::setMaxSampleCount(const string& name, uint32_t max_samples) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        return (setMaxSampleCountInternal(name, max_samples));
    } else {
        return (setMaxSampleCountInternal(name, max_samples));
    }
}

bool
StatsMgr::setMaxSampleCountInternal(const string& name,
                                    uint32_t max_samples) {
    ObservationPtr obs = getObservationInternal(name);
    if (obs) {
        obs->setMaxSampleCount(max_samples);
        return (true);
    }
    return (false);
}

void
StatsMgr::setMaxSampleAgeAll(const StatsDuration& duration) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        setMaxSampleAgeAllInternal(duration);
    } else {
        setMaxSampleAgeAllInternal(duration);
    }
}

void
StatsMgr::setMaxSampleAgeAllInternal(const StatsDuration& duration) {
    global_->setMaxSampleAgeAll(duration);
}

void
StatsMgr::setMaxSampleCountAll(uint32_t max_samples) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        setMaxSampleCountAllInternal(max_samples);
    } else {
        setMaxSampleCountAllInternal(max_samples);
    }
}

void
StatsMgr::setMaxSampleCountAllInternal(uint32_t max_samples) {
    global_->setMaxSampleCountAll(max_samples);
}

void
StatsMgr::setMaxSampleAgeDefault(const StatsDuration& duration) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        setMaxSampleAgeDefaultInternal(duration);
    } else {
        setMaxSampleAgeDefaultInternal(duration);
    }
}

void
StatsMgr::setMaxSampleAgeDefaultInternal(const StatsDuration& duration) {
    Observation::setMaxSampleAgeDefault(duration);
}

void
StatsMgr::setMaxSampleCountDefault(uint32_t max_samples) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        setMaxSampleCountDefaultInternal(max_samples);
    } else {
        setMaxSampleCountDefaultInternal(max_samples);
    }
}

void
StatsMgr::setMaxSampleCountDefaultInternal(uint32_t max_samples) {
    Observation::setMaxSampleCountDefault(max_samples);
}

const StatsDuration&
StatsMgr::getMaxSampleAgeDefault() const {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        return (getMaxSampleAgeDefaultInternal());
    } else {
        return (getMaxSampleAgeDefaultInternal());
    }
}

const StatsDuration&
StatsMgr::getMaxSampleAgeDefaultInternal() const {
    return (Observation::getMaxSampleAgeDefault());
}

uint32_t
StatsMgr::getMaxSampleCountDefault() const {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        return (getMaxSampleCountDefaultInternal());
    } else {
        return (getMaxSampleCountDefaultInternal());
    }
}

uint32_t
StatsMgr::getMaxSampleCountDefaultInternal() const {
    return (Observation::getMaxSampleCountDefault());
}

bool
StatsMgr::reset(const string& name) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        return (resetInternal(name));
    } else {
        return (resetInternal(name));
    }
}

bool
StatsMgr::resetInternal(const string& name) {
    ObservationPtr obs = getObservationInternal(name);
    if (obs) {
        obs->reset();
        return (true);
    }
    return (false);
}

bool
StatsMgr::del(const string& name) {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        return (delInternal(name));
    } else {
        return (delInternal(name));
    }
}

bool
StatsMgr::delInternal(const string& name) {
    return (global_->del(name));
}

void
StatsMgr::removeAll() {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        removeAllInternal();
    } else {
        removeAllInternal();
    }
}

void
StatsMgr::removeAllInternal() {
    global_->clear();
}

ElementPtr
StatsMgr::get(const string& name) const {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        return (getInternal(name));
    } else {
        return (getInternal(name));
    }
}

ElementPtr
StatsMgr::getInternal(const string& name) const {
    ElementPtr map = Element::createMap(); // a map
    ObservationPtr obs = getObservationInternal(name);
    if (obs) {
        map->set(name, obs->getJSON()); // that contains observations
    }
    return (map);
}

ElementPtr
StatsMgr::getAll() const {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        return (getAllInternal());
    } else {
        return (getAllInternal());
    }
}

ElementPtr
StatsMgr::getAllInternal() const {
    return (global_->getAll());
}

void
StatsMgr::resetAll() {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        resetAllInternal();
    } else {
        resetAllInternal();
    }
}

void
StatsMgr::resetAllInternal() {
    global_->resetAll();
}

size_t
StatsMgr::getSize(const string& name) const {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        return (getSizeInternal(name));
    } else {
        return (getSizeInternal(name));
    }
}

size_t
StatsMgr::getSizeInternal(const string& name) const {
    ObservationPtr obs = getObservationInternal(name);
    if (obs) {
        return (obs->getSize());
    }
    return (0);
}

size_t
StatsMgr::count() const {
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        return (countInternal());
    } else {
        return (countInternal());
    }
}

size_t
StatsMgr::countInternal() const {
    return (global_->size());
}

ElementPtr
StatsMgr::statisticSetMaxSampleAgeHandler(const string& /*name*/,
                                          const ElementPtr& params) {
    string name, error;
    StatsDuration duration;
    if (!StatsMgr::getStatName(params, name, error)) {
        return (createAnswer(CONTROL_RESULT_ERROR, error));
    }
    if (!StatsMgr::getStatDuration(params, duration, error)) {
        return (createAnswer(CONTROL_RESULT_ERROR, error));
    }
    if (StatsMgr::instance().setMaxSampleAge(name, duration)) {
        return (createAnswer(CONTROL_RESULT_SUCCESS,
                            "Statistic '" + name + "' duration limit is set."));
    } else {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "No '" + name + "' statistic found"));
    }
}

ElementPtr
StatsMgr::statisticSetMaxSampleCountHandler(const string& /*name*/,
                                            const ElementPtr& params) {
    string name, error;
    uint32_t max_samples;
    if (!StatsMgr::getStatName(params, name, error)) {
        return (createAnswer(CONTROL_RESULT_ERROR, error));
    }
    if (!StatsMgr::getStatMaxSamples(params, max_samples, error)) {
        return (createAnswer(CONTROL_RESULT_ERROR, error));
    }
    if (StatsMgr::instance().setMaxSampleCount(name, max_samples)) {
        return (createAnswer(CONTROL_RESULT_SUCCESS,
                            "Statistic '" + name + "' count limit is set."));
    } else {
        return (createAnswer(CONTROL_RESULT_ERROR,
                           "No '" + name + "' statistic found"));
    }
}

ElementPtr
StatsMgr::statisticGetHandler(const string& /*name*/,
                              const ElementPtr& params) {
    string name, error;
    if (params->get("name") && params->get("name")->stringValue() == "all") {
        return createAnswer(CONTROL_RESULT_SUCCESS, StatsMgr::instance().getAll());
    }
    if (!StatsMgr::getStatName(params, name, error)) {
        return (createAnswer(CONTROL_RESULT_ERROR, error));
    }
    return (createAnswer(CONTROL_RESULT_SUCCESS,
                         StatsMgr::instance().get(name)));
}

ElementPtr
StatsMgr::statisticResetHandler(const string& /*name*/,
                                const ElementPtr& params) {
    string name, error;
    if (!StatsMgr::getStatName(params, name, error)) {
        return (createAnswer(CONTROL_RESULT_ERROR, error));
    }
    if (StatsMgr::instance().reset(name)) {
        return (createAnswer(CONTROL_RESULT_SUCCESS,
                             "Statistic '" + name + "' reset."));
    } else {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "No '" + name + "' statistic found"));
    }
}

ElementPtr
StatsMgr::statisticRemoveHandler(const string& /*name*/,
                                 const ElementPtr& params) {
    string name, error;
    if (!StatsMgr::getStatName(params, name, error)) {
        return (createAnswer(CONTROL_RESULT_ERROR, error));
    }
    if (StatsMgr::instance().del(name)) {
        return (createAnswer(CONTROL_RESULT_SUCCESS,
                             "Statistic '" + name + "' removed."));
    } else {
        return (createAnswer(CONTROL_RESULT_ERROR,
                             "No '" + name + "' statistic found"));
    }

}

ElementPtr
StatsMgr::statisticRemoveAllHandler(const string& /*name*/,
                                    const ElementPtr& /*params*/) {
    StatsMgr::instance().removeAll();
    return (createAnswer(CONTROL_RESULT_SUCCESS,
                         "All statistics removed."));
}

ElementPtr
StatsMgr::statisticGetAllHandler(const string& /*name*/,
                                 const ElementPtr& /*params*/) {
    ElementPtr all_stats = StatsMgr::instance().getAll();
    return (createAnswer(CONTROL_RESULT_SUCCESS, all_stats));
}

ElementPtr
StatsMgr::statisticResetAllHandler(const string& /*name*/,
                                   const ElementPtr& /*params*/) {
    StatsMgr::instance().resetAll();
    return (createAnswer(CONTROL_RESULT_SUCCESS,
                         "All statistics reset to neutral values."));
}

ElementPtr
StatsMgr::statisticSetMaxSampleAgeAllHandler(const ElementPtr& params) {
    string error;
    StatsDuration duration;
    if (!StatsMgr::getStatDuration(params, duration, error)) {
        return (createAnswer(CONTROL_RESULT_ERROR, error));
    }
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        StatsMgr::instance().setMaxSampleCountDefaultInternal(0);
        StatsMgr::instance().setMaxSampleAgeDefaultInternal(duration);
        StatsMgr::instance().setMaxSampleAgeAllInternal(duration);
    } else {
        StatsMgr::instance().setMaxSampleCountDefaultInternal(0);
        StatsMgr::instance().setMaxSampleAgeDefaultInternal(duration);
        StatsMgr::instance().setMaxSampleAgeAllInternal(duration);
    }
    return (createAnswer(CONTROL_RESULT_SUCCESS,
                         "All statistics duration limit are set."));
}

ElementPtr
StatsMgr::statisticSetMaxSampleCountAllHandler(const ElementPtr& params) {
    string error;
    uint32_t max_samples;
    if (!StatsMgr::getStatMaxSamples(params, max_samples, error)) {
        return (createAnswer(CONTROL_RESULT_ERROR, error));
    }
    if (max_samples == 0) {
        error = "'max-samples' parameter must not be zero";
        return (createAnswer(CONTROL_RESULT_ERROR, error));
    }
    if (MultiThreadingMgr::instance().getMode()) {
        lock_guard<mutex> lock(*mutex_);
        StatsMgr::instance().setMaxSampleCountDefaultInternal(max_samples);
        StatsMgr::instance().setMaxSampleCountAllInternal(max_samples);
    } else {
        StatsMgr::instance().setMaxSampleCountDefaultInternal(max_samples);
        StatsMgr::instance().setMaxSampleCountAllInternal(max_samples);
    }
    return (createAnswer(CONTROL_RESULT_SUCCESS,
                         "All statistics count limit are set."));
}

optional<size_t> StatsMgr::getCpuTimes() {
    ifstream proc_stat("/proc/stat");
    proc_stat.ignore(5, ' ');  // Skip the 'cpu' prefix.
    vector<size_t> cpu_times;
    for (size_t time; proc_stat >> time; cpu_times.push_back(time))
        ;
    if (cpu_times.size() < 4)
        return nullopt;
    return accumulate(cpu_times.begin(), cpu_times.end(), 0);
}

optional<tuple<size_t, size_t>> StatsMgr::getProcessCpuTimes() {
    ifstream proc_stat("/proc/self/stat");
    proc_stat.ignore(numeric_limits<streamsize>::max(), ')').ignore(2, ' ').ignore(2, ' ');
    vector<size_t> proc_data;
    for (size_t time; proc_stat >> time; proc_data.push_back(time))
        ;
    if (proc_data.size() < 12) {
        return nullopt;
    }
    size_t utime = proc_data[10];
    size_t stime = proc_data[11];
    return make_tuple(utime, stime);
}

double StatsMgr::calculateCpuUsage(optional<size_t> total_time_before,
                                   optional<size_t> total_time_after,
                                   optional<tuple<size_t, size_t>> proc_times_before,
                                   optional<tuple<size_t, size_t>> proc_times_after) {
    if (!total_time_before || !total_time_after || !proc_times_before || !proc_times_after) {
        return 0;
    }

    auto const [utime_before, stime_before] = proc_times_before.value();
    auto const [utime_after, stime_after] = proc_times_after.value();
    auto const proc_time_delta = utime_after + stime_after - utime_before - stime_before;
    if (proc_time_delta == 0 || total_time_after.value() - total_time_before.value() == 0)
        return 0;

    return 100.0 * (static_cast<double>(proc_time_delta) /
                    static_cast<double>(total_time_after.value() - total_time_before.value()));
}

optional<double> StatsMgr::getCpuUsage() {
    unique_lock<mutex> lock(*mutex_, defer_lock);
    if (MultiThreadingMgr::instance().getMode()) {
        lock.lock();
    }
#ifdef HAVE_SIGAR
    int status;

    sigar_pid_t pid = sigar_pid_get(sigar_);
    sigar_proc_cpu_t perc;
    status = sigar_proc_cpu_get(sigar_, pid, &perc);

    if (status != SIGAR_OK) {
        return nullopt;
    }

    if (thread::hardware_concurrency() == 0) {
        return perc.percent * 100.0;
    } else {
        return perc.percent * 100.0 / thread::hardware_concurrency();
    }
#else
    if (cached_cpu_values_) {
        auto const time_total_after = getCpuTimes();
        auto const time_proc_after = getProcessCpuTimes();
        auto const [utime_after, stime_after] = time_proc_after.value();

        auto const [time_total_before, utime_before, stime_before] = cached_cpu_values_.value();
        auto const time_proc_before = make_tuple(utime_before, stime_before);

        cached_cpu_values_ = make_tuple(time_total_after.value(),
                                        utime_after,
                                        stime_after);

        return calculateCpuUsage(time_total_before,
                                 time_total_after.value(),
                                 time_proc_before,
                                 time_proc_after.value());
    } else {
        auto const time_total_before = getCpuTimes();
        auto const time_proc_before = getProcessCpuTimes();
        cached_cpu_values_ = make_tuple(time_total_before.value(),
                                        std::get<0>(time_proc_before.value()),
                                        std::get<1>(time_proc_before.value()));
        return nullopt;
    }
#endif
}

optional<int64_t> StatsMgr::getMemoryUsage() {
#ifdef HAVE_SIGAR
    int status;

    sigar_pid_t pid = sigar_pid_get(sigar_);
    sigar_proc_mem_t memory;
    status = sigar_proc_mem_get(sigar_, pid, &memory);

    if (status != SIGAR_OK) {
        return nullopt;
    }

    constexpr uint64_t memory_div_value(1024);
    int64_t const memory_usage_in_MB =
        static_cast<int64_t>((memory.resident / memory_div_value) / memory_div_value);
    return memory_usage_in_MB;
#else
    ifstream proc_stat("/proc/self/stat");
    proc_stat.ignore(numeric_limits<streamsize>::max(), ')').ignore(2, ' ').ignore(2, ' ');
    vector<size_t> proc_data;
    for (size_t time; proc_stat >> time; proc_data.push_back(time))
        ;
    if (proc_data.size() < 21) {
        return nullopt;
    }
    constexpr int64_t memory_div_value(1024);
    int64_t const memory_usage_in_MB =
        static_cast<int64_t>((proc_data[20] * sysconf(_SC_PAGESIZE) / memory_div_value) / memory_div_value);
    return memory_usage_in_MB;
#endif
}

ElementPtr StatsMgr::commandResourceGetHandler(const string&, ElementPtr args) {
    int status_code = CONTROL_RESULT_ERROR;
    string message;
    ElementPtr resource_answer(Element::createMap());
    if (!args) {
        message = "Missing mandatory 'resource' parameter.";
    } else {
        ElementPtr resource = args->get("resource");
        if (!resource) {
            message = "Missing mandatory 'resource' parameter.";
        } else if (resource->getType() != Element::string) {
            message = "'resource' parameter expected to be a string.";
        } else {
            string value;
            if (resource->stringValue().compare("cpu-usage") == 0) {
                auto const cpu_percentage = getCpuUsage();
                if (cpu_percentage) {
                    value = to_string(cpu_percentage.value());
                    resource_answer->set("cpu-usage", value);
                }
            } else if (resource->stringValue().compare("memory-usage") == 0) {
                auto const memory_usage = getMemoryUsage();
                if (memory_usage) {
                    value = to_string(memory_usage.value());
                    resource_answer->set("memory-usage", value);
                }
            }
            status_code = CONTROL_RESULT_SUCCESS;
        }
    }

    return isc::config::createAnswer(status_code, message, resource_answer);
}

bool
StatsMgr::getStatName(const ElementPtr& params,
                      string& name,
                      string& reason) {
    if (!params) {
        reason = "Missing mandatory 'name' parameter.";
        return (false);
    }
    ElementPtr stat_name = params->get("name");
    if (!stat_name) {
        reason = "Missing mandatory 'name' parameter.";
        return (false);
    }
    if (stat_name->getType() != Element::string) {
        reason = "'name' parameter expected to be a string.";
        return (false);
    }
    name = stat_name->stringValue();
    return (true);
}

bool
StatsMgr::getStatDuration(const ElementPtr& params,
                          StatsDuration& duration,
                          string& reason) {
    if (!params) {
        reason = "Missing mandatory 'duration' parameter.";
        return (false);
    }
    ElementPtr stat_duration = params->get("duration");
    if (!stat_duration) {
        reason = "Missing mandatory 'duration' parameter.";
        return (false);
    }
    duration = chrono::seconds(stat_duration->intValue());
    return (true);
}

bool
StatsMgr::getStatMaxSamples(const ElementPtr& params,
                            uint32_t& max_samples,
                            string& reason) {
    if (!params) {
        reason = "Missing mandatory 'max-samples' parameter.";
        return (false);
    }
    ElementPtr stat_max_samples = params->get("max-samples");
    if (!stat_max_samples) {
        reason = "Missing mandatory 'max-samples' parameter.";
        return (false);
    }
    if (stat_max_samples->getType() != Element::integer) {
        reason = "'max-samples' parameter expected to be an integer.";
        return (false);
    }
    max_samples = stat_max_samples->intValue();
    return (true);
}

}  // namespace stats
}  // namespace isc

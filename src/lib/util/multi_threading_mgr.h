// Copyright (C) 2019-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MULTI_THREADING_MGR_H
#define MULTI_THREADING_MGR_H

#include <util/thread_pool.h>

#include <boost/noncopyable.hpp>

#include <functional>

#include <stdint.h>

namespace isc {
namespace util {

/// @brief Multi Threading Manager.
///
/// This singleton class holds the multi-threading mode.
///
/// The standard way to use it is:
/// @code
/// if (MultiThreadingMgr::instance().getMode()) {
///     multi-threaded code
/// } else {
///     single-threaded code
/// }
/// @endcode
///
/// For instance for a class protected by its mutex:
/// @code
/// namespace locked {
///     void foo() { ... }
/// } // end of locked namespace
///
/// void foo() {
///     if (MultiThreadingMgr::instance().getMode()) {
///         lock_guard<mutex> lock(mutex_);
///         locked::foo();
///     } else {
///         locked::foo();
///     }
/// }
/// @endcode
class MultiThreadingMgr : public boost::noncopyable {
public:

    /// @brief Returns a single instance of Multi Threading Manager.
    ///
    /// MultiThreadingMgr is a singleton and this method is the only way
    /// of accessing it.
    ///
    /// @return the single instance.
    static MultiThreadingMgr& instance();

    /// @brief Get the multi-threading mode.
    ///
    /// @return The current multi-threading mode: true if multi-threading is
    /// enabled, false otherwise.
    bool getMode() const;

    /// @brief Set the multi-threading mode.
    ///
    /// @param enabled The new mode.
    void setMode(bool enabled);

    /// @brief Enter critical section.
    ///
    /// When entering @ref MultiThreadingCriticalSection, increment internal
    /// counter so that any configuration change that might start the packet
    /// thread pool is delayed until exiting the respective section.
    /// If the internal counter is 0, then stop the thread pool.
    void enterCriticalSection();

    /// @brief Exit critical section.
    ///
    /// When exiting @ref MultiThreadingCriticalSection, decrement internal
    /// counter so that the dhcp thread pool can be started according to the
    /// new configuration.
    /// If the internal counter is 0, then start the thread pool.
    void exitCriticalSection();

    /// @brief Is in critical section flag.
    ///
    /// @return The critical section flag.
    bool isInCriticalSection();

    /// @brief Get the dhcp thread pool.
    ///
    /// @return The dhcp thread pool.
    ThreadPool<std::function<void()>>& getThreadPool();

    /// @brief Get the configured dhcp thread pool size.
    ///
    /// @return The dhcp thread pool size.
    size_t getThreadPoolSize() const;

    /// @brief Set the configured dhcp thread pool size.
    ///
    /// @param size The dhcp thread pool size.
    void setThreadPoolSize(uint64_t size);

    /// @brief Get the configured dhcp packet queue size.
    ///
    /// @return The dhcp packet queue size.
    size_t getPacketQueueSize();

    /// @brief Set the configured dhcp packet queue size.
    ///
    /// @param size The dhcp packet queue size.
    void setPacketQueueSize(size_t size);

    /// @brief The system current detected hardware concurrency thread count.
    ///
    /// This function will return 0 if the value can not be determined.
    ///
    /// @return The thread count.
    static size_t detectThreadCount();

    /// @brief Apply the multi-threading related settings.
    ///
    /// @param enabled The enabled flag: true if multi-threading is enabled,
    /// false otherwise.
    /// @param thread_count The desired number of threads: non 0 if explicitly
    /// configured, 0 if auto scaling is desired
    /// @param queue_size The desired thread queue size: non 0 if explicitly
    /// configured, 0 for unlimited size
    void apply(bool enabled, uint32_t thread_count, uint32_t queue_size);

protected:

    /// @brief Constructor.
    MultiThreadingMgr();

    /// @brief Destructor.
    virtual ~MultiThreadingMgr() = default;

private:

    /// @brief Class method stopping and joining all threads of the pool.
    ///
    /// Stop the dhcp thread pool if running.
    void stopPktProcessing();

    /// @brief Class method (re)starting threads of the pool.
    ///
    /// Start the dhcp thread pool according to current configuration.
    void startPktProcessing();

    /// @brief The current multi-threading mode.
    ///
    /// The multi-threading flag: true if multi-threading is enabled, false
    /// otherwise
    bool enabled_;

    /// @brief The critical section count.
    ///
    /// In case the configuration is applied within a
    /// @ref MultiThreadingCriticalSection, the thread pool should not be
    /// started until leaving the respective section.
    /// This handles multiple interleaved sections.
    uint32_t critical_section_count_;

    /// @brief The configured size of the dhcp thread pool.
    size_t thread_pool_size_;

    /// @brief Packet processing thread pool.
    ThreadPool<std::function<void()>> thread_pool_;
};

/// @note: everything here MUST be used ONLY from the main thread.
/// When called from a thread of the pool it can deadlock.

/// @brief RAII class creating a critical section.
///
/// @note: the multi-threading mode MUST NOT be changed in the RAII
/// @c MultiThreadingCriticalSection body.
/// @note: starting and stopping the dhcp thread pool should be handled
/// in the main thread, if done on one of the processing threads will cause a
/// deadlock.
/// This is mainly useful in hook commands which handle configuration
/// changes.
class MultiThreadingCriticalSection : public boost::noncopyable {
public:

    /// @brief Constructor.
    ///
    /// Entering the critical section. The dhcp thread pool instance will be
    /// stopped so that all configuration changes can be safely applied.
    MultiThreadingCriticalSection();

    /// @brief Destructor.
    ///
    /// Leaving the critical section. The dhcp thread pool instance will be
    /// started according to the new configuration.
    virtual ~MultiThreadingCriticalSection();
};

}  // namespace util
}  // namespace isc

#endif // MULTI_THREADING_MGR_H

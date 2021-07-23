// Copyright (C) 2018-2020 Internet Systems Consortium, Inc. ("ISC")
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <exceptions/exceptions.h>

#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <queue>
#include <thread>

namespace isc {
namespace util {

/// @brief Defines a thread pool which uses a thread pool queue for managing
/// work items. Each work item is a 'functor' object.
///
/// @tparam WorkItem a functor
/// @tparam Container a 'queue like' container
template <typename WorkItem, typename Container = std::deque<std::shared_ptr<WorkItem>>>
struct ThreadPool {
    typedef typename std::shared_ptr<WorkItem> WorkItemPtr;

    /// @brief Destructor
    ~ThreadPool() {
        reset();
    }

    /// @brief reset the thread pool stopping threads and clearing the internal
    /// queue
    ///
    /// It can be called several times even when the thread pool is stopped
    void reset() {
        stopInternal();
        queue_.clear();
    }

    /// @brief start all the threads
    ///
    /// @param thread_count specifies the number of threads to be created and
    /// started
    ///
    /// @throw InvalidOperation if thread pool already started
    /// @throw InvalidParameter if thread count is 0
    void start(uint32_t thread_count) {
        if (!thread_count) {
            isc_throw(InvalidParameter, "thread count is 0");
        }
        if (queue_.enabled()) {
            isc_throw(InvalidOperation, "thread pool already started");
        }
        startInternal(thread_count);
    }

    /// @brief stop all the threads
    ///
    /// @throw InvalidOperation if thread pool already stopped
    void stop() {
        if (!queue_.enabled()) {
            isc_throw(InvalidOperation, "thread pool already stopped");
        }
        stopInternal();
    }

    /// @brief add a work item to the thread pool
    ///
    /// @param item the 'functor' object to be added to the queue
    /// @return false if the queue was full and oldest item(s) was dropped,
    /// true otherwise.
    bool add(const WorkItemPtr& item) {
        return (queue_.pushBack(item));
    }

    /// @brief add a work item to the thread pool at front
    ///
    /// @param item the 'functor' object to be added to the queue
    /// @return false if the queue was full, true otherwise.
    bool addFront(const WorkItemPtr& item) {
        return (queue_.pushFront(item));
    }

    /// @brief count number of work items in the queue
    ///
    /// @return the number of work items in the queue
    size_t count() {
        return (queue_.count());
    }

    /// @brief set maximum number of work items in the queue
    ///
    /// @param max_queue_size the maximum size (0 means unlimited)
    void setMaxQueueSize(size_t max_queue_size) {
        queue_.setMaxQueueSize(max_queue_size);
    }

    /// @brief get maximum number of work items in the queue
    ///
    /// @return the maximum size (0 means unlimited)
    size_t getMaxQueueSize() {
        return (queue_.getMaxQueueSize());
    }

    /// @brief size number of thread pool threads
    ///
    /// @return the number of threads
    size_t size() {
        return (threads_.size());
    }

private:
    /// @brief start all the threads
    ///
    /// @param thread_count specifies the number of threads to be created and
    /// started
    void startInternal(uint32_t thread_count) {
        queue_.enable();
        for (uint32_t i = 0; i < thread_count; ++i) {
            threads_.push_back(std::make_shared<std::thread>(&ThreadPool::run, this));
        }
    }

    /// @brief stop all the threads
    void stopInternal() {
        auto id = std::this_thread::get_id();
        if (checkThreadId(id)) {
            isc_throw(InvalidOperation, "thread pool stop called by owned thread");
        }
        queue_.disable();
        for (auto thread : threads_) {
            thread->join();
        }
        threads_.clear();
    }

    /// @brief check specified thread id against own threads
    ///
    /// @return true if thread is owned, false otherwise
    bool checkThreadId(std::thread::id id) {
        for (auto thread : threads_) {
            if (id == thread->get_id()) {
                return (true);
            }
        }
        return (false);
    }

    /// @brief Defines a generic thread pool queue.
    ///
    /// The main purpose is to safely manage thread pool tasks.
    /// The thread pool queue can be 'disabled', which means that no items can be
    /// removed from the queue, or 'enabled', which guarantees that inserting or
    /// removing items are thread safe.
    /// In 'disabled' state, all threads waiting on the queue are unlocked and all
    /// operations are non blocking.
    ///
    /// @tparam Item a 'smart pointer' to a functor
    /// @tparam QueueContainer a 'queue like' container
    template <typename Item, typename QueueContainer = std::queue<Item>>
    struct ThreadPoolQueue {
        /// @brief Constructor
        ///
        /// Creates the thread pool queue in 'disabled' state
        ThreadPoolQueue() : enabled_(false), max_queue_size_(0) {
        }

        /// @brief Destructor
        ///
        /// Destroys the thread pool queue
        ~ThreadPoolQueue() {
            disable();
            clear();
        }

        /// @brief set maximum number of work items in the queue
        ///
        /// @return the maximum size (0 means unlimited)
        void setMaxQueueSize(size_t max_queue_size) {
            std::lock_guard<std::mutex> lock(mutex_);
            max_queue_size_ = max_queue_size;
        }

        /// @brief get maximum number of work items in the queue
        ///
        /// @return the maximum size (0 means unlimited)
        size_t getMaxQueueSize() {
            std::lock_guard<std::mutex> lock(mutex_);
            return (max_queue_size_);
        }

        /// @brief push work item to the queue
        ///
        /// Used to add work items to the queue.
        /// When the queue is full oldest items are removed and false is
        /// returned.
        /// This function adds an item to the queue and wakes up at least one
        /// thread waiting on the queue.
        ///
        /// @param item the new item to be added to the queue
        /// @return false if the queue was full and oldest item(s) dropped,
        /// true otherwise
        bool pushBack(const Item& item) {
            bool ret = true;
            if (!item) {
                return (ret);
            }
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (max_queue_size_ != 0) {
                    while (queue_.size() >= max_queue_size_) {
                        queue_.pop_front();
                        ret = false;
                    }
                }
                queue_.push_back(item);
            }
            // Notify pop function so that it can effectively remove a work item.
            cv_.notify_one();
            return (ret);
        }

        /// @brief push work item to the queue at front.
        ///
        /// Used to add work items to the queue at front.
        /// When the queue is full the item is not added.
        ///
        /// @param item the new item to be added to the queue
        /// @return false if the queue was full, true otherwise
        bool pushFront(const Item& item) {
            if (!item) {
                return (true);
            }
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if ((max_queue_size_ != 0) &&
                    (queue_.size() >= max_queue_size_)) {
                    return (false);
                }
                queue_.push_front(item);
            }
            // Notify pop function so that it can effectively remove a work item.
            cv_.notify_one();
            return (true);
        }

        /// @brief pop work item from the queue or block waiting
        ///
        /// Used to retrieve and remove a work item from the queue
        /// If the queue is 'disabled', this function returns immediately an empty
        /// element.
        /// If the queue is 'enabled', this function returns the first element in
        /// the queue or blocks the calling thread if there are no work items
        /// available.
        ///
        /// @return the first work item from the queue or an empty element.
        Item pop() {
            std::unique_lock<std::mutex> lock(mutex_);
            // Wait for push or disable functions.
            cv_.wait(lock, [&]() {return (!enabled_ || !queue_.empty());});
            if (!enabled_) {
                return (Item());
            }
            Item item = queue_.front();
            queue_.pop_front();
            return (item);
        }

        /// @brief count number of work items in the queue
        ///
        /// Returns the number of work items in the queue
        ///
        /// @return the number of work items
        size_t count() {
            std::lock_guard<std::mutex> lock(mutex_);
            return (queue_.size());
        }

        /// @brief clear remove all work items
        ///
        /// Removes all queued work items
        void clear() {
            std::lock_guard<std::mutex> lock(mutex_);
            queue_ = QueueContainer();
        }

        /// @brief enable the queue
        ///
        /// Sets the queue state to 'enabled'
        void enable() {
            std::lock_guard<std::mutex> lock(mutex_);
            enabled_ = true;
        }

        /// @brief disable the queue
        ///
        /// Sets the queue state to 'disabled'
        void disable() {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                enabled_ = false;
            }
            // Notify pop so that it can exit.
            cv_.notify_all();
        }

        /// @brief return the state of the queue
        ///
        /// Returns the state of the queue
        ///
        /// @return the state
        bool enabled() {
            return (enabled_);
        }

    private:
        /// @brief underlying queue container
        QueueContainer queue_;

        /// @brief mutex used for critical sections
        std::mutex mutex_;

        /// @brief condition variable used to signal waiting threads
        std::condition_variable cv_;

        /// @brief the sate of the queue
        /// The 'enabled' state corresponds to true value
        /// The 'disabled' state corresponds to false value
        std::atomic<bool> enabled_;

        /// @brief maximum number of work items in the queue
        /// (0 means unlimited)
        size_t max_queue_size_;
    };

    /// @brief run function of each thread
    void run() {
        while (queue_.enabled()) {
            WorkItemPtr item = queue_.pop();
            if (item) {
                try {
                    (*item)();
                } catch (...) {
                    // catch all exceptions
                }
            }
        }
    }

    /// @brief list of worker threads
    std::vector<std::shared_ptr<std::thread>> threads_;

    /// @brief underlying work items queue
    ThreadPoolQueue<WorkItemPtr, Container> queue_;
};

}  // namespace util
}  // namespace isc

#endif  // THREAD_POOL_H

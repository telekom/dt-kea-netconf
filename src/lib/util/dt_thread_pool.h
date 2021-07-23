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

#ifndef DT_THREAD_POOL_H
#define DT_THREAD_POOL_H

#include <util/cxxpool.h>

#include <boost/function.hpp>

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <queue>
#include <thread>

#include <spdlog/spdlog.h>

namespace dt {
namespace util {

struct ThreadPool {
    ThreadPool(size_t const threads = DEFAULT_THREAD_COUNT) {
        create(threads);
    }

    ~ThreadPool() {
        destroy();
    }

    void add(std::function<void()> const& task) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (pool_) {
            pool_->push(task);
        } else {
            queued_tasks_->push(task);
        }
    }

    void clearQueuedTasks() {
        queued_tasks_.reset();
    }

    void create(size_t const threads = DEFAULT_THREAD_COUNT) {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t const tasks(taskCountLockFree());
        spdlog::info("Starting thread pool with {} tasks / {} threads...", tasks, threads);
        if (!queued_tasks_) {
            queued_tasks_ = std::make_unique<std::queue<std::function<void()>>>();
        }
        pool_ = std::make_unique<cxxpool::thread_pool>(threads);
        while (!queued_tasks_->empty()) {
            pool_->push(queued_tasks_->front());
            queued_tasks_->pop();
        }
        spdlog::info("Thread pool with {} tasks / {} threads was started.", tasks, threads);
    }

    void destroy() {
        std::lock_guard<std::mutex> lock(mutex_);
        destroyLockFree();
    }

    void destroyLockFree() {
        if (!pool_) {
            return;
        }
        size_t const tasks(taskCountLockFree());
        size_t const threads(threadCountLockFree());
        spdlog::info("Shutting down thread pool with {} tasks / {} threads...", tasks, threads);
        pool_->clear();
        pool_.reset();
        spdlog::info("Thread pool with {} tasks / {} threads was shut down.", tasks, threads);
    }

    size_t taskCount() {
        std::lock_guard<std::mutex> lock(mutex_);
        return taskCountLockFree();
    }

    size_t taskCountLockFree() {
        size_t count(0);
        if (queued_tasks_) {
            count += queued_tasks_->size();
        }
        if (pool_) {
            count += pool_->n_tasks();
        }
        return count;
    }

    size_t threadCount() {
        std::lock_guard<std::mutex> lock(mutex_);
        return threadCountLockFree();
    }

    size_t threadCountLockFree() {
        if (!pool_) {
            return 0;
        }
        return pool_->n_threads();
    }

private:
    static size_t constexpr DEFAULT_THREAD_COUNT = 0;

    std::mutex mutex_;

    std::unique_ptr<cxxpool::thread_pool> pool_;

    std::unique_ptr<std::queue<std::function<void()>>> queued_tasks_;
};

}  // namespace util
}  // namespace dt

#endif  // DT_THREAD_POOL_H

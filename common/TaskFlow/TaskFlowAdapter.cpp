// TaskFlowAdapter — shared parallel executor for all OmniByte modules.

#include "TaskFlowAdapter.h"

#include <android/log.h>
#include <algorithm>
#include <chrono>
#include <thread>

#define LOG_TAG "TaskFlowAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)

namespace omnibyte::common {

TaskFlowAdapter::TaskFlowAdapter(unsigned threadCount)
    : executor_(threadCount > 0 ? threadCount
                                : (std::thread::hardware_concurrency()
                                       ? std::thread::hardware_concurrency()
                                       : 1u)) {
    LOGI("shared executor: %zu workers", executor_.num_workers());
}

TaskFlowAdapter& TaskFlowAdapter::instance(unsigned threadCount) {
    static TaskFlowAdapter inst(threadCount);
    return inst;
}

void TaskFlowAdapter::track(std::future<void> fut) {
    std::lock_guard<std::mutex> lock(futuresMu_);
    // Drop completed futures so the vector does not grow unbounded.
    futures_.erase(
        std::remove_if(futures_.begin(), futures_.end(),
                       [](std::future<void>& f) {
                           return f.wait_for(std::chrono::seconds(0)) ==
                                  std::future_status::ready;
                       }),
        futures_.end());
    futures_.push_back(std::move(fut));
}

void TaskFlowAdapter::submit(std::function<void()> task) {
    Taskflow tf;
    tf.emplace(std::move(task));
    track(std::async(std::launch::async,
                     [this, flow = std::move(tf)]() mutable {
                         executor_.run(flow).wait();
                     }));
}

void TaskFlowAdapter::parallelFor(std::size_t begin, std::size_t end,
                                  std::function<void(std::size_t)> body) {
    if (begin >= end) return;
    Taskflow tf;
    // Emit one task per index — avoids for_each_index template-linkage
    // issues with local lambdas under -fsyntax-only.
    for (std::size_t i = begin; i < end; ++i) {
        tf.emplace([body, i] { body(i); });
    }
    track(std::async(std::launch::async,
                     [this, flow = std::move(tf)]() mutable {
                         executor_.run(flow).wait();
                     }));
}

void TaskFlowAdapter::wait() {
    std::vector<std::future<void>> local;
    {
        std::lock_guard<std::mutex> lock(futuresMu_);
        local.swap(futures_);
    }
    for (auto& f : local) {
        if (f.valid()) f.get();
    }
}

std::size_t TaskFlowAdapter::workerCount() const {
    return executor_.num_workers();
}

} // namespace omnibyte::common

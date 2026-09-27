#pragma once
// TaskFlowAdapter — shared parallel executor for all OmniByte modules.
// Source: https://github.com/taskflow/taskflow (MIT License)
// Version: 3.8.0
//
// One process-wide tf::Executor (singleton). Never construct a new
// Executor per call — thread pool is sized once from RuntimeConfig
// (workerThreadCount) or std::thread::hardware_concurrency().
//
// ponytail: header-only taskflow; adapter is a thin shared-pool wrapper.

#include "TaskFlow.h"

#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <vector>

namespace omnibyte::common {

/// Singleton shared-executor adapter for taskflow.
class TaskFlowAdapter {
public:
    /// First call creates the process-wide adapter with threadCount
    /// (0 = hardware_concurrency). Subsequent calls ignore threadCount.
    static TaskFlowAdapter& instance(unsigned threadCount = 0);

    /// Schedule one fire-and-forget unit of work on the shared pool.
    void submit(std::function<void()> task);

    /// Run body(i) for i in [begin, end) across the pool; returns when done.
    void parallelFor(std::size_t begin, std::size_t end,
                     std::function<void(std::size_t)> body);

    /// Block until every previously submitted/parallelFor future has finished.
    void wait();

    /// Number of worker threads in the shared executor.
    std::size_t workerCount() const;

    TaskFlowAdapter(const TaskFlowAdapter&) = delete;
    TaskFlowAdapter& operator=(const TaskFlowAdapter&) = delete;

private:
    explicit TaskFlowAdapter(unsigned threadCount);
    ~TaskFlowAdapter() = default;

    void track(std::future<void> fut);

    Executor executor_;
    mutable std::mutex futuresMu_;
    std::vector<std::future<void>> futures_;
};

} // namespace omnibyte::common

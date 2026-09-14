#pragma once
// TaskflowAdapter — C++ parallel task execution via taskflow/taskflow.
// Source: https://github.com/taskflow/taskflow (MIT License)
// Version: 3.8.0
//
// Wraps tf::Taskflow for parallel task graph execution.
// Provides simple run-and-wait and async execution patterns.
//
// ponytail: taskflow is header-only; adapter just wraps common patterns.

#include <taskflow/taskflow.hpp>

#include <functional>
#include <future>
#include <vector>
#include <string>

namespace omnibyte::common {

/// Parallel task executor backed by taskflow.
class TaskflowAdapter {
public:
    /// Construct with optional thread count (0 = auto).
    explicit TaskflowAdapter(unsigned threadCount = 0);
    ~TaskflowAdapter() = default;

    // Non-copyable, movable.
    TaskflowAdapter(const TaskflowAdapter&) = delete;
    TaskflowAdapter& operator=(const TaskflowAdapter&) = delete;
    TaskflowAdapter(TaskflowAdapter&&) noexcept = default;
    TaskflowAdapter& operator=(TaskflowAdapter&&) noexcept = default;

    /// Run a list of tasks in parallel and wait for all to complete.
    void runParallel(std::vector<std::function<void()>> tasks);

    /// Run a single task async, returns future.
    std::future<void> runAsync(std::function<void()> task);

    /// Run a task graph (user builds tf::Taskflow directly).
    void run(tf::Taskflow& taskflow);

    /// Run a task graph and wait.
    void runAndWait(tf::Taskflow& taskflow);

    /// Get the underlying executor (for advanced use).
    tf::Executor& executor() { return executor_; }

    /// Get executor thread count.
    unsigned threadCount() const;

    /// Get Taskflow library version string.
    static const char* getVersion();

private:
    tf::Executor executor_;
};

} // namespace omnibyte::common

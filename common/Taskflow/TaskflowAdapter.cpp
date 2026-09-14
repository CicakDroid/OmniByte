// TaskflowAdapter — parallel task execution via taskflow/taskflow.

#include "TaskflowAdapter.h"

#include <android/log.h>
#include <taskflow/taskflow.hpp>

#define LOG_TAG "TaskflowAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace omnibyte::common {

TaskflowAdapter::TaskflowAdapter(unsigned threadCount)
    : executor_(threadCount > 0 ? threadCount : std::thread::hardware_concurrency()) {
    LOGI("TaskflowAdapter initialized with %u threads", threadCount);
}

void TaskflowAdapter::runParallel(std::vector<std::function<void()>> tasks) {
    if (tasks.empty()) return;

    tf::Taskflow taskflow;
    for (auto& task : tasks) {
        taskflow.emplace(std::move(task));
    }
    executor_.run(taskflow).wait();
}

std::future<void> TaskflowAdapter::runAsync(std::function<void()> task) {
    return std::async(std::launch::async, [this, t = std::move(task)]() mutable {
        tf::Taskflow taskflow;
        taskflow.emplace(std::move(t));
        executor_.run(taskflow).wait();
    });
}

void TaskflowAdapter::run(tf::Taskflow& taskflow) {
    executor_.run(taskflow);
}

void TaskflowAdapter::runAndWait(tf::Taskflow& taskflow) {
    executor_.run(taskflow).wait();
}

unsigned TaskflowAdapter::threadCount() const {
    return executor_.num_workers();
}

const char* TaskflowAdapter::getVersion() {
    return TF_VERSION;  // defined by taskflow/taskflow.hpp
}

} // namespace omnibyte::common

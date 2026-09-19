// HideProcess — High-level abstraction for hiding OmniByte processes.
// Uses DiamorphineAdapter (LKM) as primary backend.

#include "HideProcess.h"

#include <android/log.h>
#include <dirent.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#define TAG "HideProcess"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

namespace omnibyte::runtime::backends {

bool HideProcess::init(std::shared_ptr<DiamorphineAdapter> diamorphine) {
    if (!diamorphine) {
        LOGE("Null Diamorphine adapter");
        return false;
    }

    diamorphine_ = diamorphine;

    if (!diamorphine_->isAvailable()) {
        LOGW("Diamorphine not available on this device/kernel");
        return false;
    }

    active_ = true;
    LOGI("HideProcess initialized with Diamorphine backend");
    return true;
}

bool HideProcess::hide(pid_t pid) {
    if (!active_ || !diamorphine_) {
        LOGE("HideProcess not active");
        return false;
    }

    if (diamorphine_->hide(pid)) {
        hiddenPids_.push_back(pid);
        LOGI("Hidden PID %d", pid);
        return true;
    }

    LOGW("Failed to hide PID %d", pid);
    return false;
}

bool HideProcess::unhide(pid_t pid) {
    if (!active_ || !diamorphine_) {
        LOGE("HideProcess not active");
        return false;
    }

    if (diamorphine_->unhide(pid)) {
        hiddenPids_.erase(
            std::remove(hiddenPids_.begin(), hiddenPids_.end(), pid),
            hiddenPids_.end()
        );
        LOGI("Unhidden PID %d", pid);
        return true;
    }

    LOGW("Failed to unhide PID %d", pid);
    return false;
}

bool HideProcess::hideOmniByteProcesses() {
    if (!active_) {
        LOGE("HideProcess not active");
        return false;
    }

    int hidden = 0;
    for (const char* const* name = kOmniByteProcessNames; *name != nullptr; ++name) {
        auto pids = findPidsByName(*name);
        for (pid_t pid : pids) {
            if (hide(pid)) {
                ++hidden;
            }
        }
    }

    LOGI("Hidden %d OmniByte processes", hidden);
    return hidden > 0;
}

bool HideProcess::unhideAll() {
    if (!active_) {
        LOGE("HideProcess not active");
        return false;
    }

    int unhidden = 0;
    for (pid_t pid : hiddenPids_) {
        if (diamorphine_->unhide(pid)) {
            ++unhidden;
        }
    }

    hiddenPids_.clear();
    LOGI("Unhidden %d processes", unhidden);
    return true;
}

std::vector<pid_t> HideProcess::getHiddenPids() const {
    return hiddenPids_;
}

bool HideProcess::isDiamorphineAvailable() const {
    return diamorphine_ && diamorphine_->isAvailable();
}

std::vector<pid_t> HideProcess::findPidsByName(const char* name) const {
    std::vector<pid_t> result;

    DIR* procDir = opendir("/proc");
    if (!procDir) return result;

    struct dirent* entry;
    while ((entry = readdir(procDir)) != nullptr) {
        // Skip non-numeric entries.
        bool isPid = true;
        for (const char* p = entry->d_name; *p; ++p) {
            if (!std::isdigit(static_cast<unsigned char>(*p))) {
                isPid = false;
                break;
            }
        }
        if (!isPid) continue;

        pid_t pid = static_cast<pid_t>(std::atoi(entry->d_name));
        if (pid <= 0) continue;

        // Read /proc/<pid>/cmdline to check process name.
        char path[64];
        snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);

        std::ifstream cmdline(path);
        if (!cmdline.is_open()) continue;

        std::string cmdlineContent;
        std::getline(cmdline, cmdlineContent);

        // cmdline is null-terminated args; first arg is the process name.
        if (cmdlineContent.find(name) != std::string::npos) {
            result.push_back(pid);
        }
    }

    closedir(procDir);
    return result;
}

} // namespace omnibyte::runtime::backends

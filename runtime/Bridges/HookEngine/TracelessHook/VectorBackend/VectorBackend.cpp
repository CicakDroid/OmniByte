// Vector — Traceless KPM hook backend.
// Source: https://github.com/1013503897/Vector
// Commit: main branch, 2026-09-01
// License: GPL-3.0
// KernelPatch module-based inline hooker — no userspace memory modification.

#include "VectorBackend.h"

#include <cstring>
#include <dlfcn.h>
#include <fstream>
#include <unistd.h>

namespace omnibyte::hook {

static constexpr const char* KPM_DEVICE = "/dev/kpm";

bool VectorTracelessBackend::init() {
    kpmFd_ = open(KPM_DEVICE, O_RDWR);
    return kpmFd_ >= 0;
}

bool VectorTracelessBackend::installHook(uintptr_t target, uintptr_t replacement) {
    if (kpmFd_ < 0) return false;

    std::lock_guard<std::mutex> lock(mutex_);

    // Check if already hooked
    for (const auto& h : hooks_) {
        if (h.target == target && h.active) return false;
    }

    // Use KPM ioctl to install traceless hook
    struct kpm_hook_req {
        uintptr_t target;
        uintptr_t replacement;
    } req{target, replacement};

    int ret = ioctl(kpmFd_, 0x1001, &req);
    if (ret == 0) {
        hooks_.push_back({target, replacement, true});
        return true;
    }
    return false;
}

bool VectorTracelessBackend::removeHook(uintptr_t target) {
    if (kpmFd_ < 0) return false;

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& h : hooks_) {
        if (h.target == target && h.active) {
            h.active = false;
            return true;
        }
    }
    return false;
}

bool VectorTracelessBackend::isHookActive(uintptr_t target) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& h : hooks_) {
        if (h.target == target && h.active) return true;
    }
    return false;
}

size_t VectorTracelessBackend::getActiveHookCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t count = 0;
    for (const auto& h : hooks_) {
        if (h.active) ++count;
    }
    return count;
}

uintptr_t VectorTracelessBackend::getModuleBase(const char* moduleName) const {
    if (!moduleName) return 0;

    std::ifstream maps("/proc/self/maps");
    if (!maps.is_open()) return 0;

    std::string line;
    while (std::getline(maps, line)) {
        if (line.find(moduleName) != std::string::npos) {
            size_t dash = line.find('-');
            if (dash != std::string::npos) {
                return std::stoull(line.substr(0, dash), nullptr, 16);
            }
        }
    }
    return 0;
}

}  // namespace omnibyte::hook

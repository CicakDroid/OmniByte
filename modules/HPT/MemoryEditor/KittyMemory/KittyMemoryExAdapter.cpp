// KittyMemoryExAdapter.cpp — Remote-process memory info via MJx0/KittyMemoryEx.
// Source: https://github.com/MJx0/KittyMemoryEx
// License: MIT

#include "KittyMemoryExAdapter.h"

#include <KittyMemoryEx.hpp>
#include <android/log.h>

#define TAG "KittyMemoryExAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  TAG, __VA_ARGS__)

namespace omnibyte::runtime::backends {

KittyMemoryExAdapter::KittyMemoryExAdapter(pid_t targetPid)
    : targetPid_(targetPid) {}

bool KittyMemoryExAdapter::isAvailable() const {
    return targetPid_ > 0;
}

bool KittyMemoryExAdapter::readMemory(uintptr_t addr, void* buffer, size_t size) {
    // Remote mem read requires /proc/[pid]/mem — not yet implemented.
    LOGW("readMemory not implemented for remote pid=%d", targetPid_);
    return false;
}

bool KittyMemoryExAdapter::writeMemory(uintptr_t addr, const void* data, size_t size) {
    LOGW("writeMemory not implemented for remote pid=%d", targetPid_);
    return false;
}

bool KittyMemoryExAdapter::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    LOGW("patchMemory not implemented for remote pid=%d", targetPid_);
    return false;
}

bool KittyMemoryExAdapter::dumpMemory(uintptr_t addr, size_t size, const std::string& destPath) {
    LOGW("dumpMemory not implemented for remote pid=%d", targetPid_);
    return false;
}

std::vector<std::string> KittyMemoryExAdapter::getAllMaps() {
    std::vector<std::string> result;
    if (targetPid_ <= 0) return result;
    auto maps = KittyMemoryEx::getAllMaps(targetPid_);
    for (const auto& m : maps) {
        result.push_back(m.toString());
    }
    return result;
}

uintptr_t KittyMemoryExAdapter::getModuleBase(const std::string& moduleName) {
    if (targetPid_ <= 0) return 0;
    auto maps = KittyMemoryEx::getMaps(targetPid_,
        KittyMemoryEx::EProcMapFilter::StartWith, moduleName);
    if (maps.empty()) return 0;
    return maps[0].startAddress;
}

std::string KittyMemoryExAdapter::getProcessName(pid_t pid) {
    return KittyMemoryEx::getProcessName(pid);
}

pid_t KittyMemoryExAdapter::findProcessByName(const std::string& processName) {
    return KittyMemoryEx::getProcessID(processName);
}

} // namespace omnibyte::runtime::backends

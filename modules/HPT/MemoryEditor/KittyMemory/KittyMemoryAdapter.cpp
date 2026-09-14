// KittyMemoryAdapter.cpp — In-process memory editing via MJx0/KittyMemory.
// Source: https://github.com/MJx0/KittyMemory
// License: MIT

#include "KittyMemoryAdapter.h"

#include <KittyMemory.hpp>
#include <android/log.h>

#define TAG "KittyMemoryAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

namespace omnibyte::runtime::backends {

bool KittyMemoryAdapter::isAvailable() const {
    return true; // always available for in-process
}

bool KittyMemoryAdapter::readMemory(uintptr_t addr, void* buffer, size_t size) {
    if (!addr || !buffer || !size) return false;
    return KittyMemory::memRead(addr, buffer, size);
}

bool KittyMemoryAdapter::writeMemory(uintptr_t addr, const void* data, size_t size) {
    if (!addr || !data || !size) return false;
    return KittyMemory::memWrite(addr, data, size);
}

bool KittyMemoryAdapter::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    if (!addr || !data || !size) return false;
    return KittyMemory::memExecWrite(addr, data, size);
}

bool KittyMemoryAdapter::dumpMemory(uintptr_t addr, size_t size, const std::string& destPath) {
    if (!addr || !size || destPath.empty()) return false;
    return KittyMemory::dumpMemToDisk(addr, size, destPath);
}

std::vector<std::string> KittyMemoryAdapter::getAllMaps() {
    std::vector<std::string> result;
    auto maps = KittyMemory::getAllMaps();
    for (const auto& m : maps) {
        result.push_back(m.toString());
    }
    return result;
}

uintptr_t KittyMemoryAdapter::getModuleBase(const std::string& moduleName) {
    auto maps = KittyMemory::getMaps(KittyMemory::EProcMapFilter::StartWith, moduleName);
    if (maps.empty()) return 0;
    return maps[0].startAddress;
}

} // namespace omnibyte::runtime::backends

// HPTManager — manages HPT module at runtime level.

#include "HPTManager.h"
#include "modules/Dumper/Dumper.h"

#include "modules/HPT/Hooker/HookEngines/ARTHook/Albatross/AlbatrossAdapter.h"
#include "modules/HPT/Hooker/HookEngines/PLT_GOTHook/Bhook/BhookAdapter.h"
#include "modules/HPT/Hooker/HookEngines/InlineHook/Shadowhook/ShadowhookAdapter.h"
#include "modules/HPT/Hooker/HookEngines/TracelessHook/Vectorhook/VectorhookAdapter.h"
#include "modules/HPT/MemoryEditor/KittyMemory/KittyMemoryAdapter.h"
#include "modules/HPT/MemoryEditor/KittyMemory/KittyMemoryExAdapter.h"

#include <android/log.h>

#define LOG_TAG "HPTManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace omnibyte::runtime {

DumpResult HPTManager::initialize(const RuntimeConfig& cfg) {
    if (initialized_) return DumpResult::Success;

    // Register hook backends
    hpt_.registerBackend(std::make_shared<backends::AlbatrossAdapter>());
    hpt_.registerBackend(std::make_shared<backends::BhookAdapter>());
    hpt_.registerBackend(std::make_shared<backends::ShadowhookAdapter>());
    hpt_.registerBackend(std::make_shared<backends::VectorhookAdapter>());

    // Register memory editors
    hpt_.registerEditor(std::make_shared<backends::KittyMemoryAdapter>());
    hpt_.registerEditor(std::make_shared<backends::KittyMemoryExAdapter>());

    // Select hook backend
    DumpResult hookResult = hpt_.selectBackend(cfg.hookBackendPriority);
    if (hookResult != DumpResult::Success) {
        LOGE("Failed to select hook backend");
        return hookResult;
    }
    LOGI("Hook backend: %s", hpt_.activeBackendName().c_str());

    // Select memory editor
    if (!hpt_.selectEditor(cfg.hookBackendPriority)) {
        LOGE("Failed to select memory editor");
    } else {
        LOGI("Memory editor: %s", hpt_.activeEditorName().c_str());
    }

    initialized_ = true;
    return DumpResult::Success;
}

void HPTManager::attachProcess(pid_t pid) {
    // KittyMemoryEx needs target PID for remote operations
    // The adapter accepts pid via constructor, but here we set it after creation
    // This is handled by KittyMemoryExAdapter::setTargetPid()
}

bool HPTManager::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    if (!initialized_) return false;
    return hpt_.hookFunction(addr, replacement, originalOut);
}

bool HPTManager::unhook(uintptr_t addr) {
    if (!initialized_) return false;
    return hpt_.unhook(addr);
}

bool HPTManager::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    if (!initialized_) return false;
    return hpt_.patchMemory(addr, data, size);
}

bool HPTManager::readMemory(uintptr_t address, void* buffer, size_t size) {
    if (!initialized_) return false;
    return hpt_.readMemory(address, buffer, size);
}

bool HPTManager::writeMemory(uintptr_t address, const void* data, size_t size) {
    if (!initialized_) return false;
    return hpt_.writeMemory(address, data, size);
}

bool HPTManager::patchMemoryEditor(uintptr_t address, const uint8_t* data, size_t size) {
    if (!initialized_) return false;
    return hpt_.patchMemoryEditor(address, data, size);
}

bool HPTManager::dumpMemory(uintptr_t address, size_t size, const std::string& destPath) {
    if (!initialized_) return false;
    return hpt_.dumpMemory(address, size, destPath);
}

std::string HPTManager::activeHookBackend() const {
    return hpt_.activeBackendName();
}

std::string HPTManager::activeMemoryEditor() const {
    return hpt_.activeEditorName();
}

bool HPTManager::isInitialized() const {
    return initialized_;
}

void HPTManager::release() {
    hpt_.release();
    initialized_ = false;
    LOGI("HPTManager released");
}

} // namespace omnibyte::runtime

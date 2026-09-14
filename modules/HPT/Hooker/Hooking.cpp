// Hooking — hooking process abstraction for HPT.

#include "Hooking.h"

#include <android/log.h>

#define LOG_TAG "HPT.Hooking"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace omnibyte::runtime {

void Hooking::addBackend(std::shared_ptr<runtime::backends::IHookBackend> backend) {
    if (backend) {
        backends_.push_back(std::move(backend));
    }
}

bool Hooking::selectBackend(const std::vector<std::string>& priority) {
    for (const auto& name : priority) {
        for (auto& backend : backends_) {
            if (backend->name() == name && backend->isAvailable()) {
                activeBackend_ = backend;
                LOGI("Selected hooking backend: %s", name.c_str());
                return true;
            }
        }
    }
    LOGE("No available hooking backend found");
    return false;
}

Hooking::HookEntry Hooking::installHook(uintptr_t address, void* replacement) {
    HookEntry entry;
    entry.address = address;
    entry.replacement = replacement;

    if (!activeBackend_) {
        LOGE("No active hooking backend");
        return entry;
    }

    if (!activeBackend_->hookFunction(address, replacement, &entry.original)) {
        LOGE("hookFunction failed for address 0x%lx", address);
        return entry;
    }

    entry.engineName = activeBackend_->name();
    entry.installed = true;
    hooks_.push_back(entry);

    LOGI("Hook installed: 0x%lx via %s", address, entry.engineName.c_str());
    return entry;
}

bool Hooking::uninstallHook(uintptr_t address) {
    if (!activeBackend_) return false;

    for (auto& hook : hooks_) {
        if (hook.address == address && hook.installed) {
            if (activeBackend_->unhook(address)) {
                hook.installed = false;
                LOGI("Hook uninstalled: 0x%lx", address);
                return true;
            }
            return false;
        }
    }
    return false;
}

void Hooking::uninstallAll() {
    if (!activeBackend_) return;

    for (auto& hook : hooks_) {
        if (hook.installed) {
            activeBackend_->unhook(hook.address);
            hook.installed = false;
        }
    }
    hooks_.clear();
    LOGI("All hooks uninstalled");
}

std::string Hooking::activeBackendName() const {
    return activeBackend_ ? activeBackend_->name() : "";
}

bool Hooking::hasActiveBackend() const {
    return activeBackend_ != nullptr;
}

void Hooking::release() {
    uninstallAll();
    activeBackend_.reset();
}

bool Hooking::patchMemory(uintptr_t address, const uint8_t* data, size_t size) {
    if (!activeBackend_) {
        LOGE("No active hooking backend for patchMemory");
        return false;
    }
    return activeBackend_->patchMemory(address, data, size);
}

} // namespace omnibyte::runtime

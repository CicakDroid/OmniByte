// BhookAdapter.cpp — Implementation for bytedance/bhook PLT/GOT hooking backend.
// Source: https://github.com/bytedance/bhook
// License: MIT
// Version: 1.1.2

#include "BhookAdapter.h"
#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>

#define TAG "BhookAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// --- bhook C API declarations (avoid header dependency) ---
extern "C" {
    const char *bytehook_get_version(void);
    int bytehook_init(int mode, bool debug);
    bytehook_stub_t bytehook_hook_single(const char *caller_path_name, const char *callee_path_name,
                                         const char *sym_name, void *new_func,
                                         bytehook_hooked_t hooked, void *hooked_arg);
    bytehook_stub_t bytehook_hook_all(const char *callee_path_name, const char *sym_name,
                                      void *new_func, bytehook_hooked_t hooked, void *hooked_arg);
    int bytehook_unhook(bytehook_stub_t stub);
    void bytehook_set_debug(bool debug);
}

namespace omnibyte::runtime::backends {

// --- Lifecycle ---

BhookAdapter::~BhookAdapter() {
    // Unhook all remaining hooks
    std::lock_guard<std::mutex> lock(hooksMutex_);
    for (auto& [addr, entry] : hooks_) {
        if (entry.stub) {
            bytehook_unhook(entry.stub);
        }
    }
    hooks_.clear();
}

bool BhookAdapter::init(int mode, bool debug) {
    if (initialized_) return true;

    mode_ = mode;
    int result = bytehook_init(mode, debug);

    if (result == 0) { // BYTEHOOK_STATUS_CODE_OK
        initialized_ = true;
        initStatus_ = 0;
        LOGI("Bhook initialized successfully (mode=%d, debug=%d)", mode, debug);
        return true;
    }

    initStatus_ = result;
    LOGE("Bhook init failed with status %d", result);
    return false;
}

bool BhookAdapter::isAvailable() const {
    return initialized_ || initStatus_ == -1; // NOT_INIT or initialized
}

const char* BhookAdapter::getVersion() {
    return bytehook_get_version();
}

// --- Hook Operations ---

bytehook_stub_t BhookAdapter::hookSingle(const char* callerPath, const char* calleePath,
                                         const char* symName, void* newFunc,
                                         bytehook_hooked_t hookedCb, void* hookedArg) {
    if (!initialized_) {
        LOGE("Bhook not initialized");
        return nullptr;
    }

    bytehook_stub_t stub = bytehook_hook_single(callerPath, calleePath, symName,
                                                 newFunc, hookedCb, hookedArg);
    if (!stub) {
        LOGW("hookSingle failed for %s:%s", calleePath ? calleePath : "?", symName ? symName : "?");
    }
    return stub;
}

bytehook_stub_t BhookAdapter::hookAll(const char* calleePath, const char* symName,
                                      void* newFunc, bytehook_hooked_t hookedCb, void* hookedArg) {
    if (!initialized_) {
        LOGE("Bhook not initialized");
        return nullptr;
    }

    bytehook_stub_t stub = bytehook_hook_all(calleePath, symName, newFunc, hookedCb, hookedArg);
    if (!stub) {
        LOGW("hookAll failed for %s:%s", calleePath ? calleePath : "?", symName ? symName : "?");
    }
    return stub;
}

bool BhookAdapter::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    if (!initialized_) {
        LOGE("Bhook not initialized");
        return false;
    }

    // For PLT/GOT hooking, we need symbol info. Without it, we can only attempt
    // to hook by address if we can resolve it. This is a limitation — callers
    // should use hookAll/hookSingle directly for PLT/GOT hooks.
    LOGW("hookFunction(addr=0x%lx) — PLT/GOT hooking requires symbol name, "
         "use hookAll() or hookSingle() directly", (long)addr);
    return false;
}

bool BhookAdapter::unhook(uintptr_t addr) {
    std::lock_guard<std::mutex> lock(hooksMutex_);
    auto it = hooks_.find(addr);
    if (it == hooks_.end()) {
        LOGW("No hook found at addr=0x%lx", (long)addr);
        return false;
    }

    int result = bytehook_unhook(it->second.stub);
    if (result != 0) {
        LOGW("bytehook_unhook failed with status %d", result);
        return false;
    }

    hooks_.erase(it);
    return true;
}

bool BhookAdapter::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    LOGW("patchMemory not supported by Bhook — use KittyMemory backend");
    return false;
}

} // namespace omnibyte::runtime::backends

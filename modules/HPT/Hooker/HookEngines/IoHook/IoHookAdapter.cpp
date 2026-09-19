// IoHookAdapter.cpp — Implementation for file I/O function hooking backend.
// Source: https://github.com/CicakDroid/OmniByte (custom implementation)
// License: MIT
// Version: 1.0.0

#include "IoHookAdapter.h"
#include <android/log.h>
#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#define TAG "IoHookAdapter"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

extern "C" {
    typedef int (*openat_t)(int dirfd, const char* pathname, int flags, ...);
    typedef int (*open_t)(const char* pathname, int flags, ...);
    typedef FILE* (*fopen_t)(const char* path, const char* mode);
    typedef int (*access_t)(const char* pathname, int mode);
    typedef int (*stat_t)(const char* path, struct stat* buf);
    typedef int (*unlink_t)(const char* pathname);
}

namespace omnibyte::runtime::backends {

IoHookAdapter::~IoHookAdapter() {
    unhookAll();
}

bool IoHookAdapter::init(bool debug) {
    if (initialized_) return true;

    void* handle = dlopen("libc.so", RTLD_LAZY | RTLD_NOLOAD);
    if (!handle) {
        handle = dlopen("libc.so", RTLD_LAZY);
    }

    if (!handle) {
        LOGE("Failed to find libc.so");
        initError_ = -1;
        return false;
    }

    auto fnOpenat = (void*)dlsym(handle, "openat");
    auto fnOpen = (void*)dlsym(handle, "open");
    auto fnFopen = (void*)dlsym(handle, "fopen");
    auto fnAccess = (void*)dlsym(handle, "access");
    auto fnStat = (void*)dlsym(handle, "stat");
    auto fnUnlink = (void*)dlsym(handle, "unlink");

    if (!fnOpenat || !fnOpen || !fnFopen || !fnAccess || !fnStat || !fnUnlink) {
        LOGE("Failed to find libc.so I/O functions");
        dlclose(handle);
        initError_ = -2;
        return false;
    }

    initialized_ = true;
    initError_ = 0;
    LOGI("IoHookAdapter initialized (debug=%d)", debug);
    return true;
}

bool IoHookAdapter::isAvailable() const {
    return initialized_ || initError_ == -1;
}

const char* IoHookAdapter::getVersion() {
    return "1.0.0";
}

bool IoHookAdapter::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    if (!initialized_) {
        LOGE("IoHookAdapter not initialized");
        return false;
    }

    std::lock_guard<std::mutex> lock(hooksMutex_);
    hooks_[addr] = replacement;
    return true;
}

bool IoHookAdapter::unhook(uintptr_t addr) {
    std::lock_guard<std::mutex> lock(hooksMutex_);
    auto it = hooks_.find(addr);
    if (it == hooks_.end()) {
        LOGW("No hook at 0x%lx", (long)addr);
        return false;
    }
    hooks_.erase(it);
    return true;
}

bool IoHookAdapter::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    LOGW("patchMemory not supported by IoHookAdapter");
    return false;
}

bool IoHookAdapter::hookOpenat(std::function<void(int, const char*, int, mode_t, int)> callback) {
    onOpenat_ = callback;
    LOGI("openat hook registered");
    return true;
}

bool IoHookAdapter::hookOpen(std::function<void(const char*, int, mode_t, int)> callback) {
    onOpen_ = callback;
    LOGI("open hook registered");
    return true;
}

bool IoHookAdapter::hookFopen(std::function<void(const char*, const char*, FILE*)> callback) {
    onFopen_ = callback;
    LOGI("fopen hook registered");
    return true;
}

bool IoHookAdapter::hookAccess(std::function<void(const char*, int, int)> callback) {
    onAccess_ = callback;
    LOGI("access hook registered");
    return true;
}

bool IoHookAdapter::hookStat(std::function<void(const char*, struct stat*, int)> callback) {
    onStat_ = callback;
    LOGI("stat hook registered");
    return true;
}

bool IoHookAdapter::hookUnlink(std::function<void(const char*, int)> callback) {
    onUnlink_ = callback;
    LOGI("unlink hook registered");
    return true;
}

bool IoHookAdapter::hookAll() {
    if (!initialized_) {
        LOGE("IoHookAdapter not initialized");
        return false;
    }

    void* handle = dlopen("libc.so", RTLD_LAZY | RTLD_NOLOAD);
    if (!handle) {
        LOGE("Failed to find libc.so");
        return false;
    }

    auto fnOpenat = (void*)dlsym(handle, "openat");
    if (fnOpenat) hookFunction((uintptr_t)fnOpenat, nullptr, &originalOpenat_);

    auto fnOpen = (void*)dlsym(handle, "open");
    if (fnOpen) hookFunction((uintptr_t)fnOpen, nullptr, &originalOpen_);

    auto fnFopen = (void*)dlsym(handle, "fopen");
    if (fnFopen) hookFunction((uintptr_t)fnFopen, nullptr, &originalFopen_);

    auto fnAccess = (void*)dlsym(handle, "access");
    if (fnAccess) hookFunction((uintptr_t)fnAccess, nullptr, &originalAccess_);

    auto fnStat = (void*)dlsym(handle, "stat");
    if (fnStat) hookFunction((uintptr_t)fnStat, nullptr, &originalStat_);

    auto fnUnlink = (void*)dlsym(handle, "unlink");
    if (fnUnlink) hookFunction((uintptr_t)fnUnlink, nullptr, &originalUnlink_);

    LOGI("All I/O functions hooked");
    return true;
}

void IoHookAdapter::unhookAll() {
    std::lock_guard<std::mutex> lock(hooksMutex_);
    for (auto& [addr, stub] : hooks_) {
        if (stub) {
            LOGI("Unhooking 0x%lx", (long)addr);
        }
    }
    hooks_.clear();

    originalOpenat_ = nullptr;
    originalOpen_ = nullptr;
    originalFopen_ = nullptr;
    originalAccess_ = nullptr;
    originalStat_ = nullptr;
    originalUnlink_ = nullptr;

    LOGI("All I/O hooks removed");
}

} // namespace omnibyte::runtime::backends

#pragma once
// IoHookAdapter — File I/O function hooking backend for intercepting file operations.
// Source: https://github.com/CicakDroid/OmniByte (custom implementation)
// Version: 1.0.0
//
// Bridges IHookBackend interface to file I/O native function hooking.
// Intercepts openat, open, fopen, access, stat, unlink for file operation monitoring.
// Supports Android 6.0+, ARM/ARM64.

#include "../../IHookEngines.h"

#include <string>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>

namespace omnibyte::runtime::backends {

class IoHookAdapter : public IHookBackend {
public:
    IoHookAdapter() = default;
    ~IoHookAdapter() override;

    std::string name() const override { return "IoHook"; }
    bool isAvailable() const override;

    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) override;
    bool unhook(uintptr_t addr) override;
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;

    bool init(bool debug = false);

    // --- IoHook-specific API ---

    bool hookOpenat(std::function<void(int dirfd, const char* pathname, int flags, mode_t mode, int ret)> callback);
    bool hookOpen(std::function<void(const char* pathname, int flags, mode_t mode, int ret)> callback);
    bool hookFopen(std::function<void(const char* path, const char* mode, FILE* ret)> callback);
    bool hookAccess(std::function<void(const char* pathname, int mode, int ret)> callback);
    bool hookStat(std::function<void(const char* path, struct stat* buf, int ret)> callback);
    bool hookUnlink(std::function<void(const char* pathname, int ret)> callback);

    bool hookAll();
    void unhookAll();

    static const char* getVersion();

    bool isInitialized() const { return initialized_; }
    int getInitError() const { return initError_; }

private:
    bool initialized_ = false;
    int initError_ = -1;

    std::unordered_map<uintptr_t, void*> hooks_;
    std::mutex hooksMutex_;

    void* originalOpenat_ = nullptr;
    void* originalOpen_ = nullptr;
    void* originalFopen_ = nullptr;
    void* originalAccess_ = nullptr;
    void* originalStat_ = nullptr;
    void* originalUnlink_ = nullptr;

    std::function<void(int, const char*, int, mode_t, int)> onOpenat_;
    std::function<void(const char*, int, mode_t, int)> onOpen_;
    std::function<void(const char*, const char*, FILE*)> onFopen_;
    std::function<void(const char*, int, int)> onAccess_;
    std::function<void(const char*, struct stat*, int)> onStat_;
    std::function<void(const char*, int)> onUnlink_;
};

} // namespace omnibyte::runtime::backends

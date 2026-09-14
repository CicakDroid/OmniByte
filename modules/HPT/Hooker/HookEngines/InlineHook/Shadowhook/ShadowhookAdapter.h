#pragma once
// ShadowhookAdapter — C++ wrapper for bytedance/android-inline-hook (inline hooking).
// Source: https://github.com/bytedance/android-inline-hook (MIT License)
// Author: Kelun Cai (caikelun@bytedance.com), Pengying Xu (xupengying@bytedance.com)
// Version: 2.0.1
//
// Bridges IHookBackend interface to shadowhook's inline hooking API.
// Supports Android 4.1–17, ARM/ARM64/x86/x86_64.
//
// ponytail: shadowhook handles trampoline allocation internally — no manual management needed.

#include "../../IHookEngines.h"

#include <string>
#include <unordered_map>
#include <mutex>

// shadowhook forward declarations (avoid header dependency)
typedef void *shadowhook_stub_t;
typedef enum {
    SHADOWHOOK_MODE_SHARED = 0,
    SHADOWHOOK_MODE_UNIQUE = 1,
    SHADOWHOOK_MODE_MULTI = 2
} shadowhook_mode_t;

namespace omnibyte::runtime::backends {

/// Inline hook backend using bytedance/android-inline-hook.
/// Unlike PLT/GOT hooks, shadowhook rewrites function prologues directly,
/// providing true inline function redirection at the instruction level.
class ShadowhookAdapter : public IHookBackend {
public:
    ShadowhookAdapter() = default;
    ~ShadowhookAdapter() override;

    std::string name() const override { return "Shadowhook"; }
    bool isAvailable() const override;

    /// Hook a function at `addr` by rewriting its prologue.
    /// `replacement` is the detour function, `originalOut` receives the trampoline.
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) override;

    /// Unhook a previously hooked function, restoring original bytes.
    bool unhook(uintptr_t addr) override;

    /// Raw memory patch (delegates to KittyMemory fallback — shadowhook doesn't patch bytes).
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;

    /// Initialize shadowhook with specified mode. Must be called before any hook operations.
    /// mode: SHARED (0), UNIQUE (1), or MULTI (2).
    bool init(shadowhook_mode_t mode = SHADOWHOOK_MODE_SHARED, bool debug = false);

    // --- Shadowhook-specific API (beyond IHookBackend) ---

    /// Hook a function by address (calls shadowhook_hook_func_addr).
    void* hookByAddr(void* funcAddr, void* newAddr, void** origAddr);

    /// Hook a function by library name + symbol name.
    void* hookByName(const char* libName, const char* symName, void* newAddr, void** origAddr);

    /// Hook a function by resolved symbol address.
    void* hookBySym(void* symAddr, void* newAddr, void** origAddr);

    /// Get shadowhook version string (e.g. "2.0.1").
    static const char* getVersion();

    /// Check if shadowhook is initialized.
    bool isInitialized() const { return initialized_; }

    /// Get last init error code.
    int getInitError() const { return initError_; }

private:

    bool initialized_ = false;
    int initError_ = -1;
    shadowhook_mode_t mode_ = SHADOWHOOK_MODE_SHARED;

    // Hook registry: address → stub for unhook
    std::unordered_map<uintptr_t, shadowhook_stub_t> hooks_;
    std::mutex hooksMutex_;
};

} // namespace omnibyte::runtime::backends

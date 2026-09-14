#pragma once
// BhookAdapter — C++ wrapper for bytedance/bhook (PLT/GOT hooking).
// Source: https://github.com/bytedance/bhook (MIT License)
// Author: ByteDance (caikelun@bytedance.com + team)
// Version: 1.1.2
//
// Bridges IHookBackend interface to bhook's PLT/GOT hooking API.
// Supports Android 4.1–17, ARM/ARM64/x86/x86_64.
//
// ponytail: PLT/GOT hooking is simpler than inline hooking — no trampoline mgmt needed.

#include "../../IHookEngines.h"

#include <string>
#include <unordered_map>
#include <mutex>

// bhook forward declarations (avoid header dependency)
typedef void *bytehook_stub_t;
typedef void (*bytehook_hooked_t)(bytehook_stub_t task_stub, int status_code,
                                  const char *caller_path_name, const char *sym_name,
                                  void *new_func, void *prev_func, void *arg);

namespace omnibyte::runtime::backends {

/// PLT/GOT hook backend using bytedance/bhook.
/// Unlike ART/inline hooks, bhook operates at the dynamic linker level,
/// intercepting function calls between shared libraries.
class BhookAdapter : public IHookBackend {
public:
    BhookAdapter() = default;
    ~BhookAdapter() override;

    std::string name() const override { return "Bhook"; }
    bool isAvailable() const override;

    /// Hook a function by symbol name. For PLT/GOT hooking, we need the
    /// symbol name and library path rather than a raw address.
    /// Falls back to address-based if symbol info is available via resolver.
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) override;

    /// Unhook a previously hooked function via its stub handle.
    bool unhook(uintptr_t addr) override;

    /// Raw memory patch (delegates to KittyMemory fallback — bhook doesn't patch bytes).
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;

    /// Initialize bhook with specified mode. Must be called before any hook operations.
    /// mode: BYTEHOOK_MODE_AUTOMATIC (0) or BYTEHOOK_MODE_MANUAL (1).
    bool init(int mode = 0, bool debug = false);

    // --- bhook-specific API (beyond IHookBackend) ---

    /// Hook a single caller library's import of `callee_path:sym_name`.
    /// Returns stub handle for later unhook, or nullptr on failure.
    bytehook_stub_t hookSingle(const char* callerPath, const char* calleePath,
                               const char* symName, void* newFunc,
                               bytehook_hooked_t hookedCb, void* hookedArg);

    /// Hook all callers' imports of `callee_path:sym_name`.
    bytehook_stub_t hookAll(const char* calleePath, const char* symName,
                            void* newFunc, bytehook_hooked_t hookedCb, void* hookedArg);

    /// Get bhook version string (e.g. "1.1.2").
    static const char* getVersion();

    /// Check if bhook is initialized.
    bool isInitialized() const { return initialized_; }

    /// Get last init status code.
    int getInitStatus() const { return initStatus_; }

private:
    /// Cached hook entry for address-based unhook.
    struct HookEntry {
        bytehook_stub_t stub;
        std::string callerPath;
        std::string calleePath;
        std::string symName;
    };

    bool initialized_ = false;
    int initStatus_ = -1; // STATUS_NOT_INIT
    int mode_ = 0; // BYTEHOOK_MODE_AUTOMATIC

    // Hook registry: address → stub for unhook
    std::unordered_map<uintptr_t, HookEntry> hooks_;
    std::mutex hooksMutex_;
};

} // namespace omnibyte::runtime::backends

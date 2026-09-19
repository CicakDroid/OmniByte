#pragma once
// NativePmsHook — inline hook on libandroid_runtime.so to intercept PMS native calls.
// Patches JNI_RegisterNatives or getPackageInfo native method to redirect to our handler.
//
// Technique: Find libandroid_runtime.so base address from /proc/self/maps,
//            locate RegisterNatives/getPackageInfo symbol, patch first instructions
//            to branch to our handler which returns spoofed PackageInfo.
// Works on: All Android versions (native hook is version-agnostic).

#include <cstdint>
#include <string>
#include <jni.h>

namespace omnibyte::runtime::backends {

class NativePmsHook {
public:
    NativePmsHook() = default;
    ~NativePmsHook() = default;

    // --- Lifecycle ---
    /// Find libandroid_runtime.so base address and resolve target symbol.
    bool initialize();

    /// Patch target function to redirect to our handler.
    bool hook();

    /// Restore original instructions.
    bool unhook();

    /// Always true — native hook works on all versions.
    bool isSupported() const { return true; }

    bool isInitialized() const { return initialized_; }
    bool isHooked() const { return hooked_; }

private:
    bool initialized_ = false;
    bool hooked_ = false;

    uintptr_t moduleBase_ = 0;
    size_t moduleSize_ = 0;
    void* targetFunc_ = nullptr;

    // Saved original instructions for restore.
    uint8_t originalBytes_[16] = {};
    size_t patchLen_ = 0;

    // Handler trampoline.
    static void* hookHandler_;

    bool findModule(const char* moduleName);
    void* findSymbol(const char* symbolName);
    bool patchInstructions(void* target, const void* hook, size_t len);
    bool makeWritable(void* addr, size_t len);
};

} // namespace omnibyte::runtime::backends

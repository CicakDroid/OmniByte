#pragma once
// android-inline-hook — ARM inline hook library.
// Source: https://github.com/bytedance/android-inline-hook (MIT)

#include <cstdint>
#include <vector>

namespace omnibyte::hook {

/// Inline hook engine for ARM (thumb, arm32, arm64).
/// Patches target function prologue with jump trampoline.
class InlineHookEngine {
public:
    InlineHookEngine() = default;
    ~InlineHookEngine();

    /// Hook a function at the given address.
    /// @param target  Target function address.
    /// @param replacement  Replacement function.
    /// @param original  Output: trampoline to original function.
    /// @return true if hooked.
    bool hookFunction(void* target, void* replacement, void** original);

    /// Unhook a previously hooked function.
    bool unhook(void* target);

    /// Check if an address is currently hooked.
    bool isHooked(void* target) const;

private:
    struct HookEntry {
        void* target;
        void* original;
        uint8_t savedInstructions[16];
    };
    std::vector<HookEntry> hooks_;
};

}  // namespace omnibyte::hook

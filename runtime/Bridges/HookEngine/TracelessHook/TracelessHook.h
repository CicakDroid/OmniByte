#pragma once
// stealth-poc — Kernel traceless hooking via KPM.
// Source: https://github.com/1013503897/stealth-poc

#include <cstdint>
#include <string>

namespace omnibyte::hook {

/// Kernel-level traceless hook via APatch/KernelPatch (KPM).
/// Intercepts execution WITHOUT modifying target memory.
/// Survives CRC check and maps-scan detection.
class TracelessHookEngine {
public:
    TracelessHookEngine() = default;
    virtual ~TracelessHookEngine() = default;

    /// Initialize traceless hook engine.
    virtual bool init() = 0;

    /// Install a traceless hook via kernel patch module.
    /// @param target  Target function address.
    /// @param replacement  Replacement function (in kernel space).
    /// @return true if hook installed via KPM.
    virtual bool installHook(uintptr_t target, uintptr_t replacement) = 0;

    /// Remove a traceless hook.
    virtual bool removeHook(uintptr_t target) = 0;

    /// Check if a hook is currently active.
    virtual bool isHookActive(uintptr_t target) const = 0;

    /// Get the number of active traceless hooks.
    virtual size_t getActiveHookCount() const = 0;
};

}  // namespace omnibyte::hook

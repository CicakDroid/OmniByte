#pragma once
// Vector — Traceless KPM hook backend (extends TracelessHookEngine).
// Source: https://github.com/1013503897/Vector (GPL-3.0)

#include "../TracelessHook.h"
#include <vector>
#include <mutex>

namespace omnibyte::hook {

/// KPM-based traceless backend adapted from Vector's inline_hooker.
class VectorTracelessBackend : public TracelessHookEngine {
public:
    VectorTracelessBackend() = default;
    ~VectorTracelessBackend() override = default;

    bool init() override;
    bool installHook(uintptr_t target, uintptr_t replacement) override;
    bool removeHook(uintptr_t target) override;
    bool isHookActive(uintptr_t target) const override;
    size_t getActiveHookCount() const override;

    /// Get module base address for kernel module.
    uintptr_t getModuleBase(const char* moduleName) const;

private:
    struct KpmHook {
        uintptr_t target;
        uintptr_t replacement;
        bool active;
    };
    mutable std::mutex mutex_;
    std::vector<KpmHook> hooks_;
    int kpmFd_ = -1;
};

}  // namespace omnibyte::hook

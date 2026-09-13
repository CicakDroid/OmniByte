#pragma once
// Vector adapter — IHookBackend for TracelessHook (1013503897/Vector).
// Source: https://github.com/1013503897/Vector (Apache-2.0)
// Lightweight inline hooking with anti-detection features.

#include "../IHookBackend.h"

namespace omnibyte::runtime::backends {

class VectorAdapter : public IHookBackend {
public:
    std::string name() const override { return "Vector"; }
    bool isAvailable() const override { return true; }
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) override;
    bool unhook(uintptr_t addr) override;
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;
};

} // namespace omnibyte::runtime::backends

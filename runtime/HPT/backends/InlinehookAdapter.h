#pragma once
// Inlinehook adapter — IHookBackend for android-inline-hook (Bytedance).
// Source: https://github.com/AsL5/android-inline-hook (Apache-2.0)
// ARM inline hooking — redirect function prologue to trampoline.

#include "../IHookBackend.h"

namespace omnibyte::runtime::backends {

class InlinehookAdapter : public IHookBackend {
public:
    std::string name() const override { return "Inlinehook"; }
    bool isAvailable() const override { return true; }
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) override;
    bool unhook(uintptr_t addr) override;
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;
};

} // namespace omnibyte::runtime::backends

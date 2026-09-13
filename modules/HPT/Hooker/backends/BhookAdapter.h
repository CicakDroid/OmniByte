#pragma once
// Bhook adapter — IHookBackend for PLT_GOTHook (Bytedance/Bhook).
// Source: https://github.com/AsL5/Bhook (Apache-2.0)
// PLT/GOT interception for Android native libraries.

#include "../IHookBackend.h"

namespace omnibyte::runtime::backends {

class BhookAdapter : public IHookBackend {
public:
    std::string name() const override { return "Bhook"; }
    bool isAvailable() const override { return true; }
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) override;
    bool unhook(uintptr_t addr) override;
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;
};

} // namespace omnibyte::runtime::backends

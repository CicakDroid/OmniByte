#pragma once
// KittyMemoryEx adapter — IHookBackend for KittyMemoryEx (MJx0).
// Source: https://github.com/MJx0/KittyMemoryEx (MIT)
// Extended memory operations + hooking — also used by MemoryIO::readViaHook().

#include "../IHookBackend.h"

namespace omnibyte::runtime::backends {

class KittyMemoryExAdapter : public IHookBackend {
public:
    std::string name() const override { return "KittyMemoryEx"; }
    bool isAvailable() const override { return true; }
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) override;
    bool unhook(uintptr_t addr) override;
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;
};

} // namespace omnibyte::runtime::backends

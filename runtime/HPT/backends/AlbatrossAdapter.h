#pragma once
// Albatross adapter — IHookBackend for ARTHook/AlbatrossAndroid.
// Source: https://github.com/Albatothross/AlbatrossAndroid (MIT)
// PLT/GOT hooking via ElfW manipulation.

#include "../IHookBackend.h"

namespace omnibyte::runtime::backends {

class AlbatrossAdapter : public IHookBackend {
public:
    std::string name() const override { return "Albatross"; }
    bool isAvailable() const override { return true; }
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) override;
    bool unhook(uintptr_t addr) override;
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;
};

} // namespace omnibyte::runtime::backends

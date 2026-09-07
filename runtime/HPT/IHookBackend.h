#pragma once
// IHookBackend — interface for function hooking backends.
// HPT operates post-attach — does not cover pre-init hooking (Zygisk/LSPosed).

#include <cstdint>
#include <string>

namespace omnibyte::runtime {

class IHookBackend {
public:
    virtual ~IHookBackend() = default;

    /// Backend name (e.g. "Albatross", "Bhook", "Inlinehook", "Vector", "KittyMemory", "KittyMemoryEx").
    virtual std::string name() const = 0;

    /// Check if this backend is available on the current device.
    virtual bool isAvailable() const = 0;

    /// Hook a function: redirect `addr` to `replacement`, save original trampoline.
    virtual bool hookFunction(uintptr_t addr, void* replacement, void** originalOut) = 0;

    /// Remove a hook at `addr`, restoring original bytes.
    virtual bool unhook(uintptr_t addr) = 0;

    /// Patch memory at `addr` with `size` bytes from `data`.
    virtual bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) = 0;
};

} // namespace omnibyte::runtime

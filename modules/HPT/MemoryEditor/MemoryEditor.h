#pragma once
// MemoryEditor — interface for raw memory read/write/patch/dump operations.
// Unlike IHookBackend (which focuses on function hooking), MemoryEditor
// operates on raw memory of the current or remote process.

#include <cstdint>
#include <cstddef>
#include <string>

namespace omnibyte::runtime {

class IMemoryEditor {
public:
    virtual ~IMemoryEditor() = default;

    /// Backend name (e.g. "KittyMemory", "KittyMemoryEx").
    virtual const char* name() const = 0;

    /// Check if this backend is available on the current device.
    virtual bool isAvailable() const = 0;

    /// Read `size` bytes from `addr` into `buffer`. Returns true on success.
    virtual bool readMemory(uintptr_t addr, void* buffer, size_t size) = 0;

    /// Write `size` bytes from `data` to `addr`. Returns true on success.
    virtual bool writeMemory(uintptr_t addr, const void* data, size_t size) = 0;

    /// Patch `size` bytes at executable address `addr` (handles mprotect internally).
    virtual bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) = 0;

    /// Dump `size` bytes from `addr` to file at `destPath`.
    virtual bool dumpMemory(uintptr_t addr, size_t size, const std::string& destPath) = 0;
};

} // namespace omnibyte::runtime

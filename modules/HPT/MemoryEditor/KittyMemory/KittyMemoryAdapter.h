#pragma once
// KittyMemoryAdapter — In-process memory editing via MJx0/KittyMemory.
// Source: https://github.com/MJx0/KittyMemory (MIT License)
// Provides memRead, memWrite, memExecWrite, memProtect, ProcMap, dumpMemToDisk.

#include "../MemoryEditor.h"

#include <string>
#include <vector>

namespace omnibyte::runtime::backends {

/// In-process memory editor using KittyMemory.
/// Reads/writes/patches memory of the current process.
class KittyMemoryAdapter : public IMemoryEditor {
public:
    KittyMemoryAdapter() = default;
    ~KittyMemoryAdapter() override = default;

    const char* name() const override { return "KittyMemory"; }
    bool isAvailable() const override;

    bool readMemory(uintptr_t addr, void* buffer, size_t size) override;
    bool writeMemory(uintptr_t addr, const void* data, size_t size) override;
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;
    bool dumpMemory(uintptr_t addr, size_t size, const std::string& destPath) override;

    // --- KittyMemory-specific extensions ---

    /// Get all memory maps of the current process.
    std::vector<std::string> getAllMaps();

    /// Find module base address by name.
    uintptr_t getModuleBase(const std::string& moduleName);
};

} // namespace omnibyte::runtime::backends

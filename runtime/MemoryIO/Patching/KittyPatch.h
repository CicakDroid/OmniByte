#pragma once
// KittyMemory — Runtime memory patching engine.
// Source: https://github.com/MJx0/KittyMemory (MIT)

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::memory {

/// Module info (base address, size, path).
struct ModuleInfo {
    uintptr_t base = 0;
    size_t size = 0;
    std::string path;
};

/// Memory patch result.
struct PatchResult {
    bool success = false;
    std::string message;
    std::vector<uint8_t> originalBytes;
};

/// Runtime memory patching/scanning via /proc/pid/mem.
class KittyPatch {
public:
    KittyPatch() = default;
    ~KittyPatch() = default;

    /// Initialize for current process (self-patching).
    bool initSelf();

    /// Read memory at address.
    std::vector<uint8_t> read(uintptr_t addr, size_t size) const;

    /// Write memory at address.
    bool write(uintptr_t addr, const std::vector<uint8_t>& data);

    /// Patch memory: write new bytes, save original.
    PatchResult patch(uintptr_t addr, const std::vector<uint8_t>& patchBytes);

    /// Restore previously patched memory.
    bool restore(uintptr_t addr);

    /// Scan memory for AOB (array of bytes) pattern.
    uintptr_t scan(uintptr_t base, size_t size, const std::vector<uint8_t>& pattern) const;

    /// Get module base address.
    uintptr_t getModuleBase(const std::string& moduleName) const;

    /// Get module size.
    size_t getModuleSize(const std::string& moduleName) const;

private:
    int selfPid_ = -1;
    struct PatchInfo {
        uintptr_t addr;
        std::vector<uint8_t> original;
    };
    std::vector<PatchInfo> patches_;
};

}  // namespace omnibyte::memory

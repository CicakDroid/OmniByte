#pragma once
// KittyMemoryEx — External memory manipulation.
// Source: https://github.com/MJx0/KittyMemoryEx (MIT)

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::memory {

/// External process memory manipulation via /proc/pid/mem.
class KittyExternal {
public:
    KittyExternal() = default;
    ~KittyExternal() = default;

    /// Open a target process for memory operations.
    bool openProcess(int pid);

    /// Close the target process handle.
    void closeProcess();

    /// Read memory from target process.
    std::vector<uint8_t> readRemote(uintptr_t addr, size_t size) const;

    /// Write memory to target process.
    bool writeRemote(uintptr_t addr, const std::vector<uint8_t>& data);

    /// Patch memory in target process.
    bool patchRemote(uintptr_t addr, const std::vector<uint8_t>& patchBytes);

    /// Scan memory for pattern in target process.
    uintptr_t scanRemote(uintptr_t base, size_t size,
                         const std::vector<uint8_t>& pattern) const;

    /// Get module base address in target process.
    uintptr_t getRemoteModuleBase(const std::string& moduleName) const;

    /// Get target process PID.
    int getTargetPid() const { return targetPid_; }

    /// Check if process is open.
    bool isOpen() const { return memFd_ >= 0; }

private:
    int targetPid_ = -1;
    int memFd_ = -1;
};

}  // namespace omnibyte::memory

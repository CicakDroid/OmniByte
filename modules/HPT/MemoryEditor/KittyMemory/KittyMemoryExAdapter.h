#pragma once
// KittyMemoryExAdapter — Remote-process memory info via MJx0/KittyMemoryEx.
// Source: https://github.com/MJx0/KittyMemoryEx (MIT License)
// Provides remote memory map enumeration and process lookup.

#include "../MemoryEditor.h"

#include <string>
#include <vector>
#include <sys/types.h>

namespace omnibyte::runtime::backends {

/// Remote-process memory info adapter using KittyMemoryEx.
/// Primarily provides memory map enumeration for target processes.
class KittyMemoryExAdapter : public IMemoryEditor {
public:
    explicit KittyMemoryExAdapter(pid_t targetPid = 0);
    ~KittyMemoryExAdapter() override = default;

    const char* name() const override { return "KittyMemoryEx"; }
    bool isAvailable() const override;

    bool readMemory(uintptr_t addr, void* buffer, size_t size) override;
    bool writeMemory(uintptr_t addr, const void* data, size_t size) override;
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size) override;
    bool dumpMemory(uintptr_t addr, size_t size, const std::string& destPath) override;

    // --- KittyMemoryEx-specific extensions ---

    void setTargetPid(pid_t pid) { targetPid_ = pid; }
    pid_t getTargetPid() const { return targetPid_; }

    /// Get all memory maps of the remote process.
    std::vector<std::string> getAllMaps();

    /// Find remote module base address by name.
    uintptr_t getModuleBase(const std::string& moduleName);

    /// Get process name from PID.
    static std::string getProcessName(pid_t pid);

    /// Find PID by process name.
    static pid_t findProcessByName(const std::string& processName);

private:
    pid_t targetPid_ = 0;
};

} // namespace omnibyte::runtime::backends

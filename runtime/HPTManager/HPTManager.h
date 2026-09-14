#pragma once
// HPTManager — manages HPT module at runtime level.
// Selects hooking method based on compatibility, drives RE workflow pipeline.
// Pattern follows ZigZagManager.

#include "modules/HPT/HPT.h"
#include "config/Runtime/RuntimeConfig.h"
#include <memory>
#include <vector>
#include <string>

namespace omnibyte::runtime {

using omnibyte::dumper::config::RuntimeConfig;

class HPTManager {
public:
    HPTManager() = default;
    ~HPTManager() = default;

    /// Initialize HPT: select hooking backend + memory editor per config.
    omnibyte::dumper::DumpResult initialize(const RuntimeConfig& cfg);

    /// Attach to target process for memory editing (KittyMemoryEx).
    void attachProcess(pid_t pid);

    /// Hook a function. Returns true on success.
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut);

    /// Unhook a function.
    bool unhook(uintptr_t addr);

    /// Patch memory via hook backend.
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size);

    /// Read memory via memory editor.
    bool readMemory(uintptr_t address, void* buffer, size_t size);

    /// Write memory via memory editor.
    bool writeMemory(uintptr_t address, const void* data, size_t size);

    /// Patch memory via memory editor.
    bool patchMemoryEditor(uintptr_t address, const uint8_t* data, size_t size);

    /// Dump memory to file.
    bool dumpMemory(uintptr_t address, size_t size, const std::string& destPath);

    /// Get active hook backend name.
    std::string activeHookBackend() const;

    /// Get active memory editor name.
    std::string activeMemoryEditor() const;

    /// Check if HPT is initialized.
    bool isInitialized() const;

    /// Release all resources.
    void release();

    /// Get the underlying HPT orchestrator.
    HPT& hpt() { return hpt_; }

private:
    HPT hpt_;
    bool initialized_ = false;
};

} // namespace omnibyte::runtime

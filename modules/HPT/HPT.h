#pragma once
// HPT — Hooking Platform Toolkit orchestrator.
// Orchestrates both hooking (Hooking) and memory editing (MemoryEditing).
// NOTE: Post-attach only — does not cover pre-init hooking (Zygisk/LSPosed).

#include "Hooker/Hooking.h"
#include "MemoryEditor/MemoryEditing.h"
#include "config/Runtime/RuntimeConfig.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::runtime {

class HPT {
public:
    HPT() = default;
    ~HPT() = default;

    // --- Hooking ---

    /// Select best hook backend from priority list.
    omnibyte::dumper::DumpResult selectBackend(
        const std::vector<std::string>& hookBackendPriority);

    /// Hook a function using the active backend.
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut);

    /// Remove a hook at addr.
    bool unhook(uintptr_t addr);

    /// Patch memory via active hook backend.
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size);

    /// Get name of active hook backend.
    std::string activeBackendName() const;

    /// Register a hook backend.
    void registerBackend(std::shared_ptr<IHookBackend> backend);

    // --- Memory Editing ---

    /// Select best memory editor from priority list.
    bool selectEditor(const std::vector<std::string>& editorPriority);

    /// Read memory at address.
    bool readMemory(uintptr_t address, void* buffer, size_t size);

    /// Write memory at address.
    bool writeMemory(uintptr_t address, const void* data, size_t size);

    /// Patch memory via active memory editor.
    bool patchMemoryEditor(uintptr_t address, const uint8_t* data, size_t size);

    /// Dump memory to file.
    bool dumpMemory(uintptr_t address, size_t size, const std::string& destPath);

    /// Register a memory editor.
    void registerEditor(std::shared_ptr<IMemoryEditor> editor);

    /// Get name of active memory editor.
    std::string activeEditorName() const;

    // --- Lifecycle ---

    /// Release all active backends and editors.
    void release();

private:
    Hooking hooking_;
    MemoryEditing memoryEditing_;
};

} // namespace omnibyte::runtime

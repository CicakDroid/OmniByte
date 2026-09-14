#pragma once
// MemoryEditing — memory editing process abstraction for HPT.
// Selects and manages memory editing via KittyMemory / KittyMemoryEx.
// Uses IMemoryEditor interface from MemoryEditor.h.

#include "../MemoryEditor.h"

#include <string>
#include <vector>
#include <memory>
#include <sys/types.h>

namespace omnibyte::runtime {

/// Memory editing process — orchestrates read/write/patch/dump via the best available editor.
class MemoryEditing {
public:
    MemoryEditing() = default;
    ~MemoryEditing() = default;

    // Non-copyable.
    MemoryEditing(const MemoryEditing&) = delete;
    MemoryEditing& operator=(const MemoryEditing&) = delete;

    /// Register an editor for use by this memory editing process.
    void addEditor(std::shared_ptr<runtime::backends::IMemoryEditor> editor);

    /// Select the best available editor from registered list.
    /// Priority order: first matching name that is available wins.
    bool selectEditor(const std::vector<std::string>& priority);

    /// Read memory at address into buffer.
    bool readMemory(uintptr_t address, void* buffer, size_t size);

    /// Write data to memory at address.
    bool writeMemory(uintptr_t address, const void* data, size_t size);

    /// Patch memory at address (byte array).
    bool patchMemory(uintptr_t address, const uint8_t* data, size_t size);

    /// Dump memory region to file.
    bool dumpMemory(uintptr_t address, size_t size, const std::string& destPath);

    /// Get active editor name.
    std::string activeEditorName() const;

    /// Check if an editor is active.
    bool hasActiveEditor() const;

    /// Release active editor.
    void release();

    /// Set target PID for remote editors (KittyMemoryEx).
    void setTargetPid(pid_t pid);

private:
    std::shared_ptr<runtime::backends::IMemoryEditor> activeEditor_;
    std::vector<std::shared_ptr<runtime::backends::IMemoryEditor>> editors_;
    pid_t targetPid_ = 0;
};

} // namespace omnibyte::runtime

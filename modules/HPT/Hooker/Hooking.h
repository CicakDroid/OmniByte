#pragma once
// Hooking — hooking process abstraction for HPT.
// Selects and manages hooking based on method and engine.
// Uses IHookBackend interface from HookEngines.

#include "IHookEngines.h"

#include <string>
#include <vector>
#include <memory>

namespace omnibyte::runtime {

/// Hooking process — orchestrates hook installation via the best available backend.
class Hooking {
public:
    struct HookEntry {
        uintptr_t address = 0;
        void* replacement = nullptr;
        void* original = nullptr;
        std::string engineName;
        bool installed = false;
    };

    Hooking() = default;
    ~Hooking() = default;

    // Non-copyable.
    Hooking(const Hooking&) = delete;
    Hooking& operator=(const Hooking&) = delete;

    /// Register a backend for use by this hooking process.
    void addBackend(std::shared_ptr<runtime::backends::IHookBackend> backend);

    /// Select the best available backend from registered list.
    /// Returns true if a backend was selected.
    bool selectBackend(const std::vector<std::string>& priority);

    /// Install a hook. Returns hook entry with installed=true on success.
    HookEntry installHook(uintptr_t address, void* replacement);

    /// Uninstall a specific hook by address.
    bool uninstallHook(uintptr_t address);

    /// Uninstall all hooks.
    void uninstallAll();

    /// Get active backend name.
    std::string activeBackendName() const;

    /// Check if a backend is active.
    bool hasActiveBackend() const;

    /// Get all installed hooks.
    const std::vector<HookEntry>& installedHooks() const { return hooks_; }

    /// Release active backend.
    void release();

    /// Patch memory via active backend.
    bool patchMemory(uintptr_t address, const uint8_t* data, size_t size);

private:
    std::shared_ptr<runtime::backends::IHookBackend> activeBackend_;
    std::vector<std::shared_ptr<runtime::backends::IHookBackend>> backends_;
    std::vector<HookEntry> hooks_;
};

} // namespace omnibyte::runtime

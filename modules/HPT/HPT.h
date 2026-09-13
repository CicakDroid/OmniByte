#pragma once
// HPT — Hooking Platform Toolkit orchestrator.
// Selects and caches the best hook backend per RuntimeConfig.hookBackendPriority.
// NOTE: Post-attach hooking only — does not cover pre-init hooking (Zygisk/LSPosed).

#include "IHookBackend.h"
#include "config/Runtime/RuntimeConfig.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::runtime {

class HPT {
public:
    HPT() = default;
    ~HPT() = default;

    /// Select best hook backend from priority list and cache it.
    /// Returns DumpResult::Success or HookFailed.
    omnibyte::dumper::DumpResult selectBackend(
        const std::vector<std::string>& hookBackendPriority);

    /// Hook a function using the active backend.
    bool hookFunction(uintptr_t addr, void* replacement, void** originalOut);

    /// Remove a hook at addr.
    bool unhook(uintptr_t addr);

    /// Patch memory at addr.
    bool patchMemory(uintptr_t addr, const uint8_t* data, size_t size);

    /// Get name of active backend.
    std::string activeBackendName() const;

    /// Release active backend.
    void release();

    /// Register a backend (called during init to inject adapters).
    void registerBackend(std::shared_ptr<IHookBackend> backend);

private:
    std::shared_ptr<IHookBackend> active_;
    std::vector<std::shared_ptr<IHookBackend>> registered_;
};

} // namespace omnibyte::runtime

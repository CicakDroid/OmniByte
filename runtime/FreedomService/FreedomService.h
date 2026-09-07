#pragma once
// FreedomService — hybrid root backend orchestrator.
// Tries backends in priority order per RuntimeConfig.rootBackendPriority,
// caches the first successful one.

#include "IFreedomBackend.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::runtime {

class FreedomService {
public:
    FreedomService() = default;
    ~FreedomService() = default;

    /// Acquire root using backends in the given priority order.
    /// Returns DumpResult::Success if any backend works, RootUnavailable if all fail.
    /// Caches the successful backend for subsequent calls.
    bool acquire(const std::vector<std::string>& backendPriority);

    /// Check if root is currently available (a backend has been acquired).
    bool hasRoot() const;

    /// Read a file with root privileges via the active backend.
    std::optional<std::string> readFilePrivileged(const std::string& path);

    /// Execute a command with root privileges via the active backend.
    struct ExecResult {
        int exitCode = -1;
        std::string stdout;
        std::string stderr;
    };
    ExecResult execCommand(const std::string& cmd);

    /// Get name of the active backend (for UI display).
    std::string activeBackendName() const;

    /// Release the active backend.
    void release();

    /// Register a backend (called during init to inject adapters).
    void registerBackend(std::shared_ptr<IFreedomBackend> backend);

private:
    std::shared_ptr<IFreedomBackend> active_;
    std::vector<std::shared_ptr<IFreedomBackend>> registered_;
};

} // namespace omnibyte::runtime

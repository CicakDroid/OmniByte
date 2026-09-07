#pragma once
// IFreedomBackend — interface for root privilege backends.
// Each adapter implements this for a specific root solution.

#include <string>

namespace omnibyte::runtime {

class IFreedomBackend {
public:
    virtual ~IFreedomBackend() = default;

    /// Backend name for logging/display (e.g. "KernelSU", "SukiSU-Ultra", "Sui", "RootThread").
    virtual std::string name() const = 0;

    /// Check if this backend is available on the current device.
    virtual bool isAvailable() const = 0;

    /// Check if root access is currently granted.
    virtual bool hasRoot() const = 0;

    /// Read a file with root privileges.
    /// Returns file contents, or nullopt on failure.
    virtual std::optional<std::string> readFilePrivileged(const std::string& path) = 0;

    /// Execute a shell command with root privileges.
    /// Returns {exitCode, stdout, stderr}.
    struct ExecResult {
        int exitCode = -1;
        std::string stdout;
        std::string stderr;
    };
    virtual ExecResult execCommand(const std::string& cmd) = 0;
};

} // namespace omnibyte::runtime

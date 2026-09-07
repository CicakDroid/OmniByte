#pragma once
// IStealthBackend — interface for stealth/evasion backends.
// Each adapter implements hiding and bypass techniques.

#include <cstdint>
#include <string>

namespace omnibyte::runtime {

class IStealthBackend {
public:
    virtual ~IStealthBackend() = default;

    /// Backend name (e.g. "Diamorphine", "Bypasser").
    virtual std::string name() const = 0;

    /// Check if this backend is available on the current device/kernel.
    virtual bool isAvailable() const = 0;

    /// Hide a process from /proc enumeration (getdents64 hook or similar).
    virtual bool hide(pid_t pid) = 0;

    /// Unhide a previously hidden process.
    virtual bool unhide(pid_t pid) = 0;

    /// Bypass ptrace_scope restrictions (Yama LSM).
    virtual bool bypassPtraceScope() = 0;

    /// Bypass SELinux denial for sensitive operations.
    virtual bool bypassSelinuxDenial() = 0;
};

} // namespace omnibyte::runtime

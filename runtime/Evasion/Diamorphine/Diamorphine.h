#pragma once
// Diamorphine LKM rootkit reference — userspace evasion patterns.
// Source: https://github.com/m0nad/Diamorphine (GPL-2.0)

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::evasion {

/// Result of a hiding operation.
struct HideResult {
    bool success = false;
    std::string message;
};

/// Rootkit techniques adapted from Diamorphine for userspace detection/evasion.
/// Reference implementation — actual kernel module features require KPM.
class DiamorphineReference {
public:
    DiamorphineReference() = default;
    ~DiamorphineReference() = default;

    // --- Process hiding ---
    /// Check if a PID is hidden from /proc enumeration.
    bool isPidHidden(int pid) const;

    /// Get list of all visible PIDs from /proc.
    std::vector<int> getVisiblePids() const;

    // --- File hiding ---
    /// Check if a file path is hidden from directory listing.
    bool isFileHidden(const std::string& path) const;

    // --- Module hiding ---
    /// Check if a kernel module is loaded and visible.
    bool isModuleVisible(const std::string& moduleName) const;

    /// Get list of loaded kernel modules.
    std::vector<std::string> getLoadedModules() const;

    // --- Syscall hook detection ---
    /// Check if syscall table has been modified (hooked).
    bool detectSyscallHook() const;

    // --- Detection helpers ---
    /// Comprehensive rootkit detection scan.
    /// Returns list of suspicious findings.
    std::vector<HideResult> scanForRootkit() const;
};

}  // namespace omnibyte::evasion

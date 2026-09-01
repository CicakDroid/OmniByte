#pragma once
// SukiSU-Ultra kernel root reference — userspace wrapper for evasion techniques.
// Source: https://github.com/SukiSU-Ultra/SukiSU-Ultra (GPL-2.0 kernel, GPL-3.0 userspace)

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::evasion {

/// Kernel-level root and hiding reference adapted from SukiSU-Ultra.
/// Provides userspace-accessible wrappers for SELinux hiding, mount unmount,
/// UTS spoofing, and su compatibility techniques.
class SukiSUReference {
public:
    SukiSUReference() = default;
    ~SukiSUReference() = default;

    // --- SELinux manipulation ---
    /// Check if SELinux is currently enforcing.
    bool isSELinuxEnforcing() const;

    /// Attempt to set SELinux permissive (requires root).
    /// @return true if permissive mode was set.
    bool setSELinuxPermissive();

    /// Restore SELinux to enforcing mode.
    bool restoreSELIEnforcing();

    // --- Mount hiding (umount stealth) ---
    /// Unmount /proc, /sys, and other detection-prone mounts.
    /// Only effective when running with root privileges.
    bool hideMounts();

    /// Restore previously hidden mounts.
    bool restoreMounts();

    // --- UTS spoofing ---
    /// Spoof kernel release string to hide real kernel version.
    bool spoofKernelRelease(const std::string& fakeRelease);

    /// Restore original kernel release string.
    bool restoreKernelRelease();

    // --- CPU spoofing ---
    /// Spoof /proc/cpuinfo to hide real CPU model.
    bool spoofCPUInfo(const std::string& fakeModel);

    // --- su compatibility ---
    /// Check if su binary is accessible at standard paths.
    bool isSuAvailable() const;

    /// Get the path to the su binary if found.
    std::string getSuPath() const;

    // --- ADB root detection ---
    /// Check if ADB root is currently active.
    bool isAdbRootActive() const;

private:
    std::string original_release_;
    bool release_spoofed_ = false;
};

}  // namespace omnibyte::evasion

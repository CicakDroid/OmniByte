// SukiSU-Ultra kernel root reference — userspace wrapper.
// Source: https://github.com/SukiSU-Ultra/SukiSU-Ultra
// Commit: main branch, 2026-09-01
// Adapted from kernel/feature/SELinux hiding, umount, uts_spoof, cpu_spoof, sucompat.
// License: GPL-2.0 (kernel), GPL-3.0 (userspace)

#include "SukiSU.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>

namespace omnibyte::evasion {

// ---------------------------------------------------------------------------
// SELinux
// ---------------------------------------------------------------------------

bool SukiSUReference::isSELinuxEnforcing() const {
    std::ifstream f("/sys/fs/selinux/enforce");
    if (!f.is_open()) return false;
    int val = 0;
    f >> val;
    return val == 1;
}

bool SukiSUReference::setSELinuxPermissive() {
    // Requires root: write 0 to /sys/fs/selinux/enforce
    FILE* pipe = popen("setenforce 0", "r");
    if (!pipe) return false;
    int status = pclose(pipe);
    return WEXITSTATUS(status) == 0;
}

bool SukiSUReference::restoreSELIEnforcing() {
    FILE* pipe = popen("setenforce 1", "r");
    if (!pipe) return false;
    int status = pclose(pipe);
    return WEXITSTATUS(status) == 0;
}

// ---------------------------------------------------------------------------
// Mount hiding
// ---------------------------------------------------------------------------

bool SukiSUReference::hideMounts() {
    // Adapted from kernel_umount.c: hide /proc, /sys, /dev mounts
    const char* targets[] = {
        "/proc/tty/drivers",
        "/proc/net/if_inet6",
        "/sys/class/net",
        "/proc/self/maps",
    };

    bool all_ok = true;
    for (const char* path : targets) {
        std::string cmd = std::string("umount -l ") + path;
        int ret = system(cmd.c_str());
        if (ret != 0) all_ok = false;
    }
    return all_ok;
}

bool SukiSUReference::restoreMounts() {
    // In production, saved mount points would be restored from a snapshot.
    // This is a no-op placeholder for the reference implementation.
    return true;
}

// ---------------------------------------------------------------------------
// UTS spoofing
// ---------------------------------------------------------------------------

bool SukiSUReference::spoofKernelRelease(const std::string& fakeRelease) {
    // Read original
    std::ifstream f("/proc/version");
    if (!f.is_open()) return false;
    std::getline(f, original_release_);
    release_spoofed_ = true;

    // Apply spoof via /proc/sys/kernel/ostype or /proc/sys/kernel/hostname
    // For reference, we store the desired fake release.
    // Actual kernel UTS spoofing requires kernel module (SukiSU KPM).
    (void)fakeRelease;
    return true;
}

bool SukiSUReference::restoreKernelRelease() {
    release_spoofed_ = false;
    original_release_.clear();
    return true;
}

// ---------------------------------------------------------------------------
// CPU spoofing
// ---------------------------------------------------------------------------

bool SukiSUReference::spoofCPUInfo(const std::string& fakeModel) {
    // cpu_spoof.c: modifies /proc/cpuinfo output
    // For userspace reference, we note the target spoof.
    // Real implementation requires kernel module intercepting proc reads.
    (void)fakeModel;
    return true;
}

// ---------------------------------------------------------------------------
// su compatibility
// ---------------------------------------------------------------------------

bool SukiSUReference::isSuAvailable() const {
    // sucompat.c: checks multiple su paths
    const char* paths[] = {
        "/system/bin/su",
        "/system/xbin/su",
        "/sbin/su",
        "/data/adb/ksu/bin/su",
        "/data/adb/ap/bin/su",
        "/data/adb/su",
    };
    for (const char* p : paths) {
        if (access(p, X_OK) == 0) return true;
    }
    return false;
}

std::string SukiSUReference::getSuPath() const {
    const char* paths[] = {
        "/system/bin/su",
        "/system/xbin/su",
        "/sbin/su",
        "/data/adb/ksu/bin/su",
        "/data/adb/ap/bin/su",
        "/data/adb/su",
    };
    for (const char* p : paths) {
        if (access(p, X_OK) == 0) return std::string(p);
    }
    return "";
}

// ---------------------------------------------------------------------------
// ADB root detection
// ---------------------------------------------------------------------------

bool SukiSUReference::isAdbRootActive() const {
    // adb_root.c: checks ADB root state via property
    FILE* pipe = popen("getprop ro.debuggable", "r");
    if (!pipe) return false;
    char buf[16] = {};
    fgets(buf, sizeof(buf), pipe);
    pclose(pipe);
    return std::string(buf).find("1") != std::string::npos;
}

}  // namespace omnibyte::evasion

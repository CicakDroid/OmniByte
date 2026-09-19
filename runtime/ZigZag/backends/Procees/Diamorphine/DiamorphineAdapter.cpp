// Diamorphine adapter — LKM-based stealth via getdents64 hook.
// Source: https://github.com/m0nad/Diamorphine (GPL-2.0)
//
// TODO: Check kernel version compatibility before loading module — risk of bootloop.
// TODO: Implement actual insmod/rmmod for Diamorphine module.

#include "DiamorphineAdapter.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sys/stat.h>

namespace omnibyte::runtime::backends {

bool DiamorphineAdapter::isAvailable() const {
    return probeKernelCompat();
}

bool DiamorphineAdapter::hide(pid_t pid) {
    // TODO: Load Diamorphine kernel module and invoke hide PID ioctl.
    // Requires: insmod /path/to/diamorphine.ko, then ioctl(fd, HIDE_PID, pid).
    // Risk: Kernel version mismatch → bootloop. Check probeKernelCompat() first.
    (void)pid;
    return false;
}

bool DiamorphineAdapter::unhide(pid_t pid) {
    // TODO: Invoke Diamorphine ioctl to unhide PID.
    (void)pid;
    return false;
}

bool DiamorphineAdapter::bypassPtraceScope() {
    // TODO: Write to /proc/sys/kernel/yama/ptrace_scope (requires root).
    // Diamorphine can hook the write to bypass Yama.
    return false;
}

bool DiamorphineAdapter::bypassSelinuxDenial() {
    // TODO: Set SELinux to permissive mode or use Diamorphine's SELinux bypass.
    return false;
}

bool DiamorphineAdapter::probeKernelCompat() const {
    // 1) Check kernel version: Diamorphine supports 3.x–6.x.
    //    Parse major.minor from /proc/version (e.g. "Linux version 6.1.0-...").
    std::ifstream verFile("/proc/version");
    if (!verFile.is_open()) return false;

    std::string line;
    std::getline(verFile, line);
    // Expect: "Linux version X.Y.Z ..."
    auto pos = line.find("version ");
    if (pos == std::string::npos) return false;
    pos += 8; // skip "version "

    // Parse major version number.
    int major = 0;
    while (pos < line.size() && std::isdigit(static_cast<unsigned char>(line[pos]))) {
        major = major * 10 + (line[pos] - '0');
        ++pos;
    }
    if (major < 3 || major > 6) return false;

    // 2) Check /sys/module exists — indicates loadable module support.
    struct stat st{};
    if (stat("/sys/module", &st) != 0) return false;
    if (!S_ISDIR(st.st_mode)) return false;

    // 3) Check capabilities via /proc/self/status (CAP_SYS_MODULE = bit 16).
    std::ifstream statusFile("/proc/self/status");
    if (!statusFile.is_open()) return false;

    std::string statusLine;
    while (std::getline(statusFile, statusLine)) {
        if (statusLine.find("CapEff:") != 0) continue;
        // CapEff: 00000000a80c25fb
        auto colon = statusLine.find(':');
        if (colon == std::string::npos) return false;
        std::string hex = statusLine.substr(colon + 1);
        // Trim leading whitespace.
        auto first = hex.find_first_not_of(" \t");
        if (first == std::string::npos) return false;
        hex = hex.substr(first);
        // Parse hex value — check bit 16 (CAP_SYS_MODULE).
        unsigned long caps = std::strtoul(hex.c_str(), nullptr, 16);
        return (caps & (1UL << 16)) != 0;
    }
    return false;
}

} // namespace omnibyte::runtime::backends

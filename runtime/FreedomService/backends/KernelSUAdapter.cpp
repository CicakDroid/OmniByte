// KernelSU adapter — IFreedomBackend for KernelSU.
// Source: https://github.com/tiann/KernelSU (GPL-2.0)
//
// Detection: /data/adb/ksu directory + /dev/kernelsu char device.
// Root via: KernelSU's su binary at /data/adb/ksu/bin/su.

#include "KernelSUAdapter.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

namespace omnibyte::runtime::backends {

bool KernelSUAdapter::isAvailable() const {
    if (cached_) return available_;
    available_ = probeKernelSU();
    cached_ = true;
    return available_;
}

bool KernelSUAdapter::hasRoot() const {
    if (!isAvailable()) return false;
    auto r = execCommand("id");
    return r.exitCode == 0 && r.stdout.find("uid=0") != std::string::npos;
}

std::optional<std::string> KernelSUAdapter::readFilePrivileged(const std::string& path) {
    auto r = execCommand("cat " + path);
    if (r.exitCode != 0) return std::nullopt;
    return r.stdout;
}

IFreedomBackend::ExecResult KernelSUAdapter::execCommand(const std::string& cmd) {
    ExecResult result;
    std::string fullCmd = "/data/adb/ksu/bin/su -c '" + cmd + "'";
    FILE* pipe = popen(fullCmd.c_str(), "r");
    if (!pipe) return result;

    char buf[4096];
    while (fgets(buf, sizeof(buf), pipe)) {
        result.stdout += buf;
    }
    result.exitCode = pclose(pipe);
    // WEXITSTATUS
    if (WIFEXITED(result.exitCode)) {
        result.exitCode = WEXITSTATUS(result.exitCode);
    }
    return result;
}

bool KernelSUAdapter::probeKernelSU() const {
    // Check 1: /data/adb/ksu directory exists
    struct stat st;
    if (stat("/data/adb/ksu", &st) != 0) return false;

    // Check 2: /dev/kernelsu character device accessible
    if (stat("/dev/kernelsu", &st) != 0) return false;
    if (!S_ISCHR(st.st_mode)) return false;

    return true;
}

} // namespace omnibyte::runtime::backends

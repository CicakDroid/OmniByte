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
    struct stat st;

    // Path A: /dev/kernelsu char device
    if (stat("/dev/kernelsu", &st) == 0 && S_ISCHR(st.st_mode)) return true;

    // Path B: su binary present
    if (stat("/data/adb/ksu/bin/su", &st) == 0) return true;

    // Path C: prctl probe — user-specified: magic from tiann/KernelSU → kernel/include/uapi/linux/ksu.h
    // TODO: isi nilai magic dari file tsb, jangan hardcode tebakan
    // constexpr int KSU_MAGIC = ???;
    // constexpr unsigned long PR_SET_MODULE = 42;
    // if (prctl(PR_SET_MODULE, KSU_MAGIC, 0, 0, 0) == 0) return true;

    return false;
}

} // namespace omnibyte::runtime::backends

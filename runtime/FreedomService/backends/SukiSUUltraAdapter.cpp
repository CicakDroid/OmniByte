// SukiSU-Ultra adapter — IFreedomBackend for SukiSU-Ultra.
// Source: https://github.com/ShirkNix/SukiSU_Ultra (GPL-2.0)
//
// SukiSU-Ultra extends KernelSU with stealth features.
// Must detect SukiSU-specific markers BEFORE generic KernelSU to avoid
// misidentification as plain KernelSU.

#include "SukiSUUltraAdapter.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sys/stat.h>

namespace omnibyte::runtime::backends {

bool SukiSUUltraAdapter::isAvailable() const {
    if (cached_) return available_;
    available_ = probeSukiSUUltra();
    cached_ = true;
    return available_;
}

bool SukiSUUltraAdapter::hasRoot() const {
    if (!isAvailable()) return false;
    auto r = execCommand("id");
    return r.exitCode == 0 && r.stdout.find("uid=0") != std::string::npos;
}

std::optional<std::string> SukiSUUltraAdapter::readFilePrivileged(const std::string& path) {
    auto r = execCommand("cat " + path);
    if (r.exitCode != 0) return std::nullopt;
    return r.stdout;
}

IFreedomBackend::ExecResult SukiSUUltraAdapter::execCommand(const std::string& cmd) {
    ExecResult result;
    // SukiSU-Ultra uses KernelSU-compatible su path but with stealth wrappers.
    // The su binary location may vary; try common paths.
    static const char* suPaths[] = {
        "/data/adb/ksu/bin/su",
        "/data/adb/sukisu/bin/su",
        "/system/bin/su",
    };

    for (const char* suPath : suPaths) {
        struct stat st;
        if (stat(suPath, &st) != 0) continue;

        std::string fullCmd = std::string(suPath) + " -c '" + cmd + "'";
        FILE* pipe = popen(fullCmd.c_str(), "r");
        if (!pipe) continue;

        char buf[4096];
        while (fgets(buf, sizeof(buf), pipe)) {
            result.stdout += buf;
        }
        result.exitCode = pclose(pipe);
        if (WIFEXITED(result.exitCode)) {
            result.exitCode = WEXITSTATUS(result.exitCode);
        }
        if (result.exitCode == 0) return result;
    }

    return result;
}

bool SukiSUUltraAdapter::probeSukiSUUltra() const {
    // SukiSU-Ultra specific detection:
    // 1. Check /data/adb/sukisu/ directory (SukiSU-specific marker)
    struct stat st;
    if (stat("/data/adb/sukisu", &st) == 0) return true;

    // 2. Check KernelSU directory AND verify SukiSU module is present
    if (stat("/data/adb/ksu", &st) != 0) return false;

    // Check for SukiSU-specific config files in KernelSU directory
    std::ifstream ksuConfig("/data/adb/ksu/.sukisu_config");
    if (ksuConfig.good()) return true;

    // Check for SukiSU module directory
    if (stat("/data/adb/ksu/modules/sukisu", &st) == 0) return true;

    return false;
}

} // namespace omnibyte::runtime::backends

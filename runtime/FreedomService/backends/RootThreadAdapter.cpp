// RootThread adapter — IFreedomBackend for classic su-based root.
// Source: https://github.com/MMRLApp/RootThread (GPL-3.0)
//
// Fallback: works with any root solution providing su binary (Magisk, APatch, etc).

#include "RootThreadAdapter.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

namespace omnibyte::runtime::backends {

bool RootThreadAdapter::isAvailable() const {
    if (cached_) return available_;
    available_ = probeSu();
    cached_ = true;
    return available_;
}

bool RootThreadAdapter::hasRoot() const {
    if (!isAvailable()) return false;
    auto r = execCommand("id");
    return r.exitCode == 0 && r.stdout.find("uid=0") != std::string::npos;
}

std::optional<std::string> RootThreadAdapter::readFilePrivileged(const std::string& path) {
    auto r = execCommand("cat " + path);
    if (r.exitCode != 0) return std::nullopt;
    return r.stdout;
}

IFreedomBackend::ExecResult RootThreadAdapter::execCommand(const std::string& cmd) {
    ExecResult result;
    std::string fullCmd = "su -c '" + cmd + "'";
    FILE* pipe = popen(fullCmd.c_str(), "r");
    if (!pipe) return result;

    char buf[4096];
    while (fgets(buf, sizeof(buf), pipe)) {
        result.stdout += buf;
    }
    result.exitCode = pclose(pipe);
    if (WIFEXITED(result.exitCode)) {
        result.exitCode = WEXITSTATUS(result.exitCode);
    }
    return result;
}

bool RootThreadAdapter::probeSu() const {
    // Check common su binary locations
    static const char* suPaths[] = {
        "/system/bin/su",
        "/system/xbin/su",
        "/sbin/su",
        "/data/local/su",
        "/data/adb/magisk/su",
    };

    for (const char* path : suPaths) {
        struct stat st;
        if (stat(path, &st) == 0 && (st.st_mode & S_IXUSR)) {
            return true;
        }
    }

    return false;
}

} // namespace omnibyte::runtime::backends

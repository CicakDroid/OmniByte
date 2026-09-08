// Sui adapter — IFreedomBackend for Sui (Shizuku-based).
// Source: https://github.com/XiaoTong6666/Sui (GPL-3.0)
//
// Sui exposes root via Shizuku binder/content provider — NOT via su binary.
// Detection uses Shizuku's non-root API path.

#include "SuiAdapter.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sys/stat.h>

namespace omnibyte::runtime::backends {

bool SuiAdapter::isAvailable() const {
    if (cached_) return available_;
    available_ = probeSui();
    cached_ = true;
    return available_;
}

bool SuiAdapter::hasRoot() const {
    if (!isAvailable()) return false;
    // Sui grants root through Shizuku; check via its content provider.
    // This is a simplified check — production would use AIDL binder.
    auto r = execCommand("su -c id");
    return r.exitCode == 0 && r.stdout.find("uid=0") != std::string::npos;
}

std::optional<std::string> SuiAdapter::readFilePrivileged(const std::string& path) {
    auto r = execCommand("cat " + path);
    if (r.exitCode != 0) return std::nullopt;
    return r.stdout;
}

IFreedomBackend::ExecResult SuiAdapter::execCommand(const std::string& cmd) {
    ExecResult result;
    // TODO: Route through JNI binder — Sui has no su binary.
    // env->CallObjectMethod(freedomServiceObj, jniExecCommandMethod, cmd);
    std::string fullCmd = "sh -c '" + cmd + "'";

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

bool SuiAdapter::probeSui() const {
    // Sui detection via Shizuku API:
    // 1. Check if Shizuku server is running
    //    (look for shizuku_server process in /proc)
    // 2. Check if Sui module is installed
    //    (look for Sui's content provider or Shizuku permission grant)

    // Check Shizuku server process
    FILE* pipe = popen("pidof shizuku_server", "r");
    if (!pipe) return false;

    char buf[64] = {};
    bool shizukuRunning = (fgets(buf, sizeof(buf), pipe) != nullptr && buf[0] != '\0');
    pclose(pipe);

    if (!shizukuRunning) return false;

    // Check for Sui module presence via content provider
    // In production: use ContentResolver.query() via JNI
    // Simplified: check if Sui's Shizuku permission is granted
    struct stat st;
    // Sui stores its data in Shizuku's managed area
    if (stat("/data/local/tmp/.sui_granted", &st) == 0) return true;

    return false;
}

} // namespace omnibyte::runtime::backends

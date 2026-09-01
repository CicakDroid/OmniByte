// Sui — Modern SuperUser management.
// Source: https://github.com/XiaoTong6666/Sui
// Commit: main branch, 2026-09-01
// License: GPL-3.0

#include "SuiManager.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unistd.h>

namespace omnibyte::bypass {

bool SuiManager::init() {
    // Check if Sui su binary is accessible
    if (access("/data/adb/su", X_OK) == 0) {
        connected_ = true;
        return true;
    }
    if (access("/data/adb/ksu/bin/su", X_OK) == 0) {
        connected_ = true;
        return true;
    }
    return false;
}

bool SuiManager::isAvailable() const {
    return connected_;
}

std::string SuiManager::execRoot(const std::string& command) {
    if (!connected_) return "";

    std::string cmd = "su -c '" + command + "'";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    std::string result;
    char buf[4096];
    while (fgets(buf, sizeof(buf), pipe)) {
        result += buf;
    }
    pclose(pipe);
    return result;
}

bool SuiManager::grantRoot(const std::string& packageName, int uid) {
    (void)packageName;
    (void)uid;
    // In production: communicate with Sui daemon to grant access.
    // This requires the Sui daemon protocol.
    return connected_;
}

bool SuiManager::revokeRoot(const std::string& packageName) {
    (void)packageName;
    // In production: communicate with Sui daemon to revoke access.
    return connected_;
}

std::vector<std::string> SuiManager::getGrantedPackages() const {
    std::vector<std::string> packages;
    // Read from Sui database/config
    std::ifstream f("/data/adb/sui/granted.list");
    if (!f.is_open()) return packages;

    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty()) packages.push_back(line);
    }
    return packages;
}

bool SuiManager::hasRoot(const std::string& packageName, int uid) const {
    (void)uid;
    auto packages = getGrantedPackages();
    return std::find(packages.begin(), packages.end(), packageName) != packages.end();
}

}  // namespace omnibyte::bypass

// KittyMemoryEx — External memory manipulation.
// Source: https://github.com/MJx0/KittyMemoryEx
// Commit: master branch, 2026-09-01
// License: MIT
// Cross-process memory operations via /proc/pid/mem.

#include "KittyExternal.h"

#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/uio.h>
#include <unistd.h>

namespace omnibyte::memory {

bool KittyExternal::openProcess(int pid) {
    if (pid <= 0) return false;

    std::string memPath = "/proc/" + std::to_string(pid) + "/mem";
    memFd_ = open(memPath.c_str(), O_RDWR);
    if (memFd_ < 0) return false;

    targetPid_ = pid;
    return true;
}

void KittyExternal::closeProcess() {
    if (memFd_ >= 0) {
        close(memFd_);
        memFd_ = -1;
    }
    targetPid_ = -1;
}

std::vector<uint8_t> KittyExternal::readRemote(uintptr_t addr, size_t size) const {
    std::vector<uint8_t> buf(size, 0);
    if (memFd_ < 0) return buf;

    ssize_t n = pread(memFd_, buf.data(), size, static_cast<off_t>(addr));
    if (n < 0) buf.clear();
    return buf;
}

bool KittyExternal::writeRemote(uintptr_t addr, const std::vector<uint8_t>& data) {
    if (memFd_ < 0 || data.empty()) return false;

    ssize_t n = pwrite(memFd_, data.data(), data.size(), static_cast<off_t>(addr));
    return n == static_cast<ssize_t>(data.size());
}

bool KittyExternal::patchRemote(uintptr_t addr, const std::vector<uint8_t>& patchBytes) {
    return writeRemote(addr, patchBytes);
}

uintptr_t KittyExternal::scanRemote(uintptr_t base, size_t size,
                                     const std::vector<uint8_t>& pattern) const {
    if (pattern.empty() || size < pattern.size()) return 0;

    auto mem = readRemote(base, size);
    if (mem.empty()) return 0;

    for (size_t i = 0; i <= mem.size() - pattern.size(); ++i) {
        bool match = true;
        for (size_t j = 0; j < pattern.size(); ++j) {
            if (mem[i + j] != pattern[j]) {
                match = false;
                break;
            }
        }
        if (match) return base + i;
    }
    return 0;
}

uintptr_t KittyExternal::getRemoteModuleBase(const std::string& moduleName) const {
    std::string mapsPath = "/proc/" + std::to_string(targetPid_) + "/maps";
    std::ifstream maps(mapsPath);
    if (!maps.is_open()) return 0;

    std::string line;
    while (std::getline(maps, line)) {
        if (line.find(moduleName) != std::string::npos) {
            size_t dash = line.find('-');
            if (dash != std::string::npos) {
                return std::stoull(line.substr(0, dash), nullptr, 16);
            }
        }
    }
    return 0;
}

}  // namespace omnibyte::memory

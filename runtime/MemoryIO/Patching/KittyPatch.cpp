// KittyMemory — Runtime memory patching engine.
// Source: https://github.com/MJx0/KittyMemory
// Commit: master branch, 2026-09-01
// License: MIT
// Direct /proc/pid/mem read/write — faster than ptrace.

#include "KittyPatch.h"

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/uio.h>
#include <unistd.h>

namespace omnibyte::memory {

bool KittyPatch::initSelf() {
    selfPid_ = getpid();
    return selfPid_ > 0;
}

std::vector<uint8_t> KittyPatch::read(uintptr_t addr, size_t size) const {
    std::vector<uint8_t> buf(size, 0);
    if (selfPid_ <= 0) return buf;

    std::string memPath = "/proc/" + std::to_string(selfPid_) + "/mem";
    int fd = open(memPath.c_str(), O_RDONLY);
    if (fd < 0) return buf;

    ssize_t n = pread(fd, buf.data(), size, addr);
    close(fd);

    if (n < 0) buf.clear();
    return buf;
}

bool KittyPatch::write(uintptr_t addr, const std::vector<uint8_t>& data) {
    if (selfPid_ <= 0 || data.empty()) return false;

    std::string memPath = "/proc/" + std::to_string(selfPid_) + "/mem";
    int fd = open(memPath.c_str(), O_WRONLY);
    if (fd < 0) return false;

    ssize_t n = pwrite(fd, data.data(), data.size(), addr);
    close(fd);
    return n == static_cast<ssize_t>(data.size());
}

PatchResult KittyPatch::patch(uintptr_t addr, const std::vector<uint8_t>& patchBytes) {
    PatchResult result;

    // Save original bytes
    auto original = read(addr, patchBytes.size());
    if (original.empty()) {
        result.message = "Failed to read original bytes";
        return result;
    }

    // Apply patch
    if (!write(addr, patchBytes)) {
        result.message = "Failed to write patch";
        return result;
    }

    result.success = true;
    result.originalBytes = original;
    patches_.push_back({addr, original});
    return result;
}

bool KittyPatch::restore(uintptr_t addr) {
    for (auto it = patches_.begin(); it != patches_.end(); ++it) {
        if (it->addr == addr) {
            bool ok = write(addr, it->original);
            patches_.erase(it);
            return ok;
        }
    }
    return false;
}

uintptr_t KittyPatch::scan(uintptr_t base, size_t size,
                            const std::vector<uint8_t>& pattern) const {
    if (pattern.empty() || size < pattern.size()) return 0;

    auto mem = read(base, size);
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

uintptr_t KittyPatch::getModuleBase(const std::string& moduleName) const {
    std::string mapsPath = "/proc/" + std::to_string(selfPid_) + "/maps";
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

size_t KittyPatch::getModuleSize(const std::string& moduleName) const {
    std::string mapsPath = "/proc/" + std::to_string(selfPid_) + "/maps";
    std::ifstream maps(mapsPath);
    if (!maps.is_open()) return 0;

    std::string line;
    while (std::getline(maps, line)) {
        if (line.find(moduleName) != std::string::npos) {
            size_t dash = line.find('-');
            size_t space = line.find(' ', dash);
            if (dash != std::string::npos && space != std::string::npos) {
                uintptr_t start = std::stoull(line.substr(0, dash), nullptr, 16);
                uintptr_t end = std::stoull(line.substr(dash + 1, space - dash - 1), nullptr, 16);
                return end - start;
            }
        }
    }
    return 0;
}

}  // namespace omnibyte::memory

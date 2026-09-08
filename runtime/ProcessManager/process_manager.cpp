// ProcessManager — attach/detach, /proc/<pid>/maps parsing, process discovery.
// Uses /proc filesystem on Android for process inspection.

#include "ProcessManager.h"

#include <chrono>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <unistd.h>

namespace omnibyte::runtime {

using DR = omnibyte::dumper::DumpResult;

DR ProcessManager::attach(pid_t pid, uint32_t timeoutMs, uint32_t maxRetries) {
    if (attached_) return DR::InvalidRequest;

    std::string statusPath = "/proc/" + std::to_string(pid) + "/status";

    for (uint32_t attempt = 0; attempt < maxRetries; ++attempt) {
        // Check /proc/<pid>/status exists
        std::ifstream status(statusPath);
        if (!status.is_open()) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(timeoutMs / maxRetries));
            continue;
        }

        // Check not zombie (State: Z or z)
        std::string line;
        while (std::getline(status, line)) {
            if (line.find("State:") == 0) {
                if (line.find('Z') != std::string::npos ||
                    line.find('z') != std::string::npos) {
                    break; // zombie — retry
                }
                // Process alive and not zombie
                attached_ = true;
                attachedPid_ = pid;
                return DR::Success;
            }
        }
    }

    return DR::InvalidRequest;
}

void ProcessManager::detach() {
    attached_ = false;
    attachedPid_ = 0;
}

std::vector<MemoryRegion> ProcessManager::getMemoryMaps(pid_t pid) {
    std::vector<MemoryRegion> regions;
    std::string mapsPath = "/proc/" + std::to_string(pid) + "/maps";

    std::ifstream maps(mapsPath);
    if (!maps.is_open()) return regions;

    std::string line;
    while (std::getline(maps, line)) {
        // Format: start-end perms offset dev inode pathname
        // Example: 7f8a000000-7f8a001000 r-xp 00000000 fd:00 12345 /data/app/.../libil2cpp.so
        MemoryRegion region;

        size_t dash = line.find('-');
        size_t space = line.find(' ', dash);
        if (dash == std::string::npos || space == std::string::npos) continue;

        region.start = std::stoull(line.substr(0, dash), nullptr, 16);
        region.end = std::stoull(line.substr(dash + 1, space - dash - 1), nullptr, 16);

        size_t permsEnd = line.find(' ', space + 1);
        if (permsEnd != std::string::npos) {
            region.perms = line.substr(space + 1, permsEnd - space - 1);
        }

        // Find pathname (last field after spaces)
        size_t lastSpace = line.rfind(' ');
        if (lastSpace != std::string::npos && lastSpace > permsEnd) {
            region.pathname = line.substr(lastSpace + 1);
        }

        regions.push_back(std::move(region));
    }

    return regions;
}

std::optional<pid_t> ProcessManager::findPidByPackageName(const std::string& packageName) {
    auto instances = findAllInstancesByPackageName(packageName);
    if (instances.empty()) return std::nullopt;
    return instances.front().pid;
}

std::vector<ProcessInstance> ProcessManager::findAllInstancesByPackageName(
        const std::string& packageName) {
    std::vector<ProcessInstance> result;

    DIR* procDir = opendir("/proc");
    if (!procDir) return result;

    struct dirent* entry;
    while ((entry = readdir(procDir)) != nullptr) {
        char* end = nullptr;
        long pid = strtol(entry->d_name, &end, 10);
        if (end == entry->d_name || *end != '\0' || pid <= 0) continue;

        // cmdline uses null-byte separators — read only to first null
        std::string cmdlinePath = "/proc/" + std::to_string(pid) + "/cmdline";
        int fd = open(cmdlinePath.c_str(), O_RDONLY);
        if (fd < 0) continue;

        char buf[512] = {};
        ssize_t n = read(fd, buf, sizeof(buf) - 1);
        close(fd);
        if (n <= 0) continue;

        std::string cmdline(buf, static_cast<size_t>(n));
        if (cmdline.find(packageName) == std::string::npos) continue;

        ProcessInstance inst;
        inst.pid = static_cast<pid_t>(pid);

        auto uid = readUid(inst.pid);
        if (uid) {
            inst.uid = *uid;
            inst.userProfileId = *uid / 100000;
        }

        result.push_back(inst);
    }

    closedir(procDir);
    return result;
}

std::optional<uid_t> ProcessManager::readUid(pid_t pid) const {
    std::string statusPath = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream f(statusPath);
    if (!f.is_open()) return std::nullopt;

    std::string line;
    while (std::getline(f, line)) {
        if (line.rfind("Uid:", 0) == 0) {
            // Format: "Uid:\t10086\t10086\t10086\t10086"
            size_t tab = line.find('\t');
            if (tab == std::string::npos) continue;
            size_t tab2 = line.find('\t', tab + 1);
            std::string uidStr = (tab2 != std::string::npos)
                ? line.substr(tab + 1, tab2 - tab - 1)
                : line.substr(tab + 1);
            try {
                return static_cast<uid_t>(std::stoul(uidStr));
            } catch (...) {
                return std::nullopt;
            }
        }
    }
    return std::nullopt;
}

bool ProcessManager::isAttached() const {
    return attached_;
}

} // namespace omnibyte::runtime

// Diamorphine LKM rootkit reference — userspace detection patterns.
// Source: https://github.com/m0nad/Diamorphine
// Commit: main branch, 2026-09-01
// Adapted from: process hiding (getdents hook), file hiding, module hiding,
//               syscall table hooking detection.
// License: GPL-2.0

#include "Diamorphine.h"

#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <set>
#include <sstream>
#include <unistd.h>

namespace omnibyte::evasion {

// ---------------------------------------------------------------------------
// Process hiding detection
// ---------------------------------------------------------------------------

bool DiamorphineReference::isPidHidden(int pid) const {
    // Diamorphine hides PIDs by hooking getdents64 syscall.
    // Detection: compare /proc PIDs vs /proc/<pid> existence.
    auto visible = getVisiblePids();
    for (int p : visible) {
        if (p == pid) return false;
    }
    // Check if /proc/<pid> exists but not in listing
    std::string procPath = "/proc/" + std::to_string(pid);
    return access(procPath.c_str(), F_OK) == 0;
}

std::vector<int> DiamorphineReference::getVisiblePids() const {
    std::vector<int> pids;
    DIR* dir = opendir("/proc");
    if (!dir) return pids;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            char* end = nullptr;
            long pid = strtol(entry->d_name, &end, 10);
            if (end != entry->d_name && *end == '\0' && pid > 0) {
                pids.push_back(static_cast<int>(pid));
            }
        }
    }
    closedir(dir);
    std::sort(pids.begin(), pids.end());
    return pids;
}

// ---------------------------------------------------------------------------
// File hiding detection
// ---------------------------------------------------------------------------

bool DiamorphineReference::isFileHidden(const std::string& path) const {
    // Diamorphine hides files by hooking getdents/getdents64.
    // Detection: stat() the file and check if directory listing matches.
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;  // file doesn't exist

    // Extract directory and filename
    size_t lastSlash = path.find_last_of('/');
    if (lastSlash == std::string::npos) return false;
    std::string dirPath = path.substr(0, lastSlash);
    std::string fileName = path.substr(lastSlash + 1);

    // Check if filename appears in directory listing
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) return false;

    bool found = false;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (std::string(entry->d_name) == fileName) {
            found = true;
            break;
        }
    }
    closedir(dir);
    return !found;  // hidden if stat succeeds but listing doesn't show it
}

// ---------------------------------------------------------------------------
// Module hiding detection
// ---------------------------------------------------------------------------

bool DiamorphineReference::isModuleVisible(const std::string& moduleName) const {
    auto modules = getLoadedModules();
    return std::find(modules.begin(), modules.end(), moduleName) != modules.end();
}

std::vector<std::string> DiamorphineReference::getLoadedModules() const {
    std::vector<std::string> modules;
    std::ifstream f("/proc/modules");
    if (!f.is_open()) return modules;

    std::string line;
    while (std::getline(f, line)) {
        size_t space = line.find(' ');
        if (space != std::string::npos) {
            modules.push_back(line.substr(0, space));
        }
    }
    return modules;
}

// ---------------------------------------------------------------------------
// Syscall hook detection
// ---------------------------------------------------------------------------

bool DiamorphineReference::detectSyscallHook() const {
    // Diamorphine hooks the syscall table to intercept getdents64, kill, etc.
    // Detection: compare syscall addresses in /proc/kallsyms.
    // This is a reference — full implementation requires kernel access.
    std::set<std::string> knownHooks = {
        "__x64_sys_getdents64",
        "sys_getdents64",
        "sys_kill",
        "__x64_sys_kill",
    };
    // In production: read /proc/kallsyms and verify addresses.
    return false;
}

// ---------------------------------------------------------------------------
// Comprehensive scan
// ---------------------------------------------------------------------------

std::vector<HideResult> DiamorphineReference::scanForRootkit() const {
    std::vector<HideResult> results;

    // Check for hidden processes
    auto pids = getVisiblePids();
    for (int pid : pids) {
        std::string/mapsPath = "/proc/" + std::to_string(pid) + "/maps";
        std::ifstream maps(mapsPath);
        if (!maps.is_open()) {
            results.push_back({true, "PID " + std::to_string(pid) + " has hidden maps"});
        }
    }

    // Check for hidden modules
    auto modules = getLoadedModules();
    if (modules.empty()) {
        results.push_back({true, "No modules visible — possible module hiding"});
    }

    return results;
}

}  // namespace omnibyte::evasion

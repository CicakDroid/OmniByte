#pragma once
// HideProcess — High-level abstraction for hiding OmniByte processes.
// Uses DiamorphineAdapter (LKM) as primary, falls back to BypasserAdapter (userspace).
// Hides: Hydra, modules, runtime, and any spawned OmniByte processes.

#include "IStealthBackend.h"
#include "Diamorphine/DiamorphineAdapter.h"

#include <string>
#include <vector>
#include <memory>

namespace omnibyte::runtime::backends {

/// Process hiding manager for OmniByte components.
/// Coordinates Diamorphine (kernel) and Bypasser (userspace) backends.
class HideProcess {
public:
    HideProcess() = default;
    ~HideProcess() = default;

    /// Initialize with Diamorphine backend (kernel-level hiding).
    bool init(std::shared_ptr<DiamorphineAdapter> diamorphine);

    /// Hide a specific PID from /proc enumeration.
    bool hide(pid_t pid);

    /// Unhide a previously hidden PID.
    bool unhide(pid_t pid);

    /// Hide all OmniByte component processes by name.
    /// Searches /proc for matching process names and hides them.
    bool hideOmniByteProcesses();

    /// Unhide all previously hidden OmniByte processes.
    bool unhideAll();

    /// Get list of currently hidden PIDs.
    std::vector<pid_t> getHiddenPids() const;

    /// Check if hiding is active.
    bool isActive() const { return active_; }

    /// Check if Diamorphine backend is available.
    bool isDiamorphineAvailable() const;

private:
    bool active_ = false;
    std::shared_ptr<DiamorphineAdapter> diamorphine_;
    std::vector<pid_t> hiddenPids_;

    /// Process names to hide (OmniByte components).
    static constexpr const char* kOmniByteProcessNames[] = {
        "omnibyte",      // Main app
        "hydra",         // Analysis engine
        "hpt",           // Hooking orchestrator
        "dumper",        // Universal dumper
        "freedom",       // Root service
        nullptr
    };

    /// Find PIDs by process name from /proc.
    std::vector<pid_t> findPidsByName(const char* name) const;
};

} // namespace omnibyte::runtime::backends

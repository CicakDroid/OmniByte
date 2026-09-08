#pragma once
// ZigZagManager — select and activate the best stealth backend.
// Priority driven by cfg.stealthBackendPriority (default: Diamorphine → Bypasser).

#include "ZigZag/ZigZag.h"
#include "config/Runtime/RuntimeConfig.h"
#include <memory>
#include <vector>
#include <string>

namespace omnibyte::runtime {

using omnibyte::dumper::config::RuntimeConfig;

class ZigZagManager {
public:
    ZigZagManager() = default;
    ~ZigZagManager() = default;

    /// Select best available backend per cfg.stealthBackendPriority and hide pid.
    /// Returns DumpResult::Success or StealthUnavailable.
    DumpResult selectAndActivate(pid_t pid,
                                 const RuntimeConfig& cfg);

    /// Unhide the tracked pid and release the active backend.
    void deactivate();

    /// Get the active stealth backend (or nullptr).
    IStealthBackend* activeBackend() const;

private:
    std::unique_ptr<ZigZag> active_;
    pid_t pid_ = 0;
};

} // namespace omnibyte::runtime

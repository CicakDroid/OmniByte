#pragma once
// ZigZagManager — select and activate the best stealth backend.
// Diamorphine first → fallback to Bypasser → StealthUnavailable if both fail.

#include "ZigZag/ZigZag.h"
#include "config/Runtime/RuntimeConfig.h"
#include <memory>

namespace omnibyte::runtime {

using omnibyte::dumper::config::RuntimeConfig;

class ZigZagManager {
public:
    ZigZagManager() = default;
    ~ZigZagManager() = default;

    /// Select best available backend and activate stealth for target process.
    /// Returns DumpResult::Success or StealthUnavailable.
    DumpResult selectAndActivate(pid_t pid,
                                 const RuntimeConfig& cfg);

    /// Deactivate the current stealth backend (unhide + release).
    void deactivate();

    /// Get the active stealth backend (or nullptr).
    IStealthBackend* activeBackend() const;

private:
    std::unique_ptr<ZigZag> active_;
};

} // namespace omnibyte::runtime

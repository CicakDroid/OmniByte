// ZigZagManager — select and activate the best stealth backend.
// Priority: Diamorphine → Bypasser → StealthUnavailable.

#include "ZigZagManager.h"
#include "modules/Dumper/Dumper.h"

#include "ZigZag/backends/DiamorphineAdapter.h"
#include "ZigZag/backends/BypasserAdapter.h"

namespace omnibyte::runtime {

DumpResult ZigZagManager::selectAndActivate(pid_t pid,
                                             const RuntimeConfig& cfg) {
    // Try Diamorphine first (LKM-based, stronger but riskier)
    auto diamorphine = std::make_shared<backends::DiamorphineAdapter>();
    if (diamorphine->isAvailable()) {
        active_ = std::make_unique<ZigZag>(diamorphine);
        if (active_->hide(pid)) {
            return omnibyte::dumper::DumpResult::Success;
        }
    }

    // Fallback to Bypasser (userspace, lighter)
    auto bypasser = std::make_shared<backends::BypasserAdapter>();
    if (bypasser->isAvailable()) {
        active_ = std::make_unique<ZigZag>(bypasser);
        if (active_->hide(pid)) {
            return omnibyte::dumper::DumpResult::Success;
        }
    }

    // Both failed
    active_.reset();
    return omnibyte::dumper::DumpResult::StealthUnavailable;
}

void ZigZagManager::deactivate() {
    if (active_ && active_->isActive()) {
        // Note: we don't know the pid here — unhide requires pid.
        // The Runtime facade tracks pid and calls unhide before deactivate.
    }
    active_.reset();
}

IStealthBackend* ZigZagManager::activeBackend() const {
    return active_ ? active_->backend() : nullptr;
}

} // namespace omnibyte::runtime

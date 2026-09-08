// ZigZagManager — select and activate the best stealth backend.
// Iterates cfg.stealthBackendPriority; first available backend wins.

#include "ZigZagManager.h"
#include "modules/Dumper/Dumper.h"

#include "ZigZag/backends/DiamorphineAdapter.h"
#include "ZigZag/backends/BypasserAdapter.h"

namespace omnibyte::runtime {

static std::shared_ptr<IStealthBackend> createBackend(const std::string& name) {
    if (name == "Diamorphine") return std::make_shared<backends::DiamorphineAdapter>();
    if (name == "Bypasser")    return std::make_shared<backends::BypasserAdapter>();
    return nullptr;
}

DumpResult ZigZagManager::selectAndActivate(pid_t pid,
                                             const RuntimeConfig& cfg) {
    for (const auto& name : cfg.stealthBackendPriority) {
        auto backend = createBackend(name);
        if (!backend || !backend->isAvailable()) continue;

        active_ = std::make_unique<ZigZag>(backend);
        if (active_->hide(pid)) {
            pid_ = pid;
            return omnibyte::dumper::DumpResult::Success;
        }
        active_.reset();
    }

    pid_ = 0;
    return omnibyte::dumper::DumpResult::StealthUnavailable;
}

void ZigZagManager::deactivate() {
    if (active_ && active_->isActive() && pid_ > 0) {
        active_->unhide(pid_);
    }
    active_.reset();
    pid_ = 0;
}

IStealthBackend* ZigZagManager::activeBackend() const {
    return active_ ? active_->backend() : nullptr;
}

} // namespace omnibyte::runtime

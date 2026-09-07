// FreedomService — hybrid root backend orchestrator.
// Tries backends in priority order, caches successful backend.
// Source adapters:
//   KernelSU: https://github.com/tiann/KernelSU (GPL-2.0)
//   SukiSU-Ultra: https://github.com/ShirkNix/SukiSU_Ultra (GPL-2.0)
//   Sui: https://github.com/XiaoTong6666/Sui (GPL-3.0)
//   RootThread: https://github.com/MMRLApp/RootThread (GPL-3.0)

#include "FreedomService.h"

#include <algorithm>

namespace omnibyte::runtime {

bool FreedomService::acquire(const std::vector<std::string>& backendPriority) {
    // If already acquired, return true
    if (active_ && active_->hasRoot()) return true;

    // Try each backend in priority order
    for (const auto& name : backendPriority) {
        for (auto& backend : registered_) {
            if (backend->name() == name && backend->isAvailable()) {
                if (backend->hasRoot()) {
                    active_ = backend;
                    return true;
                }
            }
        }
    }

    return false;
}

bool FreedomService::hasRoot() const {
    return active_ && active_->hasRoot();
}

std::optional<std::string> FreedomService::readFilePrivileged(const std::string& path) {
    if (!active_) return std::nullopt;
    return active_->readFilePrivileged(path);
}

FreedomService::ExecResult FreedomService::execCommand(const std::string& cmd) {
    if (!active_) return {};
    auto r = active_->execCommand(cmd);
    ExecResult result;
    result.exitCode = r.exitCode;
    result.stdout = std::move(r.stdout);
    result.stderr = std::move(r.stderr);
    return result;
}

std::string FreedomService::activeBackendName() const {
    return active_ ? active_->name() : "";
}

void FreedomService::release() {
    active_.reset();
}

void FreedomService::registerBackend(std::shared_ptr<IFreedomBackend> backend) {
    registered_.push_back(std::move(backend));
}

} // namespace omnibyte::runtime

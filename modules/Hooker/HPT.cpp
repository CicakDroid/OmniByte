// HPT — Hooking Platform Toolkit orchestrator.
// Selects backend per hookBackendPriority, caches the first working one.

#include "HPT.h"
#include "modules/Dumper/Dumper.h"

#include "backends/AlbatrossAdapter.h"
#include "backends/BhookAdapter.h"
#include "backends/InlinehookAdapter.h"
#include "backends/VectorAdapter.h"
#include "backends/KittyMemoryAdapter.h"
#include "backends/KittyMemoryExAdapter.h"

namespace omnibyte::runtime {

omnibyte::dumper::DumpResult HPT::selectBackend(
    const std::vector<std::string>& hookBackendPriority) {
    if (active_) return omnibyte::dumper::DumpResult::Success;

    for (const auto& name : hookBackendPriority) {
        for (auto& backend : registered_) {
            if (backend->name() == name && backend->isAvailable()) {
                active_ = backend;
                return omnibyte::dumper::DumpResult::Success;
            }
        }
    }

    return omnibyte::dumper::DumpResult::HookFailed;
}

bool HPT::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    return active_ && active_->hookFunction(addr, replacement, originalOut);
}

bool HPT::unhook(uintptr_t addr) {
    return active_ && active_->unhook(addr);
}

bool HPT::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    return active_ && active_->patchMemory(addr, data, size);
}

std::string HPT::activeBackendName() const {
    return active_ ? active_->name() : "";
}

void HPT::release() {
    active_.reset();
}

void HPT::registerBackend(std::shared_ptr<IHookBackend> backend) {
    registered_.push_back(std::move(backend));
}

} // namespace omnibyte::runtime

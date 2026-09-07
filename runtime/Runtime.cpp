#include "Runtime.h"

namespace omnibyte::runtime {

using DR = omnibyte::dumper::DumpResult;

DR Runtime::attach(pid_t pid, const config::RuntimeConfig& cfg) {
    if (isAttached()) return DR::InvalidRequest;

    cfg_ = cfg;
    pid_ = pid;

    DR pmResult = processMgr_.attach(pid, cfg.attachTimeoutMs, cfg.maxAttachRetries);
    if (pmResult != DR::Success) return pmResult;

    if (cfg.requireRoot) {
        if (!freedomService_.acquire(cfg.rootBackendPriority)) {
            processMgr_.detach();
            return DR::RootUnavailable;
        }
    }

    hpt_.selectBackend(cfg.hookBackendPriority);

    return DR::Success;
}

void Runtime::detach() {
    if (!isAttached()) return;

    hpt_.release();

    if (zigZagManager_.activeBackend()) {
        zigZagManager_.deactivate();
    }

    processMgr_.detach();
    symbolResolver_.clearCache();
    pid_ = 0;
}

std::optional<std::vector<uint8_t>> Runtime::readMemory(uintptr_t address, size_t size) {
    if (!isAttached()) return std::nullopt;

    if (cfg_.stealthReadStrategy == "syscall_proxy") {
        return memoryIO_.readViaProxy(pid_, address, size);
    }
    return memoryIO_.readChunk(pid_, address, size, cfg_.memReadChunkSizeBytes);
}

std::optional<uintptr_t> Runtime::resolveSymbol(const std::string& libName,
                                                  const std::string& symbolName) {
    if (!isAttached()) return std::nullopt;
    return symbolResolver_.resolveSymbol(pid_, libName, symbolName);
}

std::optional<std::vector<uint8_t>> Runtime::readFilePrivileged(
        const std::filesystem::path& path) {
    auto content = freedomService_.readFilePrivileged(path.string());
    if (!content) return std::nullopt;
    std::vector<uint8_t> bytes(content->begin(), content->end());
    return bytes;
}

DR Runtime::activateStealth(pid_t pid) {
    return zigZagManager_.selectAndActivate(pid, cfg_);
}

DR Runtime::installHook(uintptr_t addr, void* replacement, void** originalOut) {
    if (!isAttached()) return DR::InvalidRequest;
    if (!hpt_.hookFunction(addr, replacement, originalOut)) {
        return DR::HookFailed;
    }
    return DR::Success;
}

bool Runtime::isAttached() const {
    return processMgr_.isAttached();
}

bool Runtime::hasRoot() const {
    return freedomService_.hasRoot();
}

bool Runtime::isStealthActive() const {
    return zigZagManager_.activeBackend() != nullptr;
}

} // namespace omnibyte::runtime

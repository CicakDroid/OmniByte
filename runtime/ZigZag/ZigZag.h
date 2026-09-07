#pragma once
// ZigZag — thin wrapper exposing the active stealth backend.
// Backend selection is NOT ZigZag's responsibility — that's ZigZagManager.

#include "IStealthBackend.h"
#include <memory>

namespace omnibyte::runtime {

class ZigZag {
public:
    explicit ZigZag(std::shared_ptr<IStealthBackend> backend);
    ~ZigZag() = default;

    IStealthBackend* backend() const { return backend_.get(); }
    bool isActive() const { return backend_ != nullptr; }

    bool hide(pid_t pid) { return backend_ && backend_->hide(pid); }
    bool unhide(pid_t pid) { return backend_ && backend_->unhide(pid); }
    bool bypassPtraceScope() { return backend_ && backend_->bypassPtraceScope(); }
    bool bypassSelinuxDenial() { return backend_ && backend_->bypassSelinuxDenial(); }

private:
    std::shared_ptr<IStealthBackend> backend_;
};

} // namespace omnibyte::runtime

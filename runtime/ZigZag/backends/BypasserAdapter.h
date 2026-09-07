#pragma once
// Bypasser adapter — IStealthBackend for userspace stealth techniques.
// Source: https://github.com/LRFP-Team/Bypasser (MIT)
//
// Lighter-weight than Diamorphine: uses userspace techniques (file descriptor
// manipulation, /proc/self/clear_refs, process name changes).
// Fallback when LKM loading is not possible.

#include "../IStealthBackend.h"
#include <string>

namespace omnibyte::runtime::backends {

class BypasserAdapter : public IStealthBackend {
public:
    std::string name() const override { return "Bypasser"; }

    bool isAvailable() const override;
    bool hide(pid_t pid) override;
    bool unhide(pid_t pid) override;
    bool bypassPtraceScope() override;
    bool bypassSelinuxDenial() override;
};

} // namespace omnibyte::runtime::backends

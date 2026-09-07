#pragma once
// Diamorphine adapter — IStealthBackend for LKM-based process/module hiding.
// Source: https://github.com/m0nad/Diamorphine (GPL-2.0)
//
// Uses Diamorphine's getdents64 hook to hide processes from /proc.
// WARNING: Requires kernel module loading capability.
// TODO: Check kernel version compatibility before loading module — risk of bootloop.

#include "../IStealthBackend.h"
#include <string>

namespace omnibyte::runtime::backends {

class DiamorphineAdapter : public IStealthBackend {
public:
    std::string name() const override { return "Diamorphine"; }

    bool isAvailable() const override;
    bool hide(pid_t pid) override;
    bool unhide(pid_t pid) override;
    bool bypassPtraceScope() override;
    bool bypassSelinuxDenial() override;

private:
    /// Check if kernel supports module loading and Diamorphine is compatible.
    bool probeKernelCompat() const;
};

} // namespace omnibyte::runtime::backends

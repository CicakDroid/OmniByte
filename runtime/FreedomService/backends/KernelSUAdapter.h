#pragma once
// KernelSU adapter — IFreedomBackend for KernelSU root solution.
// Source: https://github.com/tiann/KernelSU (GPL-2.0)
//
// Detection: /data/adb/ksu exists + /dev/kernelsu character device accessible.
// Root via: KernelSU's built-in su binary or IPC.

#include "../IFreedomBackend.h"
#include <string>
#include <optional>

namespace omnibyte::runtime::backends {

class KernelSUAdapter : public IFreedomBackend {
public:
    std::string name() const override { return "KernelSU"; }

    bool isAvailable() const override;
    bool hasRoot() const override;
    std::optional<std::string> readFilePrivileged(const std::string& path) override;
    ExecResult execCommand(const std::string& cmd) override;

private:
    /// Check KernelSU presence via /data/adb/ksu marker + /dev/kernelsu ioctl.
    bool probeKernelSU() const;

    mutable bool cached_ = false;
    mutable bool available_ = false;
};

} // namespace omnibyte::runtime::backends

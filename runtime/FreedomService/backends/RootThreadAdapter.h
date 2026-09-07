#pragma once
// RootThread adapter — IFreedomBackend for classic su-based root.
// Source: https://github.com/MMRLApp/RootThread (GPL-3.0)
//
// Fallback backend: uses `su -c <cmd>` via popen.
// Works with any root solution that provides a su binary (Magisk, APatch, etc.).

#include "../IFreedomBackend.h"
#include <string>
#include <optional>

namespace omnibyte::runtime::backends {

class RootThreadAdapter : public IFreedomBackend {
public:
    std::string name() const override { return "RootThread"; }

    bool isAvailable() const override;
    bool hasRoot() const override;
    std::optional<std::string> readFilePrivileged(const std::string& path) override;
    ExecResult execCommand(const std::string& cmd) override;

private:
    /// Check if su binary is accessible.
    bool probeSu() const;

    mutable bool cached_ = false;
    mutable bool available_ = false;
};

} // namespace omnibyte::runtime::backends

#pragma once
// SukiSU-Ultra adapter — IFreedomBackend for SukiSU-Ultra root solution.
// Source: https://github.com/ShirkNix/SukiSU_Ultra (GPL-2.0)
//
// Detection: SukiSU-Ultra uses KernelSU-compatible interface with additional
// stealth features. Must check SukiSU-specific markers BEFORE falling back
// to generic KernelSU detection to avoid misidentification.

#include "../IFreedomBackend.h"
#include <string>
#include <optional>

namespace omnibyte::runtime::backends {

class SukiSUUltraAdapter : public IFreedomBackend {
public:
    std::string name() const override { return "SukiSU-Ultra"; }

    bool isAvailable() const override;
    bool hasRoot() const override;
    std::optional<std::string> readFilePrivileged(const std::string& path) override;
    ExecResult execCommand(const std::string& cmd) override;

private:
    /// Check SukiSU-Ultra specific markers (distinct from generic KernelSU).
    bool probeSukiSUUltra() const;

    mutable bool cached_ = false;
    mutable bool available_ = false;
};

} // namespace omnibyte::runtime::backends

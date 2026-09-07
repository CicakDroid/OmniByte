#pragma once
// Sui adapter — IFreedomBackend for Sui (Shizuku-based root management).
// Source: https://github.com/XiaoTong6666/Sui (GPL-3.0)
//
// Detection: Sui exposes root via binder/content provider (non-root API).
// Uses Shizuku IPC, NOT a su binary. Must not invoke `su` directly.

#include "../IFreedomBackend.h"
#include <string>
#include <optional>

namespace omnibyte::runtime::backends {

class SuiAdapter : public IFreedomBackend {
public:
    std::string name() const override { return "Sui"; }

    bool isAvailable() const override;
    bool hasRoot() const override;
    std::optional<std::string> readFilePrivileged(const std::string& path) override;
    ExecResult execCommand(const std::string& cmd) override;

private:
    /// Check Sui via Shizuku binder API (non-root detection path).
    bool probeSui() const;

    mutable bool cached_ = false;
    mutable bool available_ = false;
};

} // namespace omnibyte::runtime::backends

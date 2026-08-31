#pragma once
// Runtime/RuntimeConfig.h — configuration for runtime/process interaction.

#include <cstdint>
#include <string>

namespace omnibyte::dumper::config {

struct RuntimeConfig {
    // Whether root/sudo privileges are required for process attachment.
    bool requireRoot = false;

    // Timeout in milliseconds for attaching to a live process.
    uint32_t attachTimeoutMs = 5000;

    // Strategy for selecting a PID when multiple processes match.
    enum class PidSelectionPolicy {
        FirstMatch,    // pick the first matching PID
        LargestModule, // pick the process whose target module is largest in memory
        UserPrompt     // show candidates to user and let them choose
    };

    PidSelectionPolicy pidSelectionPolicy = PidSelectionPolicy::UserPrompt;

    static RuntimeConfig defaults() { return {}; }
};

} // namespace omnibyte::dumper::config

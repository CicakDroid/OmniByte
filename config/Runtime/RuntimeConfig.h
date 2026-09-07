#pragma once
// Runtime/RuntimeConfig.h — configuration for runtime/process interaction.

#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

struct RuntimeConfig {
    bool requireRoot = false;
    uint32_t attachTimeoutMs = 5000;

    enum class PidSelectionPolicy {
        FirstMatch,    // pick the first matching PID
        LargestModule, // pick the process whose target module is largest in memory
        UserPrompt,    // show candidates to user and let them choose
        Foreground     // pick the foreground app's PID
    };

    PidSelectionPolicy pidSelectionPolicy = PidSelectionPolicy::Foreground;

    // Memory read strategy for stealth detection.
    std::string stealthReadStrategy = "direct_mem_read";

    // Whether to use root for process discovery by default.
    bool discoveryUseRootDefault = false;

    // Chunk size for reading process memory in bytes.
    uint32_t memReadChunkSizeBytes = 65536;  // 64 KB

    // Maximum number of retries when attaching to a process.
    uint32_t maxAttachRetries = 3;

    static RuntimeConfig defaults() { return {}; }

    static RuntimeConfig fromJson(const nlohmann::json& j) {
        RuntimeConfig cfg;
        cfg.requireRoot     = omnibyte::common::getOr<bool>(j, "requireRoot", cfg.requireRoot);
        cfg.attachTimeoutMs = omnibyte::common::getOr<uint32_t>(j, "attachTimeoutMs", cfg.attachTimeoutMs);
        cfg.stealthReadStrategy     = omnibyte::common::getOr<std::string>(j, "stealthReadStrategy", cfg.stealthReadStrategy);
        cfg.discoveryUseRootDefault  = omnibyte::common::getOr<bool>(j, "discoveryUseRootDefault", cfg.discoveryUseRootDefault);
        cfg.memReadChunkSizeBytes    = omnibyte::common::getOr<uint32_t>(j, "memReadChunkSizeBytes", cfg.memReadChunkSizeBytes);
        cfg.maxAttachRetries         = omnibyte::common::getOr<uint32_t>(j, "maxAttachRetries", cfg.maxAttachRetries);

        if (j.contains("pidSelectionPolicy") && j.at("pidSelectionPolicy").is_string()) {
            std::string pol = j.at("pidSelectionPolicy").get<std::string>();
            if (pol == "FirstMatch")         cfg.pidSelectionPolicy = PidSelectionPolicy::FirstMatch;
            else if (pol == "LargestModule") cfg.pidSelectionPolicy = PidSelectionPolicy::LargestModule;
            else if (pol == "UserPrompt")    cfg.pidSelectionPolicy = PidSelectionPolicy::UserPrompt;
            else if (pol == "Foreground")    cfg.pidSelectionPolicy = PidSelectionPolicy::Foreground;
            // unknown string → keep default
        }

        return cfg;
    }

    nlohmann::json toJson() const {
        std::string polStr;
        switch (pidSelectionPolicy) {
            case PidSelectionPolicy::FirstMatch:    polStr = "FirstMatch"; break;
            case PidSelectionPolicy::LargestModule: polStr = "LargestModule"; break;
            case PidSelectionPolicy::UserPrompt:    polStr = "UserPrompt"; break;
            case PidSelectionPolicy::Foreground:    polStr = "Foreground"; break;
        }
        return {
            {"requireRoot",              requireRoot},
            {"attachTimeoutMs",          attachTimeoutMs},
            {"pidSelectionPolicy",       polStr},
            {"stealthReadStrategy",      stealthReadStrategy},
            {"discoveryUseRootDefault",  discoveryUseRootDefault},
            {"memReadChunkSizeBytes",    memReadChunkSizeBytes},
            {"maxAttachRetries",         maxAttachRetries}
        };
    }
};

} // namespace omnibyte::dumper::config

#pragma once
// Runtime/RuntimeLoader.h — load RuntimeConfig from JSON.

#include "RuntimeConfig.h"
#include <common/Serialization/JsonLoader.h>
#include <unordered_map>

namespace omnibyte::dumper::config {

namespace {
    const std::unordered_map<std::string, RuntimeConfig::PidSelectionPolicy>
        kPolicyMap = {
            {"FirstMatch",    RuntimeConfig::PidSelectionPolicy::FirstMatch},
            {"LargestModule", RuntimeConfig::PidSelectionPolicy::LargestModule},
            {"UserPrompt",    RuntimeConfig::PidSelectionPolicy::UserPrompt},
        };
} // anonymous namespace

inline RuntimeConfig loadRuntimeConfig(const std::string& path) {
    auto j = omnibyte::common::loadJsonFile(path);
    if (!j) return RuntimeConfig::defaults();

    RuntimeConfig cfg;
    cfg.requireRoot      = omnibyte::common::getOr<bool>(*j, "requireRoot", cfg.requireRoot);
    cfg.attachTimeoutMs  = omnibyte::common::getOr<uint32_t>(*j, "attachTimeoutMs", cfg.attachTimeoutMs);

    if (j->contains("pidSelectionPolicy") && j->at("pidSelectionPolicy").is_string()) {
        std::string pol = j->at("pidSelectionPolicy").get<std::string>();
        auto it = kPolicyMap.find(pol);
        if (it != kPolicyMap.end()) {
            cfg.pidSelectionPolicy = it->second;
        }
    }

    return cfg;
}

} // namespace omnibyte::dumper::config

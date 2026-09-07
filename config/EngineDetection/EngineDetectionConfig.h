#pragma once
// EngineDetection/EngineDetectionConfig.h — configuration for engine auto-detection.

#include <optional>
#include <string>
#include <nlohmann/json.hpp>
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

struct EngineDetectionConfig {
    // Minimum confidence (0.0–1.0) for a detection to be considered valid.
    float confidenceThreshold = 0.75f;

    // Force a specific engine, bypassing auto-detection.
    // std::nullopt = auto-detect (normal behavior).
    std::optional<std::string> manualEngineOverride;

    static EngineDetectionConfig defaults() { return {}; }

    static EngineDetectionConfig fromJson(const nlohmann::json& j) {
        EngineDetectionConfig cfg;
        cfg.confidenceThreshold = omnibyte::common::getOr<float>(j, "confidenceThreshold", cfg.confidenceThreshold);

        if (j.contains("manualEngineOverride") && j.at("manualEngineOverride").is_string()) {
            cfg.manualEngineOverride = j.at("manualEngineOverride").get<std::string>();
        }
        // If key is missing or not a string, manualEngineOverride stays nullopt (auto-detect).

        return cfg;
    }

    nlohmann::json toJson() const {
        nlohmann::json j;
        j["confidenceThreshold"] = confidenceThreshold;
        if (manualEngineOverride.has_value()) {
            j["manualEngineOverride"] = *manualEngineOverride;
        } else {
            j["manualEngineOverride"] = nullptr;
        }
        return j;
    }
};

} // namespace omnibyte::dumper::config

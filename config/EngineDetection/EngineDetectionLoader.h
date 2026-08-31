#pragma once
// EngineDetection/EngineDetectionLoader.h — load EngineDetectionConfig from JSON.

#include "EngineDetectionConfig.h"
#include "../Common/ConfigLoader.h"

namespace omnibyte::dumper::config {

inline EngineDetectionConfig loadEngineDetectionConfig(const std::string& path) {
    auto j = loadJsonFile(path);
    if (!j) return EngineDetectionConfig::defaults();

    EngineDetectionConfig cfg;
    cfg.confidenceThreshold = getOr<float>(*j, "confidenceThreshold", cfg.confidenceThreshold);

    if (j->contains("manualEngineOverride") && j->at("manualEngineOverride").is_string()) {
        cfg.manualEngineOverride = j->at("manualEngineOverride").get<std::string>();
    }
    // If key is missing or not a string, manualEngineOverride stays nullopt (auto-detect).

    return cfg;
}

} // namespace omnibyte::dumper::config

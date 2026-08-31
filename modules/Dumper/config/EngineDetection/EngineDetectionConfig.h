#pragma once
// EngineDetection/EngineDetectionConfig.h — configuration for engine auto-detection.

#include <optional>
#include <string>

namespace omnibyte::dumper::config {

struct EngineDetectionConfig {
    // Minimum confidence (0.0–1.0) for a detection to be considered valid.
    // Below this threshold, the engine is treated as "not detected".
    float confidenceThreshold = 0.6f;

    // Force a specific engine, bypassing auto-detection.
    // std::nullopt = auto-detect (normal behavior).
    // Set to a string like "UnrealEngine" to lock detection to that engine.
    std::optional<std::string> manualEngineOverride;

    static EngineDetectionConfig defaults() { return {}; }
};

} // namespace omnibyte::dumper::config

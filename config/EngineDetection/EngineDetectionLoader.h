#pragma once
// EngineDetection/EngineDetectionLoader.h — load EngineDetectionConfig from JSON.

#include "EngineDetectionConfig.h"
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

inline EngineDetectionConfig loadEngineDetectionConfig(const std::string& path) {
    auto j = omnibyte::common::loadJsonFile(path);
    if (!j) return EngineDetectionConfig::defaults();
    return EngineDetectionConfig::fromJson(*j);
}

} // namespace omnibyte::dumper::config

#pragma once
// Storage/StorageLoader.h — load StorageConfig from JSON.

#include "StorageConfig.h"
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

inline StorageConfig loadStorageConfig(const std::string& path) {
    auto j = omnibyte::common::loadJsonFile(path);
    if (!j) return StorageConfig::defaults();

    StorageConfig cfg;
    cfg.cacheDir         = omnibyte::common::getOr<std::string>(*j, "cacheDir", cfg.cacheDir);
    cfg.tempLifetimeMs   = omnibyte::common::getOr<uint64_t>(*j, "tempLifetimeMs", cfg.tempLifetimeMs);
    cfg.maxCacheSizeBytes = omnibyte::common::getOr<uint64_t>(*j, "maxCacheSizeBytes", cfg.maxCacheSizeBytes);
    return cfg;
}

} // namespace omnibyte::dumper::config

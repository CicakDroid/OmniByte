#pragma once
// Storage/StorageLoader.h — load StorageConfig from JSON.

#include "StorageConfig.h"
#include "../Common/ConfigLoader.h"

namespace omnibyte::dumper::config {

inline StorageConfig loadStorageConfig(const std::string& path) {
    auto j = loadJsonFile(path);
    if (!j) return StorageConfig::defaults();

    StorageConfig cfg;
    cfg.cacheDir         = getOr<std::string>(*j, "cacheDir", cfg.cacheDir);
    cfg.tempLifetimeMs   = getOr<uint64_t>(*j, "tempLifetimeMs", cfg.tempLifetimeMs);
    cfg.maxCacheSizeBytes = getOr<uint64_t>(*j, "maxCacheSizeBytes", cfg.maxCacheSizeBytes);
    return cfg;
}

} // namespace omnibyte::dumper::config

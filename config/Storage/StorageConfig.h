#pragma once
// Storage/StorageConfig.h — configuration for cache and temporary file storage.

#include <cstdint>
#include <string>
#include <nlohmann/json.hpp>
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

struct StorageConfig {
    std::string cacheDir = "cache/";
    uint64_t tempLifetimeMs = 86400000;             // 24 hours
    uint64_t maxCacheSizeBytes = 1ULL * 1024 * 1024 * 1024;  // 1 GB

    static StorageConfig defaults() { return {}; }

    static StorageConfig fromJson(const nlohmann::json& j) {
        StorageConfig cfg;
        cfg.cacheDir          = omnibyte::common::getOr<std::string>(j, "cacheDir", cfg.cacheDir);
        cfg.tempLifetimeMs    = omnibyte::common::getOr<uint64_t>(j, "tempLifetimeMs", cfg.tempLifetimeMs);
        cfg.maxCacheSizeBytes = omnibyte::common::getOr<uint64_t>(j, "maxCacheSizeBytes", cfg.maxCacheSizeBytes);
        return cfg;
    }

    nlohmann::json toJson() const {
        return {
            {"cacheDir",          cacheDir},
            {"tempLifetimeMs",    tempLifetimeMs},
            {"maxCacheSizeBytes", maxCacheSizeBytes}
        };
    }
};

} // namespace omnibyte::dumper::config

#pragma once
// Storage/StorageConfig.h — configuration for cache and temporary file storage.

#include <cstdint>
#include <string>

namespace omnibyte::dumper::config {

struct StorageConfig {
    // Directory for cached files (downloaded SDKs, extracted APKs, etc.)
    std::string cacheDir = "/tmp/omnibyte_cache";

    // Maximum lifetime of temporary files in milliseconds before auto-cleanup.
    // Default: 1 hour.
    uint64_t tempLifetimeMs = 3600000;

    // Maximum total cache size in bytes. When exceeded, oldest files are evicted.
    uint64_t maxCacheSizeBytes = 1ULL * 1024 * 1024 * 1024;  // 1 GB

    static StorageConfig defaults() { return {}; }
};

} // namespace omnibyte::dumper::config

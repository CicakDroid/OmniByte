#pragma once
// FileLimits/FileLimitsLoader.h — load FileLimitsConfig from JSON.

#include "FileLimitsConfig.h"
#include "../Common/ConfigLoader.h"

namespace omnibyte::dumper::config {

// Load FileLimitsConfig from a JSON file.
// If the file doesn't exist or is malformed, returns defaults().
inline FileLimitsConfig loadFileLimitsConfig(const std::string& path) {
    auto j = loadJsonFile(path);
    if (!j) return FileLimitsConfig::defaults();

    FileLimitsConfig cfg;
    cfg.maxFileSizeBytes  = getOr<uint64_t>(*j, "maxFileSizeBytes", cfg.maxFileSizeBytes);
    cfg.chunkSizeBytes    = getOr<uint64_t>(*j, "chunkSizeBytes", cfg.chunkSizeBytes);
    cfg.allowedExtensions = getOr<std::vector<std::string>>(*j, "allowedExtensions", cfg.allowedExtensions);
    cfg.exportSplitMaxLines = getOr<size_t>(*j, "exportSplitMaxLines", cfg.exportSplitMaxLines);
    return cfg;
}

} // namespace omnibyte::dumper::config

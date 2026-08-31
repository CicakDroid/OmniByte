#pragma once
// FileLimits/FileLimitsLoader.h — load FileLimitsConfig from JSON.

#include "FileLimitsConfig.h"
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

// Load FileLimitsConfig from a JSON file.
// If the file doesn't exist or is malformed, returns defaults().
inline FileLimitsConfig loadFileLimitsConfig(const std::string& path) {
    auto j = omnibyte::common::loadJsonFile(path);
    if (!j) return FileLimitsConfig::defaults();

    FileLimitsConfig cfg;
    cfg.maxFileSizeBytes  = omnibyte::common::getOr<uint64_t>(*j, "maxFileSizeBytes", cfg.maxFileSizeBytes);
    cfg.chunkSizeBytes    = omnibyte::common::getOr<uint64_t>(*j, "chunkSizeBytes", cfg.chunkSizeBytes);
    cfg.allowedExtensions = omnibyte::common::getOr<std::vector<std::string>>(*j, "allowedExtensions", cfg.allowedExtensions);
    cfg.exportSplitMaxLines = omnibyte::common::getOr<size_t>(*j, "exportSplitMaxLines", cfg.exportSplitMaxLines);
    return cfg;
}

} // namespace omnibyte::dumper::config

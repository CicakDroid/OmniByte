#pragma once
// FileLimits/FileLimitsConfig.h — configuration for file size limits and export splitting.

#include <cstdint>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <common/Serialization/JsonLoader.h>

namespace omnibyte::dumper::config {

struct FileLimitsConfig {
    uint64_t maxFileSizeBytes = 500ULL * 1024 * 1024;  // 500 MB
    uint64_t chunkSizeBytes = 4ULL * 1024 * 1024;      // 4 MB
    std::vector<std::string> allowedExtensions = {
        ".apk", ".so", ".dll", ".pak", ".win", ".pck", ".ung", ".bsp"
    };
    size_t exportSplitMaxLines = 100000;

    static FileLimitsConfig defaults() { return {}; }

    static FileLimitsConfig fromJson(const nlohmann::json& j) {
        FileLimitsConfig cfg;
        cfg.maxFileSizeBytes  = omnibyte::common::getOr<uint64_t>(j, "maxFileSizeBytes", cfg.maxFileSizeBytes);
        cfg.chunkSizeBytes    = omnibyte::common::getOr<uint64_t>(j, "chunkSizeBytes", cfg.chunkSizeBytes);
        cfg.allowedExtensions = omnibyte::common::getOr<std::vector<std::string>>(j, "allowedExtensions", cfg.allowedExtensions);
        cfg.exportSplitMaxLines = omnibyte::common::getOr<size_t>(j, "exportSplitMaxLines", cfg.exportSplitMaxLines);
        return cfg;
    }

    nlohmann::json toJson() const {
        return {
            {"maxFileSizeBytes",   maxFileSizeBytes},
            {"chunkSizeBytes",     chunkSizeBytes},
            {"allowedExtensions",  allowedExtensions},
            {"exportSplitMaxLines", exportSplitMaxLines}
        };
    }
};

} // namespace omnibyte::dumper::config

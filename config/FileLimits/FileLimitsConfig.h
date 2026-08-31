#pragma once
// FileLimits/FileLimitsConfig.h — configuration for file size limits and export splitting.

#include <cstdint>
#include <string>
#include <vector>

namespace omnibyte::dumper::config {

struct FileLimitsConfig {
    // Maximum file size in bytes that the dumper will attempt to process.
    // Files larger than this are rejected with an error.
    uint64_t maxFileSizeBytes = 500ULL * 1024 * 1024;  // 500 MB

    // Chunk size for streaming/processing large files in parts.
    uint64_t chunkSizeBytes = 4ULL * 1024 * 1024;  // 4 MB

    // File extensions allowed as valid dump targets.
    std::vector<std::string> allowedExtensions = {
        ".apk", ".so", ".dll", ".pak", ".win", ".pck", ".ung", ".bsp"
    };

    // Maximum number of lines per file when export splits output across files.
    size_t exportSplitMaxLines = 10000;

    // Factory: hardcoded defaults (used when no config file exists).
    static FileLimitsConfig defaults() { return {}; }
};

} // namespace omnibyte::dumper::config

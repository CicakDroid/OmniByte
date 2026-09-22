#pragma once
// Strings — String extraction and classification from raw binary data.
// Scans byte buffers for printable ASCII/UTF-8 sequences, then classifies
// each string (path, URL, format specifier, hex blob, numeric, identifier).
//
// Classification categories:
//   - path: /path/to/file, C:\windows\...
//   - url: http(s)://..., ftp://...
//   - format: %s, %d, %x, printf-style
//   - hex: 0xDEADBEEF, \x41\x42...
//   - numeric: integer or floating-point literals
//   - identifier: [a-zA-Z_][a-zA-Z0-9_]* (likely symbol/function names)
//   - unknown: none of the above

#include "IAnalysis.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

/// String extraction: scan binary for printable sequences and classify them.
///
/// Usage:
///   Strings analyzer;
///   auto result = analyzer.analyzeStrings(data, dataSize);
///   for (auto& [offset, str] : result.strings) { ... }
class Strings : public IAnalysis {
public:
    std::string name() const override { return "Strings"; }

    /// Extract and classify strings from binary data.
    StringsResult analyzeStrings(
        const uint8_t* data, size_t dataSize
    ) const override;

private:
    /// Scan for printable ASCII/UTF-8 sequences >= minLength bytes.
    std::vector<std::pair<uint64_t, std::string>> extractStrings(
        const uint8_t* data, size_t dataSize,
        size_t minLength = 4
    ) const;

    /// Classify a string into a category (path, url, format, etc.).
    std::string classifyString(const std::string& str) const;

    bool isFormatString(const std::string& str) const;
    bool isUrlString(const std::string& str) const;
    bool isHexString(const std::string& str) const;
};

} // namespace omnibyte::hydradis

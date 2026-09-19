#pragma once
// Json.h — JSON deserialization/serialization utilities using nlohmann/json.
//
// Provides:
//   - loadJsonFile: read JSON from file with error handling
//   - saveJsonFile: write JSON to file with formatting
//   - getOr<T>: extract value from JSON with default fallback
//   - mergeJson: deep merge two JSON objects

#include <nlohmann/json.hpp>
#include <filesystem>
#include <optional>
#include <string>

namespace omnibyte::common {

using json = nlohmann::json;

/// Load a JSON file from disk. Returns std::nullopt on error (file not found,
/// parse failure, permission denied).
std::optional<json> loadJsonFile(const std::filesystem::path& path);

/// Save a JSON object to disk with pretty-printing.
/// Returns true on success, false on write failure.
bool saveJsonFile(const std::filesystem::path& path, const json& data,
                  bool pretty = true);

/// Extract a value from JSON with a default fallback.
/// Works with any type nlohmann::json can convert to.
template <typename T>
T getOr(const json& j, const std::string& key, const T& defaultVal) {
    if (j.contains(key) && !j.at(key).is_null()) {
        try {
            return j.at(key).get<T>();
        } catch (const json::exception&) {
            return defaultVal;
        }
    }
    return defaultVal;
}

/// Deep merge two JSON objects. Values in `override` take precedence.
/// Returns the merged result.
json mergeJson(const json& base, const json& override);

} // namespace omnibyte::common

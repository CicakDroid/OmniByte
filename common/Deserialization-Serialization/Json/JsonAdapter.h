#pragma once
// JsonAdapter.h — higher-level adapter for JSON file I/O and config operations.
//
// Wraps Json.h utilities into a convenient adapter for config loading/saving,
// with optional version tracking and schema validation hooks.

#include "Json.h"
#include <filesystem>
#include <string>

namespace omnibyte::common {

/// Load a JSON config file, optionally merging with defaults.
/// Returns std::nullopt on error.
std::optional<json> loadConfig(const std::filesystem::path& path,
                               const json& defaults = json::object());

/// Save a JSON config file, optionally merging with an existing file.
/// If mergeWithExisting is true, reads the existing file first and deep-merges.
bool saveConfig(const std::filesystem::path& path, const json& data,
                bool mergeWithExisting = false);

/// Validate a JSON object against required keys (all must be present and non-null).
/// Returns a list of missing keys (empty if valid).
std::vector<std::string> validateRequired(const json& j,
                                          const std::vector<std::string>& requiredKeys);

} // namespace omnibyte::common

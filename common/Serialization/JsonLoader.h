#pragma once
// common/Serialization/JsonLoader.h — shared JSON file loading utilities.
//
// Usage:
//   auto j = omnibyte::common::loadJsonFile("file_limits.json");
//   if (j) { /* parse *j */ } else { /* use defaults */ }

#include <nlohmann/json.hpp>
#include <fstream>
#include <optional>
#include <string>

namespace omnibyte::common {

// Load a JSON file and return it as nlohmann::json.
// Returns std::nullopt if the file doesn't exist or can't be parsed.
inline std::optional<nlohmann::json> loadJsonFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }

    try {
        nlohmann::json j;
        file >> j;
        return j;
    } catch (const nlohmann::json::parse_error&) {
        return std::nullopt;
    }
}

// Safe get with default: if key missing or wrong type, return fallback.
template <typename T>
inline T getOr(const nlohmann::json& j, const std::string& key, T fallback) {
    if (!j.contains(key)) return fallback;
    try {
        return j.at(key).get<T>();
    } catch (const nlohmann::json::type_error&) {
        return fallback;
    }
}

// Specialization for vector<string>: JSON array → std::vector<std::string>
template <>
inline std::vector<std::string> getOr(const nlohmann::json& j,
                                       const std::string& key,
                                       std::vector<std::string> fallback) {
    if (!j.contains(key)) return fallback;
    if (!j.at(key).is_array()) return fallback;
    try {
        return j.at(key).get<std::vector<std::string>>();
    } catch (const nlohmann::json::type_error&) {
        return fallback;
    }
}

} // namespace omnibyte::common

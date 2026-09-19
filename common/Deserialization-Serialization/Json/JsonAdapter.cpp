// JsonAdapter.cpp — higher-level JSON config operations implementation.

#include "JsonAdapter.h"

namespace omnibyte::common {

std::optional<json> loadConfig(const std::filesystem::path& path,
                               const json& defaults) {
    auto loaded = loadJsonFile(path);
    if (!loaded) {
        if (defaults.is_null() || defaults.empty()) return std::nullopt;
        return defaults;
    }
    if (defaults.is_null() || defaults.empty()) return loaded;
    return mergeJson(defaults, *loaded);
}

bool saveConfig(const std::filesystem::path& path, const json& data,
                bool mergeWithExisting) {
    json toSave = data;
    if (mergeWithExisting) {
        auto existing = loadJsonFile(path);
        if (existing) {
            toSave = mergeJson(*existing, data);
        }
    }

    // Ensure parent directory exists
    auto parent = path.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    return saveJsonFile(path, toSave);
}

std::vector<std::string> validateRequired(const json& j,
                                          const std::vector<std::string>& requiredKeys) {
    std::vector<std::string> missing;
    for (const auto& key : requiredKeys) {
        if (!j.contains(key) || j.at(key).is_null()) {
            missing.push_back(key);
        }
    }
    return missing;
}

} // namespace omnibyte::common

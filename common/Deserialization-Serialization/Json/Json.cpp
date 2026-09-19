// Json.cpp — JSON deserialization/serialization implementation.

#include "Json.h"
#include <fstream>

namespace omnibyte::common {

std::optional<json> loadJsonFile(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) return std::nullopt;

    std::ifstream ifs(path);
    if (!ifs.is_open()) return std::nullopt;

    try {
        json j = json::parse(ifs);
        return j;
    } catch (const json::exception&) {
        return std::nullopt;
    }
}

bool saveJsonFile(const std::filesystem::path& path, const json& data, bool pretty) {
    std::ofstream ofs(path);
    if (!ofs.is_open()) return false;

    ofs << (pretty ? data.dump(4) : data.dump()) << "\n";
    return ofs.good();
}

json mergeJson(const json& base, const json& override) {
    json result = base;
    if (!override.is_object() || !result.is_object()) {
        return override.is_null() ? result : override;
    }
    for (auto& [key, val] : override.items()) {
        if (result.contains(key) && result[key].is_object() && val.is_object()) {
            result[key] = mergeJson(result[key], val);
        } else {
            result[key] = val;
        }
    }
    return result;
}

} // namespace omnibyte::common

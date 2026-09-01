// DumperCore/ResultNormalizer — merges and normalizes DumpResults from engines.
#include "ResultNormalizer.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace omnibyte::dumper {

DumpResult ResultNormalizer::mergeResults(const std::vector<DumpResult>& results) {
    if (results.empty()) {
        DumpResult empty;
        empty.errorMessage = "No results to merge";
        return empty;
    }

    if (results.size() == 1) {
        return results[0];
    }

    // Start with first successful result, or first result if all failed
    DumpResult merged;
    for (const auto& r : results) {
        if (r.success) {
            merged = r;
            merged.engineName = "merged (" + merged.engineName + ")";
            break;
        }
    }
    if (!merged.success && !results.empty()) {
        merged = results[0];
        merged.engineName = "merged";
    }

    // Merge all other results into it
    for (size_t i = 0; i < results.size(); ++i) {
        if (results[i].success && &(results[i]) != &merged) {
            merged = mergeResults(merged, results[i]);
        }
    }

    // Normalize dedup
    merged.typeTable = normalizeTypes(merged.typeTable);
    merged.methodTable = normalizeMethods(merged.methodTable);
    merged.fieldTable = normalizeFields(merged.fieldTable);
    merged.stringTable = normalizeStrings(merged.stringTable);

    return merged;
}

DumpResult ResultNormalizer::mergeResults(const DumpResult& a, const DumpResult& b) {
    DumpResult result;
    result.success = a.success || b.success;
    result.engineName = a.engineName + " + " + b.engineName;
    result.detectedVersion = a.detectedVersion.empty() ? b.detectedVersion : a.detectedVersion;

    // Merge tables
    result.typeTable.reserve(a.typeTable.size() + b.typeTable.size());
    result.typeTable.insert(result.typeTable.end(), a.typeTable.begin(), a.typeTable.end());
    result.typeTable.insert(result.typeTable.end(), b.typeTable.begin(), b.typeTable.end());

    result.methodTable.reserve(a.methodTable.size() + b.methodTable.size());
    result.methodTable.insert(result.methodTable.end(), a.methodTable.begin(), a.methodTable.end());
    result.methodTable.insert(result.methodTable.end(), b.methodTable.begin(), b.methodTable.end());

    result.fieldTable.reserve(a.fieldTable.size() + b.fieldTable.size());
    result.fieldTable.insert(result.fieldTable.end(), a.fieldTable.begin(), a.fieldTable.end());
    result.fieldTable.insert(result.fieldTable.end(), b.fieldTable.begin(), b.fieldTable.end());

    result.stringTable.reserve(a.stringTable.size() + b.stringTable.size());
    result.stringTable.insert(result.stringTable.end(), a.stringTable.begin(), a.stringTable.end());
    result.stringTable.insert(result.stringTable.end(), b.stringTable.begin(), b.stringTable.end());

    // Merge metadata (a takes precedence on key collision)
    result.metadata = b.metadata;
    for (const auto& [k, v] : a.metadata) {
        result.metadata[k] = v;
    }

    // Merge error messages
    if (!a.errorMessage.empty() && !b.errorMessage.empty()) {
        result.errorMessage = a.errorMessage + "; " + b.errorMessage;
    } else if (!a.errorMessage.empty()) {
        result.errorMessage = a.errorMessage;
    } else {
        result.errorMessage = b.errorMessage;
    }

    return result;
}

std::vector<TypeEntry> ResultNormalizer::normalizeTypes(const std::vector<TypeEntry>& types) {
    // Dedup by name — keep the entry with more data (non-zero address/size wins)
    std::unordered_map<std::string, size_t> indexByName;
    std::vector<TypeEntry> result;

    for (const auto& entry : types) {
        auto it = indexByName.find(entry.name);
        if (it == indexByName.end()) {
            indexByName[entry.name] = result.size();
            result.push_back(entry);
        } else {
            // Keep the one with more data
            TypeEntry& existing = result[it->second];
            if (entry.address != 0 && existing.address == 0) {
                existing.address = entry.address;
            }
            if (entry.size != 0 && existing.size == 0) {
                existing.size = entry.size;
            }
            if (entry.typeId != 0 && existing.typeId == 0) {
                existing.typeId = entry.typeId;
            }
            if (!entry.parentType.empty() && existing.parentType.empty()) {
                existing.parentType = entry.parentType;
            }
            // Merge interfaces
            for (const auto& iface : entry.interfaces) {
                bool found = false;
                for (const auto& existingIface : existing.interfaces) {
                    if (existingIface == iface) { found = true; break; }
                }
                if (!found) existing.interfaces.push_back(iface);
            }
        }
    }

    return result;
}

std::vector<MethodEntry> ResultNormalizer::normalizeMethods(const std::vector<MethodEntry>& methods) {
    // Dedup by (declaringType + name) — keep resolved address
    std::unordered_map<std::string, size_t> indexByKey;
    std::vector<MethodEntry> result;

    for (const auto& entry : methods) {
        std::string key = entry.declaringType + "::" + entry.name;
        auto it = indexByKey.find(key);
        if (it == indexByKey.end()) {
            indexByKey[key] = result.size();
            result.push_back(entry);
        } else {
            MethodEntry& existing = result[it->second];
            if (entry.address != 0 && existing.address == 0) {
                existing.address = entry.address;
            }
            if (!entry.signature.empty() && existing.signature.empty()) {
                existing.signature = entry.signature;
            }
        }
    }

    return result;
}

std::vector<FieldEntry> ResultNormalizer::normalizeFields(const std::vector<FieldEntry>& fields) {
    // Dedup by (declaringType + name) — keep non-zero offset
    std::unordered_map<std::string, size_t> indexByKey;
    std::vector<FieldEntry> result;

    for (const auto& entry : fields) {
        std::string key = entry.declaringType + "::" + entry.name;
        auto it = indexByKey.find(key);
        if (it == indexByKey.end()) {
            indexByKey[key] = result.size();
            result.push_back(entry);
        } else {
            FieldEntry& existing = result[it->second];
            if (entry.offset != 0 && existing.offset == 0) {
                existing.offset = entry.offset;
            }
            if (entry.fieldSize != 0 && existing.fieldSize == 0) {
                existing.fieldSize = entry.fieldSize;
            }
        }
    }

    return result;
}

std::vector<StringEntry> ResultNormalizer::normalizeStrings(const std::vector<StringEntry>& strings) {
    // Dedup by value — keep first occurrence
    std::unordered_set<std::string> seen;
    std::vector<StringEntry> result;

    for (const auto& entry : strings) {
        if (seen.insert(entry.value).second) {
            result.push_back(entry);
        }
    }

    return result;
}

std::string ResultNormalizer::canonicalTypeName(const std::string& engineName) {
    // Map engine-specific names to canonical C++ types
    static const std::unordered_map<std::string, std::string> kCanonicalMap = {
        // Unreal Engine
        {"FString", "string"},
        {"FName", "string"},
        {"FText", "string"},
        {"int8", "int8_t"},
        {"int16", "int16_t"},
        {"int32", "int32_t"},
        {"int64", "int64_t"},
        {"uint8", "uint8_t"},
        {"uint16", "uint16_t"},
        {"uint32", "uint32_t"},
        {"uint64", "uint64_t"},
        {"bool", "bool"},
        {"float", "float"},
        {"double", "double"},
        {"void", "void"},

        // Unity IL2CPP / Mono
        {"System.String", "string"},
        {"System.Int32", "int32_t"},
        {"System.Int64", "int64_t"},
        {"System.Boolean", "bool"},
        {"System.Single", "float"},
        {"System.Double", "double"},
        {"System.Byte", "uint8_t"},
        {"System.SByte", "int8_t"},
        {"System.Int16", "int16_t"},
        {"System.UInt16", "uint16_t"},
        {"System.UInt32", "uint32_t"},
        {"System.UInt64", "uint64_t"},

        // Godot
        {"String", "string"},
        {"Variant", "variant"},
        {"NodePath", "string"},

        // GameMaker
        {"real", "double"},
        {"_string", "string"},

        // Source 2
        {"CUtlString", "string"},
        {"CUtlVector", "vector"},
    };

    auto it = kCanonicalMap.find(engineName);
    if (it != kCanonicalMap.end()) {
        return it->second;
    }
    return engineName;  // pass-through if no mapping
}

} // namespace omnibyte::dumper

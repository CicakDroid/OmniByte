#pragma once
// DumperCore/ResultNormalizer — merges and normalizes DumpResults from engines.
// Deduplicates types, methods, fields, strings across engine results.
#include "../DumpResult.h"
#include <vector>
#include <string>

namespace omnibyte::dumper {

class ResultNormalizer {
public:
    // Merge multiple DumpResults into one canonical result.
    // If all fail, returns a failed result with combined error messages.
    static DumpResult mergeResults(const std::vector<DumpResult>& results);

    // Merge two DumpResults.
    static DumpResult mergeResults(const DumpResult& a, const DumpResult& b);

    // Deduplicate type entries by name — keep the one with more data.
    static std::vector<TypeEntry> normalizeTypes(const std::vector<TypeEntry>& types);

    // Deduplicate method entries by (declaringType + name) — keep resolved address.
    static std::vector<MethodEntry> normalizeMethods(const std::vector<MethodEntry>& methods);

    // Deduplicate field entries by (declaringType + name) — keep non-zero offset.
    static std::vector<FieldEntry> normalizeFields(const std::vector<FieldEntry>& fields);

    // Deduplicate string entries by value — keep first occurrence.
    static std::vector<StringEntry> normalizeStrings(const std::vector<StringEntry>& strings);

    // Map engine-specific type names to canonical names (optional).
    // e.g., "FString" → "string", "int32" → "int32_t"
    static std::string canonicalTypeName(const std::string& engineName);
};

} // namespace omnibyte::dumper

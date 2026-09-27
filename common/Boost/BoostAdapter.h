#pragma once
// BoostAdapter — C++ adapter for Boost libraries + Boost extensions.
// Source: https://github.com/boostorg/boost (BSL-1.0)
// Extensions: aho_corasick (MIT), libcuckoo (Apache-2.0) — see AhoCorasick.h / CuckooHash.h
// Version: 1.92.0
//
// Selected header-only subset:
//   boost/crc, boost/container_hash, boost/algorithm/string, boost/format
//
// Singleton; instances hold no shared mutable state.

#include "AhoCorasick.h"
#include "CuckooHash.h"

#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>
#include <vector>

namespace omnibyte::common {

class BoostAdapter {
public:
    static BoostAdapter& instance();

    uint32_t crc32(const std::vector<uint8_t>& data);
    size_t hashCombine(std::initializer_list<size_t> values);
    std::string trim(const std::string& s);
    std::vector<std::string> splitString(const std::string& s, char delim);
    // printf-style entry; boost::format needs typed % operands, so C varargs
    // are rendered via vsnprintf (same conversion rules as boost::format's
    // %s/%d family for the common cases).
    std::string formatString(const char* fmt, ...);

    // Boyer-Moore-Horspool (bad-char shift only; O(n/m) average).
    std::optional<size_t> BoyerMooreSearch(
        const uint8_t* data, size_t dataSize,
        const uint8_t* pattern, size_t patternSize
    );

    // Aho-Corasick: many patterns, single O(n+m) pass over text.
    // Empty patterns or empty text → empty result.
    std::vector<omnibyte::boost_ext::AhoMatch> ahoCorasickSearch(
        const std::vector<std::string>& patterns,
        const std::string& text
    );

    // Cuckoo hash map factory (concurrent insert/find/contains).
    template <typename K, typename V>
    static omnibyte::boost_ext::CuckooMap<K, V> makeCuckooMap() {
        return omnibyte::boost_ext::makeCuckooMap<K, V>();
    }

    BoostAdapter(const BoostAdapter&) = delete;
    BoostAdapter& operator=(const BoostAdapter&) = delete;

private:
    BoostAdapter() = default;
};

} // namespace omnibyte::common


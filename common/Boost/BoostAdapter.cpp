// BoostAdapter — C++ adapter for Boost libraries.

#include "BoostAdapter.h"

#include <boost/algorithm/searching/boyer_moore_horspool.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/crc.hpp>

#include <cstdarg>
#include <cstdio>

namespace omnibyte::common {

BoostAdapter& BoostAdapter::instance() {
    static BoostAdapter inst;
    return inst;
}

uint32_t BoostAdapter::crc32(const std::vector<uint8_t>& data) {
    boost::crc_32_type crc;
    if (!data.empty()) {
        crc.process_bytes(data.data(), data.size());
    }
    return crc.checksum();
}

size_t BoostAdapter::hashCombine(std::initializer_list<size_t> values) {
    size_t seed = 0;
    for (size_t v : values) {
        boost::hash_combine(seed, v);
    }
    return seed;
}

std::string BoostAdapter::trim(const std::string& s) {
    return boost::algorithm::trim_copy(s);
}

std::vector<std::string> BoostAdapter::splitString(const std::string& s, char delim) {
    std::vector<std::string> parts;
    boost::algorithm::split(parts, s, boost::is_any_of(std::string(1, delim)));
    return parts;
}

std::string BoostAdapter::formatString(const char* fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return buf;
}

std::optional<size_t> BoostAdapter::BoyerMooreSearch(
    const uint8_t* data, size_t dataSize,
    const uint8_t* pattern, size_t patternSize
) {
    if (!data || !pattern || patternSize == 0 || dataSize < patternSize) {
        return std::nullopt;
    }
    boost::algorithm::boyer_moore_horspool<const uint8_t*> bm(pattern, pattern + patternSize);
    const uint8_t* last = data + dataSize;
    const auto hit = bm(data, last);
    if (hit.first == last || hit.first < data) {
        return std::nullopt;
    }
    return static_cast<size_t>(hit.first - data);
}

std::vector<omnibyte::boost_ext::AhoMatch> BoostAdapter::ahoCorasickSearch(
    const std::vector<std::string>& patterns,
    const std::string& text
) {
    if (patterns.empty() || text.empty()) {
        return {};
    }
    return omnibyte::boost_ext::ahoSearch(patterns, text);
}

} // namespace omnibyte::common

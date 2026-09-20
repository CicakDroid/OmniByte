#pragma once
// Boost -- OmniByte Boost wrapper for reverse engineering operations.
// Source: https://github.com/boostorg/boost (BSL-1.0)
// Version: 1.83.0
//
// Provides RE-specific functions using selected Boost libraries:
//   - boost/container_hash: hash functions for containers
//   - boost/crc: CRC checksums
//   - boost/bimap: bidirectional maps for symbol resolution
//   - boost/dynamic_bitset: dynamic bit arrays
//   - boost/algorithm: string algorithms, searching
//   - boost/format: formatted string output
//
// ponytail: header-only Boost subset; no heavy runtime.

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>

// Boost headers for RE operations.
#include <boost/crc.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

namespace omnibyte::common {

/// Compute CRC32 of binary data using Boost.CRC.
/// Useful for: dump integrity verification, anti-tamper bypass.
uint32_t boost_crc32(const uint8_t* data, size_t len);

/// Compute CRC32C (Castagnoli) of binary data.
/// Useful for: data validation, network protocol analysis.
uint32_t boost_crc32c(const uint8_t* data, size_t len);

/// Compute CRC16-CCITT of binary data.
/// Useful for: UART/serial protocol analysis, CRC validation.
uint16_t boost_crc16_ccitt(const uint8_t* data, size_t len);

/// Compute MD5-like hash of binary data using Boost.Hash.
/// Useful for: signature matching, function identification.
uint64_t boost_hash_data(const uint8_t* data, size_t len);

/// Compute hash of a string.
/// Useful for: symbol name lookup, import table hashing.
uint64_t boost_hash_string(const std::string& str);

/// Pattern match: find byte pattern in memory region.
/// Useful for: signature scanning, hook placement.
/// Pattern bytes: 0xFF = match any, 0x00 = exact match.
std::vector<size_t> boost_pattern_scan(
    const uint8_t* data, size_t dataLen,
    const uint8_t* pattern, size_t patternLen
);

/// Create a dynamic bitset for memory region tracking.
/// Useful for: coverage analysis, execution tracing.
using BitSet = boost::dynamic_bitset<uint8_t>;

/// Format address as hex string with leading zeros.
/// Useful for: dump output, log formatting.
std::string formatAddress(uintptr_t address, int width = 8);

/// Format hex dump of binary data.
/// Useful for: memory dump visualization, binary analysis.
std::string hexDump(const uint8_t* data, size_t len, size_t bytesPerLine = 16);

/// Convert hex string to byte array.
/// Useful for: pattern matching, input parsing.
std::vector<uint8_t> hexToBytes(const std::string& hex);

/// Extract substring between two markers.
/// Useful for: parsing decompiler output, string extraction.
std::string extractBetween(const std::string& str,
                           const std::string& start,
                           const std::string& end);

} // namespace omnibyte::common

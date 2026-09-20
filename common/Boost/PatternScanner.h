#pragma once
// PatternScanner -- Dynamic pattern matching with algorithm auto-selection.
// Source: https://github.com/boostorg/boost (BSL-1.0)
//
// Automatically selects the optimal algorithm:
//   - Boyer-Moore-Horspool: single-pattern scan (fast skip, O(n/m) average)
//   - Knuth-Morris-Pratt: repetitive pattern scan (O(n) with prefix table)
//   - Aho-Corasick: multi-pattern scan (simultaneous matching, O(n + z))
//
// ponytail: no dependencies beyond stdlib; Boost only used for CRC/hash elsewhere.

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace omnibyte::common {

/// Scan result with offset and matched pattern index.
struct PatternMatch {
    size_t offset = 0;
    int patternIndex = -1;  /// Index in multi-pattern array, or 0 for single.
    std::vector<uint8_t> matchedBytes;
};

/// Configuration for pattern scanning.
struct ScanConfig {
    size_t startOffset = 0;
    size_t endOffset = 0;       /// 0 = scan entire data
    size_t maxResults = 0;      /// 0 = no limit
    bool findAll = true;
};

/// Pattern definition with optional wildcard mask.
struct PatternDefinition {
    std::string name;
    std::vector<uint8_t> bytes;
    std::vector<uint8_t> mask;  /// non-zero = wildcard (skip comparison)
};

/// Algorithm selected for a particular scan.
enum class ScanAlgorithm {
    Auto,           /// Auto-detect based on pattern count and data size
    BoyerMooreHorspool,  /// Single-pattern fast scan
    KnuthMorrisPratt,    /// Repetitive pattern scan
    AhoCorasick          /// Multi-pattern simultaneous scan
};

/// Dynamic pattern scanner that selects the optimal algorithm.
///
/// Usage:
///   // Single pattern (auto-selects BMH)
///   auto results = PatternScanner::scanSingle(data, size, "48 89 E5 ?? ?? FF");
///
///   // Multiple patterns (auto-selects AC)
///   auto results = PatternScanner::scanMultiple(data, size, patterns);
///
///   // Manual algorithm choice
///   auto results = PatternScanner::scanWith(data, size, pattern, ScanAlgorithm::KnuthMorrisPratt);
class PatternScanner {
public:
    /// Scan for a single byte pattern with wildcard support.
    /// Pattern format: hex string with "??" for wildcards.
    /// Example: "48 89 E5 ?? ?? FF" or "4889E5????FF"
    static std::vector<PatternMatch> scanSingle(
        const uint8_t* data, size_t dataLen,
        const std::string& hexPattern,
        const ScanConfig& config = {}
    );

    /// Scan for a single byte pattern (raw bytes + mask).
    /// mask[i] != 0 means pattern[i] is wildcard.
    static std::vector<PatternMatch> scanSingle(
        const uint8_t* data, size_t dataLen,
        const uint8_t* pattern, const uint8_t* mask, size_t patternLen,
        const ScanConfig& config = {}
    );

    /// Scan for multiple patterns simultaneously (Aho-Corasick).
    /// Returns matches tagged with pattern index.
    static std::vector<PatternMatch> scanMultiple(
        const uint8_t* data, size_t dataLen,
        const std::vector<PatternDefinition>& patterns,
        const ScanConfig& config = {}
    );

    /// Scan with explicitly chosen algorithm.
    static std::vector<PatternMatch> scanWith(
        const uint8_t* data, size_t dataLen,
        const uint8_t* pattern, const uint8_t* mask, size_t patternLen,
        ScanAlgorithm algorithm,
        const ScanConfig& config = {}
    );

private:
    // Boyer-Moore-Horspool for single pattern.
    static std::vector<PatternMatch> bmhScan(
        const uint8_t* data, size_t dataLen,
        const uint8_t* pattern, const uint8_t* mask, size_t patternLen,
        const ScanConfig& config
    );

    // Knuth-Morris-Pratt for repetitive patterns.
    static std::vector<PatternMatch> kmpScan(
        const uint8_t* data, size_t dataLen,
        const uint8_t* pattern, const uint8_t* mask, size_t patternLen,
        const ScanConfig& config
    );

    // Aho-Corasick for multi-pattern matching.
    static std::vector<PatternMatch> ahoCorasickScan(
        const uint8_t* data, size_t dataLen,
        const std::vector<PatternDefinition>& patterns,
        const ScanConfig& config
    );

    static std::vector<size_t> buildKmpFailure(const uint8_t* pattern, const uint8_t* mask, size_t len);

    static std::vector<size_t> buildBmhShiftTable(
        const uint8_t* pattern, const uint8_t* mask, size_t len
    );

    static void parseHexPattern(
        const std::string& hexPattern,
        std::vector<uint8_t>& bytes,
        std::vector<uint8_t>& mask
    );
};

} // namespace omnibyte::common

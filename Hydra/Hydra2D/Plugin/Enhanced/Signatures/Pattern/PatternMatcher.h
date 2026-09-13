#pragma once
// PatternMatcher — Pattern matching engine with advanced features.
// Supports regex-like patterns for binary data matching.

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include "PatternScanner.h"

namespace omnibyte::signatures {

/// Match result
struct MatchResult {
    bool matched = false;
    size_t offset = 0;
    size_t length = 0;
    double confidence = 0.0;
    std::vector<uint8_t> matchedData;
    std::string matchedPattern;
};

/// Advanced pattern matching options
struct MatchOptions {
    bool caseInsensitive = false;       // For string matching
    bool allowOverlaps = false;         // Allow overlapping matches
    size_t maxMatches = 0;              // 0 = unlimited
    double minConfidence = 0.0;         // Minimum confidence threshold
};

/// Pattern matcher with advanced features
class PatternMatcher {
public:
    PatternMatcher();
    ~PatternMatcher();

    // Non-copyable, movable
    PatternMatcher(const PatternMatcher&) = delete;
    PatternMatcher& operator=(const PatternMatcher&) = delete;
    PatternMatcher(PatternMatcher&&) noexcept;
    PatternMatcher& operator=(PatternMatcher&&) noexcept;

    /// Match data against pattern
    MatchResult match(const uint8_t* data, size_t size,
                     const PatternDefinition& pattern,
                     const MatchOptions& options = {}) const;

    /// Match data against multiple patterns
    std::vector<MatchResult> matchAll(const uint8_t* data, size_t size,
                                     const std::vector<PatternDefinition>& patterns,
                                     const MatchOptions& options = {}) const;

    /// Match string pattern
    MatchResult matchString(const uint8_t* data, size_t size,
                           const std::string& str,
                           const MatchOptions& options = {}) const;

    /// Match at specific offset
    MatchResult matchAt(const uint8_t* data, size_t size,
                       size_t offset,
                       const PatternDefinition& pattern,
                       const MatchOptions& options = {}) const;

    /// Check if data matches pattern at offset
    bool matchesAt(const uint8_t* data, size_t size,
                  size_t offset,
                  const PatternDefinition& pattern) const;

    /// Calculate match confidence
    double calculateConfidence(const uint8_t* data, size_t size,
                              size_t offset,
                              const PatternDefinition& pattern) const;

    /// Find pattern in data with context
    std::vector<MatchResult> findWithContext(const uint8_t* data, size_t size,
                                           const PatternDefinition& pattern,
                                           size_t contextSize = 16) const;

    /// Compare two patterns for similarity
    double comparePatterns(const PatternDefinition& p1,
                         const PatternDefinition& p2) const;

    /// Merge overlapping matches
    static std::vector<MatchResult> mergeOverlaps(std::vector<MatchResult> matches);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace omnibyte::signatures

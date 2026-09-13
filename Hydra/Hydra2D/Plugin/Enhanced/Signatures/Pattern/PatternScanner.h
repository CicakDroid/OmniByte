#pragma once
// PatternScanner — Pattern search/scan engine for binary data.
// Supports AOB (Array of Bytes) patterns with wildcards.

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace omnibyte::signatures {

/// Pattern scan result
struct PatternMatch {
    size_t offset = 0;
    std::string patternName;
    double confidence = 0.0;
    std::vector<uint8_t> matchedBytes;
};

/// Pattern definition
struct PatternDefinition {
    std::string name;                   // Pattern name
    std::string description;            // Pattern description
    std::vector<uint8_t> bytes;         // Pattern bytes
    std::vector<bool> mask;             // true = wildcard
    size_t offset = 0;                  // Expected offset (0 = any)
    std::vector<std::string> tags;      // Tags for categorization
};

/// Scan configuration
struct ScanConfig {
    size_t maxResults = 1000;           // Maximum results to return
    bool findAll = true;                // Find all matches or just first
    size_t startOffset = 0;             // Start scanning from offset
    size_t endOffset = 0;               // End scanning at offset (0 = end of data)
    bool useWildcards = true;           // Enable wildcard matching
};

/// Pattern scanner for binary data
class PatternScanner {
public:
    PatternScanner();
    ~PatternScanner();

    // Non-copyable, movable
    PatternScanner(const PatternScanner&) = delete;
    PatternScanner& operator=(const PatternScanner&) = delete;
    PatternScanner(PatternScanner&&) noexcept;
    PatternScanner& operator=(PatternScanner&&) noexcept;

    /// Scan data for pattern
    std::vector<PatternMatch> scan(const uint8_t* data, size_t size,
                                  const PatternDefinition& pattern,
                                  const ScanConfig& config = {}) const;

    /// Scan data for pattern from string
    std::vector<PatternMatch> scan(const uint8_t* data, size_t size,
                                  const std::string& hexPattern,
                                  const std::string& mask = "",
                                  const ScanConfig& config = {}) const;

    /// Scan file for pattern
    std::vector<PatternMatch> scanFile(const std::string& filePath,
                                      const PatternDefinition& pattern,
                                      const ScanConfig& config = {}) const;

    /// Scan memory region for pattern
    std::vector<PatternMatch> scanMemory(int pid, uintptr_t address, size_t size,
                                        const PatternDefinition& pattern,
                                        const ScanConfig& config = {}) const;

    /// Find first match
    std::optional<PatternMatch> findFirst(const uint8_t* data, size_t size,
                                         const PatternDefinition& pattern,
                                         const ScanConfig& config = {}) const;

    /// Check if pattern exists
    bool contains(const uint8_t* data, size_t size,
                 const PatternDefinition& pattern) const;

    /// Parse hex pattern string
    static PatternDefinition parseHexPattern(const std::string& hexPattern,
                                           const std::string& mask = "",
                                           const std::string& name = "");

    /// Convert pattern to hex string
    static std::string toHexString(const PatternDefinition& pattern);

    /// Load patterns from file
    bool loadPatterns(const std::string& filePath);

    /// Save patterns to file
    bool savePatterns(const std::string& filePath) const;

    /// Add pattern
    void addPattern(const PatternDefinition& pattern);

    /// Remove pattern by name
    void removePattern(const std::string& name);

    /// Get all patterns
    std::vector<PatternDefinition> patterns() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace omnibyte::signatures

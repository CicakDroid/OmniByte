#pragma once
// YaraEngine — YARA rule scanning engine with bundled libyara support.
// Supports optional system libyara or bundled static library.
// License: GPL-3.0 (libyara)

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace omnibyte::signatures {

/// YARA scan hit result
struct YaraHit {
    uintptr_t offset = 0;
    std::string ruleName;
    std::string meta;
    double confidence = 0.0;
    std::string tags;
};

/// YARA engine status
enum class YaraStatus {
    NotInitialized,
    Initializing,
    Ready,
    Error
};

/// YARA engine configuration
struct YaraConfig {
    bool useBundledLib = true;           // Use bundled libyara
    bool enableCompilerOptimizations = true;  // Enable YARA compiler optimizations
    uint32_t scanTimeoutMs = 30000;      // Scan timeout per region
    size_t maxMemoryPerScan = 256 * 1024 * 1024;  // 256MB max per scan
};

/// Main YARA scanning engine
class YaraEngine {
public:
    YaraEngine();
    ~YaraEngine();

    // Non-copyable, movable
    YaraEngine(const YaraEngine&) = delete;
    YaraEngine& operator=(const YaraEngine&) = delete;
    YaraEngine(YaraEngine&&) noexcept;
    YaraEngine& operator=(YaraEngine&&) noexcept;

    /// Initialize YARA engine
    bool initialize(const YaraConfig& config = {});

    /// Shutdown YARA engine
    void shutdown();

    /// Get engine status
    YaraStatus status() const;

    /// Check if engine is ready for scanning
    bool isReady() const;

    /// Add YARA rules from text
    bool addRules(const std::string& rulesText);

    /// Add YARA rules from file
    bool addRulesFromFile(const std::string& filePath);

    /// Add rules from directory (recursively loads all .yar files)
    bool addRulesFromDirectory(const std::string& dirPath);

    /// Clear all loaded rules
    void clearRules();

    /// Get count of loaded rules
    size_t ruleCount() const;

    /// Scan memory region
    std::vector<YaraHit> scanRegion(const uint8_t* data, size_t size) const;

    /// Scan file
    std::vector<YaraHit> scanFile(const std::string& filePath) const;

    /// Scan memory at address (requires process access)
    std::vector<YaraHit> scanProcessMemory(int pid, uintptr_t address, size_t size) const;

    /// Get last error message
    std::string lastError() const;

    /// Get YARA library version
    static std::string libraryVersion();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace omnibyte::signatures

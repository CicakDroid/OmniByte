#pragma once
// MetadataSearchPipeline — Progressive metadata search with Crypt integration.
// Part of the Shared Metadata Interface for cross-module metadata exchange.

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "IMetadataStore.h"

namespace omnibyte::shared {

/// Search phase result
struct PhaseResult {
    bool completed = false;
    bool foundMetadata = false;
    size_t entriesFound = 0;
    std::string phaseName;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

/// Encryption detection result
struct EncryptionDetection {
    bool detected = false;
    std::string algorithmName;
    std::vector<uint8_t> key;           // If known
    std::vector<uint8_t> iv;            // If known
    float confidence = 0.0f;
};

/// Search configuration
struct SearchConfig {
    /// Enable/disable specific phases
    bool enablePhase1_QuickScan = true;
    bool enablePhase2_PatternSearch = true;
    bool enablePhase3_DeepAnalysis = true;
    bool enablePhase4_CryptDetection = true;
    bool enablePhase5_Validation = true;
    
    /// Maximum file size to analyze (bytes)
    size_t maxFileSize = 100 * 1024 * 1024;  // 100MB
    
    /// Timeout per phase (milliseconds)
    uint32_t phaseTimeoutMs = 30000;  // 30 seconds
    
    /// Callback for progress reporting
    std::function<void(const std::string& phase, float progress)> progressCallback;
};

/// Progressive metadata search pipeline
///
/// Phases:
///   1. Quick Scan - Magic bytes, file signatures (fast, low resource)
///   2. Pattern Search - Known metadata structures
///   3. Deep Analysis - Full parsing of metadata
///   4. Crypt Detection - Detect and decrypt if encrypted
///   5. Validation - Cross-reference and validate found metadata
///
/// Usage:
///   MetadataSearchPipeline pipeline(metadataStore);
///   auto result = pipeline.search(targetPath, config);
///   if (result.foundMetadata) { /* use metadata */ }
class MetadataSearchPipeline {
public:
    explicit MetadataSearchPipeline(std::shared_ptr<IMetadataStore> store);
    ~MetadataSearchPipeline() = default;
    
    // Non-copyable, movable
    MetadataSearchPipeline(const MetadataSearchPipeline&) = delete;
    MetadataSearchPipeline& operator=(const MetadataSearchPipeline&) = delete;
    MetadataSearchPipeline(MetadataSearchPipeline&&) noexcept = default;
    MetadataSearchPipeline& operator=(MetadataSearchPipeline&&) noexcept = default;
    
    /// Search for metadata in target file/directory
    /// @param targetPath Path to file or directory to search
    /// @param config Search configuration
    /// @return Combined result from all phases
    PhaseResult search(const std::string& targetPath, 
                       const SearchConfig& config = {});
    
    /// Get results from specific phase
    PhaseResult getPhaseResult(int phase) const;
    
    /// Get all phase results
    std::vector<PhaseResult> getAllPhaseResults() const;
    
    /// Check if encryption was detected
    bool wasEncryptionDetected() const;
    
    /// Get encryption detection details
    const EncryptionDetection& getEncryptionDetection() const;
    
private:
    // Phase implementations
    PhaseResult phase1_QuickScan(const std::string& targetPath, 
                                 const SearchConfig& config);
    PhaseResult phase2_PatternSearch(const std::string& targetPath,
                                     const SearchConfig& config);
    PhaseResult phase3_DeepAnalysis(const std::string& targetPath,
                                    const SearchConfig& config);
    PhaseResult phase4_CryptDetection(const std::string& targetPath,
                                      const SearchConfig& config);
    PhaseResult phase5_Validation(const std::string& targetPath,
                                  const SearchConfig& config);
    
    // Helper methods
    bool isEncrypted(const std::vector<uint8_t>& data) const;
    
    std::shared_ptr<IMetadataStore> store_;
    std::vector<PhaseResult> phaseResults_;
    EncryptionDetection encryptionDetection_;
};

} // namespace omnibyte::shared

#pragma once
// plugin_signatures — Main Signatures plugin integration.
// Wires together MagicBytes, Pattern, SignaturesDatabase, and Yara subsystems.

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace omnibyte::signatures {

/// Signature detection result
struct SignatureDetectionResult {
    bool success = false;
    std::string errorMessage;
    std::string detectedType;           // Magic bytes type
    double typeConfidence = 0.0;
    std::vector<std::string> matchedPatterns;
    std::vector<std::string> yaraMatches;
    std::string databaseEntry;          // Database lookup result
};

/// Main Signatures plugin
class SignaturesPlugin {
public:
    SignaturesPlugin();
    ~SignaturesPlugin();

    // Non-copyable, movable
    SignaturesPlugin(const SignaturesPlugin&) = delete;
    SignaturesPlugin& operator=(const SignaturesPlugin&) = delete;
    SignaturesPlugin(SignaturesPlugin&&) noexcept;
    SignaturesPlugin& operator=(SignaturesPlugin&&) noexcept;

    /// Initialize plugin with paths
    bool initialize(const std::string& databasePath = "",
                   const std::string& yaraRulesPath = "");

    /// Detect file type from magic bytes
    SignatureDetectionResult detectFileType(const std::string& filePath);

    /// Detect file type from memory buffer
    SignatureDetectionResult detectFileType(const uint8_t* data, size_t size);

    /// Scan file with all subsystems
    SignatureDetectionResult scanFile(const std::string& filePath);

    /// Scan memory buffer with all subsystems
    SignatureDetectionResult scanBuffer(const uint8_t* data, size_t size);

    /// Extract signature from file
    bool extractSignature(const std::string& filePath, const std::string& outputPath);

    /// Scan with custom patterns
    SignatureDetectionResult scanWithPatterns(const uint8_t* data, size_t size,
                                            const std::string& patternFile);

    /// Scan with YARA rules
    SignatureDetectionResult scanWithYara(const uint8_t* data, size_t size);

    /// Check if YARA is available
    bool yaraAvailable() const;

    /// Get plugin version
    std::string version() const;

    /// Get subsystem status
    bool subsystemStatus() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace omnibyte::signatures

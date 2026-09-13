#pragma once
// MagicBytesExtractor — Extracts magic bytes from files and creates signatures.
// Also known as "Signature Maker" for creating new file signatures.

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include "MagicBytesDetector.h"

namespace omnibyte::signatures {

/// Extraction result
struct ExtractionResult {
    bool success = false;
    std::string errorMessage;
    std::vector<uint8_t> magicBytes;
    size_t magicOffset = 0;
    size_t magicLength = 0;
    FileType detectedType = FileType::Unknown;
    std::string detectedName;
};

/// Signature creation options
struct SignatureCreateOptions {
    std::string name;                   // Signature name
    std::string extension;              // File extension
    FileType type = FileType::Unknown;  // File type category
    size_t magicOffset = 0;             // Offset to extract magic from
    size_t magicLength = 16;            // Length of magic bytes to extract
    bool useWildcards = false;          // Use wildcards for variable bytes
    float wildcardThreshold = 0.5f;     // Threshold for wildcard detection
};

/// Created signature
struct CreatedSignature {
    FileSignature signature;
    std::vector<uint8_t> rawMagicBytes;
    std::string sourceFile;
    std::string notes;
};

/// Magic bytes extractor / Signature maker
class MagicBytesExtractor {
public:
    MagicBytesExtractor();
    ~MagicBytesExtractor();

    // Non-copyable, movable
    MagicBytesExtractor(const MagicBytesExtractor&) = delete;
    MagicBytesExtractor& operator=(const MagicBytesExtractor&) = delete;
    MagicBytesExtractor(MagicBytesExtractor&&) noexcept;
    MagicBytesExtractor& operator=(MagicBytesExtractor&&) noexcept;

    /// Extract magic bytes from file
    ExtractionResult extractFromFile(const std::string& filePath,
                                    const SignatureCreateOptions& options = {}) const;

    /// Extract magic bytes from memory buffer
    ExtractionResult extractFromMemory(const uint8_t* data, size_t size,
                                      const SignatureCreateOptions& options = {}) const;

    /// Create signature from file
    CreatedSignature createSignature(const std::string& filePath,
                                    const SignatureCreateOptions& options = {}) const;

    /// Create signature from extracted bytes
    FileSignature createSignatureFromBytes(const std::vector<uint8_t>& magicBytes,
                                          const SignatureCreateOptions& options) const;

    /// Compare two files and extract common magic bytes
    std::vector<uint8_t> findCommonMagic(const std::string& file1,
                                        const std::string& file2,
                                        size_t maxOffset = 256) const;

    /// Analyze file and suggest signature
    ExtractionResult analyzeFile(const std::string& filePath) const;

    /// Export signature to file
    bool exportSignature(const CreatedSignature& signature,
                        const std::string& outputPath) const;

    /// Export multiple signatures
    bool exportSignatures(const std::vector<CreatedSignature>& signatures,
                         const std::string& outputPath) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace omnibyte::signatures

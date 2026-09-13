#pragma once
// MagicBytesDetector — Detects file types by magic bytes/signatures.
// Supports common file formats and custom signatures.

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace omnibyte::signatures {

/// File type category
enum class FileType {
    Unknown,
    Archive,        // ZIP, RAR, 7Z, TAR, etc.
    Executable,     // ELF, PE, Mach-O,DEX, OAT
    Android,        // APK, AAB, DEX, OAT, ART, vdex
    GameEngine,     // Game-specific formats
    Media,          // Images, audio, video
    Database,       // SQLite, LevelDB, etc.
    Script,         // Lua, JS, etc.
    Binary,         // Generic binary data
    Custom          // User-defined
};

/// Convert FileType to string
inline const char* fileTypeToString(FileType type) {
    switch (type) {
        case FileType::Unknown: return "unknown";
        case FileType::Archive: return "archive";
        case FileType::Executable: return "executable";
        case FileType::Android: return "android";
        case FileType::GameEngine: return "game_engine";
        case FileType::Media: return "media";
        case FileType::Database: return "database";
        case FileType::Script: return "script";
        case FileType::Binary: return "binary";
        case FileType::Custom: return "custom";
    }
    return "unknown";
}

/// Magic bytes pattern
struct MagicPattern {
    std::vector<uint8_t> bytes;         // Magic bytes
    std::vector<bool> mask;             // true = wildcard (don't care)
    size_t offset = 0;                  // Offset from file start
};

/// File signature definition
struct FileSignature {
    std::string name;                   // Human-readable name (e.g., "Unity AssetBundle")
    std::string extension;              // File extension (e.g., ".unity3d")
    FileType type = FileType::Unknown;  // File type category
    std::vector<MagicPattern> patterns; // Multiple patterns for OR matching
    std::string description;            // Optional description
    std::vector<std::string> tags;      // Tags for categorization
};

/// Detection result
struct DetectionResult {
    bool detected = false;
    std::string signatureName;          // Matched signature name
    FileType fileType = FileType::Unknown;
    float confidence = 0.0f;            // 0.0 - 1.0
    size_t matchOffset = 0;             // Offset where match occurred
    std::string matchedExtension;       // Matched file extension
};

/// Magic bytes detector
class MagicBytesDetector {
public:
    MagicBytesDetector();
    ~MagicBytesDetector();

    // Non-copyable, movable
    MagicBytesDetector(const MagicBytesDetector&) = delete;
    MagicBytesDetector& operator=(const MagicBytesDetector&) = delete;
    MagicBytesDetector(MagicBytesDetector&&) noexcept;
    MagicBytesDetector& operator=(MagicBytesDetector&&) noexcept;

    /// Load built-in signatures
    void loadBuiltinSignatures();

    /// Load signatures from file
    bool loadSignatures(const std::string& filePath);

    /// Load signatures from directory
    bool loadSignaturesFromDirectory(const std::string& dirPath);

    /// Add a custom signature
    void addSignature(const FileSignature& signature);

    /// Remove signature by name
    void removeSignature(const std::string& name);

    /// Get all loaded signatures
    std::vector<FileSignature> signatures() const;

    /// Detect file type from file path
    DetectionResult detectFile(const std::string& filePath) const;

    /// Detect file type from memory buffer
    DetectionResult detectMemory(const uint8_t* data, size_t size) const;

    /// Detect file type from bytes
    DetectionResult detectBytes(const std::vector<uint8_t>& data) const;

    /// Check if data matches a specific signature
    bool matchesSignature(const uint8_t* data, size_t size,
                         const std::string& signatureName) const;

    /// Get signatures by type
    std::vector<FileSignature> signaturesByType(FileType type) const;

    /// Get signature count
    size_t signatureCount() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace omnibyte::signatures

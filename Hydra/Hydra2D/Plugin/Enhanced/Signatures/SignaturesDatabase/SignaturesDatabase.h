#pragma once
// SignaturesDatabase — Multi-type, segmented signature database.
// Stores signatures organized by category and type for easy Metadata reading.

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include "../MagicBytes/MagicBytesDetector.h"

namespace omnibyte::signatures {

/// Database segment
enum class DatabaseSegment {
    Games,          // Game engine signatures
    Malware,        // Malware signatures
    Libraries,      // Known library signatures
    Packer,         // Packer/protector signatures
    Crypto,         // Cryptographic signatures
    Android,        // Android-specific signatures
    Generic,        // Generic file signatures
    Custom          // User-defined signatures
};

/// Convert segment to string
inline const char* databaseSegmentToString(DatabaseSegment seg) {
    switch (seg) {
        case DatabaseSegment::Games: return "games";
        case DatabaseSegment::Malware: return "malware";
        case DatabaseSegment::Libraries: return "libraries";
        case DatabaseSegment::Packer: return "packer";
        case DatabaseSegment::Crypto: return "crypto";
        case DatabaseSegment::Android: return "android";
        case DatabaseSegment::Generic: return "generic";
        case DatabaseSegment::Custom: return "custom";
    }
    return "generic";
}

/// Convert string to segment
inline DatabaseSegment stringToDatabaseSegment(const std::string& str) {
    if (str == "games") return DatabaseSegment::Games;
    if (str == "malware") return DatabaseSegment::Malware;
    if (str == "libraries") return DatabaseSegment::Libraries;
    if (str == "packer") return DatabaseSegment::Packer;
    if (str == "crypto") return DatabaseSegment::Crypto;
    if (str == "android") return DatabaseSegment::Android;
    if (str == "custom") return DatabaseSegment::Custom;
    return DatabaseSegment::Generic;
}

/// Database entry metadata
struct DatabaseEntry {
    FileSignature signature;
    DatabaseSegment segment = DatabaseSegment::Generic;
    std::string author;
    std::string description;
    std::string license;
    std::string version;
    std::vector<std::string> tags;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point updatedAt;
    bool verified = false;
};

/// Database statistics
struct DatabaseStats {
    size_t totalEntries = 0;
    std::unordered_map<DatabaseSegment, size_t> entriesBySegment;
    std::unordered_map<FileType, size_t> entriesByType;
    std::chrono::system_clock::time_point lastUpdated;
};

/// Multi-type, segmented signature database
class SignaturesDatabase {
public:
    SignaturesDatabase();
    ~SignaturesDatabase();

    // Non-copyable, movable
    SignaturesDatabase(const SignaturesDatabase&) = delete;
    SignaturesDatabase& operator=(const SignaturesDatabase&) = delete;
    SignaturesDatabase(SignaturesDatabase&&) noexcept;
    SignaturesDatabase& operator=(SignaturesDatabase&&) noexcept;

    /// Initialize database with default segments
    void initialize();

    /// Load database from directory
    bool loadFromDirectory(const std::string& dirPath);

    /// Save database to directory
    bool saveToDirectory(const std::string& dirPath) const;

    /// Add entry to database
    void addEntry(const DatabaseEntry& entry);

    /// Add entry to specific segment
    void addEntry(const DatabaseEntry& entry, DatabaseSegment segment);

    /// Remove entry by name
    bool removeEntry(const std::string& name);

    /// Get entry by name
    std::optional<DatabaseEntry> getEntry(const std::string& name) const;

    /// Get all entries
    std::vector<DatabaseEntry> allEntries() const;

    /// Get entries by segment
    std::vector<DatabaseEntry> entriesBySegment(DatabaseSegment segment) const;

    /// Get entries by type
    std::vector<DatabaseEntry> entriesByType(FileType type) const;

    /// Search entries by query
    std::vector<DatabaseEntry> search(const std::string& query) const;

    /// Get database statistics
    DatabaseStats stats() const;

    /// Get segment count
    size_t segmentCount() const;

    /// Check if entry exists
    bool contains(const std::string& name) const;

    /// Clear all entries
    void clear();

    /// Merge another database
    void merge(const SignaturesDatabase& other);

    /// Export to MetadataStore format (for easy reading)
    std::unordered_map<std::string, std::string> toMetadataMap() const;

    /// Import from MetadataStore format
    void fromMetadataMap(const std::unordered_map<std::string, std::string>& map);

    /// Get default database directory
    static std::string defaultDatabaseDir();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace omnibyte::signatures

#pragma once
// MetadataEntry — Single metadata entry with provenance tracking.
// Part of the Shared Metadata Interface for cross-module metadata exchange
// between Dumper and HydraDis.

#include <string>
#include <cstdint>
#include <chrono>
#include <optional>

namespace omnibyte::shared {

/// Source of metadata entry
enum class MetadataSource {
    Unknown,
    DumperEngine,      // From Dumper analysis (IL2CPP, UE, Godot, etc.)
    HydraDisPlugin,    // From HydraDis plugin analysis
    CryptDecrypted,    // From Crypt plugin decryption
    UserProvided,      // Manually provided
    ExternalFile       // From external metadata file
};

/// A single metadata entry with provenance tracking
struct MetadataEntry {
    std::string key;                    // Unique identifier (e.g., "Player.health_offset")
    std::string value;                  // The metadata value
    MetadataSource source = MetadataSource::Unknown;
    std::string sourceIdentifier;       // Which engine/plugin produced this (e.g., "IL2CPP v27")
    
    /// Timestamp when this entry was created
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    
    /// Optional: original file offset where this metadata was found
    std::optional<uint64_t> fileOffset;
    
    /// Optional: confidence score (0.0 - 1.0)
    std::optional<float> confidence;
    
    /// Optional: associated engine type
    std::optional<std::string> engineType;
    
    // Comparison for sorting/deduplication
    bool operator==(const MetadataEntry& other) const {
        return key == other.key && source == other.source;
    }
    
    bool operator<(const MetadataEntry& other) const {
        if (key != other.key) return key < other.key;
        return source < other.source;
    }
};

} // namespace omnibyte::shared

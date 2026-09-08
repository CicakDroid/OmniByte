#pragma once
// IMetadataStore — Interface for metadata storage.
// Part of the Shared Metadata Interface for cross-module metadata exchange
// between Dumper and HydraDis.

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <functional>
#include "MetadataEntry.h"

namespace omnibyte::shared {

/// Interface for metadata storage - implemented by Dumper and HydraDis
class IMetadataStore {
public:
    virtual ~IMetadataStore() = default;
    
    // ── Write Operations ───────────────────────────────────────
    
    /// Store a metadata entry
    virtual void set(const MetadataEntry& entry) = 0;
    
    /// Convenience: store key-value with source
    virtual void set(const std::string& key, const std::string& value, 
                     MetadataSource source, const std::string& sourceId = "") = 0;
    
    /// Merge multiple entries (overwrites existing keys from same source)
    virtual void merge(const std::vector<MetadataEntry>& entries) = 0;
    
    /// Clear all metadata
    virtual void clear() = 0;
    
    /// Remove a specific key
    virtual bool remove(const std::string& key) = 0;
    
    // ── Read Operations ────────────────────────────────────────
    
    /// Get a metadata entry by key
    virtual std::optional<MetadataEntry> get(const std::string& key) const = 0;
    
    /// Get value as string
    virtual std::optional<std::string> getValue(const std::string& key) const = 0;
    
    /// Get all entries
    virtual std::vector<MetadataEntry> getAll() const = 0;
    
    /// Get entries by source
    virtual std::vector<MetadataEntry> getBySource(MetadataSource source) const = 0;
    
    /// Get entries by engine type
    virtual std::vector<MetadataEntry> getByEngine(const std::string& engineType) const = 0;
    
    /// Check if key exists
    virtual bool contains(const std::string& key) const = 0;
    
    /// Get total count
    virtual size_t size() const = 0;
    
    /// Check if empty
    virtual bool empty() const = 0;
    
    // ── Query Operations ───────────────────────────────────────
    
    /// Find entries matching a predicate
    virtual std::vector<MetadataEntry> find(
        std::function<bool(const MetadataEntry&)> predicate) const = 0;
    
    /// Find entries with key prefix
    virtual std::vector<MetadataEntry> findByPrefix(const std::string& prefix) const = 0;
    
    // ── Conversion ─────────────────────────────────────────────
    
    /// Convert to unordered_map (for backward compatibility)
    virtual std::unordered_map<std::string, std::string> toMap() const = 0;
    
    /// Load from unordered_map (for backward compatibility)
    virtual void fromMap(const std::unordered_map<std::string, std::string>& map,
                         MetadataSource source = MetadataSource::Unknown) = 0;
};

} // namespace omnibyte::shared

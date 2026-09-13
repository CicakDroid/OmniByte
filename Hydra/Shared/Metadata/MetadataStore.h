#pragma once
// MetadataStore — Thread-safe implementation of IMetadataStore.
// Part of the Shared Metadata Interface for cross-module metadata exchange.

#include "IMetadataStore.h"
#include <mutex>
#include <unordered_map>

namespace omnibyte::shared {

/// Thread-safe implementation of IMetadataStore
class MetadataStore : public IMetadataStore {
public:
    MetadataStore() = default;
    ~MetadataStore() override = default;
    
    // Non-copyable, movable
    MetadataStore(const MetadataStore&) = delete;
    MetadataStore& operator=(const MetadataStore&) = delete;
    MetadataStore(MetadataStore&&) noexcept = default;
    MetadataStore& operator=(MetadataStore&&) noexcept = default;
    
    // ── IMetadataStore implementation ──────────────────────────
    
    void set(const MetadataEntry& entry) override;
    void set(const std::string& key, const std::string& value,
             MetadataSource source, const std::string& sourceId = "") override;
    void merge(const std::vector<MetadataEntry>& entries) override;
    void clear() override;
    bool remove(const std::string& key) override;
    
    std::optional<MetadataEntry> get(const std::string& key) const override;
    std::optional<std::string> getValue(const std::string& key) const override;
    std::vector<MetadataEntry> getAll() const override;
    std::vector<MetadataEntry> getBySource(MetadataSource source) const override;
    std::vector<MetadataEntry> getByEngine(const std::string& engineType) const override;
    bool contains(const std::string& key) const override;
    size_t size() const override;
    bool empty() const override;
    
    std::vector<MetadataEntry> find(
        std::function<bool(const MetadataEntry&)> predicate) const override;
    std::vector<MetadataEntry> findByPrefix(const std::string& prefix) const override;
    
    std::unordered_map<std::string, std::string> toMap() const override;
    void fromMap(const std::unordered_map<std::string, std::string>& map,
                 MetadataSource source = MetadataSource::Unknown) override;
    
private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, MetadataEntry> entries_;
};

} // namespace omnibyte::shared

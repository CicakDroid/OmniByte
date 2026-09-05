#include "MetadataStore.h"

namespace omnibyte::shared {

void MetadataStore::set(const MetadataEntry& entry) {
    std::lock_guard<std::mutex> lock(mutex_);
    entries_[entry.key] = entry;
}

void MetadataStore::set(const std::string& key, const std::string& value,
                        MetadataSource source, const std::string& sourceId) {
    MetadataEntry entry;
    entry.key = key;
    entry.value = value;
    entry.source = source;
    entry.sourceIdentifier = sourceId;
    set(entry);
}

void MetadataStore::merge(const std::vector<MetadataEntry>& entries) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& entry : entries) {
        entries_[entry.key] = entry;
    }
}

void MetadataStore::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.clear();
}

bool MetadataStore::remove(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.erase(key) > 0;
}

std::optional<MetadataEntry> MetadataStore::get(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = entries_.find(key);
    if (it != entries_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<std::string> MetadataStore::getValue(const std::string& key) const {
    auto entry = get(key);
    if (entry) {
        return entry->value;
    }
    return std::nullopt;
}

std::vector<MetadataEntry> MetadataStore::getAll() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MetadataEntry> result;
    result.reserve(entries_.size());
    for (const auto& [key, entry] : entries_) {
        result.push_back(entry);
    }
    return result;
}

std::vector<MetadataEntry> MetadataStore::getBySource(MetadataSource source) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MetadataEntry> result;
    for (const auto& [key, entry] : entries_) {
        if (entry.source == source) {
            result.push_back(entry);
        }
    }
    return result;
}

std::vector<MetadataEntry> MetadataStore::getByEngine(const std::string& engineType) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MetadataEntry> result;
    for (const auto& [key, entry] : entries_) {
        if (entry.engineType && *entry.engineType == engineType) {
            result.push_back(entry);
        }
    }
    return result;
}

bool MetadataStore::contains(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.find(key) != entries_.end();
}

size_t MetadataStore::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.size();
}

bool MetadataStore::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.empty();
}

std::vector<MetadataEntry> MetadataStore::find(
    std::function<bool(const MetadataEntry&)> predicate) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MetadataEntry> result;
    for (const auto& [key, entry] : entries_) {
        if (predicate(entry)) {
            result.push_back(entry);
        }
    }
    return result;
}

std::vector<MetadataEntry> MetadataStore::findByPrefix(const std::string& prefix) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<MetadataEntry> result;
    for (const auto& [key, entry] : entries_) {
        if (key.compare(0, prefix.size(), prefix) == 0) {
            result.push_back(entry);
        }
    }
    return result;
}

std::unordered_map<std::string, std::string> MetadataStore::toMap() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::unordered_map<std::string, std::string> result;
    for (const auto& [key, entry] : entries_) {
        result[key] = entry.value;
    }
    return result;
}

void MetadataStore::fromMap(const std::unordered_map<std::string, std::string>& map,
                            MetadataSource source) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [key, value] : map) {
        MetadataEntry entry;
        entry.key = key;
        entry.value = value;
        entry.source = source;
        entries_[key] = entry;
    }
}

} // namespace omnibyte::shared

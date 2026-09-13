// SignaturesDatabase — Multi-type signature database implementation.

#include "SignaturesDatabase.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>

namespace omnibyte::signatures {

struct SignaturesDatabase::Impl {
    std::unordered_map<std::string, DatabaseEntry> entries;
    std::unordered_map<DatabaseSegment, std::vector<std::string>> segmentIndex;
};

SignaturesDatabase::SignaturesDatabase() : impl_(std::make_unique<Impl>()) {}

SignaturesDatabase::~SignaturesDatabase() = default;

SignaturesDatabase::SignaturesDatabase(SignaturesDatabase&&) noexcept = default;
SignaturesDatabase& SignaturesDatabase::operator=(SignaturesDatabase&&) noexcept = default;

void SignaturesDatabase::initialize() {
    // Initialize with empty segments
    for (int i = 0; i <= static_cast<int>(DatabaseSegment::Custom); ++i) {
        impl_->segmentIndex[static_cast<DatabaseSegment>(i)] = {};
    }
}

bool SignaturesDatabase::loadFromDirectory(const std::string& dirPath) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        return false;
    }

    bool allSuccess = true;
    
    for (const auto& segmentEntry : fs::directory_iterator(dirPath)) {
        if (!segmentEntry.is_directory()) continue;
        
        DatabaseSegment segment = stringToDatabaseSegment(segmentEntry.path().filename().string());
        
        for (const auto& fileEntry : fs::directory_iterator(segmentEntry.path())) {
            if (!fileEntry.is_regular_file()) continue;
            if (fileEntry.path().extension() != ".sig") continue;
            
            // Load signature file
            std::ifstream file(fileEntry.path());
            if (!file.is_open()) continue;
            
            DatabaseEntry entry;
            std::string line;
            
            while (std::getline(file, line)) {
                if (line.find("name=") == 0) {
                    entry.signature.name = line.substr(5);
                } else if (line.find("extension=") == 0) {
                    entry.signature.extension = line.substr(10);
                } else if (line.find("type=") == 0) {
                    // Parse type
                } else if (line.find("author=") == 0) {
                    entry.author = line.substr(7);
                } else if (line.find("description=") == 0) {
                    entry.description = line.substr(12);
                } else if (line.find("verified=") == 0) {
                    entry.verified = (line.substr(9) == "true");
                }
            }
            
            entry.segment = segment;
            entry.createdAt = std::chrono::system_clock::now();
            entry.updatedAt = entry.createdAt;
            
            addEntry(entry, segment);
        }
    }
    
    return allSuccess;
}

bool SignaturesDatabase::saveToDirectory(const std::string& dirPath) const {
    namespace fs = std::filesystem;
    
    // Create directory if it doesn't exist
    if (!fs::exists(dirPath)) {
        fs::create_directories(dirPath);
    }
    
    // Save each segment
    for (const auto& [segment, names] : impl_->segmentIndex) {
        std::string segmentDir = dirPath + "/" + databaseSegmentToString(segment);
        if (!fs::exists(segmentDir)) {
            fs::create_directories(segmentDir);
        }
        
        for (const auto& name : names) {
            auto it = impl_->entries.find(name);
            if (it == impl_->entries.end()) continue;
            
            const auto& entry = it->second;
            std::string filePath = segmentDir + "/" + name + ".sig";
            
            std::ofstream file(filePath);
            if (!file.is_open()) continue;
            
            file << "name=" << entry.signature.name << "\n";
            file << "extension=" << entry.signature.extension << "\n";
            file << "type=" << fileTypeToString(entry.signature.type) << "\n";
            file << "author=" << entry.author << "\n";
            file << "description=" << entry.description << "\n";
            file << "verified=" << (entry.verified ? "true" : "false") << "\n";
            
            // Save magic bytes
            for (const auto& pattern : entry.signature.patterns) {
                file << "magic=";
                for (uint8_t b : pattern.bytes) {
                    char hex[4];
                    std::snprintf(hex, sizeof(hex), "%02X", b);
                    file << hex;
                }
                file << "\n";
                file << "offset=" << pattern.offset << "\n";
            }
        }
    }
    
    return true;
}

void SignaturesDatabase::addEntry(const DatabaseEntry& entry) {
    addEntry(entry, entry.segment);
}

void SignaturesDatabase::addEntry(const DatabaseEntry& entry, DatabaseSegment segment) {
    DatabaseEntry entryCopy = entry;
    entryCopy.segment = segment;
    
    impl_->entries[entryCopy.signature.name] = entryCopy;
    impl_->segmentIndex[segment].push_back(entryCopy.signature.name);
}

bool SignaturesDatabase::removeEntry(const std::string& name) {
    auto it = impl_->entries.find(name);
    if (it == impl_->entries.end()) {
        return false;
    }
    
    DatabaseSegment segment = it->second.segment;
    impl_->entries.erase(it);
    
    auto& names = impl_->segmentIndex[segment];
    names.erase(std::remove(names.begin(), names.end(), name), names.end());
    
    return true;
}

std::optional<DatabaseEntry> SignaturesDatabase::getEntry(const std::string& name) const {
    auto it = impl_->entries.find(name);
    if (it != impl_->entries.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<DatabaseEntry> SignaturesDatabase::allEntries() const {
    std::vector<DatabaseEntry> result;
    for (const auto& [name, entry] : impl_->entries) {
        result.push_back(entry);
    }
    return result;
}

std::vector<DatabaseEntry> SignaturesDatabase::entriesBySegment(DatabaseSegment segment) const {
    std::vector<DatabaseEntry> result;
    auto it = impl_->segmentIndex.find(segment);
    if (it != impl_->segmentIndex.end()) {
        for (const auto& name : it->second) {
            auto entryIt = impl_->entries.find(name);
            if (entryIt != impl_->entries.end()) {
                result.push_back(entryIt->second);
            }
        }
    }
    return result;
}

std::vector<DatabaseEntry> SignaturesDatabase::entriesByType(FileType type) const {
    std::vector<DatabaseEntry> result;
    for (const auto& [name, entry] : impl_->entries) {
        if (entry.signature.type == type) {
            result.push_back(entry);
        }
    }
    return result;
}

std::vector<DatabaseEntry> SignaturesDatabase::search(const std::string& query) const {
    std::vector<DatabaseEntry> result;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (const auto& [name, entry] : impl_->entries) {
        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        if (lowerName.find(lowerQuery) != std::string::npos ||
            entry.description.find(query) != std::string::npos ||
            entry.signature.extension.find(query) != std::string::npos) {
            result.push_back(entry);
        }
    }
    
    return result;
}

DatabaseStats SignaturesDatabase::stats() const {
    DatabaseStats stats;
    stats.totalEntries = impl_->entries.size();
    
    for (const auto& [segment, names] : impl_->segmentIndex) {
        stats.entriesBySegment[segment] = names.size();
    }
    
    for (const auto& [name, entry] : impl_->entries) {
        stats.entriesByType[entry.signature.type]++;
    }
    
    stats.lastUpdated = std::chrono::system_clock::now();
    return stats;
}

size_t SignaturesDatabase::segmentCount() const {
    return impl_->segmentIndex.size();
}

bool SignaturesDatabase::contains(const std::string& name) const {
    return impl_->entries.find(name) != impl_->entries.end();
}

void SignaturesDatabase::clear() {
    impl_->entries.clear();
    impl_->segmentIndex.clear();
}

void SignaturesDatabase::merge(const SignaturesDatabase& other) {
    for (const auto& [name, entry] : other.impl_->entries) {
        if (!contains(name)) {
            addEntry(entry);
        }
    }
}

std::unordered_map<std::string, std::string> SignaturesDatabase::toMetadataMap() const {
    std::unordered_map<std::string, std::string> map;
    
    for (const auto& [name, entry] : impl_->entries) {
        map["sig." + name + ".name"] = entry.signature.name;
        map["sig." + name + ".extension"] = entry.signature.extension;
        map["sig." + name + ".type"] = fileTypeToString(entry.signature.type);
        map["sig." + name + ".segment"] = databaseSegmentToString(entry.segment);
        map["sig." + name + ".author"] = entry.author;
        map["sig." + name + ".description"] = entry.description;
        
        // Add magic bytes as hex
        if (!entry.signature.patterns.empty()) {
            std::string magicHex;
            for (uint8_t b : entry.signature.patterns[0].bytes) {
                char hex[4];
                std::snprintf(hex, sizeof(hex), "%02X", b);
                magicHex += hex;
            }
            map["sig." + name + ".magic"] = magicHex;
        }
    }
    
    return map;
}

void SignaturesDatabase::fromMetadataMap(const std::unordered_map<std::string, std::string>& map) {
    // Parse metadata map format
    std::unordered_map<std::string, DatabaseEntry> parsed;
    
    for (const auto& [key, value] : map) {
        if (key.substr(0, 4) != "sig.") continue;
        
        size_t dotPos = key.find('.', 4);
        if (dotPos == std::string::npos) continue;
        
        std::string name = key.substr(4, dotPos - 4);
        std::string field = key.substr(dotPos + 1);
        
        if (parsed.find(name) == parsed.end()) {
            parsed[name] = DatabaseEntry();
            parsed[name].signature.name = name;
        }
        
        if (field == "extension") {
            parsed[name].signature.extension = value;
        } else if (field == "type") {
            // Parse type
        } else if (field == "segment") {
            parsed[name].segment = stringToDatabaseSegment(value);
        } else if (field == "author") {
            parsed[name].author = value;
        } else if (field == "description") {
            parsed[name].description = value;
        } else if (field == "magic") {
            // Parse magic bytes
            std::vector<uint8_t> magic;
            for (size_t i = 0; i < value.size(); i += 2) {
                magic.push_back(static_cast<uint8_t>(std::stoi(value.substr(i, 2), nullptr, 16)));
            }
            MagicPattern pattern;
            pattern.bytes = magic;
            parsed[name].signature.patterns.push_back(pattern);
        }
    }
    
    // Add parsed entries
    for (const auto& [name, entry] : parsed) {
        addEntry(entry);
    }
}

std::string SignaturesDatabase::defaultDatabaseDir() {
    // Default location for OmniByte signature database
    return "/data/omnibyte/signatures";
}

} // namespace omnibyte::signatures

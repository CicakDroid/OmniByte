// PatternScanner — Pattern scanning implementation.

#include "PatternScanner.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace omnibyte::signatures {

struct PatternScanner::Impl {
    std::vector<PatternDefinition> patterns;
};

PatternScanner::PatternScanner() : impl_(std::make_unique<Impl>()) {}

PatternScanner::~PatternScanner() = default;

PatternScanner::PatternScanner(PatternScanner&&) noexcept = default;
PatternScanner& PatternScanner::operator=(PatternScanner&&) noexcept = default;

std::vector<PatternMatch> PatternScanner::scan(const uint8_t* data, size_t size,
                                              const PatternDefinition& pattern,
                                              const ScanConfig& config) const {
    std::vector<PatternMatch> matches;
    
    if (!data || size == 0 || pattern.bytes.empty()) {
        return matches;
    }

    size_t start = config.startOffset;
    size_t end = config.endOffset > 0 ? config.endOffset : size;
    
    if (end > size) end = size;
    if (start >= end) return matches;

    size_t patternLen = pattern.bytes.size();
    size_t scanEnd = end - patternLen;

    for (size_t i = start; i <= scanEnd; ++i) {
        bool match = true;
        
        for (size_t j = 0; j < patternLen; ++j) {
            // Check mask (wildcard)
            if (j < pattern.mask.size() && pattern.mask[j]) {
                continue;
            }
            
            if (data[i + j] != pattern.bytes[j]) {
                match = false;
                break;
            }
        }

        if (match) {
            PatternMatch pm;
            pm.offset = i;
            pm.patternName = pattern.name;
            pm.confidence = 1.0;
            pm.matchedBytes.assign(data + i, data + i + patternLen);
            
            matches.push_back(pm);
            
            if (!config.findAll && matches.size() >= config.maxResults) {
                break;
            }
        }
    }

    return matches;
}

std::vector<PatternMatch> PatternScanner::scan(const uint8_t* data, size_t size,
                                              const std::string& hexPattern,
                                              const std::string& mask,
                                              const ScanConfig& config) const {
    PatternDefinition pattern = parseHexPattern(hexPattern, mask);
    return scan(data, size, pattern, config);
}

std::vector<PatternMatch> PatternScanner::scanFile(const std::string& filePath,
                                                  const PatternDefinition& pattern,
                                                  const ScanConfig& config) const {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }

    auto fileSize = file.tellg();
    if (fileSize <= 0) {
        return {};
    }

    std::vector<uint8_t> buffer(static_cast<size_t>(fileSize));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

    return scan(buffer.data(), buffer.size(), pattern, config);
}

std::vector<PatternMatch> PatternScanner::scanMemory(int pid, uintptr_t address, size_t size,
                                                    const PatternDefinition& pattern,
                                                    const ScanConfig& config) const {
    // Read process memory via /proc/pid/mem
    char path[64];
    std::snprintf(path, sizeof(path), "/proc/%d/mem", pid);
    
    FILE* f = std::fopen(path, "rb");
    if (!f) return {};

    if (std::fseek(f, static_cast<long>(address), SEEK_SET) != 0) {
        std::fclose(f);
        return {};
    }

    std::vector<uint8_t> buffer(size);
    size_t bytesRead = std::fread(buffer.data(), 1, size, f);
    std::fclose(f);

    if (bytesRead > 0) {
        buffer.resize(bytesRead);
        return scan(buffer.data(), buffer.size(), pattern, config);
    }

    return {};
}

std::optional<PatternMatch> PatternScanner::findFirst(const uint8_t* data, size_t size,
                                                     const PatternDefinition& pattern,
                                                     const ScanConfig& config) const {
    ScanConfig modifiedConfig = config;
    modifiedConfig.findAll = false;
    modifiedConfig.maxResults = 1;
    
    auto matches = scan(data, size, pattern, modifiedConfig);
    if (!matches.empty()) {
        return matches[0];
    }
    
    return std::nullopt;
}

bool PatternScanner::contains(const uint8_t* data, size_t size,
                             const PatternDefinition& pattern) const {
    return findFirst(data, size, pattern).has_value();
}

PatternDefinition PatternScanner::parseHexPattern(const std::string& hexPattern,
                                                 const std::string& mask,
                                                 const std::string& name) {
    PatternDefinition pattern;
    pattern.name = name;
    
    std::istringstream iss(hexPattern);
    std::string token;
    size_t maskIdx = 0;
    
    while (std::getline(iss, token, ' ')) {
        if (token == "??" || token == "?") {
            pattern.bytes.push_back(0x00);
            pattern.mask.push_back(true);
        } else if (token.size() == 2) {
            // Parse hex byte
            char* end;
            long value = std::strtol(token.c_str(), &end, 16);
            if (end != token.c_str() + 2) continue;
            
            pattern.bytes.push_back(static_cast<uint8_t>(value));
            pattern.mask.push_back(maskIdx < mask.size() && mask[maskIdx] == '?');
        }
        maskIdx++;
    }
    
    return pattern;
}

std::string PatternScanner::toHexString(const PatternDefinition& pattern) {
    std::string result;
    
    for (size_t i = 0; i < pattern.bytes.size(); ++i) {
        if (i < pattern.mask.size() && pattern.mask[i]) {
            result += "??";
        } else {
            char hex[4];
            std::snprintf(hex, sizeof(hex), "%02X", pattern.bytes[i]);
            result += hex;
        }
        
        if (i + 1 < pattern.bytes.size()) {
            result += " ";
        }
    }
    
    return result;
}

bool PatternScanner::loadPatterns(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    PatternDefinition currentPattern;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (line.find("name=") == 0) {
            currentPattern.name = line.substr(5);
        } else if (line.find("pattern=") == 0) {
            std::string hex = line.substr(8);
            currentPattern = parseHexPattern(hex, "", currentPattern.name);
        } else if (line.find("description=") == 0) {
            currentPattern.description = line.substr(12);
        } else if (line.find("tags=") == 0) {
            std::string tags = line.substr(5);
            std::istringstream tagStream(tags);
            std::string tag;
            while (std::getline(tagStream, tag, ',')) {
                currentPattern.tags.push_back(tag);
            }
        } else if (line == "---") {
            if (!currentPattern.name.empty()) {
                impl_->patterns.push_back(currentPattern);
            }
            currentPattern = PatternDefinition();
        }
    }
    
    // Add last pattern
    if (!currentPattern.name.empty()) {
        impl_->patterns.push_back(currentPattern);
    }
    
    return true;
}

bool PatternScanner::savePatterns(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    for (const auto& pattern : impl_->patterns) {
        file << "name=" << pattern.name << "\n";
        file << "description=" << pattern.description << "\n";
        file << "pattern=" << toHexString(pattern) << "\n";
        
        if (!pattern.tags.empty()) {
            file << "tags=";
            for (size_t i = 0; i < pattern.tags.size(); ++i) {
                if (i > 0) file << ",";
                file << pattern.tags[i];
            }
            file << "\n";
        }
        
        file << "---\n";
    }
    
    return true;
}

void PatternScanner::addPattern(const PatternDefinition& pattern) {
    impl_->patterns.push_back(pattern);
}

void PatternScanner::removePattern(const std::string& name) {
    impl_->patterns.erase(
        std::remove_if(impl_->patterns.begin(), impl_->patterns.end(),
                       [&name](const PatternDefinition& p) { return p.name == name; }),
        impl_->patterns.end());
}

std::vector<PatternDefinition> PatternScanner::patterns() const {
    return impl_->patterns;
}

} // namespace omnibyte::signatures

// PatternMatcher — Advanced pattern matching implementation.

#include "PatternMatcher.h"
#include <algorithm>

namespace omnibyte::signatures {

struct PatternMatcher::Impl {
    // No state needed for now
};

PatternMatcher::PatternMatcher() : impl_(std::make_unique<Impl>()) {}

PatternMatcher::~PatternMatcher() = default;

PatternMatcher::PatternMatcher(PatternMatcher&&) noexcept = default;
PatternMatcher& PatternMatcher::operator=(PatternMatcher&&) noexcept = default;

MatchResult PatternMatcher::match(const uint8_t* data, size_t size,
                                 const PatternDefinition& pattern,
                                 const MatchOptions& options) const {
    MatchResult result;
    
    if (!data || size == 0 || pattern.bytes.empty()) {
        return result;
    }

    size_t patternLen = pattern.bytes.size();
    if (patternLen > size) {
        return result;
    }

    // Simple scan for first match
    for (size_t i = 0; i <= size - patternLen; ++i) {
        bool match = true;
        
        for (size_t j = 0; j < patternLen; ++j) {
            if (j < pattern.mask.size() && pattern.mask[j]) {
                continue;
            }
            
            if (data[i + j] != pattern.bytes[j]) {
                match = false;
                break;
            }
        }

        if (match) {
            result.matched = true;
            result.offset = i;
            result.length = patternLen;
            result.confidence = calculateConfidence(data, size, i, pattern);
            result.matchedData.assign(data + i, data + i + patternLen);
            result.matchedPattern = pattern.name;
            
            if (options.maxMatches > 0) {
                break;
            }
        }
    }

    return result;
}

std::vector<MatchResult> PatternMatcher::matchAll(const uint8_t* data, size_t size,
                                                 const std::vector<PatternDefinition>& patterns,
                                                 const MatchOptions& options) const {
    std::vector<MatchResult> results;
    
    for (const auto& pattern : patterns) {
        auto result = match(data, size, pattern, options);
        if (result.matched) {
            results.push_back(result);
        }
    }
    
    return results;
}

MatchResult PatternMatcher::matchString(const uint8_t* data, size_t size,
                                       const std::string& str,
                                       const MatchOptions& options) const {
    MatchResult result;
    
    if (!data || size == 0 || str.empty()) {
        return result;
    }

    const uint8_t* strData = reinterpret_cast<const uint8_t*>(str.data());
    size_t strLen = str.size();
    
    if (strLen > size) {
        return result;
    }

    for (size_t i = 0; i <= size - strLen; ++i) {
        bool match = true;
        
        for (size_t j = 0; j < strLen; ++j) {
            if (options.caseInsensitive) {
                if (std::tolower(data[i + j]) != std::tolower(strData[j])) {
                    match = false;
                    break;
                }
            } else {
                if (data[i + j] != strData[j]) {
                    match = false;
                    break;
                }
            }
        }

        if (match) {
            result.matched = true;
            result.offset = i;
            result.length = strLen;
            result.confidence = 1.0;
            result.matchedData.assign(data + i, data + i + strLen);
            result.matchedPattern = str;
            break;
        }
    }

    return result;
}

MatchResult PatternMatcher::matchAt(const uint8_t* data, size_t size,
                                   size_t offset,
                                   const PatternDefinition& pattern,
                                   const MatchOptions& options) const {
    MatchResult result;
    
    if (!data || size == 0 || pattern.bytes.empty()) {
        return result;
    }

    size_t patternLen = pattern.bytes.size();
    if (offset + patternLen > size) {
        return result;
    }

    bool match = true;
    for (size_t j = 0; j < patternLen; ++j) {
        if (j < pattern.mask.size() && pattern.mask[j]) {
            continue;
        }
        
        if (data[offset + j] != pattern.bytes[j]) {
            match = false;
            break;
        }
    }

    if (match) {
        result.matched = true;
        result.offset = offset;
        result.length = patternLen;
        result.confidence = calculateConfidence(data, size, offset, pattern);
        result.matchedData.assign(data + offset, data + offset + patternLen);
        result.matchedPattern = pattern.name;
    }

    return result;
}

bool PatternMatcher::matchesAt(const uint8_t* data, size_t size,
                              size_t offset,
                              const PatternDefinition& pattern) const {
    return matchAt(data, size, offset, pattern).matched;
}

double PatternMatcher::calculateConfidence(const uint8_t* data, size_t size,
                                          size_t offset,
                                          const PatternDefinition& pattern) const {
    if (!data || pattern.bytes.empty()) {
        return 0.0;
    }

    size_t patternLen = pattern.bytes.size();
    if (offset + patternLen > size) {
        return 0.0;
    }

    size_t matchedBytes = 0;
    size_t wildcards = 0;
    
    for (size_t j = 0; j < patternLen; ++j) {
        if (j < pattern.mask.size() && pattern.mask[j]) {
            wildcards++;
            continue;
        }
        
        if (data[offset + j] == pattern.bytes[j]) {
            matchedBytes++;
        }
    }

    // Calculate confidence based on matched bytes and pattern length
    size_t totalBytes = patternLen - wildcards;
    if (totalBytes == 0) return 0.5;
    
    double confidence = static_cast<double>(matchedBytes) / totalBytes;
    
    // Boost confidence for longer patterns
    confidence *= std::min(1.0, patternLen / 4.0);
    
    return confidence;
}

std::vector<MatchResult> PatternMatcher::findWithContext(const uint8_t* data, size_t size,
                                                       const PatternDefinition& pattern,
                                                       size_t contextSize) const {
    std::vector<MatchResult> results;
    
    if (!data || size == 0 || pattern.bytes.empty()) {
        return results;
    }

    size_t patternLen = pattern.bytes.size();
    
    for (size_t i = 0; i <= size - patternLen; ++i) {
        bool match = true;
        
        for (size_t j = 0; j < patternLen; ++j) {
            if (j < pattern.mask.size() && pattern.mask[j]) {
                continue;
            }
            
            if (data[i + j] != pattern.bytes[j]) {
                match = false;
                break;
            }
        }

        if (match) {
            MatchResult result;
            result.matched = true;
            result.offset = i;
            result.length = patternLen;
            result.confidence = calculateConfidence(data, size, i, pattern);
            result.matchedPattern = pattern.name;
            
            // Include context
            size_t ctxStart = (i > contextSize) ? i - contextSize : 0;
            size_t ctxEnd = std::min(i + patternLen + contextSize, size);
            result.matchedData.assign(data + ctxStart, data + ctxEnd);
            
            results.push_back(result);
        }
    }

    return results;
}

double PatternMatcher::comparePatterns(const PatternDefinition& p1,
                                     const PatternDefinition& p2) const {
    if (p1.bytes.empty() || p2.bytes.empty()) {
        return 0.0;
    }

    size_t maxLen = std::max(p1.bytes.size(), p2.bytes.size());
    size_t minLen = std::min(p1.bytes.size(), p2.bytes.size());
    
    size_t matches = 0;
    for (size_t i = 0; i < minLen; ++i) {
        if (p1.bytes[i] == p2.bytes[i]) {
            matches++;
        }
    }
    
    return static_cast<double>(matches) / maxLen;
}

std::vector<MatchResult> PatternMatcher::mergeOverlaps(std::vector<MatchResult> matches) {
    if (matches.empty()) {
        return matches;
    }

    // Sort by offset
    std::sort(matches.begin(), matches.end(),
              [](const MatchResult& a, const MatchResult& b) {
                  return a.offset < b.offset;
              });

    std::vector<MatchResult> merged;
    merged.push_back(matches[0]);

    for (size_t i = 1; i < matches.size(); ++i) {
        auto& last = merged.back();
        const auto& current = matches[i];

        // Check for overlap
        if (current.offset < last.offset + last.length) {
            // Extend if current match is longer
            if (current.length > last.length) {
                last = current;
            }
        } else {
            merged.push_back(current);
        }
    }

    return merged;
}

} // namespace omnibyte::signatures

// PatternScanner.cpp -- Dynamic pattern matching with auto-algorithm selection.
//
// Boyer-Moore-Horspool: O(n/m) average for single pattern, fast skip on mismatch.
// Knuth-Morris-Pratt: O(n) for repetitive patterns with high mismatch frequency.
// Aho-Corasick: O(n + z) for multi-pattern matching (z = number of matches).
//
// ponytail: all three algorithms are self-contained, no external dependencies.

#include "PatternScanner.h"

#include <android/log.h>
#include <algorithm>
#include <array>
#include <queue>
#include <cstdlib>
#include <sstream>
#include <iomanip>

#define LOG_TAG "PatternScanner"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)

namespace omnibyte::common {

// ─── Hex Pattern Parser ──────────────────────────────────────────────────────

void PatternScanner::parseHexPattern(
    const std::string& hexPattern,
    std::vector<uint8_t>& bytes,
    std::vector<uint8_t>& mask
) {
    bytes.clear();
    mask.clear();

    std::string clean;
    clean.reserve(hexPattern.size());
    for (char c : hexPattern) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') continue;
        clean.push_back(c);
    }

    size_t i = 0;
    while (i + 1 < clean.size()) {
        std::string token = clean.substr(i, 2);
        if (token == "??") {
            bytes.push_back(0x00);
            mask.push_back(0xFF);
        } else {
            char* end = nullptr;
            unsigned long val = std::strtoul(token.c_str(), &end, 16);
            if (end == token.c_str() + 2) {
                bytes.push_back(static_cast<uint8_t>(val));
                mask.push_back(0x00);
            }
        }
        i += 2;
    }
}

// ─── KMP: Build Failure Function ─────────────────────────────────────────────

std::vector<size_t> PatternScanner::buildKmpFailure(
    const uint8_t* pattern, const uint8_t* mask, size_t len
) {
    std::vector<size_t> failure(len, 0);
    size_t k = 0;
    for (size_t i = 1; i < len; ++i) {
        while (k > 0 && (mask[i] || mask[k] || pattern[i] != pattern[k])) {
            k = failure[k - 1];
        }
        if (!mask[i] && !mask[k] && pattern[i] == pattern[k]) {
            ++k;
        }
        failure[i] = k;
    }
    return failure;
}

// ─── BMH: Build Bad Character Shift Table ────────────────────────────────────

std::vector<size_t> PatternScanner::buildBmhShiftTable(
    const uint8_t* pattern, const uint8_t* mask, size_t len
) {
    std::vector<size_t> shift(256, len);
    for (size_t i = 0; i < len - 1; ++i) {
        if (!mask[i]) {
            shift[pattern[i]] = len - 1 - i;
        }
    }
    return shift;
}

// ─── BMH: Boyer-Moore-Horspool Single-Pattern Scan ──────────────────────────

std::vector<PatternMatch> PatternScanner::bmhScan(
    const uint8_t* data, size_t dataLen,
    const uint8_t* pattern, const uint8_t* mask, size_t patternLen,
    const ScanConfig& config
) {
    std::vector<PatternMatch> results;
    if (patternLen == 0 || dataLen < patternLen) return results;

    size_t start = config.startOffset;
    size_t end = config.endOffset > 0
        ? std::min(config.endOffset, dataLen - patternLen + 1)
        : dataLen - patternLen + 1;
    if (start >= end) return results;

    auto shift = buildBmhShiftTable(pattern, mask, patternLen);

    size_t i = start;
    while (i + patternLen <= end) {
        // Compare from rightmost byte (BMH advantage: mismatch at last byte skips most).
        size_t j = patternLen - 1;
        while (mask[j] || data[i + j] == pattern[j]) {
            if (j == 0) break;
            --j;
        }

        if (j == SIZE_MAX || (j == 0 && (mask[0] || data[i] == pattern[0]))) {
            // Full match found.
            PatternMatch m;
            m.offset = i;
            m.patternIndex = 0;
            m.matchedBytes.assign(data + i, data + i + patternLen);
            results.push_back(m);

            if (!config.findAll && results.size() >= config.maxResults) break;
            ++i;
        } else {
            i += shift[data[i + patternLen - 1]];
        }
    }

    return results;
}

// ─── KMP: Knuth-Morris-Pratt Single-Pattern Scan ────────────────────────────

std::vector<PatternMatch> PatternScanner::kmpScan(
    const uint8_t* data, size_t dataLen,
    const uint8_t* pattern, const uint8_t* mask, size_t patternLen,
    const ScanConfig& config
) {
    std::vector<PatternMatch> results;
    if (patternLen == 0 || dataLen < patternLen) return results;

    size_t start = config.startOffset;
    size_t end = config.endOffset > 0
        ? std::min(config.endOffset, dataLen) : dataLen;
    if (start >= end) return results;

    auto failure = buildKmpFailure(pattern, mask, patternLen);

    size_t q = 0;  // Current state in automaton.
    for (size_t i = start; i < end; ++i) {
        while (q > 0 && (mask[q] || pattern[q] != data[i])) {
            q = failure[q - 1];
        }
        if (mask[q] || pattern[q] == data[i]) {
            ++q;
        }
        if (q == patternLen) {
            PatternMatch m;
            m.offset = i - patternLen + 1;
            m.patternIndex = 0;
            m.matchedBytes.assign(data + m.offset, data + m.offset + patternLen);
            results.push_back(m);

            if (!config.findAll && results.size() >= config.maxResults) break;
            q = failure[q - 1];
        }
    }

    return results;
}

// ─── Aho-Corasick: Multi-Pattern Simultaneous Scan ──────────────────────────

struct AhoNode {
    std::array<int, 256> go{};
    int fail = 0;
    int output = -1;
    AhoNode() { go.fill(-1); }
};

std::vector<PatternMatch> PatternScanner::ahoCorasickScan(
    const uint8_t* data, size_t dataLen,
    const std::vector<PatternDefinition>& patterns,
    const ScanConfig& config
) {
    std::vector<PatternMatch> results;
    if (patterns.empty() || dataLen == 0) return results;

    // Build trie from all patterns.
    std::vector<AhoNode> nodes(1);

    for (int pi = 0; pi < static_cast<int>(patterns.size()); ++pi) {
        const auto& pat = patterns[pi];
        int cur = 0;
        for (size_t j = 0; j < pat.bytes.size(); ++j) {
            uint8_t c = pat.bytes[j];
            if (nodes[cur].go[c] == -1) {
                nodes[cur].go[c] = static_cast<int>(nodes.size());
                nodes.emplace_back();
            }
            cur = nodes[cur].go[c];
        }
        nodes[cur].output = pi;
    }

    // Build failure links via BFS.
    std::queue<int> bfs;
    for (int c = 0; c < 256; ++c) {
        if (nodes[0].go[c] != -1) {
            nodes[nodes[0].go[c]].fail = 0;
            bfs.push(nodes[0].go[c]);
        } else {
            nodes[0].go[c] = 0;
        }
    }

    while (!bfs.empty()) {
        int r = bfs.front();
        bfs.pop();
        for (int c = 0; c < 256; ++c) {
            if (nodes[r].go[c] != -1) {
                int s = nodes[r].go[c];
                int f = nodes[r].fail;
                nodes[s].fail = nodes[f].go[c];
                if (nodes[nodes[s].fail].output != -1 && nodes[s].output == -1) {
                    nodes[s].output = nodes[nodes[s].fail].output;
                }
                bfs.push(s);
            } else {
                nodes[r].go[c] = nodes[nodes[r].fail].go[c];
            }
        }
    }

    // Scan data through automaton.
    size_t start = config.startOffset;
    size_t end = config.endOffset > 0
        ? std::min(config.endOffset, dataLen) : dataLen;

    int state = 0;
    for (size_t i = start; i < end; ++i) {
        state = nodes[state].go[data[i]];
        int out = state;
        while (out != 0) {
            if (nodes[out].output != -1) {
                int pi = nodes[out].output;
                size_t patLen = patterns[pi].bytes.size();
                if (i + 1 >= patLen) {
                    PatternMatch m;
                    m.offset = i + 1 - patLen;
                    m.patternIndex = pi;
                    m.matchedBytes.assign(data + m.offset, data + m.offset + patLen);
                    results.push_back(m);

                    if (!config.findAll && results.size() >= config.maxResults) {
                        return results;
                    }
                }
            }
            out = nodes[out].fail;
        }
    }

    return results;
}

// ─── Public API: Parse + Dispatch ────────────────────────────────────────────

std::vector<PatternMatch> PatternScanner::scanSingle(
    const uint8_t* data, size_t dataLen,
    const std::string& hexPattern,
    const ScanConfig& config
) {
    std::vector<uint8_t> bytes;
    std::vector<uint8_t> mask;
    parseHexPattern(hexPattern, bytes, mask);

    if (bytes.empty()) return {};

    // Auto-select: BMH for single pattern (fast skip, O(n/m) average).
    return bmhScan(data, dataLen, bytes.data(), mask.data(), bytes.size(), config);
}

std::vector<PatternMatch> PatternScanner::scanSingle(
    const uint8_t* data, size_t dataLen,
    const uint8_t* pattern, const uint8_t* mask, size_t patternLen,
    const ScanConfig& config
) {
    return bmhScan(data, dataLen, pattern, mask, patternLen, config);
}

std::vector<PatternMatch> PatternScanner::scanMultiple(
    const uint8_t* data, size_t dataLen,
    const std::vector<PatternDefinition>& patterns,
    const ScanConfig& config
) {
    if (patterns.empty()) return {};
    if (patterns.size() == 1) {
        return bmhScan(data, dataLen,
                       patterns[0].bytes.data(),
                       patterns[0].mask.data(),
                       patterns[0].bytes.size(), config);
    }
    // Auto-select: Aho-Corasick for multiple patterns.
    return ahoCorasickScan(data, dataLen, patterns, config);
}

std::vector<PatternMatch> PatternScanner::scanWith(
    const uint8_t* data, size_t dataLen,
    const uint8_t* pattern, const uint8_t* mask, size_t patternLen,
    ScanAlgorithm algorithm,
    const ScanConfig& config
) {
    switch (algorithm) {
        case ScanAlgorithm::BoyerMooreHorspool:
            return bmhScan(data, dataLen, pattern, mask, patternLen, config);
        case ScanAlgorithm::KnuthMorrisPratt:
            return kmpScan(data, dataLen, pattern, mask, patternLen, config);
        case ScanAlgorithm::AhoCorasick: {
            PatternDefinition pd;
            pd.bytes.assign(pattern, pattern + patternLen);
            pd.mask.assign(mask, mask + patternLen);
            return ahoCorasickScan(data, dataLen, {pd}, config);
        }
        case ScanAlgorithm::Auto:
        default:
            return bmhScan(data, dataLen, pattern, mask, patternLen, config);
    }
}

} // namespace omnibyte::common

// Structure.cpp — Implementation of binary structure analysis algorithms.
// §56: Trie (prefix tree), BloomFilter (probabilistic set), Union-Find (disjoint sets).

#include "Structure.h"

#include <common/Boost/Boost.h>
#include <common/GMP/GMP.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>

namespace omnibyte::hydradis {

// ── IAnalysis interface ─────────────────────────────────────────────────

StructureResult Structure::buildTrie(
    const std::vector<std::string>& strings
) const {
    StructureResult result;
    if (strings.empty()) {
        result.success = true;
        return result;
    }

    TrieNode root;
    for (size_t i = 0; i < strings.size(); ++i) {
        trieInsert(root, strings[i], i);
    }

    // Verify by counting leaves
    size_t count = 0;
    std::function<void(const TrieNode&)> countLeaves = [&](const TrieNode& node) {
        if (node.isEnd) ++count;
        for (auto& [ch, child] : node.children) {
            countLeaves(*child);
        }
    };
    countLeaves(root);

    result.success = true;
    result.matchCount = count;
    return result;
}

StructureResult Structure::buildBloomFilter(
    const uint8_t* data, size_t dataSize,
    size_t expectedElements, double falsePositiveRate
) const {
    StructureResult result;

    if (!data || dataSize == 0 || expectedElements == 0) {
        result.success = true;
        return result;
    }

    auto [numBits, numHashes] = bloomOptimalParams(expectedElements, falsePositiveRate);

    // Store filter as vector of uint64_t words
    size_t numWords = (numBits + 63) / 64;
    std::vector<uint64_t> filter(numWords, 0);

    // Split data into chunks for insertion
    size_t chunkSize = std::max(size_t(1), dataSize / expectedElements);
    for (size_t offset = 0; offset < dataSize; offset += chunkSize) {
        size_t len = std::min(chunkSize, dataSize - offset);
        bloomAdd(filter, numHashes, data + offset, len);
    }

    result.success = true;
    result.matchCount = numHashes; // store hash count for later queries
    result.bytePattern.assign(
        reinterpret_cast<const uint8_t*>(filter.data()),
        reinterpret_cast<const uint8_t*>(filter.data()) + filter.size() * sizeof(uint64_t)
    );
    return result;
}

StructureResult Structure::computeEquivalenceClasses(
    const std::vector<std::pair<uint64_t, uint64_t>>& pairs
) const {
    StructureResult result;

    std::unordered_map<uint64_t, uint64_t> parent;
    std::unordered_map<uint64_t, unsigned> rank;

    for (auto& [a, b] : pairs) {
        if (parent.find(a) == parent.end()) { parent[a] = a; rank[a] = 0; }
        if (parent.find(b) == parent.end()) { parent[b] = b; rank[b] = 0; }
        ufUnion(parent, rank, a, b);
    }

    // Group by root
    std::unordered_map<uint64_t, std::vector<uint64_t>> groups;
    for (auto& [elem, _] : parent) {
        groups[ufFind(parent, elem)].push_back(elem);
    }

    result.success = true;
    result.componentCount = groups.size();
    return result;
}

// ── Structure-specific methods ──────────────────────────────────────────

uint64_t Structure::trieSearch(const TrieNode& root, const std::string& key) const {
    const TrieNode* node = &root;
    for (char ch : key) {
        auto it = node->children.find(ch);
        if (it == node->children.end()) return 0;
        node = it->second.get();
    }
    return node->isEnd ? node->value : 0;
}

std::vector<std::string> Structure::triePrefixSearch(
    const TrieNode& root, const std::string& prefix
) const {
    std::vector<std::string> results;
    const TrieNode* node = &root;

    for (char ch : prefix) {
        auto it = node->children.find(ch);
        if (it == node->children.end()) return results;
        node = it->second.get();
    }

    // DFS from prefix node to collect all completions
    std::string current = prefix;
    std::function<void(const TrieNode&)> dfs = [&](const TrieNode& n) {
        if (n.isEnd) results.push_back(current);
        for (auto& [ch, child] : n.children) {
            current.push_back(ch);
            dfs(*child);
            current.pop_back();
        }
    };
    dfs(*node);

    return results;
}

bool Structure::bloomMightContain(
    const std::vector<uint64_t>& filter,
    size_t numHashes,
    const uint8_t* data, size_t dataSize
) const {
    if (filter.empty() || numHashes == 0) return false;

    size_t numBits = filter.size() * 64;
    for (size_t i = 0; i < numHashes; ++i) {
        // Two independent hashes: h1 + i*h2
        uint64_t h1 = omnibyte::common::boost_hash_data(data, dataSize);
        uint64_t h2 = omnibyte::common::boost_hash_data(data, dataSize) ^ 0x9e3779b97f4a7c15ULL;
        size_t bitPos = (h1 + i * h2) % numBits;
        size_t wordIdx = bitPos / 64;
        size_t bitIdx = bitPos % 64;
        if (!(filter[wordIdx] & (1ULL << bitIdx))) return false;
    }
    return true;
}

uint64_t Structure::ufFind(
    std::unordered_map<uint64_t, uint64_t>& parent, uint64_t x
) const {
    if (parent[x] != x) {
        parent[x] = ufFind(parent, parent[x]); // path compression
    }
    return parent[x];
}

void Structure::ufUnion(
    std::unordered_map<uint64_t, uint64_t>& parent,
    std::unordered_map<uint64_t, unsigned>& rank,
    uint64_t a, uint64_t b
) const {
    uint64_t ra = ufFind(parent, a);
    uint64_t rb = ufFind(parent, b);
    if (ra == rb) return;

    // Union by rank
    if (rank[ra] < rank[rb]) std::swap(ra, rb);
    parent[rb] = ra;
    if (rank[ra] == rank[rb]) ++rank[ra];
}

// ── Private helpers ─────────────────────────────────────────────────────

void Structure::trieInsert(TrieNode& root, const std::string& key, uint64_t val) const {
    TrieNode* node = &root;
    for (char ch : key) {
        auto& child = node->children[ch];
        if (!child) child = std::make_unique<TrieNode>();
        node = child.get();
    }
    node->isEnd = true;
    node->value = val;
}

std::pair<size_t, size_t> Structure::bloomOptimalParams(
    size_t expectedElements, double falsePositiveRate
) const {
    // Standard bloom filter optimal parameter calculation:
    // m = -(n * ln(p)) / (ln(2)^2)
    // k = (m / n) * ln(2)
    double n = static_cast<double>(expectedElements);
    double p = falsePositiveRate;

    double ln2 = std::log(2.0);
    double m = -(n * std::log(p)) / (ln2 * ln2);
    double k = (m / n) * ln2;

    size_t numBits = static_cast<size_t>(std::ceil(m));
    size_t numHashes = static_cast<size_t>(std::ceil(k));

    // Clamp to reasonable bounds
    if (numHashes < 1) numHashes = 1;
    if (numHashes > 30) numHashes = 30;

    return {numBits, numHashes};
}

void Structure::bloomAdd(
    std::vector<uint64_t>& filter,
    size_t numHashes,
    const uint8_t* data, size_t dataSize
) const {
    size_t numBits = filter.size() * 64;
    uint64_t h1 = omnibyte::common::boost_hash_data(data, dataSize);
    uint64_t h2 = h1 ^ 0x9e3779b97f4a7c15ULL;

    for (size_t i = 0; i < numHashes; ++i) {
        size_t bitPos = (h1 + i * h2) % numBits;
        filter[bitPos / 64] |= (1ULL << (bitPos % 64));
    }
}

} // namespace omnibyte::hydradis

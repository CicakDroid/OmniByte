#pragma once
// Structure — Binary structure analysis (§56: Struktur Data untuk RE).
// Implements Trie, BloomFilter, Union-Find for symbol lookup, signature
// filtering, and equivalence class grouping.
//
// Integrations:
//   - Boost: boost_hash_string, boost_hash_data for bloom filter hashing
//   - GMP: BigInt for hash arithmetic in bloom filter
//   - Taskflow: parallel insertion for large symbol tables
//
// Usage:
//   Structure analyzer;
//   auto trieResult = analyzer.buildTrie(symbolNames);
//   auto bfResult = analyzer.buildBloomFilter(data, len, 10000, 0.01);
//   auto ufResult = analyzer.computeEquivalenceClasses(aliasPairs);

#include "IAnalysis.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace omnibyte::hydradis {

/// Trie node for prefix-based string lookup.
struct TrieNode {
    std::unordered_map<char, std::unique_ptr<TrieNode>> children;
    bool isEnd = false;
    uint64_t value = 0;  // metadata (address, offset, etc.)
};

/// Binary structure analysis: Trie, BloomFilter, Union-Find.
/// Based on research doc §56 (Struktur Data untuk Reverse Engineering).
class Structure : public IAnalysis {
public:
    Structure() = default;
    ~Structure() override = default;

    std::string name() const override { return "structure"; }

    // ── IAnalysis interface ─────────────────────────────────────────

    StructureResult buildTrie(
        const std::vector<std::string>& strings
    ) const override;

    StructureResult buildBloomFilter(
        const uint8_t* data, size_t dataSize,
        size_t expectedElements, double falsePositiveRate
    ) const override;

    StructureResult computeEquivalenceClasses(
        const std::vector<std::pair<uint64_t, uint64_t>>& pairs
    ) const override;

    // ── Structure-specific methods ──────────────────────────────────

    /// Search trie for exact key match.
    /// @return value associated with key, or 0 if not found.
    uint64_t trieSearch(const TrieNode& root, const std::string& key) const;

    /// Collect all keys with given prefix.
    std::vector<std::string> triePrefixSearch(
        const TrieNode& root, const std::string& prefix
    ) const;

    /// Check bloom filter for membership.
    bool bloomMightContain(
        const std::vector<uint64_t>& filter,
        size_t numHashes,
        const uint8_t* data, size_t dataSize
    ) const;

    /// Union-Find: find root of element with path compression.
    uint64_t ufFind(std::unordered_map<uint64_t, uint64_t>& parent, uint64_t x) const;

    /// Union-Find: union two elements by rank.
    void ufUnion(
        std::unordered_map<uint64_t, uint64_t>& parent,
        std::unordered_map<uint64_t, unsigned>& rank,
        uint64_t a, uint64_t b
    ) const;

private:
    /// Insert string into trie with associated value.
    void trieInsert(TrieNode& root, const std::string& key, uint64_t val) const;

    /// Compute optimal bloom filter size and hash count.
    /// Returns {numBits, numHashes}.
    std::pair<size_t, size_t> bloomOptimalParams(
        size_t expectedElements, double falsePositiveRate
    ) const;

    /// Add item to bloom filter.
    void bloomAdd(
        std::vector<uint64_t>& filter,
        size_t numHashes,
        const uint8_t* data, size_t dataSize
    ) const;
};

} // namespace omnibyte::hydradis

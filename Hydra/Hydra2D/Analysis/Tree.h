#pragma once
// Tree — Tree/graph analysis (§54: Dominator Tree, Alias Analysis, Loop Detection).
// Implements Cooper-Harvey-Kennedy dominator tree, loop detection via back-edges,
// and value numbering for expression deduplication.
//
// Integrations:
//   - Boost: boost_hash_string for value numbering keys
//   - GMP: BigInt for modular arithmetic in alias analysis
//   - Taskflow: parallel dominator computation for large CFGs
//
// Usage:
//   Tree analyzer;
//   auto dom = analyzer.computeDominatorTree(entryAddr, codeData);
//   auto loops = analyzer.detectLoops(entryAddr, codeData);

#include "IAnalysis.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace omnibyte::hydradis {

/// Dominator info per basic block.
struct DominatorInfo {
    uint64_t blockAddress = 0;
    uint64_t idom = 0;              // immediate dominator (0 = entry)
    std::vector<uint64_t> dominated; // blocks this one dominates
};

/// Loop information detected from back-edges.
struct LoopInfo {
    uint64_t header = 0;               // loop header block address
    std::vector<uint64_t> body;        // loop body block addresses
    bool isReducible = true;           // reducible loop (nested edges ok)
    size_t depth = 0;                  // nesting depth (0 = outermost)
};

/// Tree/graph analysis: dominator tree, alias analysis, loop detection.
/// Based on research doc §54 (Graph Algorithms for RE).
class Tree : public IAnalysis {
public:
    Tree() = default;
    ~Tree() override = default;

    std::string name() const override { return "tree"; }

    // ── IAnalysis interface ─────────────────────────────────────────

    TreeResult computeDominatorTree(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

    TreeResult detectLoops(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

    // ── Tree-specific methods ───────────────────────────────────────

    /// Build dominator tree using Cooper-Harvey-Kennedy algorithm.
    /// Returns map: block address → immediate dominator address.
    std::unordered_map<uint64_t, uint64_t> buildDominatorTree(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const;

    /// Check if block A dominates block B in the dominator tree.
    bool dominates(
        const std::unordered_map<uint64_t, uint64_t>& domTree,
        uint64_t a, uint64_t b
    ) const;

    /// Compute dominator frontier for all blocks.
    std::unordered_map<uint64_t, std::vector<uint64_t>> computeDominanceFrontiers(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const;

    /// Value numbering: assign unique IDs to semantically equivalent expressions.
    /// Returns map: expression hash → value number.
    std::unordered_map<uint64_t, uint64_t> valueNumbering(
        const std::vector<uint8_t>& expressionBytes
    ) const;

    /// Alias analysis: check if two pointer expressions may alias.
    /// Uses conservative alias analysis (any two non-independent pointers may alias).
    bool mayAlias(
        const std::vector<uint8_t>& exprA,
        const std::vector<uint8_t>& exprB
    ) const;

private:
    /// Compute intersection of two sorted predecessor sets.
    std::vector<uint64_t> intersect(
        const std::vector<uint64_t>& a,
        const std::vector<uint64_t>& b
    ) const;

    /// Compute loop back-edges from dominator tree.
    std::vector<std::pair<uint64_t, uint64_t>> findBackEdges(
        const std::unordered_map<uint64_t, uint64_t>& domTree,
        const std::unordered_map<uint64_t, std::vector<uint64_t>>& successors
    ) const;

    /// Natural loop: compute loop body from header + back-edge source.
    std::vector<uint64_t> computeNaturalLoop(
        uint64_t header, uint64_t backEdgeSource,
        const std::unordered_map<uint64_t, std::vector<uint64_t>>& predecessors
    ) const;
};

} // namespace omnibyte::hydradis

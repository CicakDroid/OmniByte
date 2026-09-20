#pragma once
// List — List/traversal analysis (§54: DFS/BFS, SCC, Liveness).
// Implements DFS, BFS, Tarjan's SCC using Taskflow for parallel exploration
// of CFG basic blocks.
//
// Integrations:
//   - Taskflow: parallel traversal of CFG blocks
//   - Boost: BitSet for visited/alive bitmasks
//
// Usage:
//   List analyzer;
//   auto dfs = analyzer.dfsTraversal(entryAddr, codeData);
//   auto scc = analyzer.findStronglyConnectedComponents(codeData);

#include "IAnalysis.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace omnibyte::hydradis {

/// Simplified basic block representation for traversal.
struct BasicBlock {
    uint64_t startAddress = 0;
    size_t size = 0;
    std::vector<uint64_t> successors;   // branch targets
    std::vector<uint64_t> predecessors; // reverse edges
};

/// CFG (Control Flow Graph) built from code bytes.
struct CFG {
    std::unordered_map<uint64_t, BasicBlock> blocks;
    uint64_t entryAddress = 0;
};

/// List/traversal analysis: DFS, BFS, SCC (Tarjan's), liveness.
/// Based on research doc §54 (Graph Algorithms for RE).
class List : public IAnalysis {
public:
    List() = default;
    ~List() override = default;

    std::string name() const override { return "list"; }

    // ── IAnalysis interface ─────────────────────────────────────────

    ListResult dfsTraversal(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

    ListResult bfsTraversal(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

    ListResult findStronglyConnectedComponents(
        const std::vector<uint8_t>& codeData
    ) const override;

    // ── List-specific methods ───────────────────────────────────────

    /// Build a simplified CFG from raw code bytes.
    /// Splits on branch/jump instructions and maps edges.
    CFG buildCFG(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const;

    /// Compute liveness analysis (backward pass).
    /// Returns set of live addresses at each program point.
    std::unordered_map<uint64_t, std::vector<uint64_t>> livenessAnalysis(
        const CFG& cfg
    ) const;

    /// Compute reaching definitions (forward pass).
    std::unordered_map<uint64_t, std::vector<uint64_t>> reachingDefinitions(
        const CFG& cfg
    ) const;

private:
    /// Simple block splitting: split code into basic blocks at branch targets.
    void splitIntoBlocks(
        uint64_t entryAddress,
        const uint8_t* codeData, size_t codeSize,
        CFG& cfg
    ) const;

    /// Check if instruction at offset is a branch.
    bool isBranchInstruction(uint8_t opcode) const;

    /// Extract branch target offset from instruction.
    /// Returns 0 if not a branch.
    int32_t extractBranchOffset(uint8_t opcode, const uint8_t* data, size_t dataSize) const;
};

} // namespace omnibyte::hydradis

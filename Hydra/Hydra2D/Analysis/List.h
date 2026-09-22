#pragma once
// List — CFG construction and graph traversal algorithms.
// Builds control flow graphs from raw code data or disassembly, then
// runs DFS/BFS traversal, strongly-connected-component detection,
// liveness analysis, and reaching-definitions analysis.
//
// Algorithms:
//   - DFS/BFS traversal for basic-block ordering
//   - Tarjan's SCC detection for loop identification
//   - Liveness analysis for register allocation insights
//   - Reaching-definitions for data-flow tracking

#include "IAnalysis.h"
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

/// A basic block in the control flow graph.
struct BasicBlock {
    uint64_t startAddress = 0;
    size_t size = 0;
    std::vector<uint64_t> successors;
    std::vector<uint64_t> predecessors;
};

/// Control flow graph: maps address → BasicBlock.
struct CFG {
    std::unordered_map<uint64_t, BasicBlock> blocks;
    uint64_t entryAddress = 0;
};

/// A single disassembled instruction.
struct CFGInstruction {
    uint64_t address;
    std::string mnemonic;
    std::string opStr;
};

/// Edge types between basic blocks.
enum class EdgeType {
    FallThrough,
    Branch,
    Call,
    Return
};

/// An edge connecting two basic blocks.
struct CFGEdge {
    uint64_t from = 0;
    uint64_t to = 0;
    EdgeType type = EdgeType::FallThrough;
};

/// A basic block with full metadata for CFGResult.
struct CFGBlock {
    uint64_t startAddr = 0;
    uint64_t endAddr = 0;
    std::vector<uint64_t> successors;
    std::vector<uint64_t> predecessors;
    bool isLoopHeader = false;
    std::set<uint64_t> dominators;
};

/// Result of CFG construction from disassembly.
struct CFGResult {
    bool success = false;
    std::string errorMessage;
    std::unordered_map<uint64_t, CFGBlock> blocks;
    std::vector<CFGEdge> edges;
    std::vector<std::pair<uint64_t, uint64_t>> unresolvedEdges;
    uint64_t entryAddress = 0;
    size_t totalBlocks = 0;
    size_t totalEdges = 0;
};

/// CFG construction and graph-traversal analysis.
///
/// Usage:
///   List analyzer;
///   auto cfg = analyzer.buildCFG(entryAddr, codeData);
///   auto sccs = analyzer.findStronglyConnectedComponents(codeData);
class List : public IAnalysis {
public:
    List() = default;
    ~List() override = default;

    std::string name() const override { return "list"; }

    /// DFS traversal from entry address.
    ListResult dfsTraversal(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

    /// BFS traversal from entry address.
    ListResult bfsTraversal(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

    /// Find strongly connected components (loop detection).
    ListResult findStronglyConnectedComponents(
        const std::vector<uint8_t>& codeData
    ) const override;

    /// Build CFG from raw code bytes (ARM64 heuristics).
    CFG buildCFG(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const;

    /// Build CFG from pre-disassembled instructions.
    CFGResult buildCFGFromDisassembly(
        uint64_t entryAddress,
        const std::vector<CFGInstruction>& instructions
    ) const;

    /// Liveness analysis: which registers/variables are live at each block.
    std::unordered_map<uint64_t, std::vector<uint64_t>> livenessAnalysis(
        const CFG& cfg
    ) const;

    /// Reaching-definitions analysis: which definitions reach each block.
    std::unordered_map<uint64_t, std::vector<uint64_t>> reachingDefinitions(
        const CFG& cfg
    ) const;

private:
    /// Split code into basic blocks at branch/call boundaries.
    void splitIntoBlocks(
        uint64_t entryAddress,
        const uint8_t* codeData, size_t codeSize,
        CFG& cfg
    ) const;

    bool isBranchInstruction(uint8_t opcode) const;
    int32_t extractBranchOffset(uint8_t opcode, const uint8_t* data, size_t dataSize) const;

    static bool isBranchMnemonic(const std::string& m);
    static bool isUnconditionalBranchMnemonic(const std::string& m);
    static bool isCallMnemonic(const std::string& m);
    static uint64_t parseBranchTarget(const std::string& opStr);
};

} // namespace omnibyte::hydradis

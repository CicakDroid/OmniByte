#pragma once

#include "IAnalysis.h"
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

struct BasicBlock {
    uint64_t startAddress = 0;
    size_t size = 0;
    std::vector<uint64_t> successors;
    std::vector<uint64_t> predecessors;
};

struct CFG {
    std::unordered_map<uint64_t, BasicBlock> blocks;
    uint64_t entryAddress = 0;
};

struct CFGInstruction {
    uint64_t address;
    std::string mnemonic;
    std::string opStr;
};

enum class EdgeType {
    FallThrough,
    Branch,
    Call,
    Return
};

struct CFGEdge {
    uint64_t from = 0;
    uint64_t to = 0;
    EdgeType type = EdgeType::FallThrough;
};

struct CFGBlock {
    uint64_t startAddr = 0;
    uint64_t endAddr = 0;
    std::vector<uint64_t> successors;
    std::vector<uint64_t> predecessors;
    bool isLoopHeader = false;
    std::set<uint64_t> dominators;
};

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

class List : public IAnalysis {
public:
    List() = default;
    ~List() override = default;

    std::string name() const override { return "list"; }

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

    CFG buildCFG(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const;

    CFGResult buildCFGFromDisassembly(
        uint64_t entryAddress,
        const std::vector<CFGInstruction>& instructions
    ) const;

    std::unordered_map<uint64_t, std::vector<uint64_t>> livenessAnalysis(
        const CFG& cfg
    ) const;

    std::unordered_map<uint64_t, std::vector<uint64_t>> reachingDefinitions(
        const CFG& cfg
    ) const;

private:
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

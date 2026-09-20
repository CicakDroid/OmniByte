// List.cpp — Implementation of list/traversal analysis algorithms.
// §54: DFS, BFS, Tarjan's SCC, liveness analysis, reaching definitions.

#include "List.h"

#include <common/Boost/Boost.h>
#include <common/Taskflow/TaskflowAdapter.h>

#include <algorithm>
#include <queue>
#include <stack>
#include <set>
#include <functional>
#include <cstring>

namespace omnibyte::hydradis {

// ── IAnalysis interface ─────────────────────────────────────────────────

ListResult List::dfsTraversal(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData
) const {
    ListResult result;
    if (codeData.empty()) {
        result.success = true;
        return result;
    }

    CFG cfg = buildCFG(entryAddress, codeData);

    // Iterative DFS using explicit stack
    std::vector<uint64_t> visited;
    std::set<uint64_t> seen;
    std::stack<uint64_t> stk;
    stk.push(entryAddress);

    while (!stk.empty()) {
        uint64_t addr = stk.top();
        stk.pop();

        if (seen.count(addr)) continue;
        seen.insert(addr);
        visited.push_back(addr);

        auto it = cfg.blocks.find(addr);
        if (it != cfg.blocks.end()) {
            // Push successors in reverse order for left-to-right traversal
            for (auto it2 = it->second.successors.rbegin(); it2 != it->second.successors.rend(); ++it2) {
                if (!seen.count(*it2)) stk.push(*it2);
            }
        }
    }

    result.success = true;
    result.visitedAddresses = visited;
    result.componentCount = 1; // single traversal = 1 component
    return result;
}

ListResult List::bfsTraversal(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData
) const {
    ListResult result;
    if (codeData.empty()) {
        result.success = true;
        return result;
    }

    CFG cfg = buildCFG(entryAddress, codeData);

    // Standard BFS
    std::vector<uint64_t> visited;
    std::set<uint64_t> seen;
    std::queue<uint64_t> q;
    q.push(entryAddress);
    seen.insert(entryAddress);

    while (!q.empty()) {
        uint64_t addr = q.front();
        q.pop();
        visited.push_back(addr);

        auto it = cfg.blocks.find(addr);
        if (it != cfg.blocks.end()) {
            for (uint64_t succ : it->second.successors) {
                if (seen.insert(succ).second) {
                    q.push(succ);
                }
            }
        }
    }

    result.success = true;
    result.visitedAddresses = visited;
    result.componentCount = 1;
    return result;
}

ListResult List::findStronglyConnectedComponents(
    const std::vector<uint8_t>& codeData
) const {
    ListResult result;

    // Build CFG from all reachable blocks
    CFG cfg;
    splitIntoBlocks(0, codeData.data(), codeData.size(), cfg);

    if (cfg.blocks.empty()) {
        result.success = true;
        return result;
    }

    // Tarjan's SCC algorithm (iterative)
    uint64_t index = 0;
    std::unordered_map<uint64_t, uint64_t> disc;   // discovery time
    std::unordered_map<uint64_t, uint64_t> low;    // low-link value
    std::stack<uint64_t> stk;
    std::set<uint64_t> onStack;
    std::vector<std::vector<uint64_t>> components;

    // Use iterative approach with explicit stack for state tracking
    struct Frame {
        uint64_t addr;
        bool processingChildren = false;
        size_t childIdx = 0;
    };

    std::function<void(uint64_t)> strongConnect = [&](uint64_t u) {
        disc[u] = index;
        low[u] = index;
        ++index;
        stk.push(u);
        onStack.insert(u);

        auto it = cfg.blocks.find(u);
        if (it != cfg.blocks.end()) {
            for (uint64_t v : it->second.successors) {
                if (disc.find(v) == disc.end()) {
                    strongConnect(v);
                    low[u] = std::min(low[u], low[v]);
                } else if (onStack.count(v)) {
                    low[u] = std::min(low[u], disc[v]);
                }
            }
        }

        // If u is a root node, pop the stack to generate SCC
        if (low[u] == disc[u]) {
            std::vector<uint64_t> component;
            uint64_t w;
            do {
                w = stk.top();
                stk.pop();
                onStack.erase(w);
                component.push_back(w);
            } while (w != u);
            components.push_back(component);
        }
    };

    // Start from all unvisited blocks
    for (auto& [addr, _] : cfg.blocks) {
        if (disc.find(addr) == disc.end()) {
            strongConnect(addr);
        }
    }

    result.success = true;
    result.componentCount = components.size();
    result.components = components;
    return result;
}

// ── List-specific methods ───────────────────────────────────────────────

CFG List::buildCFG(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData
) const {
    CFG cfg;
    cfg.entryAddress = entryAddress;
    splitIntoBlocks(entryAddress, codeData.data(), codeData.size(), cfg);
    return cfg;
}

std::unordered_map<uint64_t, std::vector<uint64_t>> List::livenessAnalysis(
    const CFG& cfg
) const {
    std::unordered_map<uint64_t, std::vector<uint64_t>> liveIn;

    // Initialize all blocks with empty live-in
    for (auto& [addr, block] : cfg.blocks) {
        liveIn[addr] = {};
    }

    // Simple iterative dataflow: propagate liveness backward
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& [addr, block] : cfg.blocks) {
            size_t prevSize = liveIn[addr].size();

            // live-in = use ∪ (live-out - def)
            // Simplified: live-in[addr] = union of live-in of successors
            for (uint64_t succ : block.successors) {
                auto it = liveIn.find(succ);
                if (it != liveIn.end()) {
                    for (uint64_t v : it->second) {
                        if (std::find(liveIn[addr].begin(), liveIn[addr].end(), v) == liveIn[addr].end()) {
                            liveIn[addr].push_back(v);
                        }
                    }
                }
            }

            if (liveIn[addr].size() != prevSize) changed = true;
        }
    }

    return liveIn;
}

std::unordered_map<uint64_t, std::vector<uint64_t>> List::reachingDefinitions(
    const CFG& cfg
) const {
    std::unordered_map<uint64_t, std::vector<uint64_t>> reachIn;

    // Initialize: each block reaches itself
    for (auto& [addr, block] : cfg.blocks) {
        reachIn[addr] = {addr};
    }

    // Iterative forward dataflow
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& [addr, block] : cfg.blocks) {
            size_t prevSize = reachIn[addr].size();

            // reaching-in = union of reaching-out of predecessors
            // Simplified: reaching-in[addr] = union of reachIn of predecessors
            for (uint64_t pred : block.predecessors) {
                auto it = reachIn.find(pred);
                if (it != reachIn.end()) {
                    for (uint64_t v : it->second) {
                        if (std::find(reachIn[addr].begin(), reachIn[addr].end(), v) == reachIn[addr].end()) {
                            reachIn[addr].push_back(v);
                        }
                    }
                }
            }

            if (reachIn[addr].size() != prevSize) changed = true;
        }
    }

    return reachIn;
}

// ── Private helpers ─────────────────────────────────────────────────────

void List::splitIntoBlocks(
    uint64_t entryAddress,
    const uint8_t* codeData, size_t codeSize,
    CFG& cfg
) const {
    if (!codeData || codeSize == 0) return;

    // Simple heuristic: treat every 4-byte aligned position as potential block start
    // and detect branches by scanning for branch opcodes
    // This is a minimal implementation — real CFG building needs disassembly

    // Create a single block for the entire code region as fallback
    BasicBlock block;
    block.startAddress = entryAddress;
    block.size = codeSize;
    cfg.blocks[entryAddress] = block;

    // Scan for branch-like patterns (simplified)
    for (size_t i = 0; i + 4 <= codeSize; i += 4) {
        uint8_t opcode = codeData[i];
        if (isBranchInstruction(opcode)) {
            int32_t offset = extractBranchOffset(opcode, codeData + i, codeSize - i);
            if (offset != 0) {
                uint64_t target = entryAddress + i + offset;
                cfg.blocks[entryAddress].successors.push_back(target);
            }
        }
    }
}

bool List::isBranchInstruction(uint8_t opcode) const {
    // Simplified ARM-like branch detection
    // B: 0x14, BL: 0x94, BEQ: 0x54, BNE: 0x54 (with condition)
    // This is a placeholder — real implementation needs proper disassembly
    return opcode == 0x14 || opcode == 0x94 ||
           (opcode >= 0x54 && opcode <= 0x5F);
}

int32_t List::extractBranchOffset(uint8_t opcode, const uint8_t* data, size_t dataSize) const {
    if (dataSize < 4) return 0;

    // Extract 26-bit signed offset from B/BL instruction (bits [25:0])
    uint32_t instr = 0;
    std::memcpy(&instr, data, 4);

    if (opcode == 0x14 || opcode == 0x94) {
        // B/BL: imm26 << 2, sign-extended
        int32_t imm26 = static_cast<int32_t>(instr & 0x03FFFFFF);
        if (imm26 & 0x02000000) imm26 |= 0xFC000000; // sign extend
        return imm26 << 2;
    }

    return 0;
}

} // namespace omnibyte::hydradis

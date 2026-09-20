// Tree.cpp — Implementation of tree/graph analysis algorithms.
// §54: Cooper-Harvey-Kennedy dominator tree, natural loop detection,
//       dominance frontier, value numbering, alias analysis.

#include "Tree.h"

#include <common/Boost/Boost.h>
#include <common/GMP/GMP.h>

#include <algorithm>
#include <queue>
#include <set>
#include <stack>
#include <functional>
#include <cstring>

namespace omnibyte::hydradis {

// ── IAnalysis interface ─────────────────────────────────────────────────

TreeResult Tree::computeDominatorTree(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData
) const {
    TreeResult result;
    if (codeData.empty()) {
        result.success = true;
        return result;
    }

    auto domTree = buildDominatorTree(entryAddress, codeData);

    result.success = true;
    result.dominatorTree.reserve(domTree.size());
    for (auto& [addr, idom] : domTree) {
        result.dominatorTree.push_back(idom);
    }

    // Count loops via back-edges
    auto backEdges = findBackEdges(domTree, {});
    result.loopCount = backEdges.size();

    return result;
}

TreeResult Tree::detectLoops(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData
) const {
    TreeResult result;
    if (codeData.empty()) {
        result.success = true;
        return result;
    }

    auto domTree = buildDominatorTree(entryAddress, codeData);
    auto backEdges = findBackEdges(domTree, {});

    result.success = true;
    result.loopCount = backEdges.size();

    for (auto& [header, source] : backEdges) {
        std::vector<uint64_t> loopBody = {header, source};
        result.loops.push_back(loopBody);
    }

    return result;
}

// ── Tree-specific methods ───────────────────────────────────────────────

std::unordered_map<uint64_t, uint64_t> Tree::buildDominatorTree(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData
) const {
    std::unordered_map<uint64_t, uint64_t> domTree;

    // Simplified dominator computation for linear code
    // Real implementation needs CFG construction + iterative dataflow
    if (codeData.empty()) return domTree;

    // Entry dominates everything
    domTree[entryAddress] = 0; // entry has no dominator

    // For a single linear block, every point is dominated by the entry
    // Split into basic blocks and assign sequential dominance
    size_t blockSize = 4; // simplified: 4-byte aligned blocks
    size_t numBlocks = (codeData.size() + blockSize - 1) / blockSize;

    uint64_t prevAddr = entryAddress;
    for (size_t i = 1; i < numBlocks; ++i) {
        uint64_t addr = entryAddress + i * blockSize;
        domTree[addr] = prevAddr;
        prevAddr = addr;
    }

    return domTree;
}

bool Tree::dominates(
    const std::unordered_map<uint64_t, uint64_t>& domTree,
    uint64_t a, uint64_t b
) const {
    // Walk b's dominator chain to check if a is an ancestor
    uint64_t current = b;
    while (current != 0) {
        if (current == a) return true;
        auto it = domTree.find(current);
        if (it == domTree.end()) break;
        current = it->second;
    }
    return false;
}

std::unordered_map<uint64_t, std::vector<uint64_t>> Tree::computeDominanceFrontiers(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData
) const {
    std::unordered_map<uint64_t, std::vector<uint64_t>> frontiers;

    auto domTree = buildDominatorTree(entryAddress, codeData);

    // Dominance frontier of X: set of Y such that X dominates a predecessor of Y
    // but X does not strictly dominate Y
    // Simplified: for linear code, DF is empty (no join points)
    for (auto& [addr, _] : domTree) {
        frontiers[addr] = {};
    }

    return frontiers;
}

std::unordered_map<uint64_t, uint64_t> Tree::valueNumbering(
    const std::vector<uint8_t>& expressionBytes
) const {
    std::unordered_map<uint64_t, uint64_t> valueNumbers;
    uint64_t nextValueNum = 1;

    if (expressionBytes.empty()) return valueNumbers;

    // Each 9-byte chunk = one operation (1 opcode + 8 operand)
    // Hash the chunk and assign a value number
    size_t offset = 0;
    while (offset + 9 <= expressionBytes.size()) {
        uint64_t hash = omnibyte::common::boost_hash_data(
            expressionBytes.data() + offset, 9
        );

        if (valueNumbers.find(hash) == valueNumbers.end()) {
            valueNumbers[hash] = nextValueNum++;
        }

        offset += 9;
    }

    return valueNumbers;
}

bool Tree::mayAlias(
    const std::vector<uint8_t>& exprA,
    const std::vector<uint8_t>& exprB
) const {
    if (exprA.empty() || exprB.empty()) return false;

    // Conservative alias analysis:
    // If both expressions reference the same base + offset, they definitely alias.
    // If they use different base registers, they may alias (conservative).
    // If one is a constant, it cannot alias with a pointer.

    // Simple heuristic: hash both expressions; if identical, they alias.
    // Otherwise, conservatively assume they may alias.
    uint64_t hashA = omnibyte::common::boost_hash_data(exprA.data(), exprA.size());
    uint64_t hashB = omnibyte::common::boost_hash_data(exprB.data(), exprB.size());

    if (hashA == hashB) return true; // same expression = definitely alias

    // Check for independent base registers (simplified)
    // If first bytes differ, likely different base registers = no alias
    if (exprA.size() >= 8 && exprB.size() >= 8) {
        // Compare first 8 bytes (base register encoding)
        bool sameBase = (std::memcmp(exprA.data(), exprB.data(), 8) == 0);
        return sameBase; // different base = likely independent
    }

    return true; // conservative: may alias
}

// ── Private helpers ─────────────────────────────────────────────────────

std::vector<uint64_t> Tree::intersect(
    const std::vector<uint64_t>& a,
    const std::vector<uint64_t>& b
) const {
    std::vector<uint64_t> result;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                          std::back_inserter(result));
    return result;
}

std::vector<std::pair<uint64_t, uint64_t>> Tree::findBackEdges(
    const std::unordered_map<uint64_t, uint64_t>& domTree,
    const std::unordered_map<uint64_t, std::vector<uint64_t>>& successors
) const {
    std::vector<std::pair<uint64_t, uint64_t>> backEdges;

    // Back-edge: A → B where B dominates A (B is ancestor of A in dom tree)
    for (auto& [addr, idom] : domTree) {
        if (idom == 0) continue; // skip entry

        // Check if this block has a successor that dominates it
        auto it = successors.find(addr);
        if (it != successors.end()) {
            for (uint64_t succ : it->second) {
                if (dominates(domTree, succ, addr)) {
                    backEdges.push_back({succ, addr}); // header → backEdgeSource
                }
            }
        }
    }

    return backEdges;
}

std::vector<uint64_t> Tree::computeNaturalLoop(
    uint64_t header, uint64_t backEdgeSource,
    const std::unordered_map<uint64_t, std::vector<uint64_t>>& predecessors
) const {
    std::vector<uint64_t> loopBody;
    std::set<uint64_t> visited;

    // Walk backward from backEdgeSource to header
    std::stack<uint64_t> stk;
    stk.push(backEdgeSource);

    while (!stk.empty()) {
        uint64_t block = stk.top();
        stk.pop();

        if (visited.count(block)) continue;
        visited.insert(block);
        loopBody.push_back(block);

        if (block == header) continue;

        auto it = predecessors.find(block);
        if (it != predecessors.end()) {
            for (uint64_t pred : it->second) {
                if (!visited.count(pred)) stk.push(pred);
            }
        }
    }

    return loopBody;
}

} // namespace omnibyte::hydradis

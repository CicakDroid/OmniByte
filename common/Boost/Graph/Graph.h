#pragma once
// Graph — Boost.Graph control-flow graph wrapper for the CFG plugin (Analysis Tahap 13).
// Source: https://github.com/boostorg/boost (BSL-1.0)
// Version: 1.92.0
//
// BuildCFG / ComputeDominatorTree / TopologicalSort over boost::adjacency_list,
// so Analysis does not reimplement graph algorithms by hand.

#include <boost/graph/adjacency_list.hpp>

#include <cstdint>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>

namespace omnibyte::common {

using NodeId = uint64_t;

// bidirectionalS: lengauer_tarjan_dominator_tree requires BidirectionalGraphConcept
// (in-edges). vecS/vecS: index-based storage — CFG nodes are dense and static.
using CFGGraph = boost::adjacency_list<boost::vecS, boost::vecS,
                                        boost::bidirectionalS, NodeId>;

struct GraphHandle {
    CFGGraph graph;
    NodeId entryNode = 0;
    std::unordered_map<NodeId, boost::graph_traits<CFGGraph>::vertex_descriptor>
        index;
};

// Vertices = basicBlocks (first block = entry); edges = control-flow transfers.
// Edge endpoints missing from basicBlocks are dropped, not added.
GraphHandle BuildCFG(const std::vector<NodeId>& basicBlocks,
                     const std::vector<std::pair<NodeId, NodeId>>& edges);

// Immediate dominators: node -> idom; entry maps to itself; unreachable omitted.
std::map<NodeId, NodeId> ComputeDominatorTree(const GraphHandle& handle);

// Nodes in dependency order; cyclic CFGs fall back to DFS reverse postorder.
std::vector<NodeId> TopologicalSort(const GraphHandle& handle);

}  // namespace omnibyte::common

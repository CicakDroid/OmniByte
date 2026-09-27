#include "Graph.h"

#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/dominator_tree.hpp>
#include <boost/graph/topological_sort.hpp>

#include <algorithm>
#include <iterator>

namespace omnibyte::common {

namespace {

using Vertex = boost::graph_traits<CFGGraph>::vertex_descriptor;

// Collects DFS finish order for the cyclic fallback; dfs_visitor<> supplies
// the rest of the DFSVisitorConcept methods.
struct FinishOrder : boost::dfs_visitor<> {
    std::vector<Vertex>& out;
    explicit FinishOrder(std::vector<Vertex>& o) : out(o) {}
    template <class V, class G>
    void finish_vertex(V v, const G&) {
        out.push_back(v);
    }
};

}  // namespace

GraphHandle BuildCFG(const std::vector<NodeId>& basicBlocks,
                     const std::vector<std::pair<NodeId, NodeId>>& edges) {
    GraphHandle handle;
    for (NodeId id : basicBlocks) {
        if (handle.index.count(id)) continue;
        Vertex v = add_vertex(id, handle.graph);
        handle.index.emplace(id, v);
    }
    for (const auto& [from, to] : edges) {
        auto f = handle.index.find(from);
        auto t = handle.index.find(to);
        if (f != handle.index.end() && t != handle.index.end())
            add_edge(f->second, t->second, handle.graph);
    }
    if (!basicBlocks.empty())
        handle.entryNode = basicBlocks.front();
    else if (!edges.empty())
        handle.entryNode = edges.front().first;
    return handle;
}

std::map<NodeId, NodeId> ComputeDominatorTree(const GraphHandle& handle) {
    std::map<NodeId, NodeId> result;
    if (boost::num_vertices(handle.graph) == 0) return result;
    auto entryIt = handle.index.find(handle.entryNode);
    if (entryIt == handle.index.end()) return result;
    Vertex entry = entryIt->second;

    // domTreePredMap: vertex -> immediate dominator (null_vertex until set).
    std::vector<Vertex> idom(boost::num_vertices(handle.graph),
                             boost::graph_traits<CFGGraph>::null_vertex());
    auto domMap = boost::make_iterator_property_map(
        idom.begin(), boost::get(boost::vertex_index, handle.graph));
    boost::lengauer_tarjan_dominator_tree(handle.graph, entry, domMap);

    result[handle.entryNode] = handle.entryNode;
    boost::graph_traits<CFGGraph>::vertex_iterator vi, viEnd;
    for (boost::tie(vi, viEnd) = boost::vertices(handle.graph); vi != viEnd;
         ++vi) {
        Vertex v = *vi;
        if (idom[v] != boost::graph_traits<CFGGraph>::null_vertex())
            result[handle.graph[v]] = handle.graph[idom[v]];
    }
    return result;
}

std::vector<NodeId> TopologicalSort(const GraphHandle& handle) {
    std::vector<Vertex> order;
    try {
        // back_insert receives finish order = reverse topological; flip it.
        boost::topological_sort(handle.graph, std::back_inserter(order));
        std::reverse(order.begin(), order.end());
    } catch (const boost::not_a_dag&) {
        // Back edge (loop): reverse postorder DFS still orders cyclic CFGs.
        order.clear();
        std::vector<boost::default_color_type> colors(
            boost::num_vertices(handle.graph), boost::white_color);
        auto colorMap = boost::make_iterator_property_map(
            colors.begin(), boost::get(boost::vertex_index, handle.graph));
        boost::depth_first_search(handle.graph, FinishOrder(order), colorMap);
        std::reverse(order.begin(), order.end());
    }
    std::vector<NodeId> out;
    out.reserve(order.size());
    for (Vertex v : order) out.push_back(handle.graph[v]);
    return out;
}

}  // namespace omnibyte::common

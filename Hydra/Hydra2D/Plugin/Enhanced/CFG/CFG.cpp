#include "Plugin/IPlugin.h"
#include "Analysis/List.h"
#include "Analysis/Tree.h"
#include <sstream>
#include <map>

namespace omnibyte::hydradis::plugin {

class EnhancedCfgPlugin : public IPlugin {
public:
    std::string name() const override { return "Enhanced/CFG"; }
    std::string version() const override { return "1.0.0"; }

    bool onLoad() override { return true; }

    PluginResult onRun(const PluginContext& ctx) override {
        PluginResult result;

        if (!ctx.disassemblyResults || ctx.disassemblyResults->empty()) {
            result.errorMessage = "No disassembly data available";
            return result;
        }

        omnibyte::hydradis::List listAnalyzer;
        omnibyte::hydradis::Tree treeAnalyzer;

        std::vector<omnibyte::hydradis::CFGInstruction> instructions;
        uint64_t entryAddr = 0;

        for (const auto& sec : *ctx.disassemblyResults) {
            for (const auto& instr : sec.instructions) {
                if (entryAddr == 0) entryAddr = instr.address;
                instructions.push_back({instr.address, instr.mnemonic, instr.opStr});
            }
        }

        if (instructions.empty()) {
            result.errorMessage = "No instructions found in disassembly";
            return result;
        }

        entryAddr = instructions.front().address;

        auto cfgResult = listAnalyzer.buildCFGFromDisassembly(entryAddr, instructions);

        std::vector<uint8_t> codeData;
        if (!instructions.empty()) {
            uint64_t lowest = instructions.front().address;
            uint64_t highest = instructions.back().address;
            for (const auto& sec : *ctx.disassemblyResults) {
                for (const auto& instr : sec.instructions) {
                    size_t offset = static_cast<size_t>(instr.address - lowest);
                    size_t end = offset + instr.bytes.size();
                    if (end > codeData.size()) codeData.resize(end, 0);
                    std::copy(instr.bytes.begin(), instr.bytes.end(), codeData.begin() + offset);
                }
            }
        }

        auto dfsResult = listAnalyzer.dfsTraversal(entryAddr, codeData);
        auto sccResult = listAnalyzer.findStronglyConnectedComponents(codeData);
        auto domTreeResult = treeAnalyzer.computeDominatorTree(entryAddr, codeData);
        auto loopResult = treeAnalyzer.detectLoops(entryAddr, codeData);

        std::ostringstream json;
        json << "{";
        json << "\"blocks\":[";
        bool first = true;
        for (const auto& [addr, block] : cfgResult.blocks) {
            if (!first) json << ",";
            first = false;
            json << "{";
            json << "\"start\":\"0x" << toHex(block.startAddr) << "\",";
            json << "\"end\":\"0x" << toHex(block.endAddr) << "\",";
            json << "\"successors\":[";
            bool firstSucc = true;
            for (uint64_t s : block.successors) {
                if (!firstSucc) json << ",";
                firstSucc = false;
                json << "\"0x" << toHex(s) << "\"";
            }
            json << "],";
            json << "\"predecessors\":[";
            bool firstPred = true;
            for (uint64_t p : block.predecessors) {
                if (!firstPred) json << ",";
                firstPred = false;
                json << "\"0x" << toHex(p) << "\"";
            }
            json << "],";
            json << "\"isLoopHeader\":" << (block.isLoopHeader ? "true" : "false") << ",";
            json << "\"dominators\":[";
            bool firstDom = true;
            for (uint64_t d : block.dominators) {
                if (!firstDom) json << ",";
                firstDom = false;
                json << "\"0x" << toHex(d) << "\"";
            }
            json << "]";
            json << "}";
        }
        json << "],";
        json << "\"edges\":[";
        first = true;
        for (const auto& edge : cfgResult.edges) {
            if (!first) json << ",";
            first = false;
            json << "{";
            json << "\"from\":\"0x" << toHex(edge.from) << "\",";
            json << "\"to\":\"0x" << toHex(edge.to) << "\",";
            json << "\"type\":\"" << edgeTypeStr(edge.type) << "\"";
            json << "}";
        }
        json << "],";
        json << "\"unresolvedEdges\":[";
        first = true;
        for (const auto& [from, to] : cfgResult.unresolvedEdges) {
            if (!first) json << ",";
            first = false;
            json << "{\"from\":\"0x" << toHex(from) << "\",\"to\":\"0x" << toHex(to) << "\"}";
        }
        json << "],";
        json << "\"totalBlocks\":" << cfgResult.totalBlocks << ",";
        json << "\"totalEdges\":" << cfgResult.totalEdges << ",";
        json << "\"totalUnresolved\":" << cfgResult.unresolvedEdges.size() << ",";
        json << "\"dfsTraversal\":[";
        first = true;
        for (uint64_t addr : dfsResult.visitedAddresses) {
            if (!first) json << ",";
            first = false;
            json << "\"0x" << toHex(addr) << "\"";
        }
        json << "],";
        json << "\"stronglyConnectedComponents\":[";
        first = true;
        for (const auto& comp : sccResult.components) {
            if (!first) json << ",";
            first = false;
            json << "[";
            bool firstAddr = true;
            for (uint64_t addr : comp) {
                if (!firstAddr) json << ",";
                firstAddr = false;
                json << "\"0x" << toHex(addr) << "\"";
            }
            json << "]";
        }
        json << "],";
        json << "\"dominatorTree\":[";
        first = true;
        for (uint64_t addr : domTreeResult.dominatorTree) {
            if (!first) json << ",";
            first = false;
            json << "\"0x" << toHex(addr) << "\"";
        }
        json << "],";
        json << "\"detectedLoops\":[";
        first = true;
        for (const auto& loop : loopResult.loops) {
            if (!first) json << ",";
            first = false;
            json << "[";
            bool firstAddr = true;
            for (uint64_t addr : loop) {
                if (!firstAddr) json << ",";
                firstAddr = false;
                json << "\"0x" << toHex(addr) << "\"";
            }
            json << "]";
        }
        json << "],";
        json << "\"loopCount\":" << loopResult.loopCount << ",";
        json << "\"componentCount\":" << sccResult.componentCount;
        json << "}";

        result.success = true;
        result.output = json.str();
        result.metadata["block_count"] = std::to_string(cfgResult.totalBlocks);
        result.metadata["edge_count"] = std::to_string(cfgResult.totalEdges);
        result.metadata["loop_count"] = std::to_string(loopResult.loopCount);
        result.metadata["component_count"] = std::to_string(sccResult.componentCount);
        return result;
    }

    void onUnload() override {}

private:
    static std::string toHex(uint64_t val) {
        std::ostringstream oss;
        oss << std::hex << val;
        return oss.str();
    }

    static const char* edgeTypeStr(omnibyte::hydradis::EdgeType t) {
        switch (t) {
            case omnibyte::hydradis::EdgeType::FallThrough: return "fallthrough";
            case omnibyte::hydradis::EdgeType::Branch:      return "branch";
            case omnibyte::hydradis::EdgeType::Call:        return "call";
            case omnibyte::hydradis::EdgeType::Return:      return "return";
            default:                                        return "unknown";
        }
    }
};

extern "C" std::unique_ptr<IPlugin> create_enhanced_cfg_plugin() {
    return std::make_unique<EnhancedCfgPlugin>();
}

extern "C" int enhanced_cfg_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

#include "Plugin/IPlugin.h"
#include <sstream>
#include <map>
#include <set>
#include <algorithm>

namespace omnibyte::hydradis::plugin {

struct BasicBlock {
    uint64_t startAddr = 0;
    uint64_t endAddr = 0;
    std::vector<const Instruction*> instructions;
    std::set<uint64_t> successors;
    std::set<uint64_t> predecessors;
};

enum class EdgeType {
    FallThrough,
    Branch,
    Call,
    Return
};

struct Edge {
    uint64_t from = 0;
    uint64_t to = 0;
    EdgeType type = EdgeType::FallThrough;
};

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

        std::map<uint64_t, const Instruction*> instrMap;
        std::map<uint64_t, const Instruction*> branchTargets;

        for (const auto& sec : *ctx.disassemblyResults) {
            for (const auto& instr : sec.instructions) {
                instrMap[instr.address] = &instr;
            }
        }

        std::set<uint64_t> leaders;
        leaders.insert(instrMap.begin()->first);

        for (const auto& [addr, instr] : instrMap) {
            if (isBranch(instr->mnemonic)) {
                uint64_t target = parseTarget(instr->opStr);
                if (target != 0 && instrMap.find(target) != instrMap.end()) {
                    leaders.insert(target);
                }
                auto next = instrMap.upper_bound(addr);
                if (next != instrMap.end()) {
                    leaders.insert(next->first);
                }
            }
        }

        std::map<uint64_t, BasicBlock> blocks;
        for (const auto& [addr, instr] : instrMap) {
            if (leaders.count(addr) || blocks.empty()) {
                auto& block = blocks[addr];
                block.startAddr = addr;
            }
            auto& block = *blocks.rbegin();
            block.second.instructions.push_back(instr);
            block.second.endAddr = addr;
        }

        std::vector<Edge> edges;
        for (auto& [addr, block] : blocks) {
            if (block.instructions.empty()) continue;
            const auto* last = block.instructions.back();

            if (last->mnemonic == "ret") {
                edges.push_back({addr, 0, EdgeType::Return});
            } else if (isBranch(last->mnemonic)) {
                uint64_t target = parseTarget(last->opStr);
                if (target != 0 && blocks.find(target) != blocks.end()) {
                    block.successors.insert(target);
                    blocks[target].predecessors.insert(addr);
                    edges.push_back({addr, target, EdgeType::Branch});
                }
                if (!isUnconditionalBranch(last->mnemonic)) {
                    auto next = instrMap.upper_bound(addr);
                    if (next != instrMap.end() && blocks.find(next->first) != blocks.end()) {
                        block.successors.insert(next->first);
                        blocks[next->first].predecessors.insert(addr);
                        edges.push_back({addr, next->first, EdgeType::FallThrough});
                    }
                }
            } else if (isCall(last->mnemonic)) {
                auto next = instrMap.upper_bound(addr);
                if (next != instrMap.end() && blocks.find(next->first) != blocks.end()) {
                    block.successors.insert(next->first);
                    blocks[next->first].predecessors.insert(addr);
                    edges.push_back({addr, next->first, EdgeType::Call});
                }
            } else {
                auto next = instrMap.upper_bound(addr);
                if (next != instrMap.end() && blocks.find(next->first) != blocks.end()) {
                    block.successors.insert(next->first);
                    blocks[next->first].predecessors.insert(addr);
                    edges.push_back({addr, next->first, EdgeType::FallThrough});
                }
            }
        }

        std::ostringstream json;
        json << "{";
        json << "\"blocks\":[";
        bool first = true;
        for (const auto& [addr, block] : blocks) {
            if (!first) json << ",";
            first = false;
            json << "{";
            json << "\"start\":\"0x" << toHex(block.startAddr) << "\",";
            json << "\"end\":\"0x" << toHex(block.endAddr) << "\",";
            json << "\"instructionCount\":" << block.instructions.size() << ",";
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
            json << "]";
            json << "}";
        }
        json << "],";
        json << "\"edges\":[";
        first = true;
        for (const auto& edge : edges) {
            if (!first) json << ",";
            first = false;
            json << "{";
            json << "\"from\":\"0x" << toHex(edge.from) << "\",";
            json << "\"to\":\"0x" << toHex(edge.to) << "\",";
            json << "\"type\":\"" << edgeTypeStr(edge.type) << "\"";
            json << "}";
        }
        json << "],";
        json << "\"totalBlocks\":" << blocks.size() << ",";
        json << "\"totalEdges\":" << edges.size();
        json << "}";

        result.success = true;
        result.output = json.str();
        result.metadata["block_count"] = std::to_string(blocks.size());
        result.metadata["edge_count"] = std::to_string(edges.size());
        return result;
    }

    void onUnload() override {}

private:
    static bool isBranch(const std::string& m) {
        return m == "b" || m == "beq" || m == "bne" || m == "blt" ||
               m == "bge" || m == "ble" || m == "bgt" || m == "bhs" ||
               m == "blo" || m == "bhi" || m == "bls" || m == "bpl" ||
               m == "bmi" || m == "bvs" || m == "bvc" || m == "bcs" ||
               m == "bcc" || m == "br" || m == "cbz" || m == "cbnz";
    }

    static bool isUnconditionalBranch(const std::string& m) {
        return m == "b" || m == "br";
    }

    static bool isCall(const std::string& m) {
        return m == "bl" || m == "blr";
    }

    static uint64_t parseTarget(const std::string& opStr) {
        std::string s = opStr;
        if (!s.empty() && s[0] == '#') s = s.substr(1);
        if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            try {
                return std::stoull(s, nullptr, 16);
            } catch (...) {
                return 0;
            }
        }
        try {
            return std::stoull(s, nullptr, 0);
        } catch (...) {
            return 0;
        }
    }

    static std::string toHex(uint64_t val) {
        std::ostringstream oss;
        oss << std::hex << val;
        return oss.str();
    }

    static const char* edgeTypeStr(EdgeType t) {
        switch (t) {
            case EdgeType::FallThrough: return "fallthrough";
            case EdgeType::Branch:      return "branch";
            case EdgeType::Call:        return "call";
            case EdgeType::Return:      return "return";
            default:                    return "unknown";
        }
    }
};

extern "C" std::unique_ptr<IPlugin> create_enhanced_cfg_plugin() {
    return std::make_unique<EnhancedCfgPlugin>();
}

extern "C" int enhanced_cfg_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

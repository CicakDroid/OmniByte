#include "Plugin/IPlugin.h"
#include <sstream>
#include <map>
#include <set>
#include <algorithm>

namespace omnibyte::hydradis::plugin {

enum class AstNodeType {
    BLOCK,
    SEQUENCE,
    LOOP,
    IF_ELSE,
    CALL,
    ASSIGN,
    BRANCH,
    UNKNOWN
};

struct AstNode {
    AstNodeType type = AstNodeType::UNKNOWN;
    uint64_t address = 0;
    std::string label;
    std::vector<AstNode*> children;

    ~AstNode() {
        for (auto* child : children) delete child;
    }
};

class EnhancedAstPlugin : public IPlugin {
public:
    std::string name() const override { return "Enhanced/AST"; }
    std::string version() const override { return "1.0.0"; }

    bool onLoad() override { return true; }

    PluginResult onRun(const PluginContext& ctx) override {
        PluginResult result;

        if (!ctx.disassemblyResults || ctx.disassemblyResults->empty()) {
            result.errorMessage = "No disassembly data available";
            return result;
        }

        std::map<uint64_t, const Instruction*> instrMap;
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

        std::map<uint64_t, std::vector<const Instruction*>> blocks;
        uint64_t currentBlockStart = 0;

        for (const auto& [addr, instr] : instrMap) {
            if (leaders.count(addr)) {
                currentBlockStart = addr;
            }
            blocks[currentBlockStart].push_back(instr);
        }

        std::set<uint64_t> visited;
        AstNode* root = buildAst(blocks, instrMap, leaders, visited);

        std::ostringstream json;
        json << "{";
        json << "\"ast\":";
        emitJson(json, root);
        json << ",";
        json << "\"totalNodes\":" << countNodes(root);
        json << "}";

        delete root;

        result.success = true;
        result.output = json.str();
        result.metadata["node_count"] = std::to_string(countNodes(root));
        return result;
    }

    void onUnload() override {}

private:
    AstNode* buildAst(
        const std::map<uint64_t, std::vector<const Instruction*>>& blocks,
        const std::map<uint64_t, const Instruction*>& instrMap,
        const std::set<uint64_t>& leaders,
        std::set<uint64_t>& visited
    ) {
        if (blocks.empty()) return nullptr;

        auto* root = new AstNode();
        root->type = AstNodeType::BLOCK;
        root->address = blocks.begin()->first;

        for (const auto& [addr, instrs] : blocks) {
            if (visited.count(addr)) continue;
            visited.insert(addr);

            const auto* last = instrs.back();

            if (last->mnemonic == "ret") {
                auto* block = new AstNode();
                block->type = AstNodeType::SEQUENCE;
                block->address = addr;
                for (const auto* instr : instrs) {
                    auto* leaf = new AstNode();
                    leaf->type = AstNodeType::UNKNOWN;
                    leaf->address = instr->address;
                    leaf->label = instr->mnemonic + " " + instr->opStr;
                    block->children.push_back(leaf);
                }
                root->children.push_back(block);
            } else if (isBranch(last->mnemonic) && !isUnconditionalBranch(last->mnemonic)) {
                uint64_t target = parseTarget(last->opStr);
                if (target != 0 && blocks.find(target) != blocks.end()) {
                    auto* ifNode = new AstNode();
                    ifNode->type = AstNodeType::IF_ELSE;
                    ifNode->address = addr;
                    ifNode->label = last->mnemonic + " " + last->opStr;

                    auto* condBlock = new AstNode();
                    condBlock->type = AstNodeType::SEQUENCE;
                    condBlock->address = addr;
                    for (size_t i = 0; i < instrs.size() - 1; ++i) {
                        auto* leaf = new AstNode();
                        leaf->type = AstNodeType::UNKNOWN;
                        leaf->address = instrs[i]->address;
                        leaf->label = instrs[i]->mnemonic + " " + instrs[i]->opStr;
                        condBlock->children.push_back(leaf);
                    }
                    ifNode->children.push_back(condBlock);

                    auto* thenBranch = buildAst(
                        filterBlocks(blocks, addr, target),
                        instrMap, leaders, visited);
                    if (thenBranch) ifNode->children.push_back(thenBranch);

                    auto next = instrMap.upper_bound(addr);
                    if (next != instrMap.end() && blocks.find(next->first) != blocks.end()) {
                        auto* elseBranch = buildAst(
                            filterBlocks(blocks, addr, next->first),
                            instrMap, leaders, visited);
                        if (elseBranch) ifNode->children.push_back(elseBranch);
                    }

                    root->children.push_back(ifNode);
                }
            } else if (last->mnemonic == "bl" || last->mnemonic == "blr") {
                auto* callNode = new AstNode();
                callNode->type = AstNodeType::CALL;
                callNode->address = addr;
                callNode->label = last->mnemonic + " " + last->opStr;
                root->children.push_back(callNode);

                auto next = instrMap.upper_bound(addr);
                if (next != instrMap.end() && leaders.count(next->first) && !visited.count(next->first)) {
                    auto* afterCall = buildAst(
                        filterBlocks(blocks, addr, next->first),
                        instrMap, leaders, visited);
                    if (afterCall) root->children.push_back(afterCall);
                }
            } else {
                auto* block = new AstNode();
                block->type = AstNodeType::SEQUENCE;
                block->address = addr;
                for (const auto* instr : instrs) {
                    auto* leaf = new AstNode();
                    leaf->type = AstNodeType::UNKNOWN;
                    leaf->address = instr->address;
                    leaf->label = instr->mnemonic + " " + instr->opStr;
                    block->children.push_back(leaf);
                }
                root->children.push_back(block);
            }
        }

        return root;
    }

    std::map<uint64_t, std::vector<const Instruction*>> filterBlocks(
        const std::map<uint64_t, std::vector<const Instruction*>>& blocks,
        uint64_t from,
        uint64_t to
    ) const {
        std::map<uint64_t, std::vector<const Instruction*>> filtered;
        for (const auto& [addr, instrs] : blocks) {
            if (addr >= from && addr <= to) {
                filtered[addr] = instrs;
            }
        }
        return filtered;
    }

    bool isBranch(const std::string& m) const {
        return m == "b" || m == "beq" || m == "bne" || m == "blt" ||
               m == "bge" || m == "ble" || m == "bgt" || m == "bhs" ||
               m == "blo" || m == "bhi" || m == "bls" || m == "bpl" ||
               m == "bmi" || m == "bvs" || m == "bvc" || m == "bcs" ||
               m == "bcc" || m == "br" || m == "cbz" || m == "cbnz";
    }

    bool isUnconditionalBranch(const std::string& m) const {
        return m == "b" || m == "br";
    }

    uint64_t parseTarget(const std::string& opStr) const {
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

    void emitJson(std::ostringstream& json, const AstNode* node) const {
        if (!node) {
            json << "null";
            return;
        }
        json << "{";
        json << "\"type\":\"" << nodeTypeStr(node->type) << "\",";
        json << "\"address\":\"0x" << toHex(node->address) << "\",";
        json << "\"label\":\"" << escapeJson(node->label) << "\",";
        json << "\"children\":[";
        bool first = true;
        for (const auto* child : node->children) {
            if (!first) json << ",";
            first = false;
            emitJson(json, child);
        }
        json << "]";
        json << "}";
    }

    int countNodes(const AstNode* node) const {
        if (!node) return 0;
        int count = 1;
        for (const auto* child : node->children) {
            count += countNodes(child);
        }
        return count;
    }

    static const char* nodeTypeStr(AstNodeType t) {
        switch (t) {
            case AstNodeType::BLOCK:    return "block";
            case AstNodeType::SEQUENCE: return "sequence";
            case AstNodeType::LOOP:     return "loop";
            case AstNodeType::IF_ELSE:  return "if_else";
            case AstNodeType::CALL:     return "call";
            case AstNodeType::ASSIGN:   return "assign";
            case AstNodeType::BRANCH:   return "branch";
            default:                    return "unknown";
        }
    }

    static std::string toHex(uint64_t val) {
        std::ostringstream oss;
        oss << std::hex << val;
        return oss.str();
    }

    static std::string escapeJson(const std::string& s) {
        std::string result;
        result.reserve(s.size() + 8);
        for (char c : s) {
            switch (c) {
                case '"':  result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default:   result += c;
            }
        }
        return result;
    }
};

extern "C" std::unique_ptr<IPlugin> create_enhanced_ast_plugin() {
    return std::make_unique<EnhancedAstPlugin>();
}

extern "C" int enhanced_ast_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

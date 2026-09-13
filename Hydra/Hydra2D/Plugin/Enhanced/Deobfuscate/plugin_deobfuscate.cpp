#include "Plugin/IPlugin.h"
#include "Disassembler/IDisassembler.h"
#include "Parser/IParser.h"
#include "DexKit/DexKitAdapter.h"
#include "hrtng/HrtngDeob.h"
#include <sstream>
#include <map>
#include <algorithm>

namespace omnibyte::hydradis::plugin {

struct OpcodeSubstitution {
    std::string obfuscated;
    std::string canonical;
    std::string description;
};

static const OpcodeSubstitution OPCODE_SUBSTITUTIONS[] = {
    {"mov x0, #0", "mov x0, #0", "zero register"},
    {"mov x1, #0", "mov x1, #0", "zero register"},
    {"mov x2, #0", "mov x2, #0", "zero register"},
    {"mov x3, #0", "mov x3, #0", "zero register"},
    {"mov x4, #0", "mov x4, #0", "zero register"},
    {"mov x5, #0", "mov x5, #0", "zero register"},
    {"mov x6, #0", "mov x6, #0", "zero register"},
    {"mov x7, #0", "mov x7, #0", "zero register"},
    {"add x0, x0, #1", "add x0, x0, #1", "increment"},
    {"add x1, x1, #1", "add x1, x1, #1", "increment"},
    {"add x2, x2, #1", "add x2, x2, #1", "increment"},
    {"sub x0, x0, #1", "sub x0, x0, #1", "decrement"},
    {"sub x1, x1, #1", "sub x1, x1, #1", "decrement"},
    {"sub x2, x2, #1", "sub x2, x2, #1", "decrement"},
    {"nop", "nop", "no-op"},
};

class DeobfuscatePlugin : public IPlugin {
public:
    std::string name() const override { return "Deobfuscate"; }
    std::string version() const override { return "3.0.0"; }

    bool onLoad() override { return true; }

    PluginResult onRun(const PluginContext& ctx) override {
        PluginResult result;

        if (!ctx.binary) {
            result.errorMessage = "no parsed binary available";
            return result;
        }

        std::ostringstream output;
        output << "{\"plugin\":\"Deobfuscate\",\"analyses\":[";

        bool first = true;

        if (ctx.disassembly) {
            const auto& instrs = ctx.disassembly->instructions;
            for (const auto& instr : instrs) {
                std::string fullInstr = instr.mnemonic + " " + instr.opStr;
                for (const auto& sub : OPCODE_SUBSTITUTIONS) {
                    if (fullInstr == sub.obfuscated && sub.obfuscated != sub.canonical) {
                        if (!first) output << ",";
                        first = false;
                        output << "{\"type\":\"opcode_substitution\","
                               << "\"address\":\"0x" << std::hex << instr.address << "\","
                               << "\"obfuscated\":\"" << escapeJson(sub.obfuscated) << "\","
                               << "\"canonical\":\"" << escapeJson(sub.canonical) << "\","
                               << "\"description\":\"" << escapeJson(sub.description) << "\","
                               << "\"confidence\":0.7}";
                        break;
                    }
                }

                if (instr.mnemonic == "nop" ||
                    (instr.mnemonic == "mov" && instr.opStr.find("x0") != std::string::npos &&
                     instr.opStr.find(", x0") != std::string::npos)) {
                    if (!first) output << ",";
                    first = false;
                    output << "{\"type\":\"junk_code\","
                           << "\"address\":\"0x" << std::hex << instr.address << "\","
                           << "\"instruction\":\"" << escapeJson(instr.mnemonic + " " + instr.opStr) << "\","
                           << "\"confidence\":0.95}";
                }

                if (instr.mnemonic == "str" || instr.mnemonic == "stp") {
                    std::string reg;
                    size_t spacePos = instr.opStr.find(' ');
                    if (spacePos != std::string::npos) {
                        reg = instr.opStr.substr(0, spacePos);
                    }
                    auto next = std::find_if(instrs.begin(), instrs.end(),
                        [&](const auto& i) { return i.address > instr.address; });
                    if (next != instrs.end() && !reg.empty()) {
                        if ((next->mnemonic == "str" || next->mnemonic == "stp") &&
                            next->opStr.find(reg) != std::string::npos) {
                            if (!first) output << ",";
                            first = false;
                            output << "{\"type\":\"dead_store\","
                                   << "\"address\":\"0x" << std::hex << instr.address << "\","
                                   << "\"instruction\":\"" << escapeJson(instr.mnemonic + " " + instr.opStr) << "\","
                                   << "\"overwrittenBy\":\"0x" << std::hex << next->address << "\","
                                   << "\"confidence\":0.85}";
                        }
                    }
                }
            }
        }

        if (ctx.binary) {
            for (const auto& sym : ctx.binary->symbols) {
                if (sym.name.find("obfuscat") != std::string::npos ||
                    sym.name.find("decrypt") != std::string::npos) {
                    if (!first) output << ",";
                    first = false;
                    output << "{\"type\":\"obfuscation_symbol\","
                           << "\"name\":\"" << escapeJson(sym.name) << "\","
                           << "\"address\":\"0x" << std::hex << sym.value << "\"}";
                }
            }
        }

        output << "]}";
        result.success = true;
        result.output = output.str();
        return result;
    }

    void onUnload() override {}

private:
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

extern "C" std::unique_ptr<IPlugin> create_deobfuscate_plugin() {
    return std::make_unique<DeobfuscatePlugin>();
}

extern "C" int plugin_deobfuscate_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

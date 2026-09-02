#include "Plugin/IPlugin.h"
#include "Disassembler/IDisassembler.h"
#include "Parser/IParser.h"
#include "Deobfuscate/DexKit/DexKitAdapter.h"
#include "Deobfuscate/hrtng/HrtngDeob.h"
#include <sstream>

namespace omnibyte::hydradis::plugin {

class DeobfuscatePlugin : public IPlugin {
public:
    std::string name() const override { return "Deobfuscate"; }

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
                if (instr.mnemonic == "eor" || instr.mnemonic == "xor") {
                    if (!first) output << ",";
                    first = false;
                    output << "{\"type\":\"xor_obfuscation\","
                           << "\"address\":\"0x" << std::hex << instr.address << "\","
                           << "\"instruction\":\"" << instr.mnemonic << " " << instr.opStr << "\"}";
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
                           << "\"name\":\"" << sym.name << "\","
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
};

extern "C" std::unique_ptr<IPlugin> create_deobfuscate_plugin() {
    return std::make_unique<DeobfuscatePlugin>();
}

extern "C" int plugin_deobfuscate_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

#include "Plugin/IPlugin.h"
#include "Disassembler/IDisassembler.h"
#include "Plugin/Enhanced/Crypt/FindCrypt3/FindCrypt3.h"
#include <sstream>

namespace omnibyte::hydradis::plugin {

class EnhancedFindCryptPlugin : public IPlugin {
public:
    std::string name() const override { return "Enhanced/Crypt"; }
    std::string version() const override { return "1.0.0"; }

    bool onLoad() override { return true; }

    PluginResult onRun(const PluginContext& ctx) override {
        PluginResult result;

        if (!ctx.disassembly) {
            result.errorMessage = "no disassembly available for crypto scan";
            return result;
        }

        const auto& instrs = ctx.disassembly->instructions;
        if (instrs.empty()) {
            result.errorMessage = "empty instruction stream";
            return result;
        }

        omnibyte::deob::FindCrypt3Engine engine;

        std::vector<omnibyte::deob::SimpleInstruction> sInstrs;
        sInstrs.reserve(instrs.size());
        for (const auto& instr : instrs) {
            sInstrs.push_back({instr.address, instr.mnemonic, instr.opStr});
        }

        std::vector<uint8_t> buffer;
        for (const auto& instr : instrs) {
            buffer.insert(buffer.end(), instr.bytes.begin(), instr.bytes.end());
        }
        auto hits = engine.scanRegion(buffer.data(), buffer.size());

        auto instrHits = engine.scanInstructions(sInstrs);

        std::ostringstream output;
        output << "{\"plugin\":\"Enhanced/Crypt\",\"constant_hits\":[";
        for (size_t i = 0; i < hits.size(); ++i) {
            if (i > 0) output << ",";
            output << "{\"offset\":\"0x" << std::hex << hits[i].offset << "\","
                   << "\"algorithm\":\"" << hits[i].algorithm << "\","
                   << "\"confidence\":" << hits[i].confidence << "}";
        }
        output << "],\"pattern_hits\":[";
        for (size_t i = 0; i < instrHits.size(); ++i) {
            if (i > 0) output << ",";
            output << "{\"offset\":\"0x" << std::hex << instrHits[i].offset << "\","
                   << "\"algorithm\":\"" << instrHits[i].algorithm << "\","
                   << "\"confidence\":" << instrHits[i].confidence << "}";
        }
        output << "],\"status\":\"scan_complete\"}";

        result.success = true;
        result.output = output.str();
        return result;
    }

    void onUnload() override {}
};

extern "C" std::unique_ptr<IPlugin> create_enhanced_findcrypt_plugin() {
    return std::make_unique<EnhancedFindCryptPlugin>();
}

extern "C" int enhanced_findcrypt_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

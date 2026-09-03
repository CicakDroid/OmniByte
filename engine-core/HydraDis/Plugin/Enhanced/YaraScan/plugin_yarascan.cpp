#include "Plugin/IPlugin.h"
#include "Disassembler/IDisassembler.h"
#include "YaraScan/YaraScan.h"
#include <sstream>

namespace omnibyte::hydradis::plugin {

class YaraScanPlugin : public IPlugin {
public:
    std::string name() const override { return "Enhanced/YaraScan"; }
    std::string version() const override { return "1.0.0"; }

    bool onLoad() override {
        return engine_.initialize();
    }

    PluginResult onRun(const PluginContext& ctx) override {
        PluginResult result;

        if (!ctx.disassembly) {
            result.errorMessage = "no disassembly available for YARA scan";
            return result;
        }

        const auto& instrs = ctx.disassembly->instructions;
        if (instrs.empty()) {
            result.errorMessage = "empty instruction stream";
            return result;
        }

        if (!engine_.isInitialized()) {
            result.errorMessage = "YARA engine not initialized (libyara not available?)";
            return result;
        }

        std::vector<uint8_t> buffer;
        for (const auto& instr : instrs) {
            buffer.insert(buffer.end(), instr.bytes.begin(), instr.bytes.end());
        }

        auto hits = engine_.scanRegion(buffer.data(), buffer.size());

        std::ostringstream output;
        output << "{\"plugin\":\"Enhanced/YaraScan\",\"hits\":[";
        for (size_t i = 0; i < hits.size(); ++i) {
            if (i > 0) output << ",";
            output << "{\"offset\":\"0x" << std::hex << hits[i].offset << "\","
                   << "\"rule\":\"" << hits[i].ruleName << "\","
                   << "\"confidence\":" << hits[i].confidence << "}";
        }
        output << "],\"status\":\"scan_complete\"}";

        result.success = true;
        result.output = output.str();
        return result;
    }

    void onUnload() override {}

private:
    omnibyte::deob::YaraScanEngine engine_;
};

extern "C" std::unique_ptr<IPlugin> create_yarascan_plugin() {
    return std::make_unique<YaraScanPlugin>();
}

extern "C" int yarascan_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

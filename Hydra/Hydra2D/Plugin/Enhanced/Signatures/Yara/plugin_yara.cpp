// Yara plugin — HydraDis plugin wrapper for YARA scanning.
// Provides YARA-based pattern detection for disassembled code.

#include "Plugin/IPlugin.h"
#include "Yara/YaraEngine.h"
#include "Yara/YaraRulesManager.h"
#include <sstream>

namespace omnibyte::hydradis::plugin {

class YaraPlugin : public IPlugin {
public:
    std::string name() const override { return "Enhanced/Yara"; }
    std::string version() const override { return "2.0.0"; }

    bool onLoad() override {
        omnibyte::signatures::YaraConfig config;
        config.useBundledLib = true;
        
        if (!engine_.initialize(config)) {
            return false;
        }

        // Load rules from default directory
        rulesManager_.setRulesDirectory("rules");
        rulesManager_.refresh();
        
        return true;
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

        if (!engine_.isReady()) {
            result.errorMessage = "YARA engine not initialized (libyara not available?)";
            return result;
        }

        // Build buffer from instructions
        std::vector<uint8_t> buffer;
        for (const auto& instr : instrs) {
            buffer.insert(buffer.end(), instr.bytes.begin(), instr.bytes.end());
        }

        // Scan with YARA
        auto hits = engine_.scanRegion(buffer.data(), buffer.size());

        // Build output
        std::ostringstream output;
        output << "{\"plugin\":\"Enhanced/Yara\","
               << "\"status\":\"scan_complete\","
               << "\"hits\":[";
        
        for (size_t i = 0; i < hits.size(); ++i) {
            if (i > 0) output << ",";
            output << "{\"offset\":\"0x" << std::hex << hits[i].offset << "\","
                   << "\"rule\":\"" << hits[i].ruleName << "\","
                   << "\"confidence\":" << hits[i].confidence;
            
            if (!hits[i].tags.empty()) {
                output << ",\"tags\":\"" << hits[i].tags << "\"";
            }
            
            output << "}";
        }
        
        output << "],"
               << "\"rules_loaded\":" << rulesManager_.totalRuleCount() << "}";

        result.success = true;
        result.output = output.str();
        return result;
    }

    void onUnload() override {
        engine_.shutdown();
    }

private:
    omnibyte::signatures::YaraEngine engine_;
    omnibyte::signatures::YaraRulesManager rulesManager_;
};

extern "C" std::unique_ptr<IPlugin> create_yara_plugin() {
    return std::make_unique<YaraPlugin>();
}

extern "C" int yara_plugin_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

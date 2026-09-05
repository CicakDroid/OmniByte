// ── enhanced_emulation.cpp ─────────────────────────────────────────
// Enhanced/Emulation — CPU emulation via Unicorn Engine (v2.1.4)
// dengan anti-detection mode via qemu-anti-detection wrapper.
// Source: unicorn-engine/unicorn, zhaodice/qemu-anti-detection

#include "Plugin/IPlugin.h"
#include "Enhanced/Emulation/Unicorn/unicorn_engine.h"
#include "Enhanced/Emulation/Qemu/qemu_antidetect.h"
#include <sstream>
#include <chrono>

namespace omnibyte::hydradis::plugin {

class EnhancedEmulationPlugin : public IPlugin {
public:
    std::string name() const override { return "Enhanced/Emulation"; }
    std::string version() const override { return "2.0.0"; }

    bool onLoad() override { return true; }

    PluginResult onRun(const PluginContext& ctx) override {
        PluginResult result;

        // ── Validasi input ───────────────────────────────────────
        if (!ctx.disassemblyResults || ctx.disassemblyResults->empty()) {
            result.errorMessage = "no disassembly data available for emulation";
            return result;
        }

        std::vector<omnibyte::hydradis::Instruction> allInstructions;
        for (const auto& sec : *ctx.disassemblyResults) {
            allInstructions.insert(allInstructions.end(),
                                  sec.instructions.begin(),
                                  sec.instructions.end());
        }

        if (allInstructions.empty()) {
            result.errorMessage = "no instructions to emulate";
            return result;
        }

        // ── Deteksi arsitektur ──────────────────────────────────
        int arch = 2;  // ARM64 default

        if (ctx.binary) {
            switch (ctx.binary->header.machine) {
                case 0xB7: arch = 2; break;
                case 0x28: arch = 0; break;
                case 0x03: arch = 3; break;
                case 0x3E: arch = 4; break;
                default:   arch = 2; break;
            }
        }

        // ── Baca konfigurasi ────────────────────────────────────
        uint64_t timeoutUs = 30000000;
        size_t maxInstr = 100000;
        bool antiDetectEnabled = false;
        uint64_t codeBase = 0x10000;

        std::string timeoutStr = ctx.getConfig("emulation_timeout_us", "30000000");
        std::string maxInstrStr = ctx.getConfig("emulation_max_instr", "100000");
        std::string antiDetectStr = ctx.getConfig("emulation_antidetect", "false");
        std::string codeBaseStr = ctx.getConfig("emulation_code_base", "0x10000");

        try { timeoutUs = std::stoull(timeoutStr); } catch (...) {}
        try { maxInstr = std::stoull(maxInstrStr); } catch (...) {}
        try { codeBase = std::stoull(codeBaseStr, nullptr, 0); } catch (...) {}
        antiDetectEnabled = (antiDetectStr == "true" || antiDetectStr == "1");

        // ── Inisialisasi Unicorn Engine ──────────────────────────
        omnibyte::hydradis::UnicornEngine unicorn;

        if (!unicorn.initialize(arch)) {
            result.errorMessage = "failed to initialize Unicorn Engine for arch " +
                                  std::to_string(arch) + " (is Unicorn installed?)";
            return result;
        }

        // ── Konfigurasi anti-detection ───────────────────────────
        omnibyte::hydradis::QemuAntiDetect antiDetect;
        if (antiDetectEnabled) {
            antiDetect.configureDefaults(arch);
        }

        // ── Setup emulation config ──────────────────────────────
        omnibyte::hydradis::UnicornConfig emuConfig;
        emuConfig.codeBaseAddress = codeBase;
        emuConfig.timeoutUs = timeoutUs;
        emuConfig.maxInstructions = maxInstr;
        emuConfig.hookMemoryAccess = true;
        emuConfig.hookInvalidAccess = true;

        // ── Eksekusi! ────────────────────────────────────────────
        auto startTime = std::chrono::steady_clock::now();

        auto emuResult = unicorn.execute(allInstructions, emuConfig);

        auto endTime = std::chrono::steady_clock::now();
        auto totalUs = std::chrono::duration_cast<std::chrono::microseconds>(
            endTime - startTime).count();

        if (!emuResult.success) {
            result.errorMessage = "emulation failed: " + emuResult.errorMessage;
            return result;
        }

        // ── Terapkan anti-detection ke hasil ─────────────────────
        if (antiDetectEnabled) {
            antiDetect.applyToRegisters(emuResult.finalRegisters);
        }

        // ── Format output JSON ───────────────────────────────────
        std::ostringstream json;
        json << "{";
        json << "\"plugin\":\"Enhanced/Emulation\",";
        json << "\"arch\":" << arch << ",";
        json << "\"instructionsTotal\":" << allInstructions.size() << ",";
        json << "\"instructionsEmulated\":" << emuResult.instructionsExecuted << ",";
        json << "\"executionTimeUs\":" << emuResult.executionTimeUs << ",";
        json << "\"wallTimeUs\":" << totalUs << ",";
        json << "\"antiDetect\":" << (antiDetectEnabled ? "true" : "false") << ",";

        json << "\"terminated\":" << (emuResult.terminated ? "true" : "false") << ",";
        json << "\"terminationReason\":\"" << escapeJson(emuResult.terminationReason) << "\",";

        json << "\"finalRegisters\":{";
        bool firstReg = true;
        for (const auto& [name, val] : emuResult.finalRegisters) {
            if (!firstReg) json << ",";
            firstReg = false;
            json << "\"" << name << "\":\"0x" << std::hex << val << "\"";
        }
        json << "},";

        size_t traceLimit = 1000;
        size_t traceSize = std::min(emuResult.trace.size(), traceLimit);
        bool traceTruncated = emuResult.trace.size() > traceLimit;

        json << "\"trace\":[";
        for (size_t i = 0; i < traceSize; ++i) {
            if (i > 0) json << ",";
            const auto& entry = emuResult.trace[i];
            json << "{";
            json << "\"pc\":\"0x" << std::hex << entry.address << "\",";
            json << "\"insn\":\"" << escapeJson(entry.instructionText) << "\",";
            json << "\"bytes\":\"";
            for (auto b : entry.bytes) {
                json << std::hex << (int)b;
            }
            json << "\"";
            json << "}";
        }
        json << "],";
        json << "\"traceTotalEntries\":" << emuResult.trace.size() << ",";
        json << "\"traceTruncated\":" << (traceTruncated ? "true" : "false");

        json << "}";

        result.success = true;
        result.output = json.str();
        result.metadata["instructions_emulated"] = std::to_string(emuResult.instructionsExecuted);
        result.metadata["execution_time_us"] = std::to_string(emuResult.executionTimeUs);
        result.metadata["arch"] = std::to_string(arch);

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

extern "C" std::unique_ptr<IPlugin> create_enhanced_emulation_plugin() {
    return std::make_unique<EnhancedEmulationPlugin>();
}

extern "C" int enhanced_emulation_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

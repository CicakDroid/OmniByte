#include "Plugin/IPlugin.h"
#include "Analysis/Types.h"
#include <map>
#include <set>
#include <sstream>

namespace omnibyte::hydradis::plugin {

class EnhancedRttiPlugin : public IPlugin {
public:
    std::string name() const override { return "Enhanced/RTTI"; }
    std::string version() const override { return "1.0.0"; }

    bool onLoad() override { return true; }

    PluginResult onRun(const PluginContext& ctx) override {
        PluginResult result;

        if (!ctx.binary) {
            result.errorMessage = "No binary data available";
            return result;
        }

        omnibyte::hydradis::Types typesAnalyzer;
        auto vtablesResult = typesAnalyzer.analyzeVtables(
            ctx.binary->symbols, ctx.binary->sections);

        std::vector<uint8_t> codeData;
        uint64_t entryAddr = ctx.binary->header.entryPoint;

        if (ctx.disassemblyResults && !ctx.disassemblyResults->empty()) {
            for (const auto& sec : *ctx.disassemblyResults) {
                for (const auto& instr : sec.instructions) {
                    if (!instr.bytes.empty() && entryAddr == ctx.binary->header.entryPoint) {
                        entryAddr = instr.address;
                    }
                }
            }

            std::map<uint64_t, const Instruction*> instrMap;
            for (const auto& sec : *ctx.disassemblyResults) {
                for (const auto& instr : sec.instructions) {
                    instrMap[instr.address] = &instr;
                }
            }
            if (!instrMap.empty()) {
                uint64_t lowest = instrMap.begin()->first;
                uint64_t highest = instrMap.rbegin()->first;
                codeData.resize(static_cast<size_t>(highest - lowest) + 4, 0);
                for (const auto& [addr, instr] : instrMap) {
                    size_t offset = static_cast<size_t>(addr - lowest);
                    if (offset + instr->bytes.size() <= codeData.size()) {
                        std::copy(instr->bytes.begin(), instr->bytes.end(),
                                  codeData.begin() + offset);
                    }
                }
                entryAddr = lowest;
            }
        }

        auto typesResult = typesAnalyzer.analyzeTypes(entryAddr, codeData);

        std::set<std::string> seen;
        std::vector<omnibyte::hydradis::VtableInfo> deduped;
        for (const auto& t : vtablesResult.vtables) {
            if (seen.find(t.mangledName) == seen.end()) {
                seen.insert(t.mangledName);
                deduped.push_back(t);
            }
        }

        for (const auto& [addr, typeName] : typesResult.typeMap) {
            bool found = false;
            for (auto& t : deduped) {
                if (t.vtableAddr == addr) {
                    t.demangledName += " [" + typeName + "]";
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::string name = "type_at_0x" + toHex(addr);
                deduped.push_back({name, name + " [" + typeName + "]", addr, {}, {}, 0,
                    omnibyte::hydradis::VtableDetectionSource::Unknown});
            }
        }

        std::ostringstream json;
        json << "{";
        json << "\"types\":[";
        bool first = true;
        for (const auto& t : deduped) {
            if (!first) json << ",";
            first = false;
            json << "{";
            json << "\"mangled\":\"" << escapeJson(t.mangledName) << "\",";
            json << "\"demangled\":\"" << escapeJson(t.demangledName) << "\",";
            json << "\"vtableAddr\":\"0x" << toHex(t.vtableAddr) << "\",";
            json << "\"baseClasses\":[";
            bool firstBase = true;
            for (const auto& b : t.baseClasses) {
                if (!firstBase) json << ",";
                firstBase = false;
                json << "\"" << escapeJson(b) << "\"";
            }
            json << "],";
            json << "\"vtableSize\":" << t.vtableSize;
            json << "}";
        }
        json << "],";
        json << "\"vtableMapping\":[";
        first = true;
        for (const auto& [addr, className] : vtablesResult.vtableToClass) {
            if (!first) json << ",";
            first = false;
            json << "{";
            json << "\"vtableAddr\":\"0x" << toHex(addr) << "\",";
            json << "\"className\":\"" << escapeJson(className) << "\"";
            json << "}";
        }
        json << "],";
        json << "\"totalTypes\":" << deduped.size() << ",";
        json << "\"inferredTypes\":" << typesResult.inferredTypes << ",";
        json << "\"typeAnalysis\":[";
        first = true;
        for (const auto& [addr, typeName] : typesResult.typeMap) {
            if (!first) json << ",";
            first = false;
            json << "{\"addr\":\"0x" << toHex(addr)
                 << "\",\"type\":\"" << escapeJson(typeName) << "\"}";
        }
        json << "]";
        json << "}";

        result.success = true;
        result.output = json.str();
        result.metadata["type_count"] = std::to_string(deduped.size());
        result.metadata["inferred_type_count"] = std::to_string(typesResult.inferredTypes);
        return result;
    }

    void onUnload() override {}

private:
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

extern "C" std::unique_ptr<IPlugin> create_enhanced_rtti_plugin() {
    return std::make_unique<EnhancedRttiPlugin>();
}

extern "C" int enhanced_rtti_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

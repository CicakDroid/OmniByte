#include "Plugin/IPlugin.h"
#include "Analysis/Types.h"
#include <map>
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
        auto recoveryResult = typesAnalyzer.recoverTypes(
            ctx.binary->symbols, ctx.binary->sections);

        std::ostringstream json;
        json << "{";
        json << "\"types\":[";
        bool first = true;
        for (const auto& ti : recoveryResult.types) {
            if (!first) json << ",";
            first = false;
            json << "{";
            json << "\"className\":\"" << escapeJson(ti.className) << "\",";
            json << "\"mangledName\":\"" << escapeJson(ti.mangledName) << "\",";
            json << "\"vtableAddr\":\"0x" << toHex(ti.vtableAddr) << "\",";
            json << "\"typeinfoAddr\":\"0x" << toHex(ti.typeinfoAddr) << "\",";
            json << "\"baseClasses\":[";
            bool firstBase = true;
            for (const auto& b : ti.baseClasses) {
                if (!firstBase) json << ",";
                firstBase = false;
                json << "\"" << escapeJson(b) << "\"";
            }
            json << "],";
            json << "\"virtualMethods\":[";
            bool firstMethod = true;
            for (const auto& m : ti.virtualMethods) {
                if (!firstMethod) json << ",";
                firstMethod = false;
                json << "{";
                json << "\"offset\":" << m.offset << ",";
                json << "\"targetAddr\":\"0x" << toHex(m.targetAddr) << "\",";
                json << "\"name\":\"" << escapeJson(m.name) << "\",";
                json << "\"isPureVirtual\":" << (m.isPureVirtual ? "true" : "false") << ",";
                json << "\"isDestructor\":" << (m.isDestructor ? "true" : "false");
                json << "}";
            }
            json << "],";
            json << "\"vtableSize\":" << ti.vtableSize << ",";
            json << "\"methodCount\":" << ti.methodCount << ",";
            json << "\"hasRTTI\":" << (ti.hasRTTI ? "true" : "false") << ",";
            json << "\"isAbstract\":" << (ti.isAbstract ? "true" : "false") << ",";
            json << "\"isPolymorphic\":" << (ti.isPolymorphic ? "true" : "false");
            json << "}";
        }
        json << "],";
        json << "\"hierarchy\":[";
        first = true;
        for (const auto& [name, node] : recoveryResult.hierarchy) {
            if (!first) json << ",";
            first = false;
            json << "{";
            json << "\"className\":\"" << escapeJson(node.className) << "\",";
            json << "\"baseClasses\":[";
            bool firstBase = true;
            for (const auto& b : node.baseClasses) {
                if (!firstBase) json << ",";
                firstBase = false;
                json << "\"" << escapeJson(b) << "\"";
            }
            json << "],";
            json << "\"derivedClasses\":[";
            bool firstDerived = true;
            for (const auto& d : node.derivedClasses) {
                if (!firstDerived) json << ",";
                firstDerived = false;
                json << "\"" << escapeJson(d) << "\"";
            }
            json << "],";
            json << "\"hasVtable\":" << (node.hasVtable ? "true" : "false") << ",";
            json << "\"hasRTTI\":" << (node.hasRTTI ? "true" : "false");
            json << "}";
        }
        json << "],";
        json << "\"totalTypes\":" << recoveryResult.totalClasses << ",";
        json << "\"totalMethods\":" << recoveryResult.totalMethods << ",";
        json << "\"totalHierarchyLinks\":" << recoveryResult.totalHierarchyLinks;
        json << "}";

        result.success = true;
        result.output = json.str();
        result.metadata["type_count"] = std::to_string(recoveryResult.totalClasses);
        result.metadata["method_count"] = std::to_string(recoveryResult.totalMethods);
        result.metadata["hierarchy_links"] = std::to_string(recoveryResult.totalHierarchyLinks);
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

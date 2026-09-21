#include "Plugin/IPlugin.h"
#include "Analysis/Functions.h"
#include "Analysis/Variables.h"
#include "Analysis/Parameters.h"
#include "Analysis/Strings.h"
#include "Analysis/Imports.h"
#include "Analysis/Confidences.h"
#include <sstream>

namespace omnibyte::hydradis::plugin {

class EnhancedFunctionResolverPlugin : public IPlugin {
public:
    std::string name() const override { return "Enhanced/FunctionResolver"; }
    std::string version() const override { return "1.0.0"; }

    bool onLoad() override { return true; }

    PluginResult onRun(const PluginContext& ctx) override {
        PluginResult result;

        if (!ctx.disassemblyResults || ctx.disassemblyResults->empty()) {
            result.errorMessage = "No disassembly data available";
            return result;
        }

        std::map<uint64_t, const Instruction*> instrMap;
        std::vector<uint8_t> codeData;
        uint64_t codeBaseAddr = 0;

        for (const auto& sec : *ctx.disassemblyResults) {
            for (const auto& instr : sec.instructions) {
                instrMap[instr.address] = &instr;
                if (codeBaseAddr == 0) codeBaseAddr = instr.address;
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
            codeBaseAddr = lowest;
        }

        omnibyte::hydradis::Functions funcAnalyzer;
        omnibyte::hydradis::Variables varAnalyzer;
        omnibyte::hydradis::Parameters paramAnalyzer;
        omnibyte::hydradis::Strings strAnalyzer;
        omnibyte::hydradis::Imports importsAnalyzer;
        omnibyte::hydradis::Confidences confAnalyzer;

        omnibyte::hydradis::FunctionsResult functionsResult;
        if (ctx.symbols() && !ctx.symbols()->empty()) {
            functionsResult = funcAnalyzer.analyzeFunctions(codeBaseAddr, codeData, *ctx.symbols());
        } else {
            functionsResult = funcAnalyzer.analyzeFunctions(codeBaseAddr, codeData);
        }

        auto variablesResult = varAnalyzer.analyzeVariables(codeBaseAddr, codeData);
        auto parametersResult = paramAnalyzer.analyzeParameters(codeBaseAddr, codeData);
        auto confidencesResult = confAnalyzer.analyzeConfidences(codeData);

        omnibyte::hydradis::ImportsResult importsResult;
        if (ctx.symbols() && !ctx.symbols()->empty()) {
            importsResult = importsAnalyzer.analyzeImports(codeBaseAddr, codeData, *ctx.symbols());
        }

        omnibyte::hydradis::StringsResult stringsResult;
        if (ctx.binary && !ctx.binary->sections.empty()) {
            for (const auto& sec : ctx.binary->sections) {
                if (sec.flags & 0x4) {
                    stringsResult = strAnalyzer.analyzeStrings(
                        codeData.data(), codeData.size());
                    break;
                }
            }
            if (stringsResult.stringAddresses.empty() && !codeData.empty()) {
                stringsResult = strAnalyzer.analyzeStrings(
                    codeData.data(), codeData.size());
            }
        }

        std::ostringstream json;
        json << "{";
        json << "\"functions\":[";
        bool first = true;
        for (const auto& fn : functionsResult.functions) {
            if (!first) json << ",";
            first = false;
            json << "{";
            json << "\"startAddr\":\"0x" << toHex(fn.startAddr) << "\",";
            json << "\"endAddr\":\"0x" << toHex(fn.endAddr) << "\",";
            json << "\"name\":\"" << escapeJson(fn.name) << "\",";
            json << "\"callers\":[";
            bool firstCaller = true;
            for (uint64_t c : fn.callers) {
                if (!firstCaller) json << ",";
                firstCaller = false;
                json << "\"0x" << toHex(c) << "\"";
            }
            json << "],";
            json << "\"callees\":[";
            bool firstCallee = true;
            for (uint64_t c : fn.callees) {
                if (!firstCallee) json << ",";
                firstCallee = false;
                json << "\"0x" << toHex(c) << "\"";
            }
            json << "],";
            json << "\"isExport\":" << (fn.isExport ? "true" : "false") << ",";
            json << "\"isImport\":" << (fn.isImport ? "true" : "false") << ",";
            json << "\"isPltStub\":" << (fn.isPltStub ? "true" : "false");
            if (!fn.demangledName.empty()) {
                json << ",\"demangledName\":\"" << escapeJson(fn.demangledName) << "\"";
            }
            json << "}";
        }
        json << "],";
        json << "\"variables\":[";
        first = true;
        for (const auto& [offset, varName] : variablesResult.variableNames) {
            if (!first) json << ",";
            first = false;
            json << "{\"offset\":\"0x" << toHex(offset)
                 << "\",\"name\":\"" << escapeJson(varName) << "\"}";
        }
        json << "],";
        json << "\"parameters\":[";
        first = true;
        for (const auto& [reg, paramName] : parametersResult.parameterNames) {
            if (!first) json << ",";
            first = false;
            json << "{\"register\":" << reg
                 << ",\"name\":\"" << escapeJson(paramName) << "\"}";
        }
        json << "],";
        json << "\"strings\":[";
        first = true;
        for (const auto& [addr, classification] : stringsResult.stringClassifications) {
            if (!first) json << ",";
            first = false;
            json << "{\"addr\":\"0x" << toHex(addr)
                 << "\",\"classification\":\"" << escapeJson(classification) << "\"}";
        }
        json << "],";
        json << "\"imports\":[";
        first = true;
        for (const auto& imp : importsResult.imports) {
            if (!first) json << ",";
            first = false;
            json << "{";
            json << "\"address\":\"0x" << toHex(imp.address) << "\",";
            json << "\"resolvedName\":\"" << escapeJson(imp.resolvedName) << "\",";
            if (!imp.libraryName.empty()) {
                json << "\"library\":\"" << escapeJson(imp.libraryName) << "\",";
            }
            json << "\"type\":\"";
            switch (imp.type) {
                case omnibyte::hydradis::ImportType::Function: json << "function"; break;
                case omnibyte::hydradis::ImportType::Object: json << "object"; break;
                case omnibyte::hydradis::ImportType::TLS: json << "tls"; break;
                default: json << "unknown"; break;
            }
            json << "\",";
            json << "\"source\":\"";
            switch (imp.source) {
                case omnibyte::hydradis::ImportDetectionSource::PLTStub: json << "plt"; break;
                case omnibyte::hydradis::ImportDetectionSource::GOTEntry: json << "got"; break;
                case omnibyte::hydradis::ImportDetectionSource::SymbolTable: json << "symbol"; break;
                case omnibyte::hydradis::ImportDetectionSource::DynamicReloc: json << "reloc"; break;
                default: json << "unknown"; break;
            }
            json << "\",";
            json << "\"isWeak\":" << (imp.isWeak ? "true" : "false");
            json << "}";
        }
        json << "],";
        json << "\"confidence\":" << confidencesResult.overallConfidence << ",";
        json << "\"totalFunctions\":" << functionsResult.functions.size() << ",";
        json << "\"totalVariables\":" << variablesResult.namedVariables << ",";
        json << "\"totalParameters\":" << parametersResult.namedParameters << ",";
        json << "\"totalStrings\":" << stringsResult.classifiedStrings << ",";
        json << "\"totalImports\":" << importsResult.imports.size();
        json << "}";

        result.success = true;
        result.output = json.str();
        result.metadata["function_count"] = std::to_string(functionsResult.functions.size());
        result.metadata["variable_count"] = std::to_string(variablesResult.namedVariables);
        result.metadata["parameter_count"] = std::to_string(parametersResult.namedParameters);
        result.metadata["string_count"] = std::to_string(stringsResult.classifiedStrings);
        result.metadata["confidence"] = std::to_string(confidencesResult.overallConfidence);
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

extern "C" std::unique_ptr<IPlugin> create_enhanced_functionresolver_plugin() {
    return std::make_unique<EnhancedFunctionResolverPlugin>();
}

extern "C" int enhanced_functionresolver_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

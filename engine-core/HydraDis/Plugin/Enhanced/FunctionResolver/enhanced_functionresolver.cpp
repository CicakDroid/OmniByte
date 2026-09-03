#include "Plugin/IPlugin.h"
#include <sstream>
#include <map>
#include <set>
#include <algorithm>

namespace omnibyte::hydradis::plugin {

struct FunctionInfo {
    uint64_t startAddr = 0;
    uint64_t endAddr = 0;
    std::string name;
    std::string demangledName;
    std::set<uint64_t> callers;
    std::set<uint64_t> callees;
    bool isExport = false;
    bool isImport = false;
    bool isPltStub = false;
};

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
        for (const auto& sec : *ctx.disassemblyResults) {
            for (const auto& instr : sec.instructions) {
                instrMap[instr.address] = &instr;
            }
        }

        std::map<uint64_t, FunctionInfo> functions;

        if (ctx.symbols()) {
            for (const auto& sym : *ctx.symbols()) {
                if (sym.value == 0) continue;

                bool isFunc = (sym.type == 2) ||
                              (sym.name.find("sub_") != std::string::npos) ||
                              (sym.name.find("_Z") == 0);

                if (isFunc) {
                    auto& fn = functions[sym.value];
                    fn.startAddr = sym.value;
                    if (fn.name.empty() || fn.name.find("sub_") == 0 || fn.name.find("plt_") == 0) {
                        fn.name = sym.name;
                    }
                    fn.isExport = true;
                    if (sym.size > 0) {
                        fn.endAddr = sym.value + sym.size - 1;
                    }
                    if (sym.name.find("_Z") == 0) {
                        fn.demangledName = demangleItanium(sym.name);
                    }
                }
            }
        }

        for (const auto& [addr, instr] : instrMap) {
            bool isPltStub = false;
            if (instr->mnemonic == "ldr" &&
                instr->opStr.find("x16") != std::string::npos) {
                auto next = instrMap.find(addr + 4);
                if (next != instrMap.end() && next->second->mnemonic == "br" &&
                    next->second->opStr.find("x16") != std::string::npos) {
                    isPltStub = true;
                }
            }

            if (isPltStub) {
                auto& fn = functions[addr];
                fn.startAddr = addr;
                fn.name = "plt_" + toHex(addr);
                fn.isPltStub = true;
                fn.isImport = true;
            }

            if (instr->mnemonic == "stp" &&
                instr->opStr.find("x29") != std::string::npos &&
                instr->opStr.find("x30") != std::string::npos) {
                if (functions.find(addr) == functions.end()) {
                    auto& fn = functions[addr];
                    fn.startAddr = addr;
                    fn.name = "sub_" + toHex(addr);
                }
            }

            if (instr->mnemonic == "bl" || instr->mnemonic == "blr") {
                uint64_t target = parseBranchTarget(instr->opStr);
                if (target != 0) {
                    auto& callee = functions[target];
                    callee.startAddr = target;
                    if (callee.name.empty()) {
                        callee.name = "sub_" + toHex(target);
                    }

                    if (functions.find(addr) != functions.end()) {
                        functions[addr].callees.insert(target);
                    }
                    callee.callers.insert(addr);
                }
            }

            if (instr->mnemonic == "ret" || instr->mnemonic == "bx") {
                auto prev = instrMap.lower_bound(addr);
                if (prev != instrMap.begin()) {
                    --prev;
                    uint64_t prevAddr = prev->first;
                    if (functions.find(prevAddr) != functions.end()) {
                        functions[prevAddr].endAddr = addr;
                    }
                }
            }
        }

        for (auto& [addr, fn] : functions) {
            if (fn.endAddr == 0) {
                auto next = functions.upper_bound(fn.startAddr);
                if (next != functions.end()) {
                    fn.endAddr = next->first - 1;
                } else {
                    auto it = instrMap.upper_bound(fn.startAddr);
                    if (it != instrMap.end()) {
                        fn.endAddr = it->first;
                    }
                }
            }
        }

        std::ostringstream json;
        json << "{";
        json << "\"functions\":[";
        bool first = true;
        for (const auto& [addr, fn] : functions) {
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
        json << "\"totalFunctions\":" << functions.size();
        json << "}";

        result.success = true;
        result.output = json.str();
        result.metadata["function_count"] = std::to_string(functions.size());
        return result;
    }

    void onUnload() override {}

private:
    static uint64_t parseBranchTarget(const std::string& opStr) {
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

    static std::string demangleItanium(const std::string& mangled) {
        if (mangled.empty() || mangled[0] != '_') return mangled;

        std::string result;
        size_t i = 0;

        if (mangled.size() > 2 && mangled[0] == '_' && mangled[1] == 'Z') {
            i = 2;
        } else {
            return mangled;
        }

        if (i < mangled.size() && mangled[i] == 'N') {
            i++;
            while (i < mangled.size() && mangled[i] != 'E') {
                if (std::isdigit(mangled[i])) {
                    size_t len = 0;
                    while (i < mangled.size() && std::isdigit(mangled[i])) {
                        len = len * 10 + (mangled[i] - '0');
                        i++;
                    }
                    if (i + len <= mangled.size()) {
                        if (!result.empty()) result += "::";
                        result += mangled.substr(i, len);
                        i += len;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }
            if (i < mangled.size() && mangled[i] == 'E') {
                i++;
            }
        } else if (i < mangled.size() && std::isdigit(mangled[i])) {
            size_t len = 0;
            while (i < mangled.size() && std::isdigit(mangled[i])) {
                len = len * 10 + (mangled[i] - '0');
                i++;
            }
            if (i + len <= mangled.size()) {
                result = mangled.substr(i, len);
            }
        }

        return result.empty() ? mangled : result;
    }
};

extern "C" std::unique_ptr<IPlugin> create_enhanced_functionresolver_plugin() {
    return std::make_unique<EnhancedFunctionResolverPlugin>();
}

extern "C" int enhanced_functionresolver_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

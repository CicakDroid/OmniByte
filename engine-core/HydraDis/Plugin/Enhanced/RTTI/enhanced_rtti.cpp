#include "Plugin/IPlugin.h"
#include <sstream>
#include <map>
#include <set>
#include <cstring>

namespace omnibyte::hydradis::plugin {

struct TypeInfo {
    std::string mangledName;
    std::string demangledName;
    uint64_t vtableAddr = 0;
    std::vector<uint64_t> vtableEntries;
    std::vector<std::string> baseClasses;
};

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

        std::vector<TypeInfo> types;
        std::map<uint64_t, std::string> vtableToClass;

        for (const auto& sym : ctx.binary->symbols) {
            if (sym.value == 0) continue;

            if (sym.name.find("_ZTV") == 0) {
                std::string className = extractClassNameFromVtable(sym.name);
                if (!className.empty()) {
                    types.push_back({
                        sym.name,
                        sym.name,
                        sym.value,
                        {},
                        parseBaseClasses(className)
                    });

                    vtableToClass[sym.value] = className;
                }
            }

            if (sym.name.find("_ZTI") == 0 || sym.name.find("_ZTS") == 0) {
                bool found = false;
                for (const auto& t : types) {
                    if (t.mangledName == sym.name) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    types.push_back({
                        sym.name,
                        sym.name,
                        0,
                        {},
                        {}
                    });
                }
            }
        }

        if (types.empty() && ctx.binary->sections.size() > 0) {
            for (const auto& sec : ctx.binary->sections) {
                if (sec.name == ".rodata" || sec.name == ".data.rel.ro") {
                    scanSectionForRtti(sec, types, vtableToClass);
                }
            }
        }

        std::ostringstream json;
        json << "{";
        json << "\"types\":[";
        bool first = true;
        for (const auto& t : types) {
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
            json << "]";
            json << "}";
        }
        json << "],";
        json << "\"vtableMapping\":[";
        first = true;
        for (const auto& [addr, className] : vtableToClass) {
            if (!first) json << ",";
            first = false;
            json << "{";
            json << "\"vtableAddr\":\"0x" << toHex(addr) << "\",";
            json << "\"className\":\"" << escapeJson(className) << "\"";
            json << "}";
        }
        json << "],";
        json << "\"totalTypes\":" << types.size();
        json << "}";

        result.success = true;
        result.output = json.str();
        result.metadata["type_count"] = std::to_string(types.size());
        return result;
    }

    void onUnload() override {}

private:
    std::string extractClassNameFromVtable(const std::string& mangled) {
        std::string s = mangled;
        if (s.find("_ZTV") == 0) s = s.substr(4);
        else if (s.find("_ZTC") == 0) s = s.substr(4);
        else return "";

        if (s.empty()) return "";
        if (std::isdigit(s[0])) {
            int len = std::stoi(s.substr(0, 1));
            if (len > 0 && s.size() > 1) {
                return s.substr(1, len);
            }
        }
        return s;
    }

    std::vector<std::string> parseBaseClasses(const std::string& mangled) {
        std::vector<std::string> bases;
        size_t pos = 0;
        while (pos < mangled.size()) {
            if (mangled[pos] == 'N' || mangled[pos] == 'I') {
                ++pos;
                while (pos < mangled.size() && std::isdigit(mangled[pos])) {
                    int len = mangled[pos] - '0';
                    ++pos;
                    if (pos + len <= mangled.size()) {
                        bases.push_back(mangled.substr(pos, len));
                        pos += len;
                    } else {
                        break;
                    }
                }
            } else {
                break;
            }
        }
        return bases;
    }

    void scanSectionForRtti(
        const omnibyte::hydradis::SectionInfo& sec,
        std::vector<TypeInfo>& types,
        std::map<uint64_t, std::string>& vtableToClass
    ) {
        if (sec.size < 8) return;

        for (uint64_t offset = 0; offset + 16 <= sec.size; offset += 8) {
            uint64_t vtablePtr = 0;
            std::memcpy(&vtablePtr, &sec.virtualAddress + offset, 8);

            if (vtablePtr > 0x1000 && vtablePtr < 0xFFFFFFFFFFFFULL) {
                std::string potentialName = "type_info_at_0x" + toHex(sec.virtualAddress + offset);
                types.push_back({
                    potentialName,
                    potentialName,
                    vtablePtr,
                    {},
                    {}
                });
            }
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
};

extern "C" std::unique_ptr<IPlugin> create_enhanced_rtti_plugin() {
    return std::make_unique<EnhancedRttiPlugin>();
}

extern "C" int enhanced_rtti_placeholder_init() { return 0; }

} // namespace omnibyte::hydradis::plugin

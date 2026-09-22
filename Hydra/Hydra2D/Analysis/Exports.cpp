#include "Exports.h"

#include <algorithm>
#include <cstring>
#include <sstream>

namespace omnibyte::hydradis {

ExportsResult Exports::analyzeExports(
    uint64_t codeBaseAddr,
    const std::vector<uint8_t>& codeData,
    const std::vector<SymbolInfo>& symbols
) const {
    ExportsResult result;
    if (codeData.empty() && symbols.empty()) {
        result.success = true;
        return result;
    }

    std::map<uint64_t, ExportInfo> exports;

    collectFromSymbols(symbols, exports);

    if (!codeData.empty()) {
        detectDynamicExports(codeData.data(), codeData.size(), codeBaseAddr, exports);
    }

    for (auto& [addr, exp] : exports) {
        if (exp.demangledName.empty() && exp.name.size() > 2 && exp.name[0] == '_' && exp.name[1] == 'Z') {
            exp.demangledName = demangleItanium(exp.name);
        }
        result.addressToName[addr] = exp.name;
        result.totalExports++;
        if (exp.type == ExportType::Function) result.functionExports++;
        if (exp.type == ExportType::Object) result.objectExports++;
        if (exp.isWeak) result.weakExports++;
        result.exports.push_back(std::move(exp));
    }

    result.success = true;
    return result;
}

void Exports::collectFromSymbols(
    const std::vector<SymbolInfo>& symbols,
    std::map<uint64_t, ExportInfo>& exports
) const {
    for (const auto& sym : symbols) {
        if (sym.value == 0) continue;
        if (sym.name.empty() || sym.name[0] == '\0') continue;
        if (sym.name == "." || sym.name == "..") continue;

        bool isGlobal = (sym.binding == 1 || sym.binding == 2);
        bool isWeak = (sym.binding == 2);
        bool isDefined = (sym.sectionIndex != 0);

        if (!isGlobal || !isDefined) continue;

        auto& exp = exports[sym.value];
        exp.address = sym.value;
        if (exp.name.empty()) {
            exp.name = sym.name;
        }
        exp.size = sym.size;
        exp.isWeak = isWeak;
        exp.symbolIndex = 0;
        exp.source = ExportDetectionSource::SymbolTable;

        if (sym.type == 2) {
            exp.type = ExportType::Function;
        } else if (sym.type == 1) {
            exp.type = ExportType::Object;
        }
    }
}

void Exports::detectDynamicExports(
    const uint8_t* data, size_t dataSize,
    uint64_t baseAddress,
    std::map<uint64_t, ExportInfo>& exports
) const {
    for (size_t i = 0; i + 8 <= dataSize; i += 4) {
        uint32_t instr1 = 0;
        std::memcpy(&instr1, data + i, 4);

        bool isAdrpX0 = ((instr1 & 0x9F000000) == 0x90000000) &&
                         ((instr1 & 0x0000001F) == 0x00);

        if (!isAdrpX0) continue;

        for (size_t j = i + 4; j < std::min(i + 12, dataSize); j += 4) {
            uint32_t instr2 = 0;
            std::memcpy(&instr2, data + j, 4);

            bool isAddX0 = ((instr2 & 0xFF0003E0) == 0x91000000) &&
                           ((instr2 & 0x0000001F) == 0x00);

            if (isAddX0) {
                uint64_t entryAddr = baseAddress + i;
                if (exports.find(entryAddr) == exports.end()) {
                    auto& exp = exports[entryAddr];
                    exp.address = entryAddr;
                    exp.name = "exp_" + toHex(entryAddr);
                    exp.type = ExportType::Function;
                    exp.source = ExportDetectionSource::DynamicTag;
                }
                break;
            }
        }
    }
}

std::string Exports::demangleItanium(const std::string& mangled) {
    if (mangled.empty() || mangled[0] != '_') return mangled;
    if (mangled.size() <= 2 || mangled[1] != 'Z') return mangled;

    std::string result;
    size_t i = 2;

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
        if (i < mangled.size() && mangled[i] == 'E') i++;
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

std::string Exports::toHex(uint64_t val) {
    std::ostringstream oss;
    oss << std::hex << val;
    return oss.str();
}

} // namespace omnibyte::hydradis

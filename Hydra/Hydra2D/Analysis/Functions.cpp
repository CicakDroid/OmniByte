#include "Functions.h"

#include <common/Boost/Boost.h>

#include <algorithm>
#include <cstring>
#include <set>

namespace omnibyte::hydradis {

FunctionsResult Functions::analyzeFunctions(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData
) const {
    FunctionsResult result;
    if (codeData.empty()) {
        result.success = true;
        return result;
    }

    auto prologues = findFunctionPrologues(codeData.data(), codeData.size());
    auto callSites = findCallSites(codeData.data(), codeData.size(), entryAddress);

    std::unordered_set<uint64_t> funcAddrs;
    for (auto addr : prologues) funcAddrs.insert(addr);
    for (auto addr : callSites) funcAddrs.insert(addr);

    result.functionAddresses.assign(funcAddrs.begin(), funcAddrs.end());
    std::sort(result.functionAddresses.begin(), result.functionAddresses.end());

    for (auto addr : result.functionAddresses) {
        std::string name = guessFunctionName(addr, codeData.data(), codeData.size());
        if (!name.empty()) {
            result.functionNames[addr] = name;
            result.namedFunctions++;
        }
    }

    result.success = true;
    return result;
}

FunctionsResult Functions::analyzeFunctions(
    uint64_t codeBaseAddr,
    const std::vector<uint8_t>& codeData,
    const std::vector<SymbolInfo>& symbols
) const {
    FunctionsResult result;
    if (codeData.empty()) {
        result.success = true;
        return result;
    }

    std::map<uint64_t, FunctionInfo> functions;

    detectFromSymbols(symbols, functions);

    auto prologues = findFunctionPrologues(codeData.data(), codeData.size());
    for (auto addr : prologues) {
        if (functions.find(addr) == functions.end()) {
            auto& fn = functions[addr];
            fn.startAddr = addr;
            fn.name = "sub_" + toHex(addr);
            fn.source = FunctionDetectionSource::Prologue;
        }
    }

    auto callSites = findCallSites(codeData.data(), codeData.size(), codeBaseAddr);
    for (auto addr : callSites) {
        if (functions.find(addr) == functions.end()) {
            auto& fn = functions[addr];
            fn.startAddr = addr;
            fn.name = "sub_" + toHex(addr);
            fn.source = FunctionDetectionSource::CallSite;
        }
    }

    for (auto& [addr, fn] : functions) {
        if (fn.demangledName.empty() && fn.name.find("_Z") == 0) {
            fn.demangledName = demangleItanium(fn.name);
        }
        if (fn.endAddr == 0) {
            auto next = functions.upper_bound(fn.startAddr);
            if (next != functions.end()) {
                fn.endAddr = next->first - 1;
            }
        }
    }

    for (const auto& [addr, fn] : functions) {
        result.functionAddresses.push_back(addr);
        result.functionNames[addr] = fn.name;
        result.functions.push_back(fn);
        if (!fn.name.empty()) result.namedFunctions++;
    }

    std::sort(result.functionAddresses.begin(), result.functionAddresses.end());
    result.success = true;
    return result;
}

void Functions::detectFromSymbols(
    const std::vector<SymbolInfo>& symbols,
    std::map<uint64_t, FunctionInfo>& functions
) const {
    for (const auto& sym : symbols) {
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
            fn.source = FunctionDetectionSource::SymbolTable;
            if (sym.size > 0) {
                fn.endAddr = sym.value + sym.size - 1;
            }
            if (sym.name.find("_Z") == 0) {
                fn.demangledName = demangleItanium(sym.name);
            }
        }
    }
}

std::vector<uint64_t> Functions::findFunctionPrologues(
    const uint8_t* data, size_t dataSize
) const {
    std::vector<uint64_t> prologues;

    for (size_t i = 0; i + 4 <= dataSize; i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, data + i, 4);

        if ((instr & 0xFFC003FF) == 0xA98003FF) {
            prologues.push_back(static_cast<uint64_t>(i));
            continue;
        }

        if ((instr & 0xFF0003FF) == 0xD10003FF) {
            if (i + 4 <= dataSize) {
                uint32_t next = 0;
                std::memcpy(&next, data + i + 4, 4);
                bool nextIsStp = (next & 0xFFC00000) == 0xA9000000;
                if (nextIsStp) {
                    prologues.push_back(static_cast<uint64_t>(i));
                }
            }
        }
    }

    return prologues;
}

std::vector<uint64_t> Functions::findCallSites(
    const uint8_t* data, size_t dataSize,
    uint64_t baseAddress
) const {
    std::vector<uint64_t> callers;

    for (size_t i = 0; i + 4 <= dataSize; i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, data + i, 4);

        uint64_t target = getBLTarget(instr, baseAddress + i);
        if (target != 0 && target >= baseAddress && target < baseAddress + dataSize) {
            callers.push_back(target);
        }
    }

    std::sort(callers.begin(), callers.end());
    callers.erase(std::unique(callers.begin(), callers.end()), callers.end());
    return callers;
}

std::string Functions::guessFunctionName(
    uint64_t address,
    const uint8_t* data, size_t dataSize
) const {
    if (address + 4 > dataSize) return "";

    uint32_t instr = 0;
    std::memcpy(&instr, data + address, 4);

    if (isLikelyThunk(instr)) {
        uint64_t target = getBLTarget(instr, address);
        if (target != 0) return "thunk_" + boost::formatAddress(target);
    }

    return "sub_" + boost::formatAddress(address);
}

bool Functions::isLikelyThunk(uint32_t instruction) const {
    if ((instruction & 0xFC000000) == 0x14000000) return true;
    if ((instruction & 0xFFFFFC1F) == 0xD61F0000) return true;
    return false;
}

uint64_t Functions::getBLTarget(uint32_t instruction, uint64_t address) const {
    if ((instruction & 0xFC000000) == 0x94000000) {
        int32_t imm26 = static_cast<int32_t>(instruction & 0x03FFFFFF);
        if (imm26 & 0x02000000) imm26 |= 0xFC000000;
        return address + (imm26 << 2);
    }
    return 0;
}

std::string Functions::demangleItanium(const std::string& mangled) {
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

uint64_t Functions::parseBranchTarget(const std::string& opStr) {
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

std::string Functions::toHex(uint64_t val) {
    std::ostringstream oss;
    oss << std::hex << val;
    return oss.str();
}

} // namespace omnibyte::hydradis

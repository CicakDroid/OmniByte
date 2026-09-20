#include "Types.h"

#include <common/Boost/Boost.h>

#include <algorithm>
#include <cstring>
#include <set>
#include <sstream>

namespace omnibyte::hydradis {

TypesResult Types::analyzeTypes(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData
) const {
    TypesResult result;
    if (codeData.empty()) {
        result.success = true;
        return result;
    }

    for (size_t i = 0; i + 4 <= codeData.size(); i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, codeData.data() + i, 4);

        std::string type = inferType(instr);
        if (!type.empty()) {
            uint64_t addr = entryAddress + i;
            result.typed_addresses.push_back(addr);
            result.typeMap[addr] = type;
        }
    }

    result.inferredTypes = result.typeMap.size();
    result.success = true;
    return result;
}

VtablesResult Types::analyzeVtables(
    const std::vector<SymbolInfo>& symbols,
    const std::vector<SectionInfo>& sections
) const {
    VtablesResult result;

    for (const auto& sym : symbols) {
        if (sym.value == 0) continue;

        if (sym.name.find("_ZTV") == 0) {
            std::string className = extractClassNameFromVtable(sym.name);
            if (!className.empty()) {
                result.vtables.push_back({
                    sym.name,
                    sym.name,
                    sym.value,
                    {},
                    parseBaseClasses(className),
                    0,
                    VtableDetectionSource::SymbolTable
                });
                result.vtableToClass[sym.value] = className;
            }
        }

        if (sym.name.find("_ZTI") == 0 || sym.name.find("_ZTS") == 0) {
            bool found = false;
            for (const auto& t : result.vtables) {
                if (t.mangledName == sym.name) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                result.vtables.push_back({
                    sym.name,
                    sym.name,
                    0,
                    {},
                    {},
                    0,
                    VtableDetectionSource::SymbolTable
                });
            }
        }
    }

    if (!result.vtables.empty()) {
        result.success = true;
        result.totalVtables = result.vtableToClass.size();
        result.totalTypeInfo = result.vtables.size();
        return result;
    }

    for (const auto& sec : sections) {
        if (sec.name != ".rodata" && sec.name != ".data.rel.ro" &&
            sec.name != ".data.rel.ro.local") {
            continue;
        }
        if (sec.size < 8) continue;

        for (uint64_t offset = 0; offset + 16 <= sec.size; offset += 8) {
            uint64_t vtablePtr = 0;
            std::memcpy(&vtablePtr, &sec.virtualAddress + offset, 8);

            if (vtablePtr > 0x1000 && vtablePtr < 0xFFFFFFFFFFFFULL) {
                size_t entryCount = 0;
                for (uint64_t scan = offset + 8; scan + 8 <= sec.size; scan += 8) {
                    uint64_t nextPtr = 0;
                    std::memcpy(&nextPtr, &sec.virtualAddress + scan, 8);
                    if (nextPtr == 0 || nextPtr > 0xFFFFFFFFFFFFULL) break;
                    entryCount++;
                }

                std::string name = "type_info_at_0x" + toHex(sec.virtualAddress + offset);
                result.vtables.push_back({
                    name,
                    name,
                    vtablePtr,
                    {},
                    {},
                    entryCount,
                    VtableDetectionSource::SectionScan
                });
            }
        }
    }

    result.success = true;
    result.totalVtables = result.vtableToClass.size();
    result.totalTypeInfo = result.vtables.size();
    return result;
}

std::string Types::inferType(uint32_t instruction) const {
    if (isPointerDereference(instruction)) {
        uint32_t size = (instruction >> 30) & 3;
        if (size == 3) return "void*";
        if (size == 2) return "uint64_t*";
        return "uint32_t*";
    }

    if (isStructAccess(instruction)) {
        return "struct*";
    }

    if (isArrayAccess(instruction)) {
        return "array";
    }

    if ((instruction & 0x7F800000) == 0x52800000) {
        uint32_t imm = (instruction >> 5) & 0xFFFF;
        if (imm <= 127) return "int8_t";
        if (imm <= 32767) return "int16_t";
        return "int32_t";
    }

    if ((instruction & 0x7F000000) == 0x11000000 ||
        (instruction & 0x7F000000) == 0x51000000) {
        return "int";
    }

    if ((instruction & 0x7FE08000) == 0x1B000000) {
        return "int";
    }

    if ((instruction & 0x7F200000) == 0x1E200000) {
        return "double";
    }
    if ((instruction & 0x7F200000) == 0x1E200000) {
        return "float";
    }

    return "";
}

bool Types::isPointerDereference(uint32_t instruction) const {
    if ((instruction & 0xFFC00000) == 0xF9400000) {
        uint32_t imm12 = (instruction >> 10) & 0xFFF;
        return imm12 == 0;
    }
    return false;
}

bool Types::isStructAccess(uint32_t instruction) const {
    if ((instruction & 0xFFC00000) == 0xF9400000) {
        uint32_t imm12 = (instruction >> 10) & 0xFFF;
        return imm12 > 0;
    }
    return false;
}

bool Types::isArrayAccess(uint32_t instruction) const {
    if ((instruction & 0xFFE00C00) == 0xF8600800) {
        return true;
    }
    return false;
}

std::string Types::extractClassNameFromVtable(const std::string& mangled) {
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

std::vector<std::string> Types::parseBaseClasses(const std::string& mangled) {
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

std::string Types::toHex(uint64_t val) {
    std::ostringstream oss;
    oss << std::hex << val;
    return oss.str();
}

} // namespace omnibyte::hydradis

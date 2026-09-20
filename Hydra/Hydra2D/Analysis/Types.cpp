#include "Types.h"

#include <common/Boost/Boost.h>

#include <algorithm>
#include <cstring>

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
            result.typedAddresses.push_back(addr);
            result.typeMap[addr] = type;
        }
    }

    result.inferredTypes = result.typeMap.size();
    result.success = true;
    return result;
}

std::string Types::inferType(uint32_t instruction) const {
    // LDR xN, [xM] — pointer dereference → void*
    if (isPointerDereference(instruction)) {
        uint32_t size = (instruction >> 30) & 3;
        if (size == 3) return "void*";
        if (size == 2) return "uint64_t*";
        return "uint32_t*";
    }

    // LDR xN, [xM, #offset] with non-zero offset — struct access
    if (isStructAccess(instruction)) {
        return "struct*";
    }

    // LDR xN, [xM, xN, LSL #scale] — array access
    if (isArrayAccess(instruction)) {
        return "array";
    }

    // MOV xN, #imm — immediate value
    if ((instruction & 0x7F800000) == 0x52800000) {
        uint32_t imm = (instruction >> 5) & 0xFFFF;
        if (imm <= 127) return "int8_t";
        if (imm <= 32767) return "int16_t";
        return "int32_t";
    }

    // ADD/SUB — arithmetic
    if ((instruction & 0x7F000000) == 0x11000000 ||
        (instruction & 0x7F000000) == 0x51000000) {
        return "int";
    }

    // MUL — multiplication
    if ((instruction & 0x7FE08000) == 0x1B000000) {
        return "int";
    }

    // FCVTSD/FMADD — floating point
    if ((instruction & 0x7F200000) == 0x1E200000) {
        return "double";
    }
    if ((instruction & 0x7F200000) == 0x1E200000) {
        return "float";
    }

    return "";
}

bool Types::isPointerDereference(uint32_t instruction) const {
    // LDR xN, [xM] — no immediate offset
    if ((instruction & 0xFFC00000) == 0xF9400000) {
        uint32_t imm12 = (instruction >> 10) & 0xFFF;
        return imm12 == 0;
    }
    return false;
}

bool Types::isStructAccess(uint32_t instruction) const {
    // LDR xN, [xM, #imm12] — struct field access
    if ((instruction & 0xFFC00000) == 0xF9400000) {
        uint32_t imm12 = (instruction >> 10) & 0xFFF;
        return imm12 > 0;
    }
    return false;
}

bool Types::isArrayAccess(uint32_t instruction) const {
    // LDR xN, [xM, xN, LSL #scale] — array indexing
    if ((instruction & 0xFFE00C00) == 0xF8600800) {
        return true;
    }
    return false;
}

} // namespace omnibyte::hydradis

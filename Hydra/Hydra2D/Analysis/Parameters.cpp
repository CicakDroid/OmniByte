#include "Parameters.h"

#include <common/Boost/Boost.h>

#include <algorithm>
#include <cstring>

namespace omnibyte::hydradis {

ParametersResult Parameters::analyzeParameters(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData
) const {
    ParametersResult result;
    if (codeData.empty()) {
        result.success = true;
        return result;
    }

    // AAPCS64: x0-x7 are parameter registers
    // Scan function prologue for loads into x0-x7 before first store to stack
    size_t prologueEnd = std::min(codeData.size(), static_cast<size_t>(256));
    std::unordered_set<uint64_t> paramRegs;

    for (size_t i = 0; i + 4 <= prologueEnd; i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, codeData.data() + i, 4);

        // Stop at first stack store (prologue end)
        if ((instr & 0xFFC003FF) == 0xF90003FF) break;

        if (isParameterValueLoad(instr)) {
            uint32_t rd = getDestRegister(instr);
            if (isParameterRegister(rd)) {
                paramRegs.insert(rd);
            }
        }
    }

    // Also detect parameters passed to BL calls
    for (size_t i = 0; i + 4 <= codeData.size(); i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, codeData.data() + i, 4);

        // Before BL, MOVs into x0-x7 indicate parameter passing
        if ((instr & 0xFC000000) == 0x94000000) {
            // Check preceding MOV instructions
            for (size_t j = (i > 16 ? i - 16 : 0); j < i; j += 4) {
                uint32_t prev = 0;
                std::memcpy(&prev, codeData.data() + j, 4);
                // MOV xN, xM or MOV xN, #imm
                if ((prev & 0x7F800000) == 0x52800000 ||
                    (prev & 0x7FE003FF) == 0xAA0003E0) {
                    uint32_t rd = getDestRegister(prev);
                    if (isParameterRegister(rd)) {
                        paramRegs.insert(rd);
                    }
                }
            }
        }
    }

    result.parameterRegisters.assign(paramRegs.begin(), paramRegs.end());
    std::sort(result.parameterRegisters.begin(), result.parameterRegisters.end());

    for (auto reg : result.parameterRegisters) {
        result.parameterNames[reg] = "arg" + std::to_string(reg);
        result.namedParameters++;
    }

    result.success = true;
    return result;
}

bool Parameters::isParameterRegister(uint32_t reg) const {
    return reg <= 7;
}

bool Parameters::isParameterValueLoad(uint32_t instruction) const {
    // LDR xN, [xN, #offset] — loading from args array
    if ((instruction & 0xFFC00000) == 0xF9400000) {
        uint32_t rn = (instruction >> 5) & 0x1F;
        return rn <= 7;
    }
    // ADD xN, xN, #imm — pointer arithmetic on args
    if ((instruction & 0x7F800000) == 0x91000000) {
        uint32_t rn = (instruction >> 5) & 0x1F;
        return rn <= 7;
    }
    return false;
}

uint32_t Parameters::getDestRegister(uint32_t instruction) const {
    return instruction & 0x1F;
}

} // namespace omnibyte::hydradis

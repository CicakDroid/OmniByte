#include "Variables.h"

#include <common/Boost/Boost.h>

#include <algorithm>
#include <cstring>

namespace omnibyte::hydradis {

VariablesResult Variables::analyzeVariables(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData
) const {
    VariablesResult result;
    if (codeData.empty()) {
        result.success = true;
        return result;
    }

    auto offsets = findStackOffsets(codeData.data(), codeData.size());

    std::unordered_set<uint64_t> seen;
    for (auto off : offsets) {
        if (seen.count(off)) continue;
        seen.insert(off);

        result.variableAddresses.push_back(off);

        // Find the instruction that first references this offset
        for (size_t i = 0; i + 4 <= codeData.size(); i += 4) {
            uint32_t instr = 0;
            std::memcpy(&instr, codeData.data() + i, 4);
            if (isStackAccess(instr) && getStackOffset(instr) == static_cast<int32_t>(off)) {
                std::string name = classifyVariable(off, instr);
                if (!name.empty()) {
                    result.variableNames[off] = name;
                    result.namedVariables++;
                }
                break;
            }
        }
    }

    std::sort(result.variableAddresses.begin(), result.variableAddresses.end());
    result.success = true;
    return result;
}

std::vector<uint64_t> Variables::findStackOffsets(
    const uint8_t* data, size_t dataSize
) const {
    std::vector<uint64_t> offsets;

    for (size_t i = 0; i + 4 <= dataSize; i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, data + i, 4);

        if (isStackAccess(instr)) {
            int32_t off = getStackOffset(instr);
            if (off != 0) {
                offsets.push_back(static_cast<uint64_t>(off < 0 ? -off : off));
            }
        }
    }

    std::sort(offsets.begin(), offsets.end());
    offsets.erase(std::unique(offsets.begin(), offsets.end()), offsets.end());
    return offsets;
}

std::string Variables::classifyVariable(uint64_t offset, uint32_t instruction) const {
    // LDR/STR with immediate offset — stack variable
    if ((instruction & 0xFFC003E0) == 0xF94003E0 ||
        (instruction & 0xFFC003E0) == 0xF90003E0) {
        return "var_" + std::to_string(offset);
    }
    return "var_" + std::to_string(offset);
}

bool Variables::isStackAccess(uint32_t instruction) const {
    // LDR xN, [sp, #imm]!
    if ((instruction & 0xFFC003FF) == 0xF94003FF) return true;
    // STR xN, [sp, #imm]!
    if ((instruction & 0xFFC003FF) == 0xF90003FF) return true;
    // LDR wN, [sp, #imm]
    if ((instruction & 0xFFC003E0) == 0xB94003E0) return true;
    // STR wN, [sp, #imm]
    if ((instruction & 0xFFC003E0) == 0xB90003E0) return true;
    return false;
}

int32_t Variables::getStackOffset(uint32_t instruction) const {
    // Extract unsigned offset from LDR/STR [sp, #imm]
    uint32_t imm12 = (instruction >> 10) & 0xFFF;
    uint32_t scale = (instruction >> 30) & 3;
    uint32_t offset = imm12 << scale;

    bool isWrite = ((instruction & 0xFFC00000) == 0xF9000000) ||
                   ((instruction & 0xFFC00000) == 0xB9000000);

    return isWrite ? -static_cast<int32_t>(offset) : static_cast<int32_t>(offset);
}

} // namespace omnibyte::hydradis

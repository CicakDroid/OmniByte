#include "Functions.h"

#include <common/Boost/Boost.h>

#include <algorithm>
#include <cstring>

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

std::vector<uint64_t> Functions::findFunctionPrologues(
    const uint8_t* data, size_t dataSize
) const {
    std::vector<uint64_t> prologues;

    for (size_t i = 0; i + 4 <= dataSize; i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, data + i, 4);

        // STP x29, x30, [sp, #-N]! — classic frame setup
        if ((instr & 0xFFC003FF) == 0xA98003FF) {
            prologues.push_back(static_cast<uint64_t>(i));
            continue;
        }

        // SUB sp, sp, #imm — frame allocation
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
    // B (unconditional branch) — tail call / thunk
    if ((instruction & 0xFC000000) == 0x14000000) return true;
    // BR xN — indirect tail call
    if ((instruction & 0xFFFFFC1F) == 0xD61F0000) return true;
    return false;
}

uint64_t Functions::getBLTarget(uint32_t instruction, uint64_t address) const {
    // BL (unconditional): 100101 imm26
    if ((instruction & 0xFC000000) == 0x94000000) {
        int32_t imm26 = static_cast<int32_t>(instruction & 0x03FFFFFF);
        if (imm26 & 0x02000000) imm26 |= 0xFC000000;
        return address + (imm26 << 2);
    }
    return 0;
}

} // namespace omnibyte::hydradis

// Taint.cpp - Taint analysis implementation.
// Worklist-based dataflow analysis tracking data from sources to sinks.

#include "Taint.h"

#include <common/Boost/Boost.h>

#include <algorithm>
#include <cstring>
#include <queue>

namespace omnibyte::hydradis {

// IAnalysis overrides

TaintResult Taint::analyzeTaint(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData,
    const std::vector<uint64_t>& taintSources
) const {
    TaintResult result;
    if (codeData.empty() || taintSources.empty()) {
        result.success = true;
        return result;
    }

    auto leaks = findTaintLeaks(entryAddress, codeData, taintSources);

    result.sourcesFound = taintSources.size();
    result.propagationPaths = leaks.size();

    std::unordered_set<uint64_t> taintedAddrs;
    for (const auto& path : leaks) {
        taintedAddrs.insert(path.sourceAddress);
        taintedAddrs.insert(path.sinkAddress);
        for (auto addr : path.propagationPath) {
            taintedAddrs.insert(addr);
        }
    }

    result.taintedAddresses.assign(taintedAddrs.begin(), taintedAddrs.end());
    std::sort(result.taintedAddresses.begin(), result.taintedAddresses.end());

    std::unordered_set<uint64_t> sinkAddrs;
    for (const auto& path : leaks) {
        sinkAddrs.insert(path.sinkAddress);
    }
    result.sinksFound = sinkAddrs.size();

    result.success = true;
    return result;
}

// Taint-specific methods

std::vector<TaintPath> Taint::findTaintLeaks(
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData,
    const std::vector<uint64_t>& taintSources
) const {
    std::vector<TaintPath> leaks;
    if (codeData.empty() || taintSources.empty()) return leaks;

    auto sinks = getDefaultSinks();

    std::unordered_map<uint64_t, TaintState> taintMap;

    for (auto srcAddr : taintSources) {
        TaintState state;
        state.isTainted = true;
        state.sourceAddress = srcAddr;
        state.propagationSteps = 0;
        state.sourceType = "explicit";
        taintMap[srcAddr] = state;
    }

    std::unordered_set<uint64_t> visited;
    std::queue<uint64_t> worklist;

    for (auto srcAddr : taintSources) {
        worklist.push(srcAddr);
    }

    while (!worklist.empty()) {
        uint64_t addr = worklist.front();
        worklist.pop();

        if (visited.count(addr)) continue;
        visited.insert(addr);

        for (const auto& sink : sinks) {
            if (sink.address == addr && taintMap[addr].isTainted) {
                TaintPath path;
                path.sourceAddress = taintMap[addr].sourceAddress;
                path.sinkAddress = addr;
                path.sourceType = taintMap[addr].sourceType;
                path.sinkType = sink.type;
                path.sanitized = false;

                uint64_t cur = addr;
                while (cur != path.sourceAddress && taintMap.count(cur)) {
                    path.propagationPath.push_back(cur);
                    cur = taintMap[cur].sourceAddress;
                }
                path.propagationPath.push_back(path.sourceAddress);
                std::reverse(path.propagationPath.begin(), path.propagationPath.end());

                leaks.push_back(path);
            }
        }

        size_t offset = static_cast<size_t>(addr - entryAddress);
        if (offset + 4 <= codeData.size()) {
            uint32_t instr = 0;
            std::memcpy(&instr, codeData.data() + offset, 4);

            propagateInstruction(instr, addr, taintMap, codeData);

            uint64_t nextAddr = addr + 4;
            if (taintMap.count(nextAddr) && taintMap[nextAddr].isTainted) {
                worklist.push(nextAddr);
            }

            uint64_t branchTarget = getBranchTarget(instr, addr);
            if (branchTarget != 0 && taintMap.count(addr) && taintMap[addr].isTainted) {
                TaintState branchState = taintMap[addr];
                branchState.propagationSteps++;
                taintMap[branchTarget] = branchState;
                worklist.push(branchTarget);
            }
        }
    }

    return leaks;
}

bool Taint::isAddressTainted(
    uint64_t address,
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData,
    const std::vector<uint64_t>& taintSources
) const {
    for (auto src : taintSources) {
        if (src == address) return true;
    }

    auto leaks = findTaintLeaks(entryAddress, codeData, taintSources);
    for (const auto& path : leaks) {
        if (path.sourceAddress == address || path.sinkAddress == address) return true;
        for (auto addr : path.propagationPath) {
            if (addr == address) return true;
        }
    }
    return false;
}

TaintState Taint::getTaintState(
    uint64_t address,
    uint64_t entryAddress,
    const std::vector<uint8_t>& codeData,
    const std::vector<uint64_t>& taintSources
) const {
    for (auto src : taintSources) {
        if (src == address) {
            return {true, address, 0, "explicit"};
        }
    }

    auto leaks = findTaintLeaks(entryAddress, codeData, taintSources);
    for (const auto& path : leaks) {
        for (size_t i = 0; i < path.propagationPath.size(); ++i) {
            if (path.propagationPath[i] == address) {
                return {true, path.sourceAddress, i, path.sourceType};
            }
        }
    }

    return {false, 0, 0, ""};
}

std::vector<TaintSink> Taint::getDefaultSinks() {
    return {
        {0, "write"},
        {0, "exec"},
        {0, "ioctl"},
        {0, "mmap"},
        {0, "open"},
    };
}

std::vector<TaintSource> Taint::getDefaultSources() {
    return {
        {0, "recv", 4096},
        {0, "read", 4096},
        {0, "getenv", 256},
        {0, "argv", 1024},
        {0, "mmap", 4096},
    };
}

// Private helpers

void Taint::propagateInstruction(
    uint32_t instruction,
    uint64_t address,
    std::unordered_map<uint64_t, TaintState>& taintMap,
    const std::vector<uint8_t>& /*codeData*/
) const {
    if (!taintMap.count(address) || !taintMap[address].isTainted) return;

    auto regs = extractRegisters(instruction);
    int rd = regs[0];

    // MOV: taint propagates through
    bool isMov = ((instruction & 0x7FE003FF) == 0xAA0003E0) ||
                 ((instruction & 0x7FE003E0) == 0x52800000);

    // ALU: ADD, SUB, AND, ORR, EOR
    bool isAlu = ((instruction & 0x1F000000) == 0x0B000000) ||
                 ((instruction & 0x1F000000) == 0x4B000000) ||
                 ((instruction & 0x7E000000) == 0x6A000000);

    if ((isMov || isAlu) && rd >= 0) {
        taintMap[address].propagationSteps++;
    }
}

bool Taint::isSource(uint64_t address, const std::vector<TaintSource>& sources) const {
    for (const auto& src : sources) {
        if (src.address == address) return true;
    }
    return false;
}

bool Taint::isSink(uint64_t address, const std::vector<TaintSink>& sinks) const {
    for (const auto& sink : sinks) {
        if (sink.address == address) return true;
    }
    return false;
}

bool Taint::isSanitizer(uint32_t instruction) const {
    // AND with immediate mask (potential bounds check)
    if ((instruction & 0x7FC00000) == 0x12000000) {
        uint32_t imm12 = (instruction >> 10) & 0xFFF;
        if (imm12 > 0 && (imm12 & (imm12 + 1)) == 0) return true; // power-of-2 mask
    }
    // CMP: comparison is a potential sanitizer
    if ((instruction & 0xFF80001F) == 0xF100001F) return true;
    return false;
}

std::array<int, 3> Taint::extractRegisters(uint32_t instruction) const {
    int rd = -1, rn = -1, rm = -1;

    // Data processing: Rd = bits [4:0], Rn = bits [9:5], Rm = bits [20:16]
    if ((instruction & 0x1F000000) == 0x0B000000 ||
        (instruction & 0x1F000000) == 0x4B000000 ||
        (instruction & 0x7E000000) == 0x6A000000) {
        rd = instruction & 0x1F;
        rn = (instruction >> 5) & 0x1F;
        rm = (instruction >> 16) & 0x1F;
    }

    return {rd, rn, rm};
}

uint64_t Taint::getBranchTarget(uint32_t instruction, uint64_t address) const {
    // B (unconditional): 000101 imm26
    if ((instruction & 0xFC000000) == 0x14000000) {
        int32_t imm26 = static_cast<int32_t>(instruction & 0x03FFFFFF);
        if (imm26 & 0x02000000) imm26 |= 0xFC000000; // sign extend
        return address + (imm26 << 2);
    }

    // B.cond: 01010100 imm19 0 cond
    if ((instruction & 0xFF000010) == 0x54000000) {
        int32_t imm19 = static_cast<int32_t>((instruction >> 5) & 0x7FFFF);
        if (imm19 & 0x40000) imm19 |= 0xFFF80000; // sign extend
        return address + (imm19 << 2);
    }

    return 0; // not a branch
}

} // namespace omnibyte::hydradis

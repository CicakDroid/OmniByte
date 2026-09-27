#include "Frame.h"

namespace omnibyte::hydradis {

namespace {

constexpr int64_t signExtend(int64_t value, int bits) {
    const int64_t mask = int64_t{1} << (bits - 1);
    return (value ^ mask) - mask;
}

} // namespace

FrameResult Frame::analyzeFrame(
    uint64_t baseAddr,
    const std::vector<uint8_t>& codeData
) const {
    FrameResult result;
    result.success = true;

    for (size_t i = 0; i + 4 <= codeData.size(); i += 4) {
        const uint32_t instr = uint32_t(codeData[i])
                             | (uint32_t(codeData[i + 1]) << 8)
                             | (uint32_t(codeData[i + 2]) << 16)
                             | (uint32_t(codeData[i + 3]) << 24);
        FrameInfo info{};
        if (decodeFrameAccess(instr, baseAddr + i, info)) {
            result.accesses.push_back(info);
        }
    }

    for (const auto& a : result.accesses) {
        if (a.baseReg == 29) result.hasFramePointer = true;
        if ((a.kind == FrameOpKind::SpSub || a.kind == FrameOpKind::SpAdd) && a.reg == 29)
            result.hasFramePointer = true;
        if (a.kind == FrameOpKind::SpSub && a.reg == 31 && a.offset > result.frameSize)
            result.frameSize = a.offset;
    }
    return result;
}

bool Frame::decodeFrameAccess(uint32_t instr, uint64_t address, FrameInfo& out) const {
    // ADD/SUB immediate (64-bit, non-flag-setting): group 0x11, sf=1, S=0.
    if (((instr >> 24) & 0x1F) == 0x11 && (instr >> 31) && !((instr >> 29) & 1)) {
        const uint8_t rd = instr & 0x1F;
        const uint8_t rn = (instr >> 5) & 0x1F;
        if (rn != 31 && rd != 31) return false;          // neither operand is sp
        const int64_t imm = int64_t((instr >> 10) & 0xFFF)
                          << (((instr >> 22) & 1) ? 12 : 0);
        out = {address, ((instr >> 30) & 1) ? FrameOpKind::SpSub : FrameOpKind::SpAdd,
               rn, rd, imm};
        return true;
    }

    // Load/store pair (integer, bits[29:26] == 0xA): post-index / offset / pre-index.
    if (((instr >> 26) & 0xF) == 0xA && !((instr >> 30) & 1)) {
        const uint8_t mode = (instr >> 23) & 7;
        if (mode < 1 || mode > 3) return false;           // exclude non-temporal / logical
        const uint8_t rn = (instr >> 5) & 0x1F;
        if (rn != 31 && rn != 29) return false;           // base must be sp or fp
        const int64_t imm = signExtend((instr >> 15) & 0x7F, 7)
                          * (((instr >> 31) & 1) ? 8 : 4);
        out = {address, ((instr >> 22) & 1) ? FrameOpKind::PairLoad : FrameOpKind::PairStore,
               rn, uint8_t(instr & 0x1F), imm};
        return true;
    }

    // Load/store register (integer, bits[29:26] == 0xE, non-PRFM): bit23 == 0.
    if (((instr >> 26) & 0xF) == 0xE && !((instr >> 23) & 1)) {
        const uint8_t rn = (instr >> 5) & 0x1F;
        if (rn != 31 && rn != 29) return false;
        int64_t imm;
        const uint8_t form = (instr >> 24) & 3;           // bits[25:24]
        if (form == 1) {
            // Unsigned offset: imm12 scaled by access size (bits[31:30])
            imm = int64_t((instr >> 10) & 0xFFF) << ((instr >> 30) & 3);
        } else if (form == 0 && !((instr >> 21) & 1)) {
            const uint8_t idxMode = (instr >> 10) & 3;    // bits[11:10]
            if (idxMode == 2) return false;               // unprivileged LDTR/STTR
            imm = signExtend((instr >> 12) & 0x1FF, 9);   // unscaled / pre / post
        } else {
            return false;                                 // register offset / excluded form
        }
        out = {address, ((instr >> 22) & 1) ? FrameOpKind::Load : FrameOpKind::Store,
               rn, uint8_t(instr & 0x1F), imm};
        return true;
    }
    return false;
}

} // namespace omnibyte::hydradis

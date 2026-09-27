#pragma once
// Frame — Stack frame access detection for ARM64 (aarch64) code.
// Scans raw little-endian instruction words for sp/fp-relative frame accesses:
//   - ADD/SUB immediate touching sp (sub sp,sp,#N / add sp,sp,#N)
//   - LDP/STP in post-index, offset, and pre-index forms on sp/fp
//   - LDR/STR unsigned-offset, unscaled (LDUR/STUR), pre- and post-index forms
// Excludes unprivileged (LDTR/STTR), register-offset, SIMD, and non-stack bases.
//
// Integration:
//   - No external dependencies; pure bitfield decoding
//
// Semantics:
//   - offset: signed instruction immediate with scale applied
//   - frameSize: largest positive offset among "sub sp, sp, #N"
//   - hasFramePointer: any access based on x29, or sp add/sub writing x29
//
// Usage:
//   Frame analyzer;
//   auto result = analyzer.analyzeFrame(baseAddr, codeData);
//   int64_t frameSize = result.frameSize;

#include "IAnalysis.h"

#include <vector>

namespace omnibyte::hydradis {

/// Stack-frame access detection: finds sp/fp-relative loads, stores, and
/// frame setup/teardown in raw ARM64 instruction bytes.
class Frame : public IAnalysis {
public:
    std::string name() const override { return "Frame"; }

    /// Detect stack-frame accesses (4-byte word walk; trailing 1–3 bytes ignored).
    FrameResult analyzeFrame(
        uint64_t baseAddr,
        const std::vector<uint8_t>& codeData
    ) const override;

private:
    /// Decode one instruction word; true + fill out when it is an sp/fp frame access.
    bool decodeFrameAccess(uint32_t instr, uint64_t address, FrameInfo& out) const;
};

} // namespace omnibyte::hydradis

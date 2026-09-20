// Obfuscate.cpp — Binary obfuscation detection implementation.
// Detection based on proven patterns: CFF dispatch, opaque predicates, junk code, XOR strings.

#include "Obfuscate.h"

#include <common/Boost/Boost.h>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace omnibyte::hydradis {

// ── IAnalysis overrides ─────────────────────────────────────────────────

ObfuscateResult Obfuscate::analyzeObfuscation(
    const uint8_t* data, size_t dataSize
) const {
    ObfuscateResult result;
    if (!data || dataSize == 0) {
        result.success = true;
        return result;
    }

    result.controlFlowFlattened = detectControlFlowFlattening(data, dataSize);
    result.opaquePredicatesFound = detectOpaquePredicates(data, dataSize);
    result.junkCodeDetected = detectJunkCode(data, dataSize);
    result.stringEncryptionDetected = detectStringEncryption(data, dataSize);

    // Count suspicious patterns
    result.suspiciousPatternCount = 0;
    if (result.controlFlowFlattened) result.suspiciousPatternCount++;
    if (result.opaquePredicatesFound) result.suspiciousPatternCount++;
    if (result.junkCodeDetected) result.suspiciousPatternCount++;
    if (result.stringEncryptionDetected) result.suspiciousPatternCount++;

    result.success = true;
    return result;
}

// ── Obfuscate-specific methods ──────────────────────────────────────────

bool Obfuscate::detectControlFlowFlattening(
    const uint8_t* data, size_t dataSize
) const {
    if (!data || dataSize < 16) return false;

    // Pattern: many CMP instructions against constants (switch dispatch)
    // CFF compilers emit: LDR r_state, [fp]; CMP r_state, #N; BEQ label_N
    size_t cmpCount = 0;

    for (size_t i = 0; i + 4 <= dataSize; i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, data + i, 4);

        // ARM64 CMP (immediate): SUBS with Rd=ZR
        // Encoding: 1 10 10001 00 imm12 Rn 11111
        if ((instr & 0xFF80001F) == 0xF100001F) {
            cmpCount++;
        }
        // ARM32 CMP: E4100000 / E3500000 pattern
        else if ((instr & 0x0E500000) == 0x03500000) {
            cmpCount++;
        }
    }

    // CFF typically has 5+ CMP dispatches in close proximity
    // Threshold: >5 CMP instructions in first 256 bytes suggests CFF
    size_t threshold = std::min(dataSize, static_cast<size_t>(256));
    size_t regionCmp = 0;
    for (size_t i = 0; i + 4 <= threshold; i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, data + i, 4);
        if ((instr & 0xFF80001F) == 0xF100001F ||
            (instr & 0x0E500000) == 0x03500000) {
            regionCmp++;
        }
    }

    return regionCmp > 5;
}

bool Obfuscate::detectOpaquePredicates(
    const uint8_t* data, size_t dataSize
) const {
    if (!data || dataSize < 4) return false;

    // Look for arithmetic patterns that are always true/false:
    // 1. TST r0, #1 after AND r0, r0, #1 (bit test = always 0 or 1)
    // 2. CMP r0, r0 after XOR r0, r0, r0 (always equal)
    // 3. AND r0, r0, #1; TST r0, #1 (always determines LSB)

    for (size_t i = 0; i + 8 <= dataSize; i += 4) {
        uint32_t instr1 = 0, instr2 = 0;
        std::memcpy(&instr1, data + i, 4);
        std::memcpy(&instr2, data + i + 4, 4);

        // Pattern: XOR rX, rX, rX followed by CMP rX, rX (always Z flag)
        // ARM64: EAD00000 (EOR Rd, Rn, Rn) + B400001F (CMP Rn, Rn)
        if ((instr1 & 0x7F200000) == 0x58000000) {
            // Check if next is CMP with same register
            if ((instr2 & 0xFF80001F) == 0xF100001F) {
                return true;
            }
        }

        // Pattern: AND r0, r0, #1; TST r0, #1 (bit isolation)
        uint32_t imm12_1 = (instr1 >> 10) & 0xFFF;
        if ((instr1 & 0x7F800000) == 0x12000000 && imm12_1 == 1) {
            // AND with immediate 1
            if ((instr2 & 0xFF80001F) == 0xF700001F) {
                // TST with immediate
                return true;
            }
        }
    }

    return false;
}

bool Obfuscate::detectJunkCode(
    const uint8_t* data, size_t dataSize
) const {
    if (!data || dataSize < 4) return false;

    // Count NOP sleds (D503201F on ARM64, E1A00000 on ARM32)
    size_t nopCount = countNOPSleds(data, dataSize);

    // Threshold: more than 3 NOP sleds = likely junk code insertion
    return nopCount > 3;
}

bool Obfuscate::detectStringEncryption(
    const uint8_t* data, size_t dataSize
) const {
    if (!data || dataSize < 16) return false;

    // Pattern: XOR decode loop
    // LDRB rX, [rY, #offset]  → load encrypted byte
    // EOR   rX, rX, #key       → XOR with key
    // STRB  rX, [rY, #offset]  → store decrypted byte
    size_t xorLoops = countXORDecodeLoops(data, dataSize);

    // Threshold: >1 XOR decode loop = likely string encryption
    return xorLoops > 1;
}

std::vector<ObfuscationIndicator> Obfuscate::getAllIndicators(
    const uint8_t* data, size_t dataSize
) const {
    std::vector<ObfuscationIndicator> indicators;
    if (!data || dataSize == 0) return indicators;

    if (detectControlFlowFlattening(data, dataSize)) {
        indicators.push_back({
            ObfuscationType::ControlFlowFlattening,
            "Control flow flattening detected (switch dispatch pattern)",
            0, dataSize, 0.8
        });
    }

    if (detectOpaquePredicates(data, dataSize)) {
        indicators.push_back({
            ObfuscationType::OpaquePredicate,
            "Opaque predicate detected (always-true/false condition)",
            0, dataSize, 0.7
        });
    }

    if (detectJunkCode(data, dataSize)) {
        indicators.push_back({
            ObfuscationType::JunkCode,
            "Junk code detected (NOP sleds / dead instructions)",
            0, dataSize, 0.6
        });
    }

    if (detectStringEncryption(data, dataSize)) {
        indicators.push_back({
            ObfuscationType::StringEncryption,
            "String encryption detected (XOR decode loops)",
            0, dataSize, 0.75
        });
    }

    return indicators;
}

double Obfuscate::obfuscationDensity(
    const uint8_t* data, size_t dataSize
) const {
    if (!data || dataSize == 0) return 0.0;

    auto indicators = getAllIndicators(data, dataSize);
    if (indicators.empty()) return 0.0;

    // Average confidence of all detected techniques
    double totalConfidence = 0.0;
    for (const auto& ind : indicators) {
        totalConfidence += ind.confidence;
    }
    return totalConfidence / static_cast<double>(indicators.size());
}

// ── Private helpers ─────────────────────────────────────────────────────

size_t Obfuscate::countSwitchPatterns(
    const uint8_t* data, size_t dataSize
) const {
    size_t count = 0;
    for (size_t i = 0; i + 4 <= dataSize; i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, data + i, 4);
        // CMP immediate
        if ((instr & 0xFF80001F) == 0xF100001F) count++;
    }
    return count;
}

size_t Obfuscate::countNOPSleds(
    const uint8_t* data, size_t dataSize
) const {
    size_t sleds = 0;
    size_t consecutiveNops = 0;

    for (size_t i = 0; i + 4 <= dataSize; i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, data + i, 4);

        // ARM64 NOP: D503201F, ARM32 NOP: E1A00000
        if (instr == 0xD503201F || instr == 0xE1A00000) {
            consecutiveNops++;
        } else {
            if (consecutiveNops >= 3) sleds++;
            consecutiveNops = 0;
        }
    }
    if (consecutiveNops >= 3) sleds++;

    return sleds;
}

size_t Obfuscate::countXORDecodeLoops(
    const uint8_t* data, size_t dataSize
) const {
    size_t count = 0;

    for (size_t i = 0; i + 12 <= dataSize; i += 4) {
        uint32_t instr1 = 0, instr2 = 0, instr3 = 0;
        std::memcpy(&instr1, data + i, 4);
        std::memcpy(&instr2, data + i + 4, 4);
        std::memcpy(&instr3, data + i + 8, 4);

        // Look for XOR pattern: EOR rX, rY, #imm (ARM64: 12-bit shift + EOR)
        // followed by STR/STRB
        bool hasEor = (instr2 & 0x7F800000) == 0x52000000 ||
                      (instr2 & 0xFFE00000) == 0xCA000000;
        bool hasStr = (instr3 & 0x3F000000) == 0x38000000;

        if (hasEor && hasStr) {
            count++;
            i += 8; // skip past this pattern
        }
    }

    return count;
}

size_t Obfuscate::countDeadCodeBlocks(
    const uint8_t* data, size_t dataSize
) const {
    size_t count = 0;

    for (size_t i = 0; i + 4 <= dataSize; i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, data + i, 4);

        // Unconditional branch: B (ARM64: 0x14xxxxxx), RET (0xD63F0000)
        bool isUncondBranch = (instr & 0xFC000000) == 0x14000000;
        bool isRet = (instr & 0xFFFFFC1F) == 0xD63F0000;

        if (isUncondBranch || isRet) {
            // Check if next instruction is reachable (not a label target)
            // Heuristic: if next 4 bytes are also a branch, it's dead code
            if (i + 8 <= dataSize) {
                uint32_t next = 0;
                std::memcpy(&next, data + i + 4, 4);
                bool nextIsBranch = (next & 0xFC000000) == 0x14000000 ||
                                    (next & 0xFFFFFC1F) == 0xD63F0000;
                if (!nextIsBranch) count++;
            }
        }
    }

    return count;
}

} // namespace omnibyte::hydradis

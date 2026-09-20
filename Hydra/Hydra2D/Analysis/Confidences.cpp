#include "Confidences.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace omnibyte::hydradis {

ConfidencesResult Confidences::analyzeConfidences(
    const std::vector<uint8_t>& codeData
) const {
    ConfidencesResult result;
    if (codeData.empty()) {
        result.success = true;
        return result;
    }

    double instrConf = computeInstructionConfidence(codeData.data(), codeData.size());
    double entropyConf = computeEntropyConfidence(codeData.data(), codeData.size());
    double patternConf = computePatternConfidence(codeData.data(), codeData.size());

    result.categoryConfidence["instructions"] = instrConf;
    result.categoryConfidence["entropy"] = entropyConf;
    result.categoryConfidence["patterns"] = patternConf;

    result.overallConfidence = (instrConf + entropyConf + patternConf) / 3.0;
    result.success = true;
    return result;
}

double Confidences::computeInstructionConfidence(
    const uint8_t* data, size_t dataSize
) const {
    if (dataSize < 4) return 0.0;

    size_t validInstrs = 0;
    size_t totalInstrs = dataSize / 4;

    for (size_t i = 0; i + 4 <= dataSize; i += 4) {
        uint32_t instr = 0;
        std::memcpy(&instr, data + i, 4);

        // Known ARM64 instruction patterns
        bool isKnown = (instr & 0xFC000000) == 0x14000000 || // B
                       (instr & 0xFC000000) == 0x94000000 || // BL
                       (instr & 0xFF80001F) == 0xD03FD400 || // ADRP
                       (instr == 0xD503201F) ||               // NOP
                       (instr & 0xFFC003FF) == 0xF94003FF || // LDR xN, [sp]
                       (instr & 0xFFC003FF) == 0xF90003FF;   // STR xN, [sp]

        if (isKnown) validInstrs++;
    }

    return totalInstrs > 0 ? static_cast<double>(validInstrs) / static_cast<double>(totalInstrs) : 0.0;
}

double Confidences::computeEntropyConfidence(
    const uint8_t* data, size_t dataSize
) const {
    double entropy = shannonEntropy(data, dataSize);
    // ARM64 code typically has entropy 4.5-6.5 bits/byte
    // Lower entropy suggests data sections, higher suggests encryption
    if (entropy >= 4.5 && entropy <= 6.5) return 0.9;
    if (entropy >= 3.0 && entropy <= 7.5) return 0.6;
    return 0.3;
}

double Confidences::computePatternConfidence(
    const uint8_t* data, size_t dataSize
) const {
    if (dataSize < 16) return 0.0;

    size_t patternHits = 0;
    for (size_t i = 0; i + 8 <= dataSize; i += 4) {
        uint32_t instr1 = 0, instr2 = 0;
        std::memcpy(&instr1, data + i, 4);
        std::memcpy(&instr2, data + i + 4, 4);

        // Common patterns: STP/LDP pair, CBZ/CBNZ branch
        if (((instr1 & 0xFFC00000) == 0xA9000000 && (instr2 & 0xFFC00000) == 0xA9400000) ||
            ((instr1 & 0xFF000010) == 0x34000000) ||
            ((instr1 & 0xFF000010) == 0x35000000)) {
            patternHits++;
        }
    }

    size_t maxPatterns = (dataSize / 4) > 0 ? (dataSize / 4) - 1 : 0;
    return maxPatterns > 0 ? static_cast<double>(patternHits) / static_cast<double>(maxPatterns) : 0.0;
}

double Confidences::shannonEntropy(const uint8_t* data, size_t dataSize) const {
    if (dataSize == 0) return 0.0;

    size_t freq[256] = {};
    for (size_t i = 0; i < dataSize; ++i) {
        freq[data[i]]++;
    }

    double entropy = 0.0;
    for (size_t i = 0; i < 256; ++i) {
        if (freq[i] == 0) continue;
        double p = static_cast<double>(freq[i]) / static_cast<double>(dataSize);
        entropy -= p * std::log2(p);
    }

    return entropy;
}

} // namespace omnibyte::hydradis

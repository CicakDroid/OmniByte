#pragma once

#include "IAnalysis.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

class Confidences : public IAnalysis {
public:
    std::string name() const override { return "Confidences"; }

    ConfidencesResult analyzeConfidences(
        const std::vector<uint8_t>& codeData
    ) const override;

private:
    double computeInstructionConfidence(
        const uint8_t* data, size_t dataSize
    ) const;

    double computeEntropyConfidence(
        const uint8_t* data, size_t dataSize
    ) const;

    double computePatternConfidence(
        const uint8_t* data, size_t dataSize
    ) const;

    double shannonEntropy(const uint8_t* data, size_t dataSize) const;
};

} // namespace omnibyte::hydradis

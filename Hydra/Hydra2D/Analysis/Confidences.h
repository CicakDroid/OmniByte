#pragma once
// Confidences — Confidence scoring for binary analysis results.
// Combines instruction-level, entropy-based, and pattern-based signals
// to produce a unified confidence score (0.0–1.0) for RE analysis.
//
// Integration:
//   - No external dependencies; pure arithmetic heuristics
//
// Scoring model:
//   - Instruction confidence: ratio of recognized instructions to total
//   - Entropy confidence: Shannon entropy of code section (0.0–8.0 mapped to 0–1)
//   - Pattern confidence: presence of known code patterns (function prologues, etc.)
//   - Combined score: weighted average of the three signals
//
// Usage:
//   Confidences analyzer;
//   auto result = analyzer.analyzeConfidences(codeData);
//   double score = result.overallConfidence;

#include "IAnalysis.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

/// Confidence scoring: combines instruction, entropy, and pattern signals
/// into a unified confidence score (0.0–1.0) for binary analysis results.
class Confidences : public IAnalysis {
public:
    std::string name() const override { return "Confidences"; }

    /// Analyze code data and return confidence scores.
    ConfidencesResult analyzeConfidences(
        const std::vector<uint8_t>& codeData
    ) const override;

private:
    /// Compute confidence from instruction recognition rate.
    double computeInstructionConfidence(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Compute confidence from Shannon entropy of code section.
    double computeEntropyConfidence(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Compute confidence from presence of known code patterns.
    double computePatternConfidence(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Shannon entropy (bits per byte) of data buffer.
    double shannonEntropy(const uint8_t* data, size_t dataSize) const;
};

} // namespace omnibyte::hydradis

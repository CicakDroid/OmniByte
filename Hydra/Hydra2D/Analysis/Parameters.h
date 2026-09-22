#pragma once
// Parameters — Function parameter detection for ARM64 binaries.
// Identifies function parameters by analyzing register usage patterns
// (X0–X7 for ARM64 calling convention) and value loads at function entry.

#include "IAnalysis.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

/// Parameter detection: identify function parameters from ARM64 register usage.
///
/// Usage:
///   Parameters analyzer;
///   auto result = analyzer.analyzeParameters(entryAddr, codeData);
class Parameters : public IAnalysis {
public:
    std::string name() const override { return "Parameters"; }

    /// Analyze parameters at function entry address.
    ParametersResult analyzeParameters(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

private:
    /// Check if register is a parameter register (X0–X7).
    bool isParameterRegister(uint32_t reg) const;

    /// Check if instruction loads a value into a parameter register.
    bool isParameterValueLoad(uint32_t instruction) const;

    /// Extract destination register from instruction.
    uint32_t getDestRegister(uint32_t instruction) const;
};

} // namespace omnibyte::hydradis

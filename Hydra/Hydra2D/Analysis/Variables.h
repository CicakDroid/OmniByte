#pragma once
// Variables — Stack variable detection for ARM64 binaries.
// Identifies local variables by analyzing SP-relative memory accesses
// (LDR/STR with SP as base register) in function prologues and bodies.

#include "IAnalysis.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

/// Variable detection: identify stack variables from SP-relative accesses.
///
/// Usage:
///   Variables analyzer;
///   auto result = analyzer.analyzeVariables(entryAddr, codeData);
class Variables : public IAnalysis {
public:
    std::string name() const override { return "Variables"; }

    /// Analyze variables at function entry address.
    VariablesResult analyzeVariables(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

private:
    /// Find stack offsets accessed via SP-relative instructions.
    std::vector<uint64_t> findStackOffsets(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Classify a variable by its offset and usage pattern.
    std::string classifyVariable(uint64_t offset, uint32_t instruction) const;

    /// Check if instruction accesses the stack pointer.
    bool isStackAccess(uint32_t instruction) const;

    /// Extract SP-relative offset from instruction.
    int32_t getStackOffset(uint32_t instruction) const;
};

} // namespace omnibyte::hydradis

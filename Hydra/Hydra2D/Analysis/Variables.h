#pragma once

#include "IAnalysis.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

class Variables : public IAnalysis {
public:
    std::string name() const override { return "Variables"; }

    VariablesResult analyzeVariables(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

private:
    std::vector<uint64_t> findStackOffsets(
        const uint8_t* data, size_t dataSize
    ) const;

    std::string classifyVariable(uint64_t offset, uint32_t instruction) const;
    bool isStackAccess(uint32_t instruction) const;
    int32_t getStackOffset(uint32_t instruction) const;
};

} // namespace omnibyte::hydradis

#pragma once

#include "IAnalysis.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

class Functions : public IAnalysis {
public:
    std::string name() const override { return "Functions"; }

    FunctionsResult analyzeFunctions(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

private:
    std::vector<uint64_t> findFunctionPrologues(
        const uint8_t* data, size_t dataSize
    ) const;

    std::vector<uint64_t> findCallSites(
        const uint8_t* data, size_t dataSize,
        uint64_t baseAddress
    ) const;

    std::string guessFunctionName(
        uint64_t address,
        const uint8_t* data, size_t dataSize
    ) const;

    bool isLikelyThunk(uint32_t instruction) const;
    uint64_t getBLTarget(uint32_t instruction, uint64_t address) const;
};

} // namespace omnibyte::hydradis

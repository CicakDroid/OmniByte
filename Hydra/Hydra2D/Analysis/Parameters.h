#pragma once

#include "IAnalysis.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

class Parameters : public IAnalysis {
public:
    std::string name() const override { return "Parameters"; }

    ParametersResult analyzeParameters(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

private:
    bool isParameterRegister(uint32_t reg) const;
    bool isParameterValueLoad(uint32_t instruction) const;
    uint32_t getDestRegister(uint32_t instruction) const;
};

} // namespace omnibyte::hydradis

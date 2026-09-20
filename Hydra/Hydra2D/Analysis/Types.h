#pragma once

#include "IAnalysis.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

class Types : public IAnalysis {
public:
    std::string name() const override { return "Types"; }

    TypesResult analyzeTypes(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

private:
    std::string inferType(uint32_t instruction) const;
    bool isPointerDereference(uint32_t instruction) const;
    bool isStructAccess(uint32_t instruction) const;
    bool isArrayAccess(uint32_t instruction) const;
};

} // namespace omnibyte::hydradis

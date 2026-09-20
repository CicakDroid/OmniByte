#pragma once

#include "IAnalysis.h"
#include "Parser/IParser.h"

#include <map>
#include <set>
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

    VtablesResult analyzeVtables(
        const std::vector<SymbolInfo>& symbols,
        const std::vector<SectionInfo>& sections
    ) const;

private:
    std::string inferType(uint32_t instruction) const;
    bool isPointerDereference(uint32_t instruction) const;
    bool isStructAccess(uint32_t instruction) const;
    bool isArrayAccess(uint32_t instruction) const;

    static std::string extractClassNameFromVtable(const std::string& mangled);
    static std::vector<std::string> parseBaseClasses(const std::string& mangled);
    static std::string toHex(uint64_t val);
};

} // namespace omnibyte::hydradis

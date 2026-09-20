#pragma once

#include "IAnalysis.h"
#include "Parser/IParser.h"
#include "Disassembler/IDisassembler.h"

#include <map>
#include <set>
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

    FunctionsResult analyzeFunctions(
        uint64_t codeBaseAddr,
        const std::vector<uint8_t>& codeData,
        const std::vector<SymbolInfo>& symbols
    ) const;

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

    void detectFromSymbols(
        const std::vector<SymbolInfo>& symbols,
        std::map<uint64_t, FunctionInfo>& functions
    ) const;

    static std::string demangleItanium(const std::string& mangled);
    static uint64_t parseBranchTarget(const std::string& opStr);
    static std::string toHex(uint64_t val);
};

} // namespace omnibyte::hydradis

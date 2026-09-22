#pragma once
// Functions — Function discovery and naming for ARM64 binaries.
// Detects function boundaries via prologue scanning, call-site analysis,
// and symbol table lookup. Generates human-readable names from patterns.
//
// Detection methods:
//   - Prologue scan: STP X29, X30, [SP, #offset]! (0xA9xx7BFD)
//   - Call-site tracing: BL target resolution from call instructions
//   - Symbol integration: use .dynsym names when available
//   - Thunk detection: BL → RET sequences marked as thunks

#include "IAnalysis.h"
#include "Parser/IParser.h"
#include "Disassembler/IDisassembler.h"

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

/// Function discovery: find function boundaries, names, and call graph.
///
/// Usage:
///   Functions analyzer;
///   auto result = analyzer.analyzeFunctions(entryAddr, codeData);
///   for (auto& [addr, info] : result.functions) { ... }
class Functions : public IAnalysis {
public:
    std::string name() const override { return "Functions"; }

    /// Analyze functions from code data (no symbols).
    FunctionsResult analyzeFunctions(
        uint64_t entryAddress,
        const std::vector<uint8_t>& codeData
    ) const override;

    /// Analyze functions with symbol table integration.
    FunctionsResult analyzeFunctions(
        uint64_t codeBaseAddr,
        const std::vector<uint8_t>& codeData,
        const std::vector<SymbolInfo>& symbols
    ) const;

private:
    /// Scan for ARM64 function prologues (STP X29, X30, ...).
    std::vector<uint64_t> findFunctionPrologues(
        const uint8_t* data, size_t dataSize
    ) const;

    /// Find BL call sites and their targets.
    std::vector<uint64_t> findCallSites(
        const uint8_t* data, size_t dataSize,
        uint64_t baseAddress
    ) const;

    /// Generate a human-readable name from code patterns.
    std::string guessFunctionName(
        uint64_t address,
        const uint8_t* data, size_t dataSize
    ) const;

    /// Check if instruction is a thunk (BL → RET).
    bool isLikelyThunk(uint32_t instruction) const;

    /// Extract BL branch target from instruction.
    uint64_t getBLTarget(uint32_t instruction, uint64_t address) const;

    /// Import function names from symbol table.
    void detectFromSymbols(
        const std::vector<SymbolInfo>& symbols,
        std::map<uint64_t, FunctionInfo>& functions
    ) const;

    /// Demangle Itanium ABI mangled names.
    static std::string demangleItanium(const std::string& mangled);

    /// Parse branch target from disassembly operand string.
    static uint64_t parseBranchTarget(const std::string& opStr);

    /// Format address as hex string.
    static std::string toHex(uint64_t val);
};

} // namespace omnibyte::hydradis

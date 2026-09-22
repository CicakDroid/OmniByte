#pragma once
// Exports — Export symbol analysis for ELF/Mach-O binaries.
// Resolves exported functions/data from symbol tables and detects
// dynamic exports via PLT/GOT patterns and relocation entries.
//
// Sources of export info:
//   - Symbol table (.dynsym / .symtab): primary source for named exports
//   - Dynamic relocations: DT_FUNC/DT_OBJECT entries
//   - Itanium ABI demangling: human-readable C++ names from mangled symbols

#include "IAnalysis.h"
#include "Parser/IParser.h"

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

/// Export analysis: resolve exported symbols from ELF/Mach-O binaries.
///
/// Usage:
///   Exports analyzer;
///   auto result = analyzer.analyzeExports(baseAddr, codeData, symbols);
///   for (auto& [addr, info] : result.exports) { ... }
class Exports : public IAnalysis {
public:
    std::string name() const override { return "Exports"; }

    /// Analyze exports from code data + symbol table.
    ExportsResult analyzeExports(
        uint64_t codeBaseAddr,
        const std::vector<uint8_t>& codeData,
        const std::vector<SymbolInfo>& symbols
    ) const override;

private:
    /// Collect exports from .dynsym / .symtab symbol entries.
    void collectFromSymbols(
        const std::vector<SymbolInfo>& symbols,
        std::map<uint64_t, ExportInfo>& exports
    ) const;

    /// Detect dynamic exports via relocation patterns.
    void detectDynamicExports(
        const uint8_t* data, size_t dataSize,
        uint64_t baseAddress,
        std::map<uint64_t, ExportInfo>& exports
    ) const;

    /// Demangle Itanium ABI mangled names (e.g., _ZN3Foo3barEv → Foo::bar()).
    static std::string demangleItanium(const std::string& mangled);

    /// Format address as hex string.
    static std::string toHex(uint64_t val);
};

} // namespace omnibyte::hydradis

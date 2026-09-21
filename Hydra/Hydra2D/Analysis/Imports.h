#pragma once

#include "IAnalysis.h"
#include "Parser/IParser.h"

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace omnibyte::hydradis {

/// Source of import detection.
enum class ImportDetectionSource {
    Unknown,
    PLTStub,        // ARM64 ldr x16; br x16 pattern
    GOTEntry,       // GOT indirect call pattern
    SymbolTable,    // .dynsym / .symtab entry
    DynamicReloc    // DT_REL/DT_RELA relocation
};

/// Type of import reference.
enum class ImportType {
    Unknown,
    Function,       // function import (PLT/GOT)
    Object,         // data import (GOT)
    TLS             // thread-local storage import
};

/// Information about one resolved import.
struct ImportInfo {
    uint64_t address = 0;           // address of PLT stub or GOT entry
    std::string resolvedName;       // actual library function name (e.g., "strcmp")
    std::string mangledName;        // mangled name if available
    std::string libraryName;        // source library (e.g., "libc.so")
    ImportType type = ImportType::Unknown;
    ImportDetectionSource source = ImportDetectionSource::Unknown;
    uint32_t symbolIndex = 0;      // .dynsym index
    bool isWeak = false;            // WEAK binding
};

/// Result of import analysis.
struct ImportsResult {
    bool success = false;
    std::string errorMessage;
    std::vector<ImportInfo> imports;
    std::unordered_map<uint64_t, std::string> resolvedNames;  // address → name
    size_t totalImports = 0;
    size_t resolvedCount = 0;
};

/// Import analysis: resolve PLT stubs/GOT entries to library function names.
///
/// Usage:
///   Imports importer;
///   auto result = importer.analyzeImports(entryAddr, codeData, symbols);
///   for (auto& imp : result.imports) { ... }
class Imports : public IAnalysis {
public:
    std::string name() const override { return "Imports"; }

    /// Analyze imports from code data + symbol table.
    ImportsResult analyzeImports(
        uint64_t codeBaseAddr,
        const std::vector<uint8_t>& codeData,
        const std::vector<SymbolInfo>& symbols
    ) const;

private:
    /// Detect ARM64 PLT stub pattern: ldr x16, [ip]; add ip, ip, #0; br x16
    void detectPltStubs(
        const uint8_t* data, size_t dataSize,
        uint64_t baseAddress,
        std::map<uint64_t, ImportInfo>& imports
    ) const;

    /// Resolve PLT stubs using .dynsym symbols.
    void resolveFromSymbols(
        const std::vector<SymbolInfo>& symbols,
        std::map<uint64_t, ImportInfo>& imports
    ) const;

    /// Detect GOT indirect calls: ldr x16, [x16, #offset]; br x16
    void detectGotEntries(
        const uint8_t* data, size_t dataSize,
        uint64_t baseAddress,
        std::map<uint64_t, ImportInfo>& imports
    ) const;

    static std::string toHex(uint64_t val);
};

} // namespace omnibyte::hydradis

#include "Imports.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <set>

namespace omnibyte::hydradis {

ImportsResult Imports::analyzeImports(
    uint64_t codeBaseAddr,
    const std::vector<uint8_t>& codeData,
    const std::vector<SymbolInfo>& symbols
) const {
    ImportsResult result;
    if (codeData.empty()) {
        result.success = true;
        return result;
    }

    std::map<uint64_t, ImportInfo> imports;

    // Phase 1: resolve from symbol table (most reliable)
    resolveFromSymbols(symbols, imports);

    // Phase 2: detect PLT stubs via pattern matching
    detectPltStubs(codeData.data(), codeData.size(), codeBaseAddr, imports);

    // Phase 3: detect GOT entries via pattern matching
    detectGotEntries(codeData.data(), codeData.size(), codeBaseAddr, imports);

    // Fill result
    result.totalImports = imports.size();
    for (auto& [addr, imp] : imports) {
        if (!imp.resolvedName.empty()) {
            result.resolvedCount++;
            result.resolvedNames[addr] = imp.resolvedName;
        }
        result.imports.push_back(std::move(imp));
    }

    result.success = true;
    return result;
}

void Imports::resolveFromSymbols(
    const std::vector<SymbolInfo>& symbols,
    std::map<uint64_t, ImportInfo>& imports
) const {
    for (const auto& sym : symbols) {
        if (sym.value == 0 && sym.sectionIndex != -1) continue;

        // Identify import symbols:
        // - UND (sectionIndex == 0 or -1) with value != 0 → PLT stub address
        // - FUNC/OBJECT with GLOBAL/WEAK binding
        bool isFunc = (sym.type == 2);   // STT_FUNC
        bool isObj = (sym.type == 1);    // STT_OBJECT
        bool isGlobal = (sym.binding == 1 || sym.binding == 2); // STB_GLOBAL/WEAK
        bool isWeak = (sym.binding == 2); // STB_WEAK

        if (!isFunc && !isObj) continue;
        if (!isGlobal) continue;

        // Skip internal symbols
        if (sym.name.empty() || sym.name[0] == '\0') continue;
        if (sym.name == "." || sym.name == "..") continue;

        // Skip symbols that start with underscore- prefix (internal)
        if (sym.name.size() > 1 && sym.name[0] == '_' && sym.name[1] == 'Z') {
            // C++ mangled names are internal, but may be exported
            // Still include them as they could be useful
        }

        // Determine address: use value directly
        uint64_t addr = sym.value;

        // Create or update import entry
        auto& imp = imports[addr];
        imp.address = addr;
        imp.mangledName = sym.name;
        imp.isWeak = isWeak;
        imp.symbolIndex = static_cast<uint32_t>(std::distance(
            symbols.begin(),
            std::find_if(symbols.begin(), symbols.end(),
                [&](const SymbolInfo& s) { return s.name == sym.name && s.value == sym.value; })
        ));

        if (isFunc) {
            imp.type = ImportType::Function;
        } else if (isObj) {
            imp.type = ImportType::Object;
        }

        // Use symbol name as resolved name (may be overridden by PLT detection)
        if (imp.resolvedName.empty()) {
            imp.resolvedName = sym.name;
        }

        imp.source = ImportDetectionSource::SymbolTable;
    }
}

void Imports::detectPltStubs(
    const uint8_t* data, size_t dataSize,
    uint64_t baseAddress,
    std::map<uint64_t, ImportInfo>& imports
) const {
    // ARM64 PLT stub pattern (typical):
    //   ldr x16, [ip, #offset]   ; 0xF9400190 (ldr x16, [x16])
    //   add ip, ip, #offset      ; 0x9100023f (mov ip, x16)
    //   br x16                   ; 0xD61F0200 (br x16)
    //
    // Simpler pattern we look for:
    //   ldr x16, [x16, #offset]  ; 0xF9400210
    //   br x16                   ; 0xD61F0200
    //
    // Or:
    //   ldr x16, [ip, #offset]   ; 0xF9400190
    //   br x16                   ; 0xD61F0200

    for (size_t i = 0; i + 8 <= dataSize; i += 4) {
        uint32_t instr1 = 0;
        std::memcpy(&instr1, data + i, 4);

        // Check for ldr x16, [x16, #offset] or ldr x16, [ip, #offset]
        bool isLdrX16 = ((instr1 & 0xFFC00000) == 0xF9400000) &&
                        ((instr1 & 0x0000001F) == 0x10); // Rt = x16

        if (!isLdrX16) continue;

        // Check for branch to x16 within next 2 instructions
        for (size_t j = i + 4; j < std::min(i + 12, dataSize); j += 4) {
            uint32_t instr2 = 0;
            std::memcpy(&instr2, data + j, 4);

            // br x16 = 0xD61F0200
            if (instr2 == 0xD61F0200) {
                uint64_t stubAddr = baseAddress + i;

                // Extract offset from ldr instruction
                uint32_t imm12 = (instr1 >> 10) & 0xFFF;
                bool isUnscaled = (instr1 >> 11) & 1;
                uint32_t offset = isUnscaled ? (imm12 * 8) : (imm12 * 8);

                // If we don't have this import yet, create it
                if (imports.find(stubAddr) == imports.end()) {
                    auto& imp = imports[stubAddr];
                    imp.address = stubAddr;
                    imp.type = ImportType::Function;
                    imp.source = ImportDetectionSource::PLTStub;
                    imp.resolvedName = "plt_" + toHex(stubAddr);
                } else {
                    // Update source if not already from symbol table
                    auto& imp = imports[stubAddr];
                    if (imp.source != ImportDetectionSource::SymbolTable) {
                        imp.source = ImportDetectionSource::PLTStub;
                    }
                }
                break;
            }
        }
    }
}

void Imports::detectGotEntries(
    const uint8_t* data, size_t dataSize,
    uint64_t baseAddress,
    std::map<uint64_t, ImportInfo>& imports
) const {
    // ARM64 GOT indirect call pattern:
    //   ldr x16, [x16, #offset]  ; load from GOT
    //   br x16                   ; indirect branch
    //
    // Or for calls:
    //   ldr x16, [x16, #offset]
    //   blr x16

    for (size_t i = 0; i + 8 <= dataSize; i += 4) {
        uint32_t instr1 = 0;
        std::memcpy(&instr1, data + i, 4);

        // ldr x16, [x16, #offset]
        bool isLdrX16 = ((instr1 & 0xFFC00000) == 0xF9400000) &&
                        ((instr1 & 0x0000001F) == 0x10);

        if (!isLdrX16) continue;

        // Check for branch/blr within next 2 instructions
        for (size_t j = i + 4; j < std::min(i + 12, dataSize); j += 4) {
            uint32_t instr2 = 0;
            std::memcpy(&instr2, data + j, 4);

            // br x16 or blr x16
            bool isBrX16 = (instr2 == 0xD61F0200);  // br x16
            bool isBlrX16 = ((instr2 & 0xFFFFFC1F) == 0xD63F0000) &&
                            ((instr2 & 0x0000001F) == 0x10); // blr x16

            if (isBrX16 || isBlrX16) {
                uint64_t entryAddr = baseAddress + i;

                // Only add if not already present from PLT detection
                if (imports.find(entryAddr) == imports.end()) {
                    auto& imp = imports[entryAddr];
                    imp.address = entryAddr;
                    imp.type = ImportType::Function;
                    imp.source = ImportDetectionSource::GOTEntry;
                    imp.resolvedName = "got_" + toHex(entryAddr);
                }
                break;
            }
        }
    }
}

std::string Imports::toHex(uint64_t val) {
    std::ostringstream oss;
    oss << std::hex << val;
    return oss.str();
}

} // namespace omnibyte::hydradis

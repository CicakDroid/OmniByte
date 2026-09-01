#include "Orchestrator/HydraDisOrchestrator.h"
#include <algorithm>
#include <fstream>

namespace omnibyte::hydradis {

// ── Constructor ───────────────────────────────────────────────────

HydraDisOrchestrator::HydraDisOrchestrator(const OrchestratorConfig& config)
    : config_(config) {}

// ── Public API ────────────────────────────────────────────────────

std::unique_ptr<IParser> HydraDisOrchestrator::createParser() const {
    return ParserFactory::create(config_.parserBackend);
}

std::unique_ptr<IDisassembler> HydraDisOrchestrator::createDisassembler(
    DisassemblerArch arch
) const {
    return DisassemblerFactory::create(arch, config_.disasmBackend);
}

AnalysisResult HydraDisOrchestrator::analyze(const std::string& filePath) const {
    AnalysisResult result;

    // ── Tahap 1: Parse ────────────────────────────────────────────
    auto parser = createParser();
    if (!parser) {
        result.errorMessage = "Failed to create parser";
        return result;
    }

    result.parsedBinary = parser->parseFile(filePath);
    if (!result.parsedBinary.success) {
        result.errorMessage = "Parse failed: " + result.parsedBinary.errorMessage;
        return result;
    }

    // ── Tahap 2: Disassemble ──────────────────────────────────────
    auto arch = machineToArch(
        result.parsedBinary.header.machine,
        result.parsedBinary.header.is64Bit
    );
    if (!arch) {
        result.errorMessage = "Unsupported architecture (machine=" +
            std::to_string(result.parsedBinary.header.machine) + ")";
        return result;
    }

    auto disasm = createDisassembler(*arch);
    if (!disasm) {
        result.errorMessage = "Failed to create disassembler for arch";
        return result;
    }

    // Cari .text section atau semua executable sections
    auto textSection = findTextSection(result.parsedBinary);
    if (textSection) {
        // Baca .text section dari file
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            result.errorMessage = "Failed to open file for reading sections";
            return result;
        }

        std::vector<uint8_t> code(textSection->size);
        file.seekg(textSection->fileOffset);
        file.read(reinterpret_cast<char*>(code.data()), textSection->size);

        auto disasmResult = disasm->disassemble(
            code, textSection->virtualAddress
        );

        AnalysisResult::DisasmSection ds;
        ds.sectionName = textSection->name;
        ds.result = std::move(disasmResult);
        result.disassemblyResults.push_back(std::move(ds));
    }

    result.success = true;
    return result;
}

AnalysisResult HydraDisOrchestrator::analyzeBuffer(
    const uint8_t* data,
    size_t dataSize
) const {
    AnalysisResult result;

    // ── Tahap 1: Parse from buffer ────────────────────────────────
    auto parser = createParser();
    if (!parser) {
        result.errorMessage = "Failed to create parser";
        return result;
    }

    result.parsedBinary = parser->parseBuffer(data, dataSize);
    if (!result.parsedBinary.success) {
        result.errorMessage = "Parse failed: " + result.parsedBinary.errorMessage;
        return result;
    }

    // ── Tahap 2: Disassemble .text ────────────────────────────────
    auto arch = machineToArch(
        result.parsedBinary.header.machine,
        result.parsedBinary.header.is64Bit
    );
    if (!arch) {
        result.errorMessage = "Unsupported architecture";
        return result;
    }

    auto disasm = createDisassembler(*arch);
    if (!disasm) {
        result.errorMessage = "Failed to create disassembler";
        return result;
    }

    auto textSection = findTextSection(result.parsedBinary);
    if (textSection && textSection->fileOffset + textSection->size <= dataSize) {
        const uint8_t* code = data + textSection->fileOffset;
        auto disasmResult = disasm->disassemble(
            code, textSection->size, textSection->virtualAddress
        );

        AnalysisResult::DisasmSection ds;
        ds.sectionName = textSection->name;
        ds.result = std::move(disasmResult);
        result.disassemblyResults.push_back(std::move(ds));
    }

    result.success = true;
    return result;
}

// ── Private helpers ───────────────────────────────────────────────

std::optional<DisassemblerArch> HydraDisOrchestrator::machineToArch(
    uint16_t machine,
    bool is64Bit
) {
    switch (machine) {
        case 40:    return DisassemblerArch::ARM;     // EM_ARM
        case 183:   return DisassemblerArch::ARM64;   // EM_AARCH64
        case 3:     return DisassemblerArch::x86;      // EM_386
        case 62:    return DisassemblerArch::x86_64;   // EM_X86_64
        case 8:     return DisassemblerArch::MIPS;     // EM_MIPS
        case 20:    return DisassemblerArch::PPC;      // EM_PPC
        case 2:     return DisassemblerArch::SPARC;    // EM_SPARC
        case 22:    return DisassemblerArch::SystemZ;  // EM_S390
        default:    return std::nullopt;
    }
}

std::optional<SectionInfo> HydraDisOrchestrator::findTextSection(
    const ParsedBinary& binary
) {
    // Cari .text section — ini adalah executable code section
    for (const auto& section : binary.sections) {
        if (section.name == ".text") {
            return section;
        }
    }

    // Fallback: cari section dengan EXECUTE flag
    // ELF PF_X = 1 << 0 = 1
    for (const auto& section : binary.sections) {
        if (section.flags & 1) {  // PF_X
            return section;
        }
    }

    return std::nullopt;
}

} // namespace omnibyte::hydradis

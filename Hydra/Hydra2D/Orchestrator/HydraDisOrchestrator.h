#pragma once
// ── HydraDisOrchestrator.h ────────────────────────────────────────
// Pipeline orchestrator: parse → disassemble → decompile → (symbolic exec).
// Menyederhanakan usage front-end: caller cukup panggil analyze() dengan
// file path, orchestrator handle semua tahap dan kembalikan hasil gabungan.

#include "Factory/DisassemblerFactory.h"
#include "Factory/ParserFactory.h"
#include "Factory/DecompilerFactory.h"
#include "Factory/SolverFactory.h"
#include "Disassembler/IDisassembler.h"
#include "Parser/IParser.h"
#include "Decompiler/IDecompiler.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::hydradis {

/// Configuration untuk orchestrator.
struct OrchestratorConfig {
    // Backend preferences
    DisassemblerBackend disasmBackend = DisassemblerBackend::Capstone;
    ParserBackend parserBackend = ParserBackend::Lief;
    DecompilerBackend decompilerBackend = DecompilerBackend::Auto;

    // Analysis options
    bool runDecompiler = true;      // jalankan decompilation?
    bool runSymbolicExec = false;   // jalankan symbolic execution? (butuh Z3/CVC5)

    // Rizin path (untuk rizin-native decompiler)
    std::string rizinPath = "rizin";
};

/// Hasil analisis lengkap dari satu binary.
struct AnalysisResult {
    bool success = false;
    std::string errorMessage;

    // Tahap 1: Parse
    ParsedBinary parsedBinary;

    // Tahap 2: Disassemble (per-section yang executable)
    struct DisasmSection {
        std::string sectionName;
        DisassemblyResult result;
    };
    std::vector<DisasmSection> disassemblyResults;

    // Tahap 3: Decompile (per-function atau per-region)
    struct DecompyleResult {
        uint64_t address = 0;
        DecompiledFunction result;
    };
    std::vector<DecompyleResult> decompilationResults;
};

/// Orchestrator untuk HydraDis analysis pipeline.
///
/// Usage:
///   OrchestratorConfig config;
///   config.decompilerBackend = DecompilerBackend::RzGhidra;
///
///   HydraDisOrchestrator orch(config);
///   auto result = orch.analyze("/path/to/libil2cpp.so");
///
///   if (result.success) {
///       for (auto& ds : result.disassemblyResults) {
///           for (auto& instr : ds.result.instructions) { ... }
///       }
///   }
class HydraDisOrchestrator {
public:
    explicit HydraDisOrchestrator(const OrchestratorConfig& config = {});

    /// Analisis satu binary file — jalankan seluruh pipeline.
    AnalysisResult analyze(const std::string& filePath) const;

    /// Analisis dari buffer in-memory.
    AnalysisResult analyzeBuffer(
        const uint8_t* data,
        size_t dataSize
    ) const;

    /// Akses parser (untuk caller yang mau baca header/sections manual).
    std::unique_ptr<IParser> createParser() const;

    /// Akses disassembler (untuk caller yang mau disassemble manual per-section).
    std::unique_ptr<IDisassembler> createDisassembler(DisassemblerArch arch) const;

private:
    /// Dari BinaryHeader.machine + is64Bit → DisassemblerArch.
    /// Return nullopt kalau machine type tidak dikenal.
    static std::optional<DisassemblerArch> machineToArch(
        uint16_t machine,
        bool is64Bit
    );

    /// Cari .text section (executable) dari parsed binary.
    static std::optional<SectionInfo> findTextSection(
        const ParsedBinary& binary
    );

    OrchestratorConfig config_;
};

} // namespace omnibyte::hydradis

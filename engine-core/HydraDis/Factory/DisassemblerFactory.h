#pragma once
// ── DisassemblerFactory.h ─────────────────────────────────────────
// Factory untuk membuat IDisassembler instance berdasarkan arch + backend preference.
// Memisahkan logic pemilihan backend dari caller (Analyzer/Orchestrator).

#include "Disassembler/IDisassembler.h"
#include <memory>
#include <string>

namespace omnibyte::hydradis {

/// Backend pilihan untuk disassembly.
enum class DisassemblerBackend {
    Capstone,   // default — selalu tersedia, ringan
    Rizin,      // via subprocess (belum aktif)
};

/// Factory untuk IDisassembler.
/// Caller cukup specify arch + backend preference, factory handle sisanya.
///
/// Usage:
///   auto disasm = DisassemblerFactory::create(DisassemblerArch::ARM64);
///   auto result = disasm->disassemble(codeBytes, 0x10000);
class DisassemblerFactory {
public:
    /// Buat IDisassembler instance.
    ///
    /// @param arch     target architecture (dari BinaryHeader.machine via IParser)
    /// @param backend  backend pilihan (default: Capstone)
    /// @return unique_ptr ke IDisassembler, nullptr kalau arch/backend tidak didukung
    static std::unique_ptr<IDisassembler> create(
        DisassemblerArch arch,
        DisassemblerBackend backend = DisassemblerBackend::Capstone
    );

    /// Convenience: auto-detect arch dari ELF e_machine value.
    /// Return nullptr kalau machine type tidak dikenal.
    static std::unique_ptr<IDisassembler> createFromMachine(
        uint16_t machine,
        bool is64Bit,
        DisassemblerBackend backend = DisassemblerBackend::Capstone
    );
};

} // namespace omnibyte::hydradis

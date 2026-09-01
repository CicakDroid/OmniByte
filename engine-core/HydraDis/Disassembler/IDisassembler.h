#pragma once
// ── IDisassembler.h ────────────────────────────────────────────────
// Kontrak dasar untuk semua disassembler backend (Capstone, Ghidra, dst).
// Backend yang implement interface ini menangani detail per-arsitektur
// (ARM, ARM64, x86, dst) -- caller tidak perlu tahu.
//
// Design principles:
//   - Arch-agnostic: Instruction struct tidak expose detail Capstone-specific
//   - Minimal: hanya fungsi disassemble() -- extension point lain ditambah saat
//     ada kebutuhan nyata, bukan antisipasi
//   - Ikuti pola IEngineProfile.h / IDumperEngine.h: abstract interface,
//     backend implement detail

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::hydradis {

// ── Data types ─────────────────────────────────────────────────────

/// Satu instruksi hasil disassembly.
/// Sengaja arch-agnostic -- backend Capstone yang isi field-field ini
/// sesuai arsitektur target, tapi caller hanya lihat struct generik.
struct Instruction {
    uint64_t address = 0;       // alamat virtual instruksi ini
    uint16_t size = 0;          // panjang instruksi dalam byte
    std::string mnemonic;       // mnemonic singkat (mis. "mov", "bl", "ret")
    std::string opStr;          // operand string lengkap (mis. "x0, #0x10")
    std::vector<uint8_t> bytes; // raw bytes instruksi (untuk hex dump / re-encode)
};

/// Hasil disassembly dari satu region code.
struct DisassemblyResult {
    bool success = false;
    std::string errorMessage;   // kosong kalau success == true
    std::vector<Instruction> instructions;
    size_t totalBytes = 0;      // total byte yang berhasil di-disassemble
};

// ── Interface ──────────────────────────────────────────────────────

/// Abstract interface untuk disassembler backend.
/// Capstone-adapter, Ghidra-adapter, dll implement ini.
///
/// Usage:
///   std::unique_ptr<IDisassembler> disasm = /* factory */;
///   auto result = disasm->disassemble(codeBytes, 0x10000);
///   for (auto& instr : result.instructions) { ... }
class IDisassembler {
public:
    virtual ~IDisassembler() = default;

    /// Nama backend (mis. "capstone", "ghidra") -- untuk logging/diagnostic.
    virtual std::string name() const = 0;

    /// Decode buffer of bytes jadi list instruksi.
    ///
    /// @param code     raw bytes instruksi (bukan file, hanya code section)
    /// @param baseAddr alamat virtual awal (instruksi pertama = baseAddr)
    /// @param count    max instruksi yang di-decode (0 = unlimited)
    /// @return DisassemblyResult dengan instructions atau errorMessage
    virtual DisassemblyResult disassemble(
        const uint8_t* code,
        size_t codeSize,
        uint64_t baseAddr,
        size_t count = 0  // 0 = decode semua
    ) const = 0;

    /// Convenience overload: decode dari vector<uint8_t>.
    DisassemblyResult disassemble(
        const std::vector<uint8_t>& code,
        uint64_t baseAddr,
        size_t count = 0
    ) const {
        return disassemble(code.data(), code.size(), baseAddr, count);
    }
};

} // namespace omnibyte::hydradis

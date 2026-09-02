#pragma once
// ── IDisassembler.h ────────────────────────────────────────────────
// Kontrak dasar untuk semua disassembler backend (Capstone, Ghidra, dst).
// Backend yang implement interface ini menangani detail per-arsitektur
// (ARM, ARM64, x86, dst) -- caller tidak perlu tahu.
//
// Design principles:
//   - Instance-per-arch: satu IDisassembler instance = satu arsitektur target.
//     Caller (Detector/Analyzer) sudah tahu arch dari ELF e_machine header
//     via IParser SEBELUM panggil disassemble(), jadi arch di-pass ke
//     constructor backend, bukan ke disassemble().
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

/// Architecture target untuk disassembly.
/// Satu instance IDisassembler = satu arch + mode. Caller tentukan arch saat
/// construct backend via factory, bukan saat panggil disassemble().
///
/// Alasan Opsi A (instance-per-arch) dipilih:
/// - Detector/Analyzer sudah tahu arch dari ELF e_machine header via IParser
///   SEBELUM panggil disassemble()
/// - Backend Capstone butuh arch+mode saat cs_open(), tidak bisa diubah setelah
/// - Cleaner API: tidak perlu pass arch di setiap panggilan disassemble()
/// - Ikuti pola "segitiga terbalik" -- arch knowledge di atas, spesialisasi di bawah
enum class DisassemblerArch {
    ARM,        // ARM 32-bit, CS_MODE_ARM (ARM instruction set)
    ARM_Thumb,  // ARM 32-bit, CS_MODE_THUMB (Thumb/Thumb-2 instruction set)
                // Known limitation: satu instance = satu mode tetap. Dalam praktik
                // nyata, .so armeabi-v7a sering punya MIX kode ARM dan Thumb
                // (interworking) tergantung compiler flag per fungsi. Auto-detect
                // per-fungsi (via LSB entry point / symbol flags) di luar scope
                // Tahap ini. Gunakan ARM_Thumb untuk .so yang mayoritas Thumb-2
                // (default NDK armeabi-v7a).
    ARM64,      // AArch64
    x86,        // Intel x86 32-bit
    x86_64,     // Intel x86 64-bit
    MIPS,       // MIPS
    PPC,        // PowerPC
    SPARC,      // SPARC
    SystemZ,    // IBM System/z
    XCore,      // XCore
    M68K,       // Motorola 68000
    TMS320C64X, // TMS320C64x
    M680X,      // Motorola 68000 family
    EVM,        // Ethereum Virtual Machine
};

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
///   // Caller sudah tahu arch dari IParser->parseFile().header.machine
///   std::unique_ptr<IDisassembler> disasm = factory->create(DisassemblerArch::ARM64);
///   auto result = disasm->disassemble(codeBytes, 0x10000);
///   for (auto& instr : result.instructions) { ... }
class IDisassembler {
public:
    virtual ~IDisassembler() = default;

    /// Nama backend (mis. "capstone", "ghidra") -- untuk logging/diagnostic.
    virtual std::string name() const = 0;

    /// Architecture yang di-disassemble oleh instance ini.
    /// Dipilih saat construction, tidak berubah sepanjang lifetime.
    virtual DisassemblerArch arch() const = 0;

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

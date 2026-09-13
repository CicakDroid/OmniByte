#pragma once
// ── IDecompiler.h ──────────────────────────────────────────────────
// Kontrak dasar untuk semua decompiler backend (rz-ghidra, rizin-native, dst).
// Backend yang implement interface ini menangani detail per-tool
// (Ghidra P-code, Rizin ESIL, dst) -- caller tidak perlu tahu.
//
// Design principles:
//   - Tool-agnostic: DecompiledFunction struct tidak expose detail ESIL/P-code
//   - Minimal: hanya fungsi decompile() -- extension point lain ditambah saat
//     ada kebutuhan nyata, bukan antisipasi
//   - Ikuti pola IEngineProfile.h / IDumperEngine.h: abstract interface,
//     backend implement detail
//   - Raw bytes input (bukan Instruction[]): kedua backend nyata (rizin-native,
//     rz-ghidra) membutuhkan raw bytes -- rizin pakai ESIL yang re-disassemble,
//     rz-ghidra feed bytes ke Ghidra decompiler. Tidak ada backend yang bisa
//     consume Instruction objects dari IDisassembler.

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::hydradis {

// ── Data types ─────────────────────────────────────────────────────

/// Seberapa "berat" backend decompiler ini.
/// Dipakai caller untuk keputusan runtime -- jangan auto-jalankan
/// backend Heavy tiap kali, sesuai keputusan lama soal rz-ghidra opt-in.
enum class DecompilerCapability {
    Light,  // cepat, ringan, bisa auto-run (mis. ESIL-based)
    Heavy   // lambat, berat, butuh opt-in (mis. Ghidra-based)
};

/// Hasil decompilation dari satu fungsi.
struct DecompiledFunction {
    bool success = false;
    std::string errorMessage;   // kosong kalau success == true
    uint64_t address = 0;       // alamat awal fungsi
    std::string name;           // nama fungsi (opsional, kosong kalau belum diketahui)
    std::string pseudocode;     // hasil decompilation dalam bahasa C-like
};

// ── Interface ──────────────────────────────────────────────────────

/// Abstract interface untuk decompiler backend.
/// rz-ghidra-adapter, rizin-native, dst implement ini.
///
/// Usage:
///   std::unique_ptr<IDecompiler> decompiler = /* factory */;
///   auto result = decompiler->decompile(codeBytes, codeSize, 0x10000);
///   if (result.success) { use(result.pseudocode); }
class IDecompiler {
public:
    virtual ~IDecompiler() = default;

    /// Nama backend (mis. "rz-ghidra", "rizin-native") -- untuk logging/diagnostic.
    virtual std::string name() const = 0;

    /// Capability level -- caller gunakan untuk keputusan auto-run vs opt-in.
    virtual DecompilerCapability capability() const = 0;

    /// Dekompilasi satu fungsi dari raw bytes.
    ///
    /// @param code     raw bytes fungsi (bukan file, hanya code section)
    /// @param codeSize panjang code dalam byte
    /// @param baseAddr alamat virtual awal (fungsi dimulai dari baseAddr)
    /// @return DecompiledFunction dengan pseudocode atau errorMessage
    virtual DecompiledFunction decompile(
        const uint8_t* code,
        size_t codeSize,
        uint64_t baseAddr
    ) const = 0;

    /// Convenience overload: decompile dari vector<uint8_t>.
    DecompiledFunction decompile(
        const std::vector<uint8_t>& code,
        uint64_t baseAddr
    ) const {
        return decompile(code.data(), code.size(), baseAddr);
    }
};

} // namespace omnibyte::hydradis

#pragma once
// ── IPlugin.h ────────────────────────────────────────────────────
// Kontrak dasar untuk semua HydraDis plugin.
// Semua plugin di Plugin/ wajib implement interface ini.
//
// Design principles:
//   - PluginContext menggabungkan output dari 3 backend HydraDis
//     (IDisassembler, IParser, IDecompiler) supaya plugin tidak perlu
//     akses langsung ke instance backend.
//   - Context menyediakan akses ke SEMUA hasil analisis (multi-section,
//     multi-function), bukan hanya satu — konsisten dengan AnalysisResult
//     yang menyimpan vector<DisasmSection> dan vector<DecompyleResult>.
//   - sourceLabel: opsional, boleh kosong. Untuk logging/laporan
//     (mis. FindCrypt sebut nama file di hasil temuan). Tidak dipakai
//     sebagai dependency — analisis inti jalan dari buffer/bytes,
//     konsisten dengan AnalysisTarget yang represent file atau live dump.
//   - PluginResult bersifat free-form — plugin bisa return JSON, teks,
//     atau structured data via output string + metadata map.
//   - Lifecycle: onLoad() → onRun() (bisa dipanggil berulang) → onUnload()
//   - version(): mandatory tapi non-pure-virtual — default "0.1.0-dev".
//     Plugin yang mau override silakan, yang belum tetap compile.
//   - Config: getConfig(key, default) baca dari unordered_map yang di-pass
//     via context. Minimal, retrofit-friendly untuk 9 plugin yang sudah ada.
//     Beberapa plugin (Emulation/timeout, Runner/sandbox limit) butuh ini
//     sekarang, jadi worth dimasukkan dari awal.

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// Include kontrak HydraDis — dibutuhkan oleh convenience accessors di PluginContext
// yang perlu akses member ParsedBinary, DisassemblyResult, DecompiledFunction.
// Path relatif dari Plugin/ ke HydraDis root (../).
#include "../Parser/IParser.h"
#include "../Disassembler/IDisassembler.h"
#include "../Decompiler/IDecompiler.h"

namespace omnibyte::hydradis::plugin {

/// Plugin interface version — increment saat interface berubah.
/// Plugin lama yang di-load engine baru tetap jalan (backward compatible).
constexpr uint32_t PLUGIN_INTERFACE_VERSION = 1;

/// Context yang di-pass ke IPlugin::onRun().
/// Pointer bisa nullptr kalau backend terkait tidak dijalankan.
/// Menyediakan akses ke SEMUA hasil analisis (multi-section, multi-function).
struct PluginContext {
    // ── Source Label (opsional) ────────────────────────────────────
    // Nama/identifier untuk logging. Bisa kosong untuk live dump tanpa path.
    // Contoh: "libil2cpp.so", "dump_0x7fff0000", "process://pid:1234"
    std::string sourceLabel;

    // ── Parser Output (IParser) ───────────────────────────────────
    // Binary header, semua sections, semua symbols
    const omnibyte::hydradis::ParsedBinary* binary = nullptr;

    // ── Disassembler Output (IDisassembler) ───────────────────────
    // Semua section yang berhasil di-disassemble
    const std::vector<omnibyte::hydradis::DisassemblyResult>* disassemblyResults = nullptr;

    // ── Decompiler Output (IDecompiler) ───────────────────────────
    // Semua fungsi yang berhasil di-decompile
    const std::vector<omnibyte::hydradis::DecompiledFunction>* decompilationResults = nullptr;

    // ── Convenience: single pointers ──────────────────────────────
    // Untuk plugin yang hanya butuh satu section/fungsi:
    //   ctx.disassembly  → section pertama (biasanya .text)
    //   ctx.decompiled   → fungsi pertama
    const omnibyte::hydradis::DisassemblyResult* disassembly = nullptr;
    const omnibyte::hydradis::DecompiledFunction* decompiled = nullptr;

    // ── User Config (opsional) ────────────────────────────────────
    // Key-value config dari user untuk plugin ini.
    // Baca dari sistem config/ yang sudah ada — jangan bikin skema baru.
    // Contoh: {"timeout": "30000", "max_iterations": "100"}
    const std::unordered_map<std::string, std::string>* config = nullptr;

    // ── Convenience Accessors ─────────────────────────────────────

    /// Ambil header binary (mudah akses is64Bit, machine, dst)
    const omnibyte::hydradis::BinaryHeader* header() const {
        return binary ? &binary->header : nullptr;
    }

    /// Ambil semua sections
    const std::vector<omnibyte::hydradis::SectionInfo>* sections() const {
        return binary ? &binary->sections : nullptr;
    }

    /// Ambil semua symbols
    const std::vector<omnibyte::hydradis::SymbolInfo>* symbols() const {
        return binary ? &binary->symbols : nullptr;
    }

    /// Cari section berdasarkan nama
    const omnibyte::hydradis::SectionInfo* findSection(const std::string& name) const {
        if (!binary) return nullptr;
        for (const auto& sec : binary->sections) {
            if (sec.name == name) return &sec;
        }
        return nullptr;
    }

    /// Cari symbol berdasarkan nama
    const omnibyte::hydradis::SymbolInfo* findSymbol(const std::string& name) const {
        if (!binary) return nullptr;
        for (const auto& sym : binary->symbols) {
            if (sym.name == name) return &sym;
        }
        return nullptr;
    }

    /// Ambil config value dengan default
    std::string getConfig(const std::string& key, const std::string& defaultVal = "") const {
        if (!config) return defaultVal;
        auto it = config->find(key);
        return (it != config->end()) ? it->second : defaultVal;
    }
};

/// Hasil eksekusi plugin.
struct PluginResult {
    bool        success = false;
    std::string errorMessage;
    std::string output;         // free-form: JSON, teks, binary, dst

    // Structured output (opsional) — untuk plugin yang mau return metadata
    std::unordered_map<std::string, std::string> metadata;
};

/// Kontrak dasar untuk semua HydraDis plugin.
class IPlugin {
public:
    virtual ~IPlugin() = default;

    // ── Identity (mandatory) ──────────────────────────────────────

    /// Nama plugin (unique identifier, mis. "Enhanced/AST", "FindCrypt")
    virtual std::string name() const = 0;

    /// Versi plugin (semver, mis. "1.0.0").
    /// Default: "0.1.0-dev" — plugin yang belum override tetap compile.
    virtual std::string version() const { return "0.1.0-dev"; }

    // ── Lifecycle ─────────────────────────────────────────────────

    /// Dipanggil sekali saat plugin di-load.
    /// Return false jika plugin tidak bisa initialize (missing dependency, dst).
    virtual bool onLoad() = 0;

    /// Dipanggil berulang kali untuk setiap binary yang dianalisis.
    /// Context berisi output dari parser, disassembler, decompiler.
    virtual PluginResult onRun(const PluginContext& ctx) = 0;

    /// Dipanggil sekali saat plugin di-unload (cleanup resources).
    virtual void onUnload() = 0;

    // ── Metadata (opsional, non-pure-virtual) ─────────────────────

    /// Interface version yang dibutuhkan plugin ini.
    /// Default: PLUGIN_INTERFACE_VERSION. Plugin lama tetap jalan.
    virtual uint32_t requiredInterfaceVersion() const {
        return PLUGIN_INTERFACE_VERSION;
    }

    /// Deskripsi singkat plugin (untuk UI/help text).
    virtual std::string description() const { return ""; }

    /// Dependencies (nama plugin lain yang harus di-load dulu).
    virtual std::vector<std::string> dependencies() const { return {}; }
};

} // namespace omnibyte::hydradis::plugin

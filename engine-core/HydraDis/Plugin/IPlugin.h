#pragma once
// ── IPlugin.h ────────────────────────────────────────────────────
// Kontrak dasar untuk semua HydraDis plugin.
// Semua plugin di Plugin/ wajib implement interface ini.
//
// Design principles:
//   - PluginContext menggabungkan output dari 3 backend HydraDis
//     (IDisassembler, IParser, IDecompiler) supaya plugin tidak perlu
//     akses langsung ke instance backend.
//   - PluginResult bersifat free-form — plugin bisa return JSON, teks,
//     atau structured data via output string.
//   - Lifecycle: onLoad() → onRun() (bisa dipanggil berulang) → onUnload()

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::hydradis {

// Forward declarations dari kontrak HydraDis
struct DisassemblyResult;
struct ParsedBinary;
struct DecompiledFunction;

} // namespace omnibyte::hydradis

namespace omnibyte::hydradis::plugin {

/// Context yang di-pass ke IPlugin::onRun().
/// Pointer bisa nullptr kalau backend terkait tidak dijalankan.
struct PluginContext {
    const omnibyte::hydradis::DisassemblyResult* disassembly = nullptr;
    const omnibyte::hydradis::ParsedBinary*       binary      = nullptr;
    const omnibyte::hydradis::DecompiledFunction* decompiled  = nullptr;
};

/// Hasil eksekusi plugin.
struct PluginResult {
    bool        success = false;
    std::string errorMessage;
    std::string output;
};

/// Kontrak dasar untuk semua HydraDis plugin.
class IPlugin {
public:
    virtual ~IPlugin() = default;

    virtual std::string name() const = 0;

    virtual bool onLoad() = 0;

    virtual PluginResult onRun(const PluginContext& ctx) = 0;

    virtual void onUnload() = 0;
};

} // namespace omnibyte::hydradis::plugin

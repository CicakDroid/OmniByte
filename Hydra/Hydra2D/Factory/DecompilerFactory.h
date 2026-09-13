#pragma once
// ── DecompilerFactory.h ───────────────────────────────────────────
// Factory untuk membuat IDecompiler instance berdasarkan capability preference.
// rz-ghidra = Heavy (opt-in), rizin-native = Light (auto-run).

#include "Decompiler/IDecompiler.h"
#include <memory>
#include <string>

namespace omnibyte::hydradis {

/// Backend pilihan untuk decompilation.
enum class DecompilerBackend {
    Auto,           // pilih berdasarkan capability: Light dulu, Heavy opt-in
    RizinNative,    // subprocess rizin, Light capability
    RzGhidra,       // rz-ghidra library, Heavy capability
};

/// Factory untuk IDecompiler.
///
/// Usage:
///   // Auto: rizin-native untuk quick analysis
///   auto decompiler = DecompilerFactory::create();
///
///   // Explicit: rz-ghidra untuk deep analysis
///   auto decompiler = DecompilerFactory::create(DecompilerBackend::RzGhidra);
class DecompilerFactory {
public:
    /// Buat IDecompiler instance.
    ///
    /// @param backend     backend pilihan (default: Auto → rizin-native)
    /// @param rizinPath   path ke rizin binary (untuk rizin-native, default: "rizin")
    /// @return unique_ptr ke IDecompiler
    static std::unique_ptr<IDecompiler> create(
        DecompilerBackend backend = DecompilerBackend::Auto,
        const std::string& rizinPath = "rizin"
    );

    /// Convenience: buat decompiler berdasarkan DecompilerCapability.
    /// Light → rizin-native, Heavy → rz-ghidra.
    static std::unique_ptr<IDecompiler> createByCapability(
        DecompilerCapability capability,
        const std::string& rizinPath = "rizin"
    );
};

} // namespace omnibyte::hydradis

#pragma once
#include <string>
#include <vector>
#include <memory>
#include "IEngineProfile.h"
#include "DumpResult.h"
#include "AnalysisTarget.h"

namespace omnibyte::dumper {

enum class EngineType {
    UnrealEngine,
    UnityIL2CPP,
    UnityMono,
    Source,
    Source2,
    Godot,
    CryEngine,
    GameMaker,
    Unigine,
    Unknown
};

struct DetectionResult {
    bool matched = false;
    float confidence = 0.0f;      // 0.0 - 1.0, dipakai saat beberapa engine match sebagian
    std::string detectedVersion;  // versi mentah, mis. "IL2CPP metadata v27"
};

// Kontrak dasar: setiap engine (Unreal, IL2CPP, Mono, Source, dst) implement ini.
// DumperCore hanya bicara lewat interface ini -- tidak tahu detail internal tiap engine.
class IDumperEngine {
public:
    virtual ~IDumperEngine() = default;

    virtual EngineType type() const = 0;
    virtual std::string name() const = 0;

    // Dipanggil EngineRegistry/Detector untuk cek apakah target cocok dengan engine ini.
    // Target bisa berupa file (APK/pak/dll) atau live process (via runtime/MemoryIO).
    virtual DetectionResult detect(const AnalysisTarget& target) const = 0;

    // Setelah versi terdeteksi, resolve & load profile struct-offset yang sesuai.
    // Return nullptr kalau versi tidak dikenali -> caller fallback ke profile "generic".
    virtual std::shared_ptr<IEngineProfile> resolveProfile(
        const std::string& detectedVersion) const = 0;

    // Analyzer: baca struktur metadata/header/global-table secara statis.
    // Tidak butuh proses live -- bisa jalan dari file saja.
    virtual DumpResult analyze(const AnalysisTarget& target,
                                const std::shared_ptr<IEngineProfile>& profile) = 0;

    // Resolver: resolve alamat symbol/type/method konkret.
    // Biasanya butuh live process (runtime/SymbolResolver + MemoryIO) untuk ASLR-safe address.
    virtual DumpResult resolveSymbols(const AnalysisTarget& target,
                                       const std::shared_ptr<IEngineProfile>& profile) = 0;

    // Daftar versi SDK yang secara eksplisit didukung (punya Profile sendiri).
    // Dipakai UI untuk info dan untuk validasi sebelum proses jalan.
    virtual std::vector<std::string> supportedVersions() const = 0;
};

} // namespace omnibyte::dumper

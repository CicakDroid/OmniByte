#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <functional>
#include "IEngineProfile.h"
#include "DumpResult.h"
#include "AnalysisTarget.h"

namespace omnibyte::dumper {

// === DumperMode ===
// Mode operasi dumper — menentukan bagaimana target dipilih dan diproses.
enum class DumperMode {
    Auto,       // deteksi otomatis: pilih engine berdasarkan signature/pattern
    Manual,     // user menentukan engine secara eksplisit
    MultiDump   // dump beberapa engine sekaligus (untuk target multi-engine)
};

// === Input target untuk dump request ===
// IFileTarget — target berupa file statis (APK, .pak, .so, .dll, .ung, .pck, .bsp)
struct IFileTarget {
    std::string filePath;
};

// IProcessTarget — target berupa proses live di memori
struct IProcessTarget {
    int pid = 0;
    uintptr_t baseAddress = 0;     // ASLR-resolved base address
    std::string moduleName;         // e.g. "libil2cpp.so"
};

// === DumpRequest ===
// Permintaan dump — semua informasi yang dibutuhkan engine untuk melakukan dump.
struct DumpRequest {
    DumperMode mode = DumperMode::Auto;
    std::variant<IFileTarget, IProcessTarget> target;
    std::string engineOverride;     // kosong = auto-detect

    bool isFileTarget() const { return std::holds_alternative<IFileTarget>(target); }
    bool isProcessTarget() const { return std::holds_alternative<IProcessTarget>(target); }
    const IFileTarget& asFile() const { return std::get<IFileTarget>(target); }
    const IProcessTarget& asProcess() const { return std::get<IProcessTarget>(target); }
};

// === IEngineAnalyzer ===
// Interface untuk analyzer spesifik engine.
// Setiap engine (Unity, Unreal, Godot, Cocos) mengimplementasi interface ini.
class IEngineAnalyzer {
public:
    virtual ~IEngineAnalyzer() = default;
    virtual std::string engineName() const = 0;
    virtual float detectConfidence(const DumpRequest& request) = 0;
    virtual DumpData analyze(const DumpRequest& request) = 0;
};

// === IEngineResolver ===
// Interface untuk resolver spesifik engine.
// Resolver resolve alamat simbol dari dump data mentah.
class IEngineResolver {
public:
    virtual ~IEngineResolver() = default;
    virtual std::string engineName() const = 0;
    virtual void resolve(DumpData& data, int pid, uintptr_t baseAddress) = 0;
};

enum class EngineType {
    UnrealEngine,
    UnityIL2CPP,
    UnityMono,
    Source2,
    Godot,
    GameMaker,
    Cocos2d,
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
    virtual DumpData analyze(const AnalysisTarget& target,
                                const std::shared_ptr<IEngineProfile>& profile) = 0;

    // Resolver: resolve alamat symbol/type/method konkret.
    // Biasanya butuh live process (runtime/SymbolResolver + MemoryIO) untuk ASLR-safe address.
    virtual DumpData resolveSymbols(const AnalysisTarget& target,
                                       const std::shared_ptr<IEngineProfile>& profile) = 0;

    // Daftar versi SDK yang secara eksplisit didukung (punya Profile sendiri).
    // Dipakai UI untuk info dan untuk validasi sebelum proses jalan.
    virtual std::vector<std::string> supportedVersions() const = 0;
};

} // namespace omnibyte::dumper

#pragma once
// ── UnityIL2CPPEngine.h (menyatukan Analyzer + Resolver + Registry glue) ──
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/UnityIL2CPPAnalyzer.h"
#include "Resolver/UnityIL2CPPResolver.h"
#include "Profiles/V24Profile.h"
#include "Profiles/V24_1Profile.h"
#include "Profiles/V24_2Profile.h"
#include "Profiles/V27Profile.h"
#include "Profiles/V29Profile.h"
#include "Profiles/V31Profile.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::unityil2cpp {

class UnityIL2CPPEngine : public IDumperEngine {
public:
    EngineType type() const override { return EngineType::UnityIL2CPP; }
    std::string name() const override { return "Unity IL2CPP"; }

    DetectionResult detect(const AnalysisTarget& target) const override {
        // 1. cari libil2cpp.so di target (APK lib/ atau live process maps)
        // 2. cari global-metadata.dat, cek magic 0xAF1BB1FA
        // 3. kalau match, baca version field mentah dari header
        DetectionResult r;
        (void)target;
        // r.matched = ...; r.confidence = ...; r.detectedVersion = "27";
        return r;
    }

    std::shared_ptr<IEngineProfile> resolveProfile(
        const std::string& detectedVersion) const override {
        if (detectedVersion == "24") return std::make_shared<V24Profile>();
        if (detectedVersion == "24.1") return std::make_shared<V24_1Profile>();
        if (detectedVersion == "24.2") return std::make_shared<V24_2Profile>();
        if (detectedVersion == "27") return std::make_shared<V27Profile>();
        if (detectedVersion == "29") return std::make_shared<V29Profile>();
        if (detectedVersion == "31") return std::make_shared<V31Profile>();
        return nullptr; // -> caller fallback ke generic profile / minta pilih manual
    }

    // Analyzer: baca global-metadata.dat pakai profile->offsetOf(...)
    DumpResult analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        return UnityIL2CPPAnalyzer::analyze(target, profile);
    }

    // Resolver: butuh live process -- pakai runtime/SymbolResolver (xDL)
    // untuk resolve alamat libil2cpp.so yang sudah di-load, lalu cross-reference
    // dengan hasil analyze() di atas untuk dapat alamat konkret tiap method.
    DumpResult resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        return UnityIL2CPPResolver::resolveSymbols(target, profile);
    }

    std::vector<std::string> supportedVersions() const override {
        return {"24", "24.1", "24.2", "27", "29", "31"}; // v31 = versi metadata terbaru saat ini (Unity 6000.x & 2022.3 LTS masih pakai v31)
    }
};

} // namespace omnibyte::dumper::unityil2cpp
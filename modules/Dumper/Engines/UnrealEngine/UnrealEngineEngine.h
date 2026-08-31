#pragma once
// ── UnrealEngineEngine.h (menyatukan Analyzer + Resolver + Registry glue) ──
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/UnrealEngineAnalyzer.h"
#include "Resolver/UnrealEngineResolver.h"
#include "Profiles/UE5_3Profile.h"
#include "Profiles/UE5_4Profile.h"
#include "Profiles/UE5_5Profile.h"
#include "Profiles/UE5_6Profile.h"
#include "Profiles/UE5_7Profile.h"
#include "Profiles/UE5_8Profile.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::unrealengine {

class UnrealEngineDumper : public IDumperEngine {
public:
    EngineType type() const override { return EngineType::UnrealEngine; }
    std::string name() const override { return "Unreal Engine"; }

    DetectionResult detect(const AnalysisTarget& target) const override {
        // sinyal: .pak magic 0x5A6F12E1, string "UE4"/"UE5" di binary,
        // struct GEngineVersion untuk build number persis
        DetectionResult r;
        (void)target;
        return r;
    }

    std::shared_ptr<IEngineProfile> resolveProfile(const std::string& detectedVersion) const override {
        // Semua versi yang didukung sekarang di rentang UE5 (5.3-5.8, lihat supportedVersions()).
        // Profile konkret: UE5_3 (oldest) & UE5_8 (latest); versi 5.4/5.5/5.6/5.7
        // pakai skeleton sampai offset-nya diisi.
        if (detectedVersion == "5.3") return std::make_shared<UE5_3Profile>();
        if (detectedVersion == "5.4") return std::make_shared<UE5_4Profile>();
        if (detectedVersion == "5.5") return std::make_shared<UE5_5Profile>();
        if (detectedVersion == "5.6") return std::make_shared<UE5_6Profile>();
        if (detectedVersion == "5.7") return std::make_shared<UE5_7Profile>();
        if (detectedVersion == "5.8") return std::make_shared<UE5_8Profile>();
        return nullptr; // -> caller fallback ke generic profile / minta pilih manual
    }

    // Analyzer: baca GNames/GObjects table statis dari .pak / binary section
    DumpResult analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        return UnrealEngineAnalyzer::analyze(target, profile);
    }

    // Resolver: resolve GWorld/GObjects live via runtime/SymbolResolver (xDL) + MemoryIO
    DumpResult resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        return UnrealEngineResolver::resolveSymbols(target, profile);
    }

    std::vector<std::string> supportedVersions() const override {
        return {"5.3", "5.4", "5.5", "5.6", "5.7", "5.8"}; // 5.8 = rilis mayor UE5 terakhir (Jun 2026), UE6 belum publik
    }
};

} // namespace omnibyte::dumper::unrealengine
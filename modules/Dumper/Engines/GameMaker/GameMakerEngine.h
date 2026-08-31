#pragma once
// ── GameMakerEngine.h (menyatukan Analyzer + Resolver + Registry glue) ──
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/GameMakerAnalyzer.h"
#include "Resolver/GameMakerResolver.h"
#include "Profiles/GM2_3Profile.h"
#include "Profiles/GM2022Profile.h"
#include "Profiles/GM2023Profile.h"
#include "Profiles/GM2024Profile.h"
#include "Profiles/GM2024_14Profile.h"
#include "Profiles/GM2026Profile.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::gamemaker {

class GameMakerDumper : public IDumperEngine {
public:
    EngineType type() const override { return EngineType::GameMaker; }
    std::string name() const override { return "GameMaker"; }

    DetectionResult detect(const AnalysisTarget& target) const override {
        // sinyal: data.win magic "YYYG"; yang lama "FORM" (GMS1 / <2.3)
        DetectionResult r;
        (void)target;
        return r;
    }

    std::shared_ptr<IEngineProfile> resolveProfile(const std::string& detectedVersion) const override {
        // GameMaker pakai PREFIX match (version string bisa "2.3.7" / "2024.14.0").
        if (detectedVersion.rfind("2.", 0) == 0) return std::make_shared<GM2_3Profile>();
        if (detectedVersion.rfind("2022", 0) == 0) return std::make_shared<GM2022Profile>();
        if (detectedVersion.rfind("2023", 0) == 0) return std::make_shared<GM2023Profile>();
        if (detectedVersion.rfind("2024.14", 0) == 0) return std::make_shared<GM2024_14Profile>(); // cek dulu yg lebih spesifik
        if (detectedVersion.rfind("2024", 0) == 0) return std::make_shared<GM2024Profile>();
        if (detectedVersion.rfind("2026", 0) == 0) return std::make_shared<GM2026Profile>();
        return nullptr; // -> caller fallback ke generic profile / minta pilih manual
    }

    // Analyzer: parse data.win chunk statis (FORM/YYYG -> GEN8/STRG/OBJT)
    DumpResult analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        return GameMakerAnalyzer::analyze(target, profile);
    }

    // Resolver: resolve runtime struct offets via live process (yoyorun)
    DumpResult resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        return GameMakerResolver::resolveSymbols(target, profile);
    }

    std::vector<std::string> supportedVersions() const override {
        return {"2.3", "2022", "2023", "2024", "2024.14", "2026.0"}; // seri 2023+ pakai penomoran tahun
    }
};

} // namespace omnibyte::dumper::gamemaker
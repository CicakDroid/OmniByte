#pragma once
// ── CryEngineEngine.h (menyatukan Analyzer + Resolver + Registry glue) ──
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/CryEngineAnalyzer.h"
#include "Resolver/CryEngineResolver.h"
#include "Profiles/CryEngine5_2Profile.h"
#include "Profiles/CryEngine5_3Profile.h"
#include "Profiles/CryEngine5_4Profile.h"
#include "Profiles/CryEngine5_5Profile.h"
#include "Profiles/CryEngine5_6Profile.h"
#include "Profiles/CryEngine5_7Profile.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::cryengine {

class CryEngineDumper : public IDumperEngine {
public:
    EngineType type() const override { return EngineType::CryEngine; }
    std::string name() const override { return "CryEngine"; }

    DetectionResult detect(const AnalysisTarget& target) const override {
        // sinyal: .pak container, CrySystem.dll/.so, string "CryEngine"
        // di engine binary / system.cfg
        DetectionResult r;
        (void)target;
        return r;
    }

    std::shared_ptr<IEngineProfile> resolveProfile(const std::string& detectedVersion) const override {
        // 5.2 & 5.7 konkret; 5.3-5.6 skeleton sampai offset-nya diisi.
        if (detectedVersion == "5.2") return std::make_shared<CryEngine5_2Profile>();
        if (detectedVersion == "5.3") return std::make_shared<CryEngine5_3Profile>();
        if (detectedVersion == "5.4") return std::make_shared<CryEngine5_4Profile>();
        if (detectedVersion == "5.5") return std::make_shared<CryEngine5_5Profile>();
        if (detectedVersion == "5.6") return std::make_shared<CryEngine5_6Profile>();
        if (detectedVersion == "5.7") return std::make_shared<CryEngine5_7Profile>();
        return nullptr; // -> caller fallback ke generic profile / minta pilih manual
    }

    // Analyzer: parse .pak container statis (CryPak magic)
    DumpResult analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        return CryEngineAnalyzer::analyze(target, profile);
    }

    // Resolver: resolve ISystem::GetISystem / class registry via live process
    DumpResult resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        return CryEngineResolver::resolveSymbols(target, profile);
    }

    std::vector<std::string> supportedVersions() const override {
        return {"5.2", "5.3", "5.4", "5.5", "5.6", "5.7"}; // seri 5.x mulai 5.2
    }
};

} // namespace omnibyte::dumper::cryengine
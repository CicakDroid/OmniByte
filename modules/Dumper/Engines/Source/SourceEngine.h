#pragma once
// ── SourceEngine.h (menyatukan Analyzer + Resolver + Registry glue) ──
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/SourceAnalyzer.h"
#include "Resolver/SourceResolver.h"
#include "Profiles/Source2004Profile.h"
#include "Profiles/Source2007Profile.h"
#include "Profiles/Source2013Profile.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::source {

class SourceDumper : public IDumperEngine {
public:
    EngineType type() const override { return EngineType::Source; }
    std::string name() const override { return "Source Engine"; }

    DetectionResult detect(const AnalysisTarget& target) const override {
        // sinyal: .bsp map format, engine.dll/.so, string "Source Engine" / build number
        DetectionResult r;
        (void)target;
        return r;
    }

    std::shared_ptr<IEngineProfile> resolveProfile(const std::string& detectedVersion) const override {
        if (detectedVersion == "2004") return std::make_shared<Source2004Profile>();
        if (detectedVersion == "2007") return std::make_shared<Source2007Profile>();
        if (detectedVersion == "2013") return std::make_shared<Source2013Profile>();
        return nullptr; // -> caller fallback ke generic profile / minta pilih manual
    }

    // Analyzer: parse .bsp lump table + entity string statis
    DumpResult analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        return SourceAnalyzer::analyze(target, profile);
    }

    // Resolver: resolve interface factory (CreateInterface) live via SymbolResolver
    DumpResult resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        return SourceResolver::resolveSymbols(target, profile);
    }

    std::vector<std::string> supportedVersions() const override {
        return {"2004", "2007", "2013"}; // Source 1 cuma 3 branch publik dikenal, tidak ada versi granular lain
    }
};

} // namespace omnibyte::dumper::source
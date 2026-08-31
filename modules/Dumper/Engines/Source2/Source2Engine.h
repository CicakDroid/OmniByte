#pragma once
// ── Source2Engine.h (menyatukan Analyzer + Resolver + Registry glue) ──
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/Source2Analyzer.h"
#include "Resolver/Source2Resolver.h"
#include "Profiles/Source2_2015Profile.h"
#include "Profiles/Source2_2020Profile.h"
#include "Profiles/Source2_2023Profile.h"
#include "Profiles/Source2_2025Profile.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::source2 {

class Source2Dumper : public IDumperEngine {
public:
    EngineType type() const override { return EngineType::Source2; }
    std::string name() const override { return "Source 2"; }

    DetectionResult detect(const AnalysisTarget& target) const override {
        // sinyal: resource compiler format .vpk_c (beda dari .vpk Source 1)
        DetectionResult r;
        (void)target;
        return r;
    }

    std::shared_ptr<IEngineProfile> resolveProfile(const std::string& detectedVersion) const override {
        if (detectedVersion == "2015") return std::make_shared<Source2_2015Profile>();
        if (detectedVersion == "2020") return std::make_shared<Source2_2020Profile>();
        if (detectedVersion == "2023") return std::make_shared<Source2_2023Profile>();
        if (detectedVersion == "2025") return std::make_shared<Source2_2025Profile>();
        return nullptr; // -> caller fallback ke generic profile / minta pilih manual
    }

    // Analyzer: parse resource block header .vpk_c statis
    DumpResult analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        return Source2Analyzer::analyze(target, profile);
    }

    // Resolver: resolve symbol/address konkret (butuh live process)
    DumpResult resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        return Source2Resolver::resolveSymbols(target, profile);
    }

    std::vector<std::string> supportedVersions() const override {
        return {"2015", "2020", "2023", "2025"}; // Source 2 versioning tidak granular per-tahun seperti engine lain
    }
};

} // namespace omnibyte::dumper::source2
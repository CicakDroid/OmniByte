#pragma once
// ── UnigineEngine.h (menyatukan Analyzer + Resolver + Registry glue) ──
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/UnigineAnalyzer.h"
#include "Resolver/UnigineResolver.h"
#include "Profiles/Unigine2_16Profile.h"
#include "Profiles/Unigine2_17Profile.h"
#include "Profiles/Unigine2_18Profile.h"
#include "Profiles/Unigine2_19Profile.h"
#include "Profiles/Unigine2_20Profile.h"
#include "Profiles/Unigine2_21Profile.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::unigine {

class UnigineDumper : public IDumperEngine {
public:
    EngineType type() const override { return EngineType::Unigine; }
    std::string name() const override { return "UNIGINE"; }

    DetectionResult detect(const AnalysisTarget& target) const override {
        // sinyal: .unisync/.dds package, libUnigine_x64.so/.dll,
        // string "UNIGINE" / version di metadata project; .xml world files
        DetectionResult r;
        (void)target;
        return r;
    }

    std::shared_ptr<IEngineProfile> resolveProfile(const std::string& detectedVersion) const override {
        // 2.16 == rilis terawal yg didukung; sisanya skeleton sampai offset diisi.
        if (detectedVersion == "2.16") return std::make_shared<Unigine2_16Profile>();
        if (detectedVersion == "2.17") return std::make_shared<Unigine2_17Profile>();
        if (detectedVersion == "2.18") return std::make_shared<Unigine2_18Profile>();
        if (detectedVersion == "2.19") return std::make_shared<Unigine2_19Profile>();
        if (detectedVersion == "2.20") return std::make_shared<Unigine2_20Profile>();
        if (detectedVersion == "2.21") return std::make_shared<Unigine2_21Profile>();
        return nullptr; // -> caller fallback ke generic profile / minta pilih manual
    }

    // Analyzer: parse .unisync package & .xml world statis
    DumpResult analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        return UnigineAnalyzer::analyze(target, profile);
    }

    // Resolver: resolve Engine::get() / class registry via live process
    DumpResult resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        return UnigineResolver::resolveSymbols(target, profile);
    }

    std::vector<std::string> supportedVersions() const override {
        return {"2.16", "2.17", "2.18", "2.19", "2.20", "2.21"}; // seri 2.x LTS dari 2.16 sampai 2.21
    }
};

} // namespace omnibyte::dumper::unigine
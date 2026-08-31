#pragma once
// ── UnityMonoEngine.h (menyatukan Analyzer + Resolver + Registry glue) ──
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/UnityMonoAnalyzer.h"
#include "Resolver/UnityMonoResolver.h"
#include "Profiles/Mono2022_3Profile.h"
#include "Profiles/Mono6000_0Profile.h"
#include "Profiles/Mono6000_1Profile.h"
#include "Profiles/Mono6000_2Profile.h"
#include "Profiles/Mono6000_3Profile.h"
#include "Profiles/Mono6000_4Profile.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::unitymono {

class UnityMonoDumper : public IDumperEngine {
public:
    EngineType type() const override { return EngineType::UnityMono; }
    std::string name() const override { return "Unity Mono"; }

    DetectionResult detect(const AnalysisTarget& target) const override {
        // sinyal: libmono.so / libmonobdwgc-2.0.so, Assembly-CSharp.dll,
        // symbol mono_get_root_domain; versi dari string di globalgamemanagers
        DetectionResult r;
        (void)target;
        return r;
    }

    std::shared_ptr<IEngineProfile> resolveProfile(const std::string& detectedVersion) const override {
        // 2 profile konkret dulu (oldest LTS & latest Unity 6). Versi 6000.0-6000.3
        // pakai skeleton sampai offset-nya diisi.
        if (detectedVersion == "2022.3") return std::make_shared<Mono2022_3Profile>();
        if (detectedVersion == "6000.0") return std::make_shared<Mono6000_0Profile>();
        if (detectedVersion == "6000.1") return std::make_shared<Mono6000_1Profile>();
        if (detectedVersion == "6000.2") return std::make_shared<Mono6000_2Profile>();
        if (detectedVersion == "6000.3") return std::make_shared<Mono6000_3Profile>();
        if (detectedVersion == "6000.4") return std::make_shared<Mono6000_4Profile>();
        return nullptr; // -> caller fallback ke generic profile / minta pilih manual
    }

    // Analyzer: enumerasi MonoImage -> MonoClass -> MonoMethod dari
    // Assembly-CSharp.dll (statis, via IL parsing)
    DumpResult analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        return UnityMonoAnalyzer::analyze(target, profile);
    }

    // Resolver: resolve mono_get_root_domain & domain assemblies
    // via runtime/SymbolResolver (xDL)
    DumpResult resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        return UnityMonoResolver::resolveSymbols(target, profile);
    }

    std::vector<std::string> supportedVersions() const override {
        return {"2022.3", "6000.0", "6000.1", "6000.2", "6000.3", "6000.4"}; // seri Unity 6 (6000.x), 2022.3 LTS masih banyak dipakai
    }
};

} // namespace omnibyte::dumper::unitymono
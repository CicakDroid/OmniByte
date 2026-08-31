#pragma once
// ── GodotEngine.h (menyatukan Analyzer + Resolver + Registry glue) ──
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/GodotAnalyzer.h"
#include "Resolver/GodotResolver.h"
#include "Profiles/Godot4_2Profile.h"
#include "Profiles/Godot4_3Profile.h"
#include "Profiles/Godot4_4Profile.h"
#include "Profiles/Godot4_5Profile.h"
#include "Profiles/Godot4_6Profile.h"
#include "Profiles/Godot4_7Profile.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::godot {

class GodotDumper : public IDumperEngine {
public:
    EngineType type() const override { return EngineType::Godot; }
    std::string name() const override { return "Godot Engine"; }

    DetectionResult detect(const AnalysisTarget& target) const override {
        // sinyal: .pck package footer magic "GDPC", .godot project file,
        // exe name godot_linux / godot4(.exe)
        DetectionResult r;
        (void)target;
        return r;
    }

    std::shared_ptr<IEngineProfile> resolveProfile(const std::string& detectedVersion) const override {
        // 4.2 & 4.7 konkret; 4.3-4.6 skeleton sampai offset-nya diisi.
        if (detectedVersion == "4.2") return std::make_shared<Godot4_2Profile>();
        if (detectedVersion == "4.3") return std::make_shared<Godot4_3Profile>();
        if (detectedVersion == "4.4") return std::make_shared<Godot4_4Profile>();
        if (detectedVersion == "4.5") return std::make_shared<Godot4_5Profile>();
        if (detectedVersion == "4.6") return std::make_shared<Godot4_6Profile>();
        if (detectedVersion == "4.7") return std::make_shared<Godot4_7Profile>();
        return nullptr; // -> caller fallback ke generic profile / minta pilih manual
    }

    // Analyzer: parse .pck package statis (GDPC magic + file table)
    DumpResult analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        return GodotAnalyzer::analyze(target, profile);
    }

    // Resolver: resolve StringName::setup / ClassDB singleton via live process
    DumpResult resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        return GodotResolver::resolveSymbols(target, profile);
    }

    std::vector<std::string> supportedVersions() const override {
        return {"4.2", "4.3", "4.4", "4.5", "4.6", "4.7"}; // rilis 4.x mulai 4.2
    }
};

} // namespace omnibyte::dumper::godot
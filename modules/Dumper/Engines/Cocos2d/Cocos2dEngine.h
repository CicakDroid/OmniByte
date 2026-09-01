#pragma once
// ── Cocos2dEngine.h (menyatukan Analyzer + Resolver + Registry glue) ──
// Supports two distinct architectures:
//   1. Cocos2d-x (classic): Lua / JavaScript / Pure C++ scripting
//   2. Cocos Creator (modern): TypeScript / JavaScript with entity-component
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/Cocos2dAnalyzer.h"
#include "Resolver/Cocos2dResolver.h"
#include "Profiles/Cocos2dxLuaProfile.h"
#include "Profiles/Cocos2dxJsProfile.h"
#include "Profiles/CocosCreatorProfile.h"
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::cocos2d {

// Cocos2d variant classification
enum class Cocos2dVariant {
    Unknown,
    Cocos2dxLua,    // Cocos2d-x with Lua scripting (libcocos2dlua.so)
    Cocos2dxJs,     // Cocos2d-x with JavaScript scripting (libcocos2djs.so)
    Cocos2dxCpp,    // Cocos2d-x pure C++ (libgame.so / libcocos2dcpp.so)
    CocosCreator    // Cocos Creator modern (libcocos.so or generic)
};

class Cocos2dDumper : public IDumperEngine {
public:
    EngineType type() const override { return EngineType::Cocos2d; }
    std::string name() const override { return "Cocos2d"; }

    DetectionResult detect(const AnalysisTarget& target) const override {
        // Detection signals:
        //   Cocos2d-x: libcocos2dlua.so / libcocos2djs.so / libgame.so in lib/<abi>/
        //              org.cocos2dx.* DEX classes, .lua/.luac/.jsc in assets/
        //   Cocos Creator: libcocos.so, main.js/game.js in assets/, cc.* API patterns
        DetectionResult r;
        (void)target;
        return r;
    }

    std::shared_ptr<IEngineProfile> resolveProfile(const std::string& detectedVersion) const override {
        // Variant detection determines profile:
        //   "cocos2dx-lua"  -> Cocos2dxLuaProfile
        //   "cocos2dx-js"   -> Cocos2dxJsProfile
        //   "cocos-creator" -> CocosCreatorProfile
        if (detectedVersion == "cocos2dx-lua")  return std::make_shared<Cocos2dxLuaProfile>();
        if (detectedVersion == "cocos2dx-js")   return std::make_shared<Cocos2dxJsProfile>();
        if (detectedVersion == "cocos-creator") return std::make_shared<CocosCreatorProfile>();
        return nullptr;
    }

    // Analyzer: parse APK assets for Cocos2d detection signals (static, no live process)
    DumpResult analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        return Cocos2dAnalyzer::analyze(target, profile);
    }

    // Resolver: resolve engine symbols via live process (if needed)
    DumpResult resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        return Cocos2dResolver::resolveSymbols(target, profile);
    }

    std::vector<std::string> supportedVersions() const override {
        return {"cocos2dx-lua", "cocos2dx-js", "cocos-creator"};
    }

    // Utility: detect Cocos2d variant from library name
    static Cocos2dVariant classifyVariant(const std::string& libName) {
        if (libName == "libcocos2dlua.so") return Cocos2dVariant::Cocos2dxLua;
        if (libName == "libcocos2djs.so")  return Cocos2dVariant::Cocos2dxJs;
        if (libName == "libgame.so" || libName == "libcocos2dcpp.so")
            return Cocos2dVariant::Cocos2dxCpp;
        if (libName == "libcocos.so")      return Cocos2dVariant::CocosCreator;
        return Cocos2dVariant::Unknown;
    }
};

} // namespace omnibyte::dumper::cocos2d

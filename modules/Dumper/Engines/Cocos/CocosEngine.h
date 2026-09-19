#pragma once
// ── Cocos2dEngine.h (menyatukan Analyzer + Resolver + Registry glue) ──
// Supports two distinct architectures:
//   1. Cocos2d-x (classic): Lua / JavaScript / Pure C++ scripting
//      - v2.x (2.0-2.2): CC-prefixed symbols (CCDirector, CCSprite)
//      - v3.x (3.0-3.17): Non-prefixed symbols, EventDispatcher added
//      - v4.x (4.0): Metal support, JSB removed, Lua only
//   2. Cocos Creator (modern): TypeScript / JavaScript with entity-component
//      - v1.x (1.0-1.10): Early Creator, based on Cocos2d-x runtime
//      - v2.x (2.0-2.4): Mature Creator, Asset Manager
//      - v3.x (3.0-3.8): Completely rewritten, new 3D core (libcocos.so)
#include "../../DumperCore/IDumperEngine.h"
#include "../../DumperCore/IEngineProfile.h"
#include "Analyzer/Cocos2dAnalyzer.h"
#include "Resolver/Cocos2dResolver.h"
// Variant profiles (scripting-based detection)
#include "Profiles/Cocos2dxLuaProfile.h"
#include "Profiles/Cocos2dxJsProfile.h"
#include "Profiles/CocosCreatorProfile.h"
// Version-specific profiles (per-version symbol differences)
#include "Profiles/Cocos2d-xV2Profile.h"
#include "Profiles/Cocos2d-xV3Profile.h"
#include "Profiles/Cocos2d-xV4Profile.h"
#include "Profiles/CocosCreatorV1Profile.h"
#include "Profiles/CocosCreatorV2Profile.h"
#include "Profiles/CocosCreatorV3Profile.h"
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
        // Variant detection determines profile (version-specific when available):
        //   "cocos2dx-v2"        -> Cocos2dxV2Profile
        //   "cocos2dx-v3"        -> Cocos2dxV3Profile
        //   "cocos2dx-v4"        -> Cocos2dxV4Profile
        //   "cocos2dx-lua"       -> Cocos2dxLuaProfile (fallback, version unknown)
        //   "cocos2dx-js"        -> Cocos2dxJsProfile (fallback, version unknown)
        //   "cocos-creator-v1"   -> CocosCreatorV1Profile
        //   "cocos-creator-v2"   -> CocosCreatorV2Profile
        //   "cocos-creator-v3"   -> CocosCreatorV3Profile
        //   "cocos-creator"      -> CocosCreatorProfile (fallback, version unknown)
        if (detectedVersion == "cocos2dx-v2")        return std::make_shared<Cocos2dxV2Profile>();
        if (detectedVersion == "cocos2dx-v3")        return std::make_shared<Cocos2dxV3Profile>();
        if (detectedVersion == "cocos2dx-v4")        return std::make_shared<Cocos2dxV4Profile>();
        if (detectedVersion == "cocos2dx-lua")       return std::make_shared<Cocos2dxLuaProfile>();
        if (detectedVersion == "cocos2dx-js")        return std::make_shared<Cocos2dxJsProfile>();
        if (detectedVersion == "cocos-creator-v1")   return std::make_shared<CocosCreatorV1Profile>();
        if (detectedVersion == "cocos-creator-v2")   return std::make_shared<CocosCreatorV2Profile>();
        if (detectedVersion == "cocos-creator-v3")   return std::make_shared<CocosCreatorV3Profile>();
        if (detectedVersion == "cocos-creator")      return std::make_shared<CocosCreatorProfile>();
        return nullptr;
    }

    // Analyzer: parse APK assets for Cocos2d detection signals (static, no live process)
    DumpData analyze(const AnalysisTarget& target,
                        const std::shared_ptr<IEngineProfile>& profile) override {
        return Cocos2dAnalyzer::analyze(target, profile);
    }

    // Resolver: resolve engine symbols via live process (if needed)
    DumpData resolveSymbols(const AnalysisTarget& target,
                               const std::shared_ptr<IEngineProfile>& profile) override {
        return Cocos2dResolver::resolveSymbols(target, profile);
    }

    std::vector<std::string> supportedVersions() const override {
        return {
            // Cocos2d-x version-specific
            "cocos2dx-v2", "cocos2dx-v3", "cocos2dx-v4",
            // Cocos2d-x variant fallbacks
            "cocos2dx-lua", "cocos2dx-js",
            // Cocos Creator version-specific
            "cocos-creator-v1", "cocos-creator-v2", "cocos-creator-v3",
            // Cocos Creator variant fallback
            "cocos-creator"
        };
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

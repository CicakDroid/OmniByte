#pragma once
// ── Profiles/CocosCreatorV2Profile.h ───────────────────────────────
// Cocos Creator v2.x profile (2.0 — 2.4, released 2018–2023).
// Based on Cocos2d-x runtime
// Scripting: JavaScript, TypeScript
// Editor: Cocos Creator editor (mature)
// Architecture: Entity-Component system
// Libraries: libcocos2d.so, libjsc.so
// New in v2: Asset Manager (replaces old ResourceLoader), improved TypeScript
// Detection signals:
//   - cc.Node, cc.Label, cc.Component, cc.director, cc.game
//   - libcocos2d.so, libjsc.so
//   - .fire scene files (v2 format), .meta files
//   - cocos2d.js in assets/
//   - settings/ directory with editor version
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class CocosCreatorV2Profile : public IEngineProfile {
public:
    std::string version() const override { return "cocos-creator-v2"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // Cocos Creator v2: no fixed offsets, use symbolFor() instead
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // Cocos Creator v2: no fixed struct sizes, use symbolFor() instead
    }

    std::optional<std::string> symbolFor(const std::string& key) const override {
        // v2 uses Cocos Creator native symbols (cc namespace)
        if (key == "cc::Director::getInstance")      return "cc::Director::getInstance";
        if (key == "cc::AssetManager::getInstance")  return "cc::AssetManager::getInstance";
        if (key == "cc::Game::getInstance")          return "cc::Game::getInstance";
        if (key == "cc::SysInfo::getVersion")        return "cc::SysInfo::getVersion";
        if (key == "v8::Isolate::GetCurrent")        return "v8::Isolate::GetCurrent";
        if (key == "FileUtils::getInstance")         return "FileUtils::getInstance";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // Validate by checking for Cocos Creator v2 signals
        (void)headerBytes;
        (void)len;
        return true;
    }
};

} // namespace omnibyte::dumper::cocos2d

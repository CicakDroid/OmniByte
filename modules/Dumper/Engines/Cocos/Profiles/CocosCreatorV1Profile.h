#pragma once
// ── Profiles/CocosCreatorV1Profile.h ───────────────────────────────
// Cocos Creator v1.x profile (1.0 — 1.10, released 2017–2018).
// Based on Cocos2d-x runtime underneath
// Scripting: JavaScript (TypeScript support added in later 1.x)
// Editor: Cocos Creator editor (early version)
// Architecture: Entity-Component system
// Libraries: libcocos2d.so, libjsc.so
// Detection signals:
//   - cc.Node, cc.Label, cc.Component
//   - libcocos2d.so, libjsc.so
//   - .fire scene files (v1 format)
//   - cocos2d.js in assets/
//   - settings/ directory with editor version
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class CocosCreatorV1Profile : public IEngineProfile {
public:
    std::string version() const override { return "cocos-creator-v1"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // Cocos Creator v1: no fixed offsets, use symbolFor() instead
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // Cocos Creator v1: no fixed struct sizes, use symbolFor() instead
    }

    std::optional<std::string> symbolFor(const std::string& key) const override {
        // v1 uses Cocos2d-x runtime symbols (cc namespace in JS, cocos2d:: in native)
        if (key == "cc::Director::getInstance")      return "cc::Director::getInstance";
        if (key == "cc::AssetManager::getInstance")  return "cc::AssetManager::getInstance";
        if (key == "cc::Game::getInstance")          return "cc::Game::getInstance";
        // v1 uses cocos2d-x runtime — no cc::SysInfo yet
        if (key == "v8::Isolate::GetCurrent")        return "v8::Isolate::GetCurrent";
        if (key == "FileUtils::getInstance")         return "FileUtils::getInstance";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // Validate by checking for Cocos Creator v1 signals
        (void)headerBytes;
        (void)len;
        return true;
    }
};

} // namespace omnibyte::dumper::cocos2d

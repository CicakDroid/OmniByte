#pragma once
// ── Profiles/CocosCreatorProfile.h ─────────────────────────────────
// Cocos Creator (modern) profile.
// Primary library: libcocos.so (or generic project name)
// Script location: assets/ (main.js, game.js, *.ts compiled to *.js)
// Architecture: Entity-Component system (not Scene/Layer/Node)
// Detection signals:
//   - libcocos.so in lib/<abi>/
//   - main.js / game.js in assets/
//   - .json texture atlases (not .plist)
//   - cc.assetManager, cc.director API patterns
//   - settings/ directory with editor version
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class CocosCreatorProfile : public IEngineProfile {
public:
    std::string version() const override { return "cocos-creator"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // Cocos Creator: no fixed offsets, use symbolFor() instead
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // Cocos Creator: no fixed struct sizes, use symbolFor() instead
    }

    std::optional<std::string> symbolFor(const std::string& key) const override {
        if (key == "cc::AssetManager::getInstance")  return "cc::AssetManager::getInstance";
        if (key == "cc::Director::getInstance")      return "cc::Director::getInstance";
        if (key == "cc::Game::getInstance")          return "cc::Game::getInstance";
        if (key == "cc::SysInfo::getVersion")        return "cc::SysInfo::getVersion";
        if (key == "v8::Isolate::GetCurrent")        return "v8::Isolate::GetCurrent";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // Validate by checking for libcocos.so presence (done at detector level)
        // This profile is selected only after variant detection confirms Cocos Creator
        (void)headerBytes;
        (void)len;
        return true;
    }
};

} // namespace omnibyte::dumper::cocos2d

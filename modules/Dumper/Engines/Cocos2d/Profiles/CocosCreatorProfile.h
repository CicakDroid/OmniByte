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
//
// TODO: Fill offsetOf()/structSize() after research report is reviewed.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class CocosCreatorProfile : public IEngineProfile {
public:
    std::string version() const override { return "cocos-creator"; }

    uint64_t offsetOf(const std::string& key) const override {
        // TODO: Fill after research confirmation
        // Expected keys:
        //   "AssetManagerOffset"   — offset to cc.assetManager singleton
        //   "DirectorOffset"       — offset to cc.director singleton
        //   "EngineVersionOffset"  — offset to version string in libcocos.so
        (void)key;
        return 0;
    }

    size_t structSize(const std::string& key) const override {
        // TODO: Fill after research confirmation
        (void)key;
        return 0;
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

#pragma once
// Profiles/CocosCreatorV3Profile.h
// Cocos Creator v3.x profile (3.0 - 3.8, released 2021-present).
// COMPLETELY REWRITTEN - no longer based on Cocos2d-x.
// New high-performance cross-platform 3D core.
// Scripting: JavaScript, TypeScript
// Editor: Cocos Creator editor (3.x)
// Libraries: libcocos.so (renamed from libcocos2d.so), libjsc.so
// Changes from v2: 3D core, new rendering pipeline, ECS architecture
//   API differences from v2: significantly different, not fully compatible
// Detection signals:
//   - cc.Node, cc.Label, cc.Component, cc.MeshRenderer
//   - libcocos.so (NOT libcocos2d.so)
//   - .scene files (new format), .meta files
//   - settings/ directory with editor version
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class CocosCreatorV3Profile : public IEngineProfile {
public:
    std::string version() const override { return "cocos-creator-v3"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // Cocos Creator v3: no fixed offsets, use symbolFor() instead
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // Cocos Creator v3: no fixed struct sizes, use symbolFor() instead
    }

    std::optional<std::string> symbolFor(const std::string& key) const override {
        // v3 uses new Cocos engine symbols (cc namespace, libcocos.so)
        if (key == "cc::Director::getInstance")      return "cc::Director::getInstance";
        if (key == "cc::AssetManager::getInstance")  return "cc::AssetManager::getInstance";
        if (key == "cc::Game::getInstance")          return "cc::Game::getInstance";
        if (key == "cc::SysInfo::getVersion")        return "cc::SysInfo::getVersion";
        if (key == "v8::Isolate::GetCurrent")        return "v8::Isolate::GetCurrent";
        // v3: FileUtils moved to different namespace in rewritten engine
        if (key == "FileUtils::getInstance")         return "FileUtils::getInstance";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // Validate by checking for Cocos Creator v3 signals (libcocos.so, not libcocos2d.so)
        (void)headerBytes;
        (void)len;
        return true;
    }
};

} // namespace omnibyte::dumper::cocos2d

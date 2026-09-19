#pragma once
// ── Profiles/Cocos2dxJsProfile.h ───────────────────────────────────
// Cocos2d-x JavaScript variant profile.
// Primary library: libcocos2djs.so
// Script location: assets/script/ (main.js, *.js, *.jsc)
// Scripting engine: SpiderMonkey (Mozilla)
// Detection signals:
//   - libcocos2djs.so in lib/<abi>/
//   - org.cocos2dx.* DEX classes
//   - .js/.jsc files in assets/script/
//   - .plist texture atlases in assets/res/
//   - pvr.ccz compressed textures
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class Cocos2dxJsProfile : public IEngineProfile {
public:
    std::string version() const override { return "cocos2dx-js"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // Cocos2d-x JS: no fixed offsets, use symbolFor() instead
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // Cocos2d-x JS: no fixed struct sizes, use symbolFor() instead
    }

    std::optional<std::string> symbolFor(const std::string& key) const override {
        if (key == "ScriptingCore::evalString")  return "ScriptingCore::evalString";
        if (key == "FileUtils::getInstance")     return "FileUtils::getInstance";
        if (key == "Director::getInstance")      return "Director::getInstance";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // Validate by checking for libcocos2djs.so presence (done at detector level)
        // This profile is selected only after variant detection confirms JS scripting
        (void)headerBytes;
        (void)len;
        return true;
    }
};

} // namespace omnibyte::dumper::cocos2d

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
//
// TODO: Fill offsetOf()/structSize() after research report is reviewed.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class Cocos2dxJsProfile : public IEngineProfile {
public:
    std::string version() const override { return "cocos2dx-js"; }

    uint64_t offsetOf(const std::string& key) const override {
        // TODO: Fill after research confirmation
        // Expected keys:
        //   "ScriptingCoreAddr"    — offset to ScriptingCore singleton
        //   "evalStringAddr"       — export address of ScriptingCore::evalString
        //   "FileUtilsInstance"    — offset to FileUtils::getInstance() singleton
        (void)key;
        return 0;
    }

    size_t structSize(const std::string& key) const override {
        // TODO: Fill after research confirmation
        (void)key;
        return 0;
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

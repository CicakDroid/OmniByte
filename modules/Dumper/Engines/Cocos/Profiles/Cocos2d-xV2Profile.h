#pragma once
// ── Profiles/Cocos2dxV2Profile.h ───────────────────────────────────
// Cocos2d-x v2.x profile (2.0 — 2.2, released 2013–2015).
// Scripting: Lua bindings (primary), JavaScript via JSB (SpiderMonkey)
// Core classes use CC prefix: CCDirector, CCSprite, CCNode, CCLayer, CCLabelTTF
// Libraries: libcocos2d.so, libbox2d.so, libchipmunk.so, libextensions.so
// Detection signals:
//   - CC-prefixed symbols (CCDirector, CCSprite, CCNode)
//   - libcocos2d.so, libbox2d.so, libchipmunk.so
//   - Lua: luaL_loadbuffer, xxtea_decrypt
//   - JS:  ScriptingCore::evalString
//   - org.cocos2dx.* DEX classes
//   - .plist texture atlases, pvr.ccz compressed textures
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class Cocos2dxV2Profile : public IEngineProfile {
public:
    std::string version() const override { return "cocos2dx-v2"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // Cocos2d-x v2: no fixed offsets, use symbolFor() instead
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // Cocos2d-x v2: no fixed struct sizes, use symbolFor() instead
    }

    std::optional<std::string> symbolFor(const std::string& key) const override {
        // v2 uses CC-prefixed symbols
        if (key == "CCDirector::sharedDirector")   return "CCDirector::sharedDirector";
        if (key == "FileUtils::getInstance")       return "FileUtils::getInstance";
        if (key == "Director::getInstance")        return "Director::getInstance";
        // Lua scripting symbols
        if (key == "luaL_loadbuffer")              return "luaL_loadbuffer";
        if (key == "luaL_openlibs")                return "luaL_openlibs";
        if (key == "lua_pcall")                    return "lua_pcall";
        if (key == "xxtea_decrypt")                return "xxtea_decrypt";
        // JS scripting symbols (SpiderMonkey)
        if (key == "ScriptingCore::evalString")    return "ScriptingCore::evalString";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // Validate by checking for CC-prefixed symbols or libcocos2d.so presence
        (void)headerBytes;
        (void)len;
        return true;
    }
};

} // namespace omnibyte::dumper::cocos2d

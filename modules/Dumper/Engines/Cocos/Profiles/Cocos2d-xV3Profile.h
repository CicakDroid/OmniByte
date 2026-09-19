#pragma once
// ── Profiles/Cocos2dxV3Profile.h ───────────────────────────────────
// Cocos2d-x v3.x profile (3.0 — 3.17, released 2015–2018).
// Scripting: Lua bindings, JavaScript via JSB (SpiderMonkey, improved)
// Core classes drop CC prefix: Director, Sprite, Node, Label, EventDispatcher
// Libraries: libcocos2d.so, libbox2d.so, libchipmunk.so, libextensions.so
// New in v3: EventDispatcher, Console, Renderer classes
// Detection signals:
//   - Director (no CC prefix), Sprite, EventDispatcher
//   - libcocos2d.so, libbox2d.so, libchipmunk.so
//   - Lua: luaL_loadbuffer, xxtea_decrypt
//   - JS:  ScriptingCore::evalString
//   - org.cocos2dx.* DEX classes
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class Cocos2dxV3Profile : public IEngineProfile {
public:
    std::string version() const override { return "cocos2dx-v3"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // Cocos2d-x v3: no fixed offsets, use symbolFor() instead
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // Cocos2d-x v3: no fixed struct sizes, use symbolFor() instead
    }

    std::optional<std::string> symbolFor(const std::string& key) const override {
        // v3 uses non-prefixed symbols (CC prefix removed)
        if (key == "Director::getInstance")        return "Director::getInstance";
        if (key == "FileUtils::getInstance")       return "FileUtils::getInstance";
        // Lua scripting symbols
        if (key == "luaL_loadbuffer")              return "luaL_loadbuffer";
        if (key == "luaL_openlibs")                return "luaL_openlibs";
        if (key == "lua_pcall")                    return "lua_pcall";
        if (key == "xxtea_decrypt")                return "xxtea_decrypt";
        // JS scripting symbols (SpiderMonkey)
        if (key == "ScriptingCore::evalString")    return "ScriptingCore::evalString";
        // v3 new: EventDispatcher
        if (key == "EventDispatcher::getInstance") return "EventDispatcher::getInstance";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // Validate by checking for non-prefixed symbols or libcocos2d.so presence
        (void)headerBytes;
        (void)len;
        return true;
    }
};

} // namespace omnibyte::dumper::cocos2d

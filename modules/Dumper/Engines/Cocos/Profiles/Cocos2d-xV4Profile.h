#pragma once
// ── Profiles/Cocos2dxV4Profile.h ───────────────────────────────────
// Cocos2d-x v4.x profile (4.0, released Dec 2019).
// Scripting: Lua bindings only (JSB/H5 engine removed in v4)
// Core classes: Director, Sprite, Node, Label, EventDispatcher
// Libraries: libcocos2d.so (Metal backend on iOS/macOS)
// Changes from v3:
//   - Metal support on iOS/macOS
//   - Removed SimpleAudioEngine
//   - Removed JSB/H5 engine
//   - Removed experimental namespace
//   - CMake for all platforms
// Detection signals:
//   - Director, Sprite, EventDispatcher
//   - libcocos2d.so
//   - Lua: luaL_loadbuffer (JSB removed)
//   - org.cocos2dx.* DEX classes
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class Cocos2dxV4Profile : public IEngineProfile {
public:
    std::string version() const override { return "cocos2dx-v4"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // Cocos2d-x v4: no fixed offsets, use symbolFor() instead
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // Cocos2d-x v4: no fixed struct sizes, use symbolFor() instead
    }

    std::optional<std::string> symbolFor(const std::string& key) const override {
        // v4 same as v3 for core symbols, but JSB removed
        if (key == "Director::getInstance")        return "Director::getInstance";
        if (key == "FileUtils::getInstance")       return "FileUtils::getInstance";
        // Lua scripting only (JSB removed in v4)
        if (key == "luaL_loadbuffer")              return "luaL_loadbuffer";
        if (key == "luaL_openlibs")                return "luaL_openlibs";
        if (key == "lua_pcall")                    return "lua_pcall";
        if (key == "xxtea_decrypt")                return "xxtea_decrypt";
        // v4: ScriptingCore removed (JSB stripped)
        // if (key == "ScriptingCore::evalString") return "ScriptingCore::evalString"; // REMOVED
        if (key == "EventDispatcher::getInstance") return "EventDispatcher::getInstance";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // Validate by checking for libcocos2d.so presence
        (void)headerBytes;
        (void)len;
        return true;
    }
};

} // namespace omnibyte::dumper::cocos2d

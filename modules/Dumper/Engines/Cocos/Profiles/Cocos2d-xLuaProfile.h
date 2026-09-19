#pragma once
// ── Profiles/Cocos2dxLuaProfile.h ──────────────────────────────────
// Cocos2d-x Lua variant profile.
// Primary library: libcocos2dlua.so
// Script location: assets/src/ (main.lua, *.lua, *.luac)
// Entry point: main.lua loaded via luaL_loadbuffer
// Encryption: XXTEA (key in native lib, set via FileUtils::setXXTEAKeyAndSign)
// Detection signals:
//   - libcocos2dlua.so in lib/<abi>/
//   - org.cocos2dx.* DEX classes
//   - .lua/.luac files in assets/src/
//   - .plist texture atlases in assets/res/
//   - pvr.ccz compressed textures
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class Cocos2dxLuaProfile : public IEngineProfile {
public:
    std::string version() const override { return "cocos2dx-lua"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0; // Cocos2d-x: no fixed offsets, use symbolFor() instead
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0; // Cocos2d-x: no fixed struct sizes, use symbolFor() instead
    }

    std::optional<std::string> symbolFor(const std::string& key) const override {
        if (key == "luaL_loadbuffer")          return "luaL_loadbuffer";
        if (key == "FileUtils::getInstance")   return "FileUtils::getInstance";
        if (key == "Director::getInstance")    return "Director::getInstance";
        if (key == "xxtea_decrypt")            return "xxtea_decrypt";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // Validate by checking for libcocos2dlua.so presence (done at detector level)
        // This profile is selected only after variant detection confirms Lua scripting
        (void)headerBytes;
        (void)len;
        return true;
    }
};

} // namespace omnibyte::dumper::cocos2d

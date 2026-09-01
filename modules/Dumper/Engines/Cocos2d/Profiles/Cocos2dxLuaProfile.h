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
//
// TODO: Fill offsetOf()/structSize() after research report is reviewed.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class Cocos2dxLuaProfile : public IEngineProfile {
public:
    std::string version() const override { return "cocos2dx-lua"; }

    uint64_t offsetOf(const std::string& key) const override {
        // TODO: Fill after research confirmation
        // Expected keys:
        //   "XXTEAKeyOffset"       — offset to XXTEA key in libcocos2dlua.so
        //   "XXTEASignOffset"      — offset to XXTEA signature in libcocos2dlua.so
        //   "luaL_loadbufferAddr"  — export address of luaL_loadbuffer
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
        // Validate by checking for libcocos2dlua.so presence (done at detector level)
        // This profile is selected only after variant detection confirms Lua scripting
        (void)headerBytes;
        (void)len;
        return true;
    }
};

} // namespace omnibyte::dumper::cocos2d

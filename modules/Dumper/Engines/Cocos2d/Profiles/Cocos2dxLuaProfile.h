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

    // ── Detection approach ──────────────────────────────────────────
    // Unlike Unity IL2CPP (fixed struct offsets), Cocos2d-x Lua variant
    // is detected and analyzed via SYMBOL LOOKUP, not struct offsets.
    //
    // Key exported symbols (use dlsym or ELF dynamic symbol table):
    //   "luaL_loadbuffer"          — Lua script loading (from Lua C API)
    //   "FileUtils getInstance"    — FileUtils singleton accessor (v3.x)
    //   "FileUtils getInstance"    — FileUtils singleton accessor (v4.x)
    //   "Director getInstance"     — Director singleton accessor
    //
    // XXTEA key extraction:
    //   The XXTEA key is NOT at a fixed offset. It is passed to
    //   setXXTEAKeyAndSign() at runtime. To extract:
    //   1. Hook xxtea_decrypt (Cocos2d-x uses cocos2d/external/xxtea/xxtea.c)
    //   2. Read 3rd argument (key pointer) from function call
    //   3. Read 4th argument (key length) from function call
    //
    //   Example frida hook (from lambwheit/cocos2dx-xxtea-decryptor):
    //     var xxtea_decrypt = Module.findExportByName(null, "xxtea_decrypt");
    //     Interceptor.attach(xxtea_decrypt, {
    //       onEnter: function(args) {
    //         this.plain = args[1];
    //         this.plainLen = args[2].toInt32();
    //         this.key = args[3];
    //         this.keyLen = args[4].toInt32();
    //       },
    //       onLeave: function(retval) {
    //         var key = Memory.readUtf8String(this.key, this.keyLen);
    //         send("[XXTEA KEY] " + key);
    //       }
    //     });
    //
    // Source: cocos2d-x external/xxtea/xxtea.c
    //   https://github.com/cocos2d/cocos2d-x/blob/v3/external/xxtea/xxtea.c
    // Source: cocos2d-x FileUtils API docs
    //   https://docs.cocos2d-x.org/api-ref/cplusplus/v4x/dc/d69/classcocos2d_1_1_file_utils.html
    //
    // For our purposes: offsetOf() returns 0 because offsets are build-specific.
    // Use symbol lookup at runtime instead.
    uint64_t offsetOf(const std::string& key) const override {
        // Symbol lookup keys (use dlsym/ELF parsing):
        //   "luaL_loadbuffer"      — Lua script loading function
        //   "FileUtils::getInstance" — FileUtils singleton (static method)
        //   "Director::getInstance" — Director singleton (static method)
        //
        // XXTEA: hook xxtea_decrypt, read args[3] (key), args[4] (keyLen)
        // No fixed offset for key — must extract at runtime.
        //
        // Source: lambwheit/cocos2dx-xxtea-decryptor (frida hook pattern)
        //   https://github.com/lambwheit/cocos2dx-xxtea-decryptor
        // Source: xpol/lua-cocos2d-x-xxtea (integration reference)
        //   https://github.com/xpol/lua-cocos2d-x-xxtea
        (void)key;
        return 0; // Build-specific — use symbol lookup instead
    }

    // ── Struct sizes ────────────────────────────────────────────────
    // Cocos2d-x FileUtils class layout (v3.x, from doxygen docs):
    //   Protected members:
    //     _defaultResRootPath       (std::string)  — ~24 bytes (SBO)
    //     _searchPathArray          (std::vector)  — ~24 bytes
    //     _searchResolutionsOrderArray (std::vector) — ~24 bytes
    //     _fullPathCache            (std::map)     — ~48 bytes
    //   Static protected:
    //     s_sharedFileUtils        (FileUtils*)    — 8 bytes (singleton pointer)
    //
    // Exact layout is build-specific (STL implementation varies).
    // Use symbol lookup for FileUtils::getInstance() instead.
    //
    // Source: cocos2d-x FileUtils doxygen
    //   https://docs.cocos2d-x.org/api-ref/cplusplus/v4x/dc/d69/classcocos2d_1_1_file_utils.html
    size_t structSize(const std::string& key) const override {
        // Symbol lookup keys (use dlsym/ELF parsing):
        //   "FileUtils::getInstance" — returns singleton pointer
        //   "Director::getInstance"  — returns Director singleton
        //
        // No fixed struct sizes — build-specific STL layout.
        (void)key;
        return 0; // Build-specific — use symbol lookup instead
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

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

    // ── Detection approach ──────────────────────────────────────────
    // Unlike Unity IL2CPP (fixed struct offsets), Cocos2d-x JS variant
    // is detected and analyzed via SYMBOL LOOKUP, not struct offsets.
    //
    // Key exported symbols (use dlsym or ELF dynamic symbol table):
    //   "ScriptingCore evalString" — JS script evaluation (SpiderMonkey)
    //   "FileUtils getInstance"    — FileUtils singleton accessor (v3.x)
    //   "Director getInstance"     — Director singleton accessor
    //
    // ScriptingCore is the JS binding layer (cocos2d-x/scripting/js-bindings/).
    // It wraps SpiderMonkey's JSContext/JSRuntime for Cocos2d integration.
    //
    // Source: cocos2d-x ScriptingCore reference
    //   https://github.com/cocos2d/cocos2d-x/blob/v3/scripting/js-bindings/manual/ScriptingCore.h
    // Source: cocos2d-x FileUtils API docs
    //   https://docs.cocos2d-x.org/api-ref/cplusplus/v4x/dc/d69/classcocos2d_1_1_file_utils.html
    //
    // For our purposes: offsetOf() returns 0 because offsets are build-specific.
    // Use symbol lookup at runtime instead.
    uint64_t offsetOf(const std::string& key) const override {
        // Symbol lookup keys (use dlsym/ELF parsing):
        //   "ScriptingCore::evalString" — JS evaluation function
        //   "FileUtils::getInstance"    — FileUtils singleton (static method)
        //   "Director::getInstance"     — Director singleton (static method)
        //
        // No fixed offsets — all build-specific. Use symbol lookup.
        //
        // Source: cocos2d-x ScriptingCore header
        //   https://github.com/cocos2d/cocos2d-x/blob/v3/scripting/js-bindings/manual/ScriptingCore.h
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
        // Validate by checking for libcocos2djs.so presence (done at detector level)
        // This profile is selected only after variant detection confirms JS scripting
        (void)headerBytes;
        (void)len;
        return true;
    }
};

} // namespace omnibyte::dumper::cocos2d

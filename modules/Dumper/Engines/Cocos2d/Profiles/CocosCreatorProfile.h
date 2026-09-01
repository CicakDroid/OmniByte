#pragma once
// ── Profiles/CocosCreatorProfile.h ─────────────────────────────────
// Cocos Creator (modern) profile.
// Primary library: libcocos.so (or generic project name)
// Script location: assets/ (main.js, game.js, *.ts compiled to *.js)
// Architecture: Entity-Component system (not Scene/Layer/Node)
// Detection signals:
//   - libcocos.so in lib/<abi>/
//   - main.js / game.js in assets/
//   - .json texture atlases (not .plist)
//   - cc.assetManager, cc.director API patterns
//   - settings/ directory with editor version
//
// TODO: Fill offsetOf()/structSize() after research report is reviewed.
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace omnibyte::dumper::cocos2d {

class CocosCreatorProfile : public IEngineProfile {
public:
    std::string version() const override { return "cocos-creator"; }

    // ── Detection approach ──────────────────────────────────────────
    // Cocos Creator (v2.x–v3.x) uses symbol lookup, not fixed struct offsets.
    //
    // Key exported symbols (use dlsym or ELF dynamic symbol table):
    //   "cc AssetManager getInstance" — AssetManager singleton accessor
    //   "cc Director getInstance"     — Director singleton accessor
    //   "cc Game getInstance"         — Game singleton accessor
    //   "cc SysInfo getVersion"       — Engine version string
    //
    // JavaScript engine: V8 (not SpiderMonkey like Cocos2d-x JS)
    //   "v8::Isolate GetCurrent"      — V8 isolate accessor
    //
    // Scene format: JSON (not binary plist like Cocos2d-x)
    //   assets/scene/*.scene — JSON scene files
    //   assets/ — main.js / game.js entry points
    //
    // Texture atlases: JSON (not .plist)
    //   assets/**/\*.json — texture atlas metadata
    //
    // Source: Cocos Creator API docs
    //   https://docs.cocos.com/creator/manual/en/
    // Source: cocos-engine GitHub repo
    //   https://github.com/cocos/cocos-engine
    //
    // For our purposes: offsetOf() returns 0 because offsets are build-specific.
    // Use symbol lookup at runtime instead.
    uint64_t offsetOf(const std::string& key) const override {
        // Symbol lookup keys (use dlsym/ELF parsing):
        //   "cc::AssetManager::getInstance" — AssetManager singleton
        //   "cc::Director::getInstance"     — Director singleton
        //   "cc::Game::getInstance"         — Game singleton
        //   "cc::SysInfo::getVersion"       — Engine version
        //   "v8::Isolate::GetCurrent"       — V8 isolate
        //
        // No fixed offsets — all build-specific. Use symbol lookup.
        //
        // Source: cocos-engine GitHub
        //   https://github.com/cocos/cocos-engine
        (void)key;
        return 0; // Build-specific — use symbol lookup instead
    }

    // ── Struct sizes ────────────────────────────────────────────────
    // Cocos Creator uses V8 JavaScript engine, not native C++ structs.
    // All game logic lives in compiled JavaScript (main.js, game.js).
    //
    // AssetManager, Director, Game are C++ singletons with V8 bindings.
    // Exact layout is build-specific (cocos-engine version varies).
    //
    // Use symbol lookup for singletons instead of fixed struct sizes.
    //
    // Source: cocos-engine GitHub
    //   https://github.com/cocos/cocos-engine
    size_t structSize(const std::string& key) const override {
        // Symbol lookup keys (use dlsym/ELF parsing):
        //   "cc::AssetManager::getInstance" — returns singleton pointer
        //   "cc::Director::getInstance"     — returns Director singleton
        //   "cc::Game::getInstance"         — returns Game singleton
        //
        // No fixed struct sizes — build-specific engine layout.
        (void)key;
        return 0; // Build-specific — use symbol lookup instead
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        // Validate by checking for libcocos.so presence (done at detector level)
        // This profile is selected only after variant detection confirms Cocos Creator
        (void)headerBytes;
        (void)len;
        return true;
    }
};

} // namespace omnibyte::dumper::cocos2d

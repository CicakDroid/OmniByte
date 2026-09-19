#pragma once
// Cocos2d-x v2.x profile (2.0 — 2.2, released 2013–2015).
// Sub-version detection:
//   v2.0: Initial release, CC-prefixed symbols
//   v2.1: Spine runtime added, improved Lua bindings
//   v2.2: Final v2 release, performance improvements
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <optional>

namespace omnibyte::dumper::cocos2d {

class Cocos2dxV2Profile : public IEngineProfile {
public:
    std::string version() const override { return "cocos2dx-v2"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0;
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0;
    }

    std::optional<std::string> symbolFor(const std::string& key) const override {
        if (key == "CCDirector::sharedDirector")   return "CCDirector::sharedDirector";
        if (key == "CCSprite::create")              return "CCSprite::create";
        if (key == "CCNode::node")                  return "CCNode::node";
        if (key == "CCLayer::layer")                return "CCLayer::layer";
        if (key == "CCLabelTTF::labelWithString")   return "CCLabelTTF::labelWithString";
        if (key == "FileUtils::getInstance")        return "FileUtils::getInstance";
        if (key == "FileUtils::fullPathForFilename") return "FileUtils::fullPathForFilename";
        if (key == "luaL_loadbuffer")               return "luaL_loadbuffer";
        if (key == "luaL_openlibs")                 return "luaL_openlibs";
        if (key == "lua_pcall")                     return "lua_pcall";
        if (key == "xxtea_decrypt")                 return "xxtea_decrypt";
        if (key == "ScriptingCore::evalString")     return "ScriptingCore::evalString";
        if (key == "Spine::Skeleton::create")       return "Spine::Skeleton::create";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        (void)len;
        return true;
    }

    // Detect specific sub-version based on symbol presence
    std::string detectSubVersion(const uint8_t* data, size_t len) const override {
        std::string subVersion = "2.0";

        bool hasSpineRuntime = false;
        for (size_t i = 0; i + 15 < len; ++i) {
            std::string chunk(reinterpret_cast<const char*>(data + i), 15);
            if (chunk.find("Spine::Skeleton") != std::string::npos) {
                hasSpineRuntime = true;
                break;
            }
        }

        if (hasSpineRuntime) subVersion = "2.1";

        return subVersion;
    }
};

} // namespace omnibyte::dumper::cocos2d

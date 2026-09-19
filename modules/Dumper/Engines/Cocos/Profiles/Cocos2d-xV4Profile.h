#pragma once
// Cocos2d-x v4.x profile (4.0, released Dec 2019).
// Sub-version detection:
//   v4.0: Initial release, Metal support on iOS/macOS
//   v4.0.0+: Bug fixes, JSB completely removed
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <optional>

namespace omnibyte::dumper::cocos2d {

class Cocos2dxV4Profile : public IEngineProfile {
public:
    std::string version() const override { return "cocos2dx-v4"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0;
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0;
    }

    std::optional<std::string> symbolFor(const std::string& key) const override {
        if (key == "Director::getInstance")        return "Director::getInstance";
        if (key == "Director::getSafeAreaRect")    return "Director::getSafeAreaRect";
        if (key == "Sprite::create")               return "Sprite::create";
        if (key == "Label::createWithTTF")         return "Label::createWithTTF";
        if (key == "FileUtils::getInstance")       return "FileUtils::getInstance";
        if (key == "FileUtils::fullPathForFilename") return "FileUtils::fullPathForFilename";
        if (key == "EventDispatcher::getInstance") return "EventDispatcher::getInstance";
        if (key == "EventDispatcher::addEventListener") return "EventDispatcher::addEventListener";
        if (key == "luaL_loadbuffer")              return "luaL_loadbuffer";
        if (key == "luaL_openlibs")                return "luaL_openlibs";
        if (key == "lua_pcall")                    return "lua_pcall";
        if (key == "xxtea_decrypt")                return "xxtea_decrypt";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        (void)len;
        return true;
    }

    // v4.0 has Metal support symbols on iOS/macOS
    std::string detectSubVersion(const uint8_t* data, size_t len) const override {
        std::string subVersion = "4.0";

        bool hasMetalBackend = false;
        for (size_t i = 0; i + 10 < len; ++i) {
            std::string chunk(reinterpret_cast<const char*>(data + i), 10);
            if (chunk.find("MTLDevice") != std::string::npos ||
                chunk.find("MTLCommandQueue") != std::string::npos) {
                hasMetalBackend = true;
                break;
            }
        }

        if (hasMetalBackend) subVersion = "4.0.0+";

        return subVersion;
    }
};

} // namespace omnibyte::dumper::cocos2d

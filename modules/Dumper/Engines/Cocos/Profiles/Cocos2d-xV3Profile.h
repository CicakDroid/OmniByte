#pragma once
// Cocos2d-x v3.x profile (3.0 — 3.17, released 2015–2018).
// Sub-version detection for v3.15 - v3.17.2:
//   v3.15: Flatbuffer update, EventDispatcher stable_sort
//   v3.16: RadialGradientLayer added
//   v3.17: Director::getSafeAreaRect() added, iPhone X support
//   v3.17.1: LuaJIT 2.1.0-beta3
//   v3.17.2: Bug fixes (last official v3)
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <optional>

namespace omnibyte::dumper::cocos2d {

class Cocos2dxV3Profile : public IEngineProfile {
public:
    std::string version() const override { return "cocos2dx-v3"; }

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
        if (key == "EventDispatcher::dispatchCustomEvent") return "EventDispatcher::dispatchCustomEvent";
        if (key == "RadialGradientLayer::create")  return "ui::RadialGradientLayer::create";
        if (key == "GLProgramState::getOrCreateWithGLProgramName") return "GLProgramState::getOrCreateWithGLProgramName";
        if (key == "luaL_loadbuffer")              return "luaL_loadbuffer";
        if (key == "luaL_openlibs")                return "luaL_openlibs";
        if (key == "lua_pcall")                    return "lua_pcall";
        if (key == "xxtea_decrypt")                return "xxtea_decrypt";
        if (key == "ScriptingCore::evalString")    return "ScriptingCore::evalString";
        if (key == "v8::Isolate::GetCurrent")      return "v8::Isolate::GetCurrent";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        (void)len;
        return true;
    }

    // Detect specific sub-version based on symbol presence in binary
    std::string detectSubVersion(const uint8_t* data, size_t len) const override {
        std::string subVersion = "3.15";

        bool hasSafeAreaRect = false;
        bool hasRadialGradient = false;

        for (size_t i = 0; i + 20 < len; ++i) {
            std::string chunk(reinterpret_cast<const char*>(data + i), 20);
            if (chunk.find("getSafeAreaRect") != std::string::npos) hasSafeAreaRect = true;
            if (chunk.find("RadialGradient") != std::string::npos) hasRadialGradient = true;
        }

        if (hasSafeAreaRect) subVersion = "3.17";
        else if (hasRadialGradient) subVersion = "3.16";

        return subVersion;
    }
};

} // namespace omnibyte::dumper::cocos2d

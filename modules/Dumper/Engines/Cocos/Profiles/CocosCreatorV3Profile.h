#pragma once
// Cocos Creator v3.x profile (3.0 - 3.8, released 2021-present).
// Sub-version detection:
//   v3.0: Initial release, new 3D core
//   v3.5: Built project upgrade guide
//   v3.6: New build template format, settings.json
//   v3.7: XR support, Procedural Animation
//   v3.8: Custom Render Pipeline, High Precision Text
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <optional>

namespace omnibyte::dumper::cocos2d {

class CocosCreatorV3Profile : public IEngineProfile {
public:
    std::string version() const override { return "cocos-creator-v3"; }

    uint64_t offsetOf(const std::string& key) const override {
        (void)key;
        return 0;
    }

    size_t structSize(const std::string& key) const override {
        (void)key;
        return 0;
    }

    std::optional<std::string> symbolFor(const std::string& key) const override {
        if (key == "cc::Director::getInstance")      return "cc::Director::getInstance";
        if (key == "cc::AssetManager::getInstance")  return "cc::AssetManager::getInstance";
        if (key == "cc::Game::getInstance")          return "cc::Game::getInstance";
        if (key == "cc::SysInfo::getVersion")        return "cc::SysInfo::getVersion";
        if (key == "cc::Scene::load")                return "cc::Scene::load";
        if (key == "cc::resources::load")            return "cc::resources::load";
        if (key == "cc::input::Input::on")           return "cc::input::Input::on";
        if (key == "v8::Isolate::GetCurrent")        return "v8::Isolate::GetCurrent";
        if (key == "FileUtils::getInstance")         return "FileUtils::getInstance";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        (void)len;
        return true;
    }

    // Detect specific sub-version based on symbol presence
    std::string detectSubVersion(const uint8_t* data, size_t len) const override {
        std::string subVersion = "3.0";

        bool hasXRSupport = false;
        bool hasCustomPipeline = false;

        for (size_t i = 0; i + 15 < len; ++i) {
            std::string chunk(reinterpret_cast<const char*>(data + i), 15);
            if (chunk.find("XRSession") != std::string::npos ||
                chunk.find("XRReferenceSpace") != std::string::npos) {
                hasXRSupport = true;
            }
            if (chunk.find("RenderPipeline") != std::string::npos) {
                hasCustomPipeline = true;
            }
        }

        if (hasCustomPipeline) subVersion = "3.8";
        else if (hasXRSupport) subVersion = "3.7";
        else subVersion = "3.5";

        return subVersion;
    }
};

} // namespace omnibyte::dumper::cocos2d

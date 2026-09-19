#pragma once
// Cocos Creator v2.x profile (2.0 — 2.4, released 2018–2023).
// Sub-version detection:
//   v2.0: Initial release, Asset Manager
//   v2.1: Improved TypeScript support
//   v2.4: Final v2 release
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <optional>

namespace omnibyte::dumper::cocos2d {

class CocosCreatorV2Profile : public IEngineProfile {
public:
    std::string version() const override { return "cocos-creator-v2"; }

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
        if (key == "cc::resources::load")            return "cc::resources::load";
        if (key == "v8::Isolate::GetCurrent")        return "v8::Isolate::GetCurrent";
        if (key == "FileUtils::getInstance")         return "FileUtils::getInstance";
        if (key == "FileUtils::fullPathForFilename") return "FileUtils::fullPathForFilename";
        return std::nullopt;
    }

    bool validate(const uint8_t* headerBytes, size_t len) const override {
        (void)headerBytes;
        (void)len;
        return true;
    }

    // v2.1+ has improved Asset Manager
    std::string detectSubVersion(const uint8_t* data, size_t len) const override {
        std::string subVersion = "2.0";

        bool hasAssetManager = false;
        for (size_t i = 0; i + 15 < len; ++i) {
            std::string chunk(reinterpret_cast<const char*>(data + i), 15);
            if (chunk.find("AssetManager") != std::string::npos) {
                hasAssetManager = true;
                break;
            }
        }

        if (hasAssetManager) subVersion = "2.1";

        return subVersion;
    }
};

} // namespace omnibyte::dumper::cocos2d

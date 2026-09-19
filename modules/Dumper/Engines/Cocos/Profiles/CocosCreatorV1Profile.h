#pragma once
// Cocos Creator v1.x profile (1.0 — 1.10, released 2017–2018).
// Sub-version detection:
//   v1.0: Initial release, based on Cocos2d-x runtime
//   v1.5: TypeScript support added
//   v1.10: Final v1 release
#include "../../../DumperCore/IEngineProfile.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <optional>

namespace omnibyte::dumper::cocos2d {

class CocosCreatorV1Profile : public IEngineProfile {
public:
    std::string version() const override { return "cocos-creator-v1"; }

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

    // v1.5+ has TypeScript support symbols
    std::string detectSubVersion(const uint8_t* data, size_t len) const override {
        std::string subVersion = "1.0";

        bool hasTypeScript = false;
        for (size_t i = 0; i + 15 < len; ++i) {
            std::string chunk(reinterpret_cast<const char*>(data + i), 15);
            if (chunk.find("TypeScript") != std::string::npos ||
                chunk.find("typescript") != std::string::npos) {
                hasTypeScript = true;
                break;
            }
        }

        if (hasTypeScript) subVersion = "1.5";

        return subVersion;
    }
};

} // namespace omnibyte::dumper::cocos2d

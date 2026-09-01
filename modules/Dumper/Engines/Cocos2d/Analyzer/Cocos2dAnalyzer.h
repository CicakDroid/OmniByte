#pragma once
// Cocos2d — static analyzer for Cocos2d-x / Cocos Creator APK detection.
// Scans APK for native libraries, DEX classes, and asset files.
// Does not require live process.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::cocos2d {

class Cocos2dAnalyzer {
public:
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "Cocos2d";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!target.isFile()) {
            result.errorMessage = "Cocos2d analyzer only supports file targets (APK)";
            return result;
        }

        // Read APK as raw bytes — scan for known library/asset patterns
        auto fileData = utils::readFileBytes(target.filePath);
        if (fileData.empty()) {
            result.errorMessage = "Failed to read file: " + target.filePath;
            return result;
        }

        // Scan for native libraries
        scanLibraries(fileData, result);

        // Scan for script/asset files
        scanAssets(fileData, result);

        // Scan for DEX class patterns (org.cocos2dx.*)
        scanDexPatterns(fileData, result);

        result.success = true;
        return result;
    }

private:
    // Known Cocos2d native library names
    static constexpr const char* kCocosLibs[] = {
        "libcocos2dlua.so",
        "libcocos2djs.so",
        "libcocos2dcpp.so",
        "libgame.so",
        "libcocos.so",
        "libcocos2d.so",
    };

    // Known script/asset extensions
    static constexpr const char* kScriptExts[] = {
        ".lua", ".luac", ".jsc", ".jsb", ".ts",
    };

    static void scanLibraries(const std::vector<uint8_t>& data, DumpResult& result) {
        int libCount = 0;
        std::string variant = "unknown";

        for (const char* lib : kCocosLibs) {
            std::string needle(lib);
            auto pos = utils::findCaseInsensitive(
                std::string(reinterpret_cast<const char*>(data.data()), data.size()),
                needle);
            if (pos) {
                libCount++;
                // Classify variant from library name
                if (needle == "libcocos2dlua.so") variant = "cocos2dx-lua";
                else if (needle == "libcocos2djs.so") variant = "cocos2dx-js";
                else if (needle == "libcocos.so") variant = "cocos-creator";
                else if (needle == "libcocos2dcpp.so" || needle == "libgame.so")
                    variant = "cocos2dx-cpp";
            }
        }

        result.setMeta("libraryFound", libCount > 0 ? "true" : "false");
        result.setMeta("variant", variant);
        result.setMeta("libraryCount", std::to_string(libCount));
    }

    static void scanAssets(const std::vector<uint8_t>& data, DumpResult& result) {
        int scriptCount = 0;

        // Quick scan for script file extensions in the raw APK data
        std::string dataStr(reinterpret_cast<const char*>(data.data()),
                            std::min(data.size(), static_cast<size_t>(1024 * 1024)));

        for (const char* ext : kScriptExts) {
            size_t pos = 0;
            while ((pos = dataStr.find(ext, pos)) != std::string::npos) {
                scriptCount++;
                pos += strlen(ext);
            }
        }

        result.setMeta("scriptCount", std::to_string(scriptCount));
    }

    static void scanDexPatterns(const std::vector<uint8_t>& data, DumpResult& result) {
        // Look for org.cocos2dx package prefix in DEX strings
        std::string needle("org/cocos2dx");
        auto pos = utils::findCaseInsensitive(
            std::string(reinterpret_cast<const char*>(data.data()),
                        std::min(data.size(), static_cast<size_t>(2 * 1024 * 1024))),
            needle);

        result.setMeta("dexClassesFound", pos.has_value() ? "true" : "false");
    }
};

} // namespace omnibyte::dumper::cocos2d

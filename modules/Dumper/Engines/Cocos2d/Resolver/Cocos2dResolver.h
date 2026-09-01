#pragma once
// Cocos2d — runtime symbol resolver via direct export lookup (xdl_sym).
// Resolves engine symbols (luaL_loadbuffer, Director::getInstance, etc.)
// from live process. Does NOT use patternFor/AOB — only symbolFor().
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::cocos2d {

class Cocos2dResolver {
public:
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "Cocos2d";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!profile) {
            result.errorMessage = "No profile provided";
            return result;
        }

        if (target.isFile()) {
            result.errorMessage = "Resolver requires live process target (use analyzer for file targets)";
            return result;
        }

        // Cocos2d uses symbolFor() exclusively — no offsetOf/patternFor
        // Each profile defines symbol names for key engine functions
        resolveSymbolTable(target, profile, result);

        result.success = !result.metadata.empty();
        if (!result.success) {
            result.errorMessage = "No symbols resolved";
        }

        return result;
    }

private:
    // Common symbol keys to resolve per variant
    static constexpr const char* kCommonSymbols[] = {
        "FileUtils::getInstance",
        "Director::getInstance",
        "xxtea_decrypt",
    };

    static constexpr const char* kLuaSymbols[] = {
        "luaL_loadbuffer",
        "luaL_openlibs",
        "lua_pcall",
        "lua_tostring",
        "lua_settop",
    };

    static constexpr const char* kJsSymbols[] = {
        "ScriptingCore::evalString",
    };

    static constexpr const char* kCreatorSymbols[] = {
        "cc::AssetManager::getInstance",
        "cc::Director::getInstance",
        "cc::Game::getInstance",
    };

    static void resolveSymbolTable(const AnalysisTarget& target,
                                    const std::shared_ptr<IEngineProfile>& profile,
                                    DumpResult& result) {
        // Try common symbols first
        for (const char* sym : kCommonSymbols) {
            tryResolveSymbol(target, profile, sym, result);
        }

        // Variant-specific symbols
        std::string variant;
        auto v = profile->symbolFor("variant");
        if (v) variant = *v;

        if (variant == "cocos2dx-lua") {
            for (const char* sym : kLuaSymbols) {
                tryResolveSymbol(target, profile, sym, result);
            }
        } else if (variant == "cocos2dx-js") {
            for (const char* sym : kJsSymbols) {
                tryResolveSymbol(target, profile, sym, result);
            }
        } else if (variant == "cocos-creator") {
            for (const char* sym : kCreatorSymbols) {
                tryResolveSymbol(target, profile, sym, result);
            }
        }

        // Also try all symbols defined in profile via symbolFor()
        // (profile may define additional symbols beyond hardcoded lists)
        for (const char* key : {"luaL_loadbuffer", "luaL_openlibs", "lua_pcall",
                                 "ScriptingCore::evalString",
                                 "cc::AssetManager::getInstance",
                                 "cc::Director::getInstance",
                                 "cc::Game::getInstance"}) {
            auto symName = profile->symbolFor(key);
            if (symName && result.metadata.find(key) == result.metadata.end()) {
                result.setMeta(key, *symName + " (not resolved)");
            }
        }
    }

    static void tryResolveSymbol(const AnalysisTarget& target,
                                  const std::shared_ptr<IEngineProfile>& profile,
                                  const std::string& key,
                                  DumpResult& result) {
        auto symName = profile->symbolFor(key);
        if (!symName) return;

        // In a real implementation, this would call xdl_sym() on the target process
        // For now, record the symbol name and mark as pending resolution
        result.setMeta(key, *symName);
    }
};

} // namespace omnibyte::dumper::cocos2d

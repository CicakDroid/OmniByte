#pragma once
// GameMaker — runtime symbol resolver via yoyorun library.
// Resolves runtime struct offsets and function addresses from live process.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::gamemaker {

class GameMakerResolver {
public:
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "GameMaker";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!profile) {
            result.errorMessage = "No profile provided";
            return result;
        }

        if (target.isFile()) {
            result.errorMessage = "Resolver requires live process target (use analyzer for file targets)";
            return result;
        }

        // Resolve yoyorun symbols via profile->symbolFor()
        resolveYoyorunSymbols(target, profile, result);

        result.success = !result.metadata.empty();
        if (!result.success) {
            result.errorMessage = "No symbols resolved";
        }

        return result;
    }

private:
    static void resolveYoyorunSymbols(const AnalysisTarget& target,
                                       const std::shared_ptr<IEngineProfile>& profile,
                                       DumpResult& result) {
        // GameMaker runtime symbols
        static const char* kGMSymbols[] = {
            "yy_object_new",
            "yy_object_free",
            "yy_func_execute",
            "yy_func_create",
            "yy_str_create",
            "yy_array_create",
        };

        for (const char* sym : kGMSymbols) {
            auto symName = profile->symbolFor(sym);
            if (symName) {
                result.setMeta(sym, *symName);
            }
        }

        // Runtime value accessors (GM 2.3+)
        static const char* kRValSymbols[] = {
            "yy_rval_make",
            "yy_rval_free",
            "yy_rval_get_real",
            "yy_rval_get_string",
        };

        for (const char* sym : kRValSymbols) {
            auto symName = profile->symbolFor(sym);
            if (symName) {
                result.setMeta(sym, *symName);
            }
        }
    }
};

} // namespace omnibyte::dumper::gamemaker

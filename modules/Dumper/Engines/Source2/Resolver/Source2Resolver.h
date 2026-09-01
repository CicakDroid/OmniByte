#pragma once
// Source2 — runtime schema resolver via xdl_sym.
// Resolves schema class definitions and resource system symbols
// from live process.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::source2 {

class Source2Resolver {
public:
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "Source2";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!profile) {
            result.errorMessage = "No profile provided";
            return result;
        }

        if (target.isFile()) {
            result.errorMessage = "Resolver requires live process target (use analyzer for file targets)";
            return result;
        }

        // Resolve schema system symbols via profile->symbolFor()
        resolveSchemaSymbols(target, profile, result);

        result.success = !result.metadata.empty();
        if (!result.success) {
            result.errorMessage = "No symbols resolved";
        }

        return result;
    }

private:
    static void resolveSchemaSymbols(const AnalysisTarget& target,
                                      const std::shared_ptr<IEngineProfile>& profile,
                                      DumpResult& result) {
        // Source 2 schema symbols
        static const char* kSchemaSymbols[] = {
            "ResourceSystem::Cache",
            "ResourceSystem::Load",
            "Schema::FindClass",
            "Schema::GetClassInfo",
        };

        for (const char* sym : kSchemaSymbols) {
            auto symName = profile->symbolFor(sym);
            if (symName) {
                result.setMeta(sym, *symName);
            }
        }

        // Resolve any additional symbols defined in profile
        static const char* kExtraSymbols[] = {
            "CMorphData",
            "CVProperty",
            "CVResource",
        };

        for (const char* sym : kExtraSymbols) {
            auto symName = profile->symbolFor(sym);
            if (symName) {
                result.setMeta(sym, *symName);
            }
        }
    }
};

} // namespace omnibyte::dumper::source2

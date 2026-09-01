#pragma once
// Godot — runtime ClassDB/StringName resolver via xdl_sym.
// Resolves class database and string name table from live process.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include "../../../DumperCore/SharedUtils/SharedUtils.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::godot {

class GodotResolver {
public:
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        result.engineName = "Godot";
        result.detectedVersion = profile ? profile->version() : "unknown";

        if (!profile) {
            result.errorMessage = "No profile provided";
            return result;
        }

        if (target.isFile()) {
            result.errorMessage = "Resolver requires live process target (use analyzer for file targets)";
            return result;
        }

        // Resolve ClassDB and StringName symbols via profile->symbolFor()
        resolveClassDBSymbols(target, profile, result);

        result.success = !result.metadata.empty();
        if (!result.success) {
            result.errorMessage = "No symbols resolved";
        }

        return result;
    }

private:
    static void resolveClassDBSymbols(const AnalysisTarget& target,
                                       const std::shared_ptr<IEngineProfile>& profile,
                                       DumpResult& result) {
        // Core Godot symbols
        static const char* kClassDBSymbols[] = {
            "ClassDB::classes",
            "ClassDB::get_class_tag",
            "ClassDB::get_parent_class",
            "StringName::setup",
            "StringName::intern",
        };

        for (const char* sym : kClassDBSymbols) {
            auto symName = profile->symbolFor(sym);
            if (symName) {
                result.setMeta(sym, *symName);
            }
        }

        // Additional Godot 4.x symbols
        static const char* kGodot4Symbols[] = {
            "ClassDB::get_method_info",
            "ClassDB::get_property_info",
            "ClassDB::get_signal_info",
        };

        for (const char* sym : kGodot4Symbols) {
            auto symName = profile->symbolFor(sym);
            if (symName) {
                result.setMeta(sym, *symName);
            }
        }
    }
};

} // namespace omnibyte::dumper::godot

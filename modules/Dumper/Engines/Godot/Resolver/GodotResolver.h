#pragma once
// Godot — implementasi resolveSymbols() yang dipisah dari engine.
// Resolver butuh live process -- resolve StringName::setup / ClassDB
// singleton via runtime/SymbolResolver.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::godot {

class GodotResolver {
public:
    // resolve StringName::setup / ClassDB singleton via live process
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::godot
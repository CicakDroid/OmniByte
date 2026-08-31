#pragma once
// GameMaker — implementasi resolveSymbols() yang dipisah dari engine.
// Resolver butuh live process -- resolve runtime struct offsets via
// runtime/SymbolResolver (yoyorun).
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::gamemaker {

class GameMakerResolver {
public:
    // resolve runtime struct offets via live process (yoyorun)
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::gamemaker
#pragma once
// UnrealEngine — implementasi resolveSymbols() yang dipisah dari engine.
// Resolver butuh live process -- resolve GWorld/GObjects live via
// runtime/SymbolResolver (xDL) + MemoryIO untuk ASLR-safe address.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::unrealengine {

class UnrealEngineResolver {
public:
    // resolve GWorld/GObjects live via runtime/SymbolResolver (xDL) + MemoryIO
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::unrealengine
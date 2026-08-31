#pragma once
// CryEngine — implementasi resolveSymbols() yang dipisah dari engine.
// Resolver butuh live process -- resolve ISystem::GetISystem / class registry
// via runtime/SymbolResolver.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::cryengine {

class CryEngineResolver {
public:
    // resolve ISystem::GetISystem / class registry via live process
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::cryengine
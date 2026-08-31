#pragma once
// Source — implementasi resolveSymbols() yang dipisah dari engine.
// Resolver butuh live process -- resolve interface factory (CreateInterface)
// live via runtime/SymbolResolver.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::source {

class SourceResolver {
public:
    // resolve interface factory (CreateInterface) live via SymbolResolver
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::source
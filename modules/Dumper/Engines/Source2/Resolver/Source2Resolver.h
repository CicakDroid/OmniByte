#pragma once
// Source2 — implementasi resolveSymbols() yang dipisah dari engine.
// Resolver butuh live process -- resolve symbol/address konkret via runtime/SymbolResolver.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::source2 {

class Source2Resolver {
public:
    static DumpResult resolveSymbols(const AnalysisTarget& target,
                                     const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::source2
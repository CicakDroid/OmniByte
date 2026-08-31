#pragma once
// Source — implementasi analyze() yang dipisah dari engine.
// Analyzer parse .bsp lump table + entity string secara statis (tidak butuh proses live).
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::source {

class SourceAnalyzer {
public:
    // parse .bsp lump table + entity string statis
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::source
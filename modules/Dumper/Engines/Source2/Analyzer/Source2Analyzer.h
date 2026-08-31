#pragma once
// Source2 — implementasi analyze() yang dipisah dari engine.
// Analyzer parse resource block header .vpk_c secara statis (tidak butuh proses live).
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::source2 {

class Source2Analyzer {
public:
    // parse resource block header .vpk_c statis
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::source2
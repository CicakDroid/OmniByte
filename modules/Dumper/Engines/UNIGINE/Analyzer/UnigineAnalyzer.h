#pragma once
// UNIGINE — implementasi analyze() yang dipisah dari engine.
// Analyzer parse .unisync package & .xml world secara statis (tidak butuh proses live).
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::unigine {

class UnigineAnalyzer {
public:
    // parse .unisync package & .xml world statis
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::unigine
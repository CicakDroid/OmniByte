#pragma once
// CryEngine — implementasi analyze() yang dipisah dari engine.
// Analyzer parse .pak container secara statis (CryPak magic), tidak butuh proses live.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::cryengine {

class CryEngineAnalyzer {
public:
    // parse .pak container statis (CryPak magic)
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::cryengine
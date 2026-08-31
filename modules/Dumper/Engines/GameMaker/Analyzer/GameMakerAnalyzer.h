#pragma once
// GameMaker — implementasi analyze() yang dipisah dari engine.
// Analyzer parse data.win chunk secara statis (FORM/YYYG -> GEN8/STRG/OBJT),
// tidak butuh proses live.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::gamemaker {

class GameMakerAnalyzer {
public:
    // parse data.win chunk statis (FORM/YYYG -> GEN8/STRG/OBJT)
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::gamemaker
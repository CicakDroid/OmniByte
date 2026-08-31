#pragma once
// UnrealEngine — implementasi analyze() yang dipisah dari engine.
// Analyzer baca GNames/GObjects table statis dari .pak / binary section (tidak butuh proses live).
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::unrealengine {

class UnrealEngineAnalyzer {
public:
    // baca GNames/GObjects table statis dari .pak / binary section
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::unrealengine
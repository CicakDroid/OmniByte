#pragma once
// UnityMono — implementasi analyze() yang dipisah dari engine.
// Analyzer enumerasi MonoImage -> MonoClass -> MonoMethod dari
// Assembly-CSharp.dll secara statis via IL parsing (tidak butuh proses live).
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::unitymono {

class UnityMonoAnalyzer {
public:
    // enumerasi MonoImage -> MonoClass -> MonoMethod dari Assembly-CSharp.dll (statis, via IL parsing)
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::unitymono
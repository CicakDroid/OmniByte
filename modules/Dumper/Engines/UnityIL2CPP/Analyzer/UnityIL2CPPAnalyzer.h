#pragma once
// UnityIL2CPP — implementasi analyze() yang dipisah dari engine.
// Analyzer baca global-metadata.dat secara statis (tidak butuh proses live).
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::unityil2cpp {

class UnityIL2CPPAnalyzer {
public:
    // Baca global-metadata.dat pakai profile->offsetOf(...)
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        // baca typeDefinitionsOffset, stringLiteralOffset dari profile,
        // parse struct sesuai structSize("Il2CppTypeDefinition")
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::unityil2cpp
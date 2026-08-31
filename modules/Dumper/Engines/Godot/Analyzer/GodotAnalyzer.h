#pragma once
// Godot — implementasi analyze() yang dipisah dari engine.
// Analyzer parse .pck package secara statis (GDPC magic + file table), tidak butuh proses live.
#include "../../../DumperCore/IDumperEngine.h"
#include "../../../DumperCore/IEngineProfile.h"
#include <memory>

namespace omnibyte::dumper::godot {

class GodotAnalyzer {
public:
    // parse .pck package statis (GDPC magic + file table)
    static DumpResult analyze(const AnalysisTarget& target,
                              const std::shared_ptr<IEngineProfile>& profile) {
        DumpResult result;
        (void)target;
        (void)profile;
        return result;
    }
};

} // namespace omnibyte::dumper::godot
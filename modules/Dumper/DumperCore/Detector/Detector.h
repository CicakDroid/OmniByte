#pragma once
// DumperCore/Detector — orchestrates engine detection via EngineRegistry.
// Entry point: Detector::detect(target) returns the best-matching engine + profile.
#include "../EngineRegistry.h"
#include "../IDumperEngine.h"
#include "../DumpResult.h"
#include "../AnalysisTarget.h"
#include <memory>
#include <optional>

namespace omnibyte::dumper {

// Detection outcome — engine + profile ready for analyze/resolveSymbols.
struct DetectionOutcome {
    std::shared_ptr<IDumperEngine> engine;
    std::shared_ptr<IEngineProfile> profile;
    DetectionResult detection;
    bool isAmbiguous = false;  // true when multiple engines matched within 0.1 confidence
    std::vector<std::pair<std::shared_ptr<IDumperEngine>, DetectionResult>> alternatives;
};

class Detector {
public:
    // Main entry: detect best engine + resolve profile for target.
    // Returns nullopt if no engine matched.
    static std::optional<DetectionOutcome> detect(const AnalysisTarget& target);

    // Convenience: detect + analyze in one call.
    static std::optional<DumpResult> detectAndAnalyze(const AnalysisTarget& target);

    // Convenience: detect + resolveSymbols in one call.
    static std::optional<DumpResult> detectAndResolve(const AnalysisTarget& target);
};

} // namespace omnibyte::dumper

// DumperCore/Detector — orchestrates engine detection via EngineRegistry.
#include "Detector.h"

namespace omnibyte::dumper {

std::optional<DetectionOutcome> Detector::detect(const AnalysisTarget& target) {
    auto& registry = EngineRegistry::instance();

    auto matchResult = registry.detectBestMatch(target);
    if (!matchResult) {
        return std::nullopt;
    }

    DetectionOutcome outcome;
    outcome.engine = matchResult->best;
    outcome.detection = matchResult->bestDetection;
    outcome.isAmbiguous = !matchResult->allCandidates.empty() &&
                          matchResult->allCandidates.size() > 1;

    if (outcome.isAmbiguous) {
        outcome.alternatives = std::move(matchResult->allCandidates);
    }

    // Resolve profile from detected version
    if (outcome.engine && !outcome.detection.detectedVersion.empty()) {
        outcome.profile = outcome.engine->resolveProfile(outcome.detection.detectedVersion);
    }

    return outcome;
}

std::optional<DumpResult> Detector::detectAndAnalyze(const AnalysisTarget& target) {
    auto outcome = detect(target);
    if (!outcome || !outcome->engine) {
        return std::nullopt;
    }

    return outcome->engine->analyze(target, outcome->profile);
}

std::optional<DumpResult> Detector::detectAndResolve(const AnalysisTarget& target) {
    auto outcome = detect(target);
    if (!outcome || !outcome->engine) {
        return std::nullopt;
    }

    return outcome->engine->resolveSymbols(target, outcome->profile);
}

} // namespace omnibyte::dumper

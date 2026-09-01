// DumperCore/EngineRegistry — singleton registry for all IDumperEngine implementations.
// Provides detectBestMatch() with confidence-based ranking and tie handling.
#include "EngineRegistry.h"
#include <algorithm>

namespace omnibyte::dumper {

EngineRegistry& EngineRegistry::instance() {
    static EngineRegistry reg;
    return reg;
}

void EngineRegistry::registerEngine(std::shared_ptr<IDumperEngine> engine) {
    if (!engine) return;

    // Prevent duplicate engine types
    for (const auto& existing : engines_) {
        if (existing->type() == engine->type()) {
            return;  // already registered
        }
    }

    engines_.push_back(std::move(engine));
}

std::optional<EngineRegistry::MatchResult> EngineRegistry::detectBestMatch(
    const AnalysisTarget& target) const {

    // Collect detection results from all registered engines
    struct Candidate {
        std::shared_ptr<IDumperEngine> engine;
        DetectionResult detection;
    };

    std::vector<Candidate> matched;

    for (const auto& engine : engines_) {
        auto detection = engine->detect(target);
        if (detection.matched) {
            matched.push_back({engine, std::move(detection)});
        }
    }

    if (matched.empty()) {
        return std::nullopt;
    }

    // Sort by confidence descending
    std::sort(matched.begin(), matched.end(),
        [](const Candidate& a, const Candidate& b) {
            return a.detection.confidence > b.detection.confidence;
        });

    MatchResult result;
    result.best = matched[0].engine;
    result.bestDetection = matched[0].detection;

    // Check for ties: if second-best is within 0.1 confidence, treat as ambiguous
    if (matched.size() > 1) {
        float diff = result.bestDetection.confidence - matched[1].detection.confidence;
        if (diff < 0.1f) {
            // Ambiguous — return all candidates for UI selection
            for (auto& c : matched) {
                result.allCandidates.emplace_back(
                    std::move(c.engine), std::move(c.detection));
            }
            // Keep best/bestDetection from first element
            result.best = result.allCandidates[0].first;
            result.bestDetection = result.allCandidates[0].second;
        }
    }

    return result;
}

std::vector<std::shared_ptr<IDumperEngine>> EngineRegistry::allEngines() const {
    return engines_;
}

} // namespace omnibyte::dumper

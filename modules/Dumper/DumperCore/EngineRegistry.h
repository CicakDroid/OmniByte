#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <utility>
#include "IDumperEngine.h"
#include "AnalysisTarget.h"

namespace omnibyte::dumper {

// Bagian dari DumperCore -- satu-satunya tempat yang tahu semua engine yang ada.
// Menambah engine baru = registerEngine() satu baris di init, tidak ada if-else
// per-engine yang tersebar di tempat lain.
class EngineRegistry {
public:
    static EngineRegistry& instance();

    void registerEngine(std::shared_ptr<IDumperEngine> engine);

    // Jalankan detect() ke semua engine terdaftar, kembalikan yang confidence tertinggi.
    // Kalau ada >1 kandidat dengan confidence berdekatan (selisih < 0.1), kembalikan
    // seluruh kandidat lewat allCandidates agar UI bisa minta user pilih manual --
    // penting untuk kasus signature ambigu/false-positive.
    struct MatchResult {
        std::shared_ptr<IDumperEngine> best;
        DetectionResult bestDetection;
        std::vector<std::pair<std::shared_ptr<IDumperEngine>, DetectionResult>> allCandidates;
    };

    std::optional<MatchResult> detectBestMatch(const AnalysisTarget& target) const;

    std::vector<std::shared_ptr<IDumperEngine>> allEngines() const;

private:
    std::vector<std::shared_ptr<IDumperEngine>> engines_;
};

} // namespace omnibyte::dumper
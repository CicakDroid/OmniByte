#include "Highway.h"

#include <hwy/highway.h>

namespace hn = hwy::HWY_NAMESPACE;

namespace omnibyte::common {

HighwayAdapter& HighwayAdapter::instance() {
    static HighwayAdapter s;
    return s;
}

namespace {

inline bool MatchAt(const uint8_t* p, const PatternSignature& sig) {
    const size_t n = sig.bytes.size();
    for (size_t i = 0; i < n; ++i) {
        const bool wild = sig.mask.empty() || sig.mask[i] != 0;
        if (wild) continue;
        if (p[i] != sig.bytes[i]) return false;
    }
    return true;
}

inline bool IsExact(const PatternSignature& sig) {
    if (sig.mask.empty()) return true;
    for (uint8_t m : sig.mask) {
        if (m != 0) return false;
    }
    return true;
}

inline std::optional<size_t> AnchorIndex(const PatternSignature& sig) {
    if (sig.mask.empty()) return 0;
    for (size_t i = 0; i < sig.mask.size(); ++i) {
        if (sig.mask[i] == 0) return i;
    }
    return std::nullopt;
}

template <typename F>
void ForEachEqLane(hn::ScalableTag<uint8_t> d, const decltype(hn::Eq(
    hn::Zero(d), hn::Zero(d)))& eq, F&& f) {
    uint8_t bits[(64 + 7) / 8] = {};
    hn::StoreMaskBits(d, eq, bits);
    for (size_t byte = 0; byte < sizeof(bits); ++byte) {
        uint8_t b = bits[byte];
        int base = 0;
        while (b != 0) {
            const int bit = base + __builtin_ctz(static_cast<unsigned>(b));
            f(static_cast<size_t>(bit));
            b &= static_cast<uint8_t>(b - 1);
            base += 8;
        }
    }
}

} // namespace

std::optional<size_t> HighwayAdapter::FindPattern(
    const uint8_t* data, size_t size, const PatternSignature& sig
) {
    if (!data || sig.bytes.empty() || size < sig.bytes.size()) return std::nullopt;
    const size_t n = sig.bytes.size();
    const size_t limit = size - n;

    const auto anchor = AnchorIndex(sig);
    if (!anchor) return size_t{0};
    const size_t ai = *anchor;
    const uint8_t need = sig.bytes[ai];

    hn::ScalableTag<uint8_t> d;
    const size_t L = hn::Lanes(d);
    const auto want = hn::Set(d, need);

    size_t i = 0;
    if (IsExact(sig)) {
        for (; i + L <= limit + 1; i += L) {
            const auto chunk = hn::LoadU(d, data + i);
            const auto eq = hn::Eq(chunk, want);
            std::optional<size_t> found;
            ForEachEqLane(d, eq, [&](size_t k) {
                if (found) return;
                const size_t cand = i + k;
                if (cand > limit) return;
                if (MatchAt(data + cand, sig)) found = cand;
            });
            if (found) return found;
        }
    } else {
        if (ai >= size) return std::nullopt;
        for (; i + L <= limit + 1; i += L) {
            if (i + ai + L > size) break;
            const auto chunk = hn::LoadU(d, data + i + ai);
            const auto eq = hn::Eq(chunk, want);
            std::optional<size_t> found;
            ForEachEqLane(d, eq, [&](size_t k) {
                if (found) return;
                const size_t cand = i + k;
                if (cand > limit) return;
                if (MatchAt(data + cand, sig)) found = cand;
            });
            if (found) return found;
        }
    }

    for (; i <= limit; ++i) {
        if (data[i + ai] != need) continue;
        if (MatchAt(data + i, sig)) return i;
    }
    return std::nullopt;
}

std::vector<size_t> HighwayAdapter::FindAllPatterns(
    const uint8_t* data, size_t size, const PatternSignature& sig
) {
    std::vector<size_t> out;
    if (!data || sig.bytes.empty() || size < sig.bytes.size()) return out;
    const size_t n = sig.bytes.size();
    const size_t limit = size - n;

    const auto anchor = AnchorIndex(sig);
    if (!anchor) {
        for (size_t i = 0; i <= limit; ++i) out.push_back(i);
        return out;
    }
    const size_t ai = *anchor;
    const uint8_t need = sig.bytes[ai];

    hn::ScalableTag<uint8_t> d;
    const size_t L = hn::Lanes(d);
    const auto want = hn::Set(d, need);
    const bool exact = IsExact(sig);

    size_t i = 0;
    for (; i + L <= limit + 1; i += L) {
        const size_t probe = exact ? i : i + ai;
        if (probe + L > size) break;
        const auto chunk = hn::LoadU(d, data + probe);
        const auto eq = hn::Eq(chunk, want);
        ForEachEqLane(d, eq, [&](size_t k) {
            const size_t cand = i + k;
            if (cand > limit) return;
            if (MatchAt(data + cand, sig)) out.push_back(cand);
        });
    }
    for (; i <= limit; ++i) {
        if (data[i + ai] != need) continue;
        if (MatchAt(data + i, sig)) out.push_back(i);
    }
    return out;
}

} // namespace omnibyte::common

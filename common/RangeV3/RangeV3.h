#ifndef OMNIBYTE_COMMON_RANGEV3_H
#define OMNIBYTE_COMMON_RANGEV3_H

#include <cstddef>
#include <vector>

#include <range/v3/all.hpp>

namespace omnibyte::common {

class RangeV3Adapter {
public:
    static RangeV3Adapter& instance();

    template <typename Range, typename Pred, typename Fn>
    static auto FilterTransform(Range&& range, Pred pred, Fn fn)
        -> std::vector<decltype(fn(*ranges::begin(range)))> {
        auto view = range | ranges::views::filter(pred) | ranges::views::transform(fn);
        std::vector<decltype(fn(*ranges::begin(range)))> out;
        for (auto&& x : view) out.push_back(x);
        return out;
    }

    template <typename Range>
    static auto Chunk(Range&& range, std::size_t size) {
        auto view = range | ranges::views::chunk(size);
        using ChunkRef = decltype(*ranges::begin(view));
        std::vector<ChunkRef> out;
        for (auto&& c : view) out.push_back(c);
        return out;
    }
};

} // namespace omnibyte::common

#endif // OMNIBYTE_COMMON_RANGEV3_H

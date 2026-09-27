#ifndef OMNIBYTE_COMMON_HIGHWAY_H
#define OMNIBYTE_COMMON_HIGHWAY_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace omnibyte::common {

struct PatternSignature {
    std::vector<uint8_t> bytes;
    std::vector<uint8_t> mask;
};

class HighwayAdapter {
public:
    static HighwayAdapter& instance();

    std::optional<size_t> FindPattern(
        const uint8_t* data, size_t size, const PatternSignature& sig
    );

    std::vector<size_t> FindAllPatterns(
        const uint8_t* data, size_t size, const PatternSignature& sig
    );
};

} // namespace omnibyte::common

#endif // OMNIBYTE_COMMON_HIGHWAY_H


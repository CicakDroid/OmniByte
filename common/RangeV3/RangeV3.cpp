#include "RangeV3.h"

namespace omnibyte::common {

RangeV3Adapter& RangeV3Adapter::instance() {
    static RangeV3Adapter s;
    return s;
}

} // namespace omnibyte::common

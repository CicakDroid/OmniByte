// ZigZag — thin wrapper exposing active stealth backend.

#include "ZigZag.h"

namespace omnibyte::runtime {

ZigZag::ZigZag(std::shared_ptr<IStealthBackend> backend)
    : backend_(std::move(backend)) {}

} // namespace omnibyte::runtime

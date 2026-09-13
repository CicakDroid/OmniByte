// Bhook adapter — PLT/GOT hooking stub.
// Source: https://github.com/AsL5/Bhook (Apache-2.0)

#include "BhookAdapter.h"

namespace omnibyte::runtime::backends {

bool BhookAdapter::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    // TODO: Use Bhook's PLT/GOT interception API.
    (void)addr; (void)replacement; (void)originalOut;
    return false;
}

bool BhookAdapter::unhook(uintptr_t addr) {
    (void)addr;
    return false;
}

bool BhookAdapter::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    (void)addr; (void)data; (void)size;
    return false;
}

} // namespace omnibyte::runtime::backends

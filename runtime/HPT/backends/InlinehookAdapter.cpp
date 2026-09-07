// Inlinehook adapter — ARM inline hook stub.
// Source: https://github.com/AsL5/android-inline-hook (Apache-2.0)

#include "InlinehookAdapter.h"

namespace omnibyte::runtime::backends {

bool InlinehookAdapter::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    // TODO: Use android-inline-hook API:
    //   substrace_thumb(addr, replacement, originalOut);
    //   substrace_arm(addr, replacement, originalOut);
    (void)addr; (void)replacement; (void)originalOut;
    return false;
}

bool InlinehookAdapter::unhook(uintptr_t addr) {
    (void)addr;
    return false;
}

bool InlinehookAdapter::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    (void)addr; (void)data; (void)size;
    return false;
}

} // namespace omnibyte::runtime::backends

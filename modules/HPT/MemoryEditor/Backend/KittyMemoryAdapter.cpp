// KittyMemory adapter — ARM inline hook stub.
// Source: https://github.com/MJx0/KittyMemory (MIT)

#include "KittyMemoryAdapter.h"

namespace omnibyte::runtime::backends {

bool KittyMemoryAdapter::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    // TODO: Use KittyMemory API:
    //   KittyMemory::makeHook(addr, replacement, originalOut);
    (void)addr; (void)replacement; (void)originalOut;
    return false;
}

bool KittyMemoryAdapter::unhook(uintptr_t addr) {
    (void)addr;
    return false;
}

bool KittyMemoryAdapter::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    // TODO: Use KittyMemory::write_raw(addr, data, size).
    (void)addr; (void)data; (void)size;
    return false;
}

} // namespace omnibyte::runtime::backends

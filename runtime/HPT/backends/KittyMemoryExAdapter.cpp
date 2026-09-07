// KittyMemoryEx adapter — extended memory + hook stub.
// Source: https://github.com/MJx0/KittyMemoryEx (MIT)

#include "KittyMemoryExAdapter.h"

namespace omnibyte::runtime::backends {

bool KittyMemoryExAdapter::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    // TODO: Use KittyMemoryEx API (extends KittyMemory with more hook types).
    (void)addr; (void)replacement; (void)originalOut;
    return false;
}

bool KittyMemoryExAdapter::unhook(uintptr_t addr) {
    (void)addr;
    return false;
}

bool KittyMemoryExAdapter::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    // TODO: Use KittyMemoryEx's write API.
    (void)addr; (void)data; (void)size;
    return false;
}

} // namespace omnibyte::runtime::backends

// Vector adapter — TracelessHook stub.
// Source: https://github.com/1013503897/Vector (Apache-2.0)

#include "VectorAdapter.h"

namespace omnibyte::runtime::backends {

bool VectorAdapter::hookFunction(uintptr_t addr, void* replacement, void** originalOut) {
    // TODO: Use Vector's traceless inline hook API.
    (void)addr; (void)replacement; (void)originalOut;
    return false;
}

bool VectorAdapter::unhook(uintptr_t addr) {
    (void)addr;
    return false;
}

bool VectorAdapter::patchMemory(uintptr_t addr, const uint8_t* data, size_t size) {
    (void)addr; (void)data; (void)size;
    return false;
}

} // namespace omnibyte::runtime::backends

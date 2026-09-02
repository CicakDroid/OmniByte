#pragma once
// ── triton/archEnums.hpp ─────────────────────────────────────────────
// STUB HEADER for compile-checking only.
// Based on Triton library: https://triton-library.github.io/
// Enums used by symbolicexec_triton.cpp.

#include <cstdint>

namespace triton {

enum MODE {
    ALIGNED_MEMORY = 0
};

namespace arch {

enum architecture_e {
    ARCH_INVALID = 0,
    ARCH_X86 = 1,
    ARCH_X86_64 = 2,
    ARCH_ARM = 3,
    ARCH_ARM64 = 4,
    ARCH_MIPS = 5
};

enum exception_e {
    NO_FAULT = 0
};

} // namespace arch
} // namespace triton

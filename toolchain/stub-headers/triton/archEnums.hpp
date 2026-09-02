// STUB HEADER -- untuk g++ -fsyntax-only lokal SAJA.
//  BUKAN library asli. Build production pakai header dari
//  FetchContent (lihat CMakeLists.txt backend terkait).
//  Signature di sini mengikuti dokumentasi resmi versi 1.0 --
//  jangan percaya buta, verifikasi ulang saat library asli di-install.
//
// Sources:
//   - JonathanSalwan/Triton: src/libtriton/includes/triton/archEnums.hpp
//   - https://github.com/JonathanSalwan/Triton/blob/master/src/libtriton/includes/triton/archEnums.hpp

#ifndef TRITON_ARCHENUMS_STUB_H
#define TRITON_ARCHENUMS_STUB_H

#include <cstdint>
#include <cstddef>
#include <functional>

namespace triton {

// ── arch namespace ──────────────────────────────────────────────
namespace arch {

/// Types of architecture.
/// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/archEnums.hpp
enum architecture_e {
    ARCH_INVALID = 0,
    ARCH_AARCH64,
    ARCH_ARM32,
    ARCH_RV32,
    ARCH_RV64,
    ARCH_X86,
    ARCH_X86_64,
};

/// Types of endianness.
/// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/archEnums.hpp
enum endianness_e {
    LE_ENDIANNESS,
    BE_ENDIANNESS,
};

/// Types of operand.
/// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/archEnums.hpp
enum operand_e {
    OP_INVALID = 0,
    OP_IMM,
    OP_MEM,
    OP_REG
};

/// Types of exceptions.
/// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/archEnums.hpp
enum exception_e {
    NO_FAULT = 0,
    FAULT_DE,
    FAULT_BP,
    FAULT_UD,
    FAULT_GP,
};

/// Types of register (simplified for syntax check).
/// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/archEnums.hpp
/// In the real API this is generated from x86.spec, aarch64.spec, etc.
enum register_e {
    ID_REG_INVALID = 0,
    ID_REG_LAST_ITEM = 1
};

} // namespace arch

} // namespace triton

#endif // TRITON_ARCHENUMS_STUB_H

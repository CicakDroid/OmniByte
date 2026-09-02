// STUB HEADER -- untuk g++ -fsyntax-only lokal SAJA.
//  BUKAN library asli. Build production pakai header dari
//  FetchContent (lihat CMakeLists.txt backend terkait).
//  Signature di sini mengikuti dokumentasi resmi versi 1.0 --
//  jangan percaya buta, verifikasi ulang saat library asli di-install.
//
// Sources:
//   - JonathanSalwan/Triton: src/libtriton/includes/triton/cpuSize.hpp
//   - https://github.com/JonathanSalwan/Triton/blob/master/src/libtriton/includes/triton/cpuSize.hpp

#ifndef TRITON_CPUSIZE_STUB_H
#define TRITON_CPUSIZE_STUB_H

#include <cstdint>

namespace triton {

// ── Basic type aliases ──────────────────────────────────────────
// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/tritonTypes.hpp
typedef std::uint8_t  uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;
typedef std::size_t   usize;

// ── Size constants ──────────────────────────────────────────────
// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/cpuSize.hpp
namespace size {
    constexpr triton::uint32 byte          = 1;
    constexpr triton::uint32 word          = 2;
    constexpr triton::uint32 dword         = 4;
    constexpr triton::uint32 qword         = 8;
    constexpr triton::uint32 fword         = 10;
    constexpr triton::uint32 dqword        = 16;
    constexpr triton::uint32 qqword        = 32;
    constexpr triton::uint32 dqqword       = 64;
    constexpr triton::uint32 max_supported = dqqword;
} // namespace size

namespace bitsize {
    constexpr triton::uint32 flag          = 1;
    constexpr triton::uint32 byte          = 8;
    constexpr triton::uint32 word          = 16;
    constexpr triton::uint32 dword         = 32;
    constexpr triton::uint32 qword         = 64;
    constexpr triton::uint32 fword         = 80;
    constexpr triton::uint32 dqword        = 128;
    constexpr triton::uint32 qqword        = 256;
    constexpr triton::uint32 dqqword       = 512;
    constexpr triton::uint32 max_supported = dqqword;
} // namespace bitsize

} // namespace triton

#endif // TRITON_CPUSIZE_STUB_H

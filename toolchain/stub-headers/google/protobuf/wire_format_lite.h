#pragma once
// ── STUB HANYA UNTUK VERIFIKASI LOKAL ──────────────────────────────
// Dipakai `g++ -fsyntax-only` (lihat toolchain/stub-headers/README.md).
// JANGAN ditambahkan ke CMakeLists produksi; tidak ada implementasi di sini.
//
// Signature disalin dari google/protobuf/wire_format_lite.h (protobuf v25.x).

#include <cstdint>

namespace google {
namespace protobuf {
namespace internal {

// API subset yang dipakai ProtobufAdapter.
class WireFormatLite {
public:
    enum WireType {
        WIRETYPE_VARINT = 0,
        WIRETYPE_FIXED64 = 1,
        WIRETYPE_LENGTH_DELIMITED = 2,
        WIRETYPE_START_GROUP = 3,
        WIRETYPE_END_GROUP = 4,
        WIRETYPE_FIXED32 = 5,
    };

    static int GetTagFieldNumber(uint32_t tag);
    static WireType GetTagWireType(uint32_t tag);
};

} // namespace internal
} // namespace protobuf
} // namespace google

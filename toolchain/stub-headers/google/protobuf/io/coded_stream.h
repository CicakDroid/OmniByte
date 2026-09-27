#pragma once
// ── STUB HANYA UNTUK VERIFIKASI LOKAL ──────────────────────────────
// Dipakai `g++ -fsyntax-only` (lihat toolchain/stub-headers/README.md).
// JANGAN ditambahkan ke CMakeLists produksi; tidak ada implementasi di sini.
//
// Signature disalin dari google/protobuf/io/coded_stream.h (protobuf v25.x).

#include <cstdint>

namespace google {
namespace protobuf {
namespace io {

// API subset yang dipakai ProtobufAdapter.
class CodedInputStream {
public:
    typedef int Limit;

    explicit CodedInputStream(const uint8_t* data, int size);

    uint32_t ReadTag();

    bool ReadVarint32(uint32_t* value);
    bool ReadVarint64(uint64_t* value);
    bool ReadLittleEndian32(uint32_t* value);
    bool ReadLittleEndian64(uint64_t* value);
    bool ReadRaw(void* buffer, int size);
    bool Skip(int count);

    Limit PushLimit(int byte_limit);
    void PopLimit(Limit old_limit);

    int CurrentPosition() const;
    int BytesUntilLimit() const;
};

} // namespace io
} // namespace protobuf
} // namespace google

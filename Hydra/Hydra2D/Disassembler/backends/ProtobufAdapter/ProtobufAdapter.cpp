#include "Disassembler/backends/ProtobufAdapter/ProtobufAdapter.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite.h>

#include <cstdio>
#include <limits>

namespace omnibyte::hydradis {
namespace {

using google::protobuf::io::CodedInputStream;
using google::protobuf::internal::WireFormatLite;

// ponytail: depth cap 10 -- AST stream praktis biasanya jauh lebih dangkal.
// Kalau ada stream sah yang lebih dalam, naikkan konstanta ini saja;
// tidak ada alasan lain untuk membatasi.
constexpr int kMaxDepth = 10;

/// 0x... dari nilai fixed/varint -- opStr harus tetap terbaca untuk hex dump.
std::string toHex(uint64_t value) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "0x%llx", static_cast<unsigned long long>(value));
    return buf;
}

/// Path field jadi mnemonic, mis. f3.f7.varint.
/// size = end-start dibulatkan ke uint16 (batas struct Instruction).
void emitLeaf(std::vector<Instruction>& out,
              uint64_t baseAddr,
              int64_t start,
              int64_t end,
              std::string mnemonic,
              std::string opStr) {
    Instruction instr;
    instr.address = baseAddr + static_cast<uint64_t>(start);

    const uint64_t span = (end > start) ? static_cast<uint64_t>(end - start) : 0;
    instr.size = (span > 0xFFFFu) ? 0xFFFFu : static_cast<uint16_t>(span);

    instr.mnemonic = std::move(mnemonic);
    instr.opStr = std::move(opStr);
    out.push_back(std::move(instr));
}

/// Walk semua field dalam satu message (cis sudah dibatasi ke message ini).
///
/// @return false kalau format rusak / wire type tak dikenal. Field yang sudah
///         ter-walk tetap ada di `out`; pemanggil memutuskan mau dipakai
///         sebagian atau dibuang.
bool walkFields(CodedInputStream& cis,
                uint64_t baseAddr,
                std::vector<Instruction>& out,
                const std::string& path,
                int depth) {
    while (true) {
        const int64_t fieldStart = cis.CurrentPosition();
        const uint32_t tag = cis.ReadTag();
        if (tag == 0) {
            return true;  // batas message / stream habis
        }

        const int fieldNum = WireFormatLite::GetTagFieldNumber(tag);
        const WireFormatLite::WireType wireType = WireFormatLite::GetTagWireType(tag);
        const std::string here = path + "f" + std::to_string(fieldNum);

        switch (wireType) {
            case WireFormatLite::WIRETYPE_VARINT: {
                uint64_t value = 0;
                if (!cis.ReadVarint64(&value)) {
                    return false;
                }
                emitLeaf(out, baseAddr, fieldStart, cis.CurrentPosition(),
                         here + ".varint", std::to_string(value));
                break;
            }
            case WireFormatLite::WIRETYPE_FIXED64: {
                uint64_t value = 0;
                if (!cis.ReadLittleEndian64(&value)) {
                    return false;
                }
                emitLeaf(out, baseAddr, fieldStart, cis.CurrentPosition(),
                         here + ".fixed64", toHex(value));
                break;
            }
            case WireFormatLite::WIRETYPE_FIXED32: {
                uint32_t value = 0;
                if (!cis.ReadLittleEndian32(&value)) {
                    return false;
                }
                emitLeaf(out, baseAddr, fieldStart, cis.CurrentPosition(),
                         here + ".fixed32", toHex(value));
                break;
            }
            case WireFormatLite::WIRETYPE_LENGTH_DELIMITED: {
                uint32_t length = 0;
                if (!cis.ReadVarint32(&length)) {
                    return false;
                }

                const int64_t payloadStart = cis.CurrentPosition();
                const int oldLimit = cis.PushLimit(static_cast<int>(length));

                // Coba tafsirkan sebagai sub-message. Kalau gagal (bukan
                // message: string/bytes/arus acak) atau melebihi depth cap,
                // buang hasil parsialnya dan laporkan sebagai satu field opaq.
                std::vector<Instruction> children;
                const bool isMessage =
                    (depth + 1 <= kMaxDepth) &&
                    walkFields(cis, baseAddr, children, here + ".", depth + 1);

                // walk bisa berhenti sebelum ujung payload -- majukan dulu,
                // baru pop limit, supaya field berikutnya mulai dari posisi benar.
                const int64_t payloadEnd = payloadStart + length;
                const int64_t pos = cis.CurrentPosition();
                if (pos < payloadEnd) {
                    cis.Skip(static_cast<int>(payloadEnd - pos));
                }
                cis.PopLimit(oldLimit);

                if (isMessage && !children.empty()) {
                    out.insert(out.end(), children.begin(), children.end());
                } else {
                    emitLeaf(out, baseAddr, fieldStart, cis.CurrentPosition(),
                             here + ".len", std::to_string(length) + " bytes");
                }
                break;
            }
            case WireFormatLite::WIRETYPE_START_GROUP:
            case WireFormatLite::WIRETYPE_END_GROUP:
            default:
                // group sudah deprecated sejak proto2; default menangkap wire
                // type baru yang mungkin ditambah protobuf di masa depan.
                return false;
        }
    }
}

} // namespace

DisassemblyResult ProtobufAdapter::disassemble(
    const uint8_t* code,
    size_t codeSize,
    uint64_t baseAddr,
    size_t count
) const {
    DisassemblyResult result;

    if (!code || codeSize == 0) {
        result.errorMessage = "Empty code buffer";
        return result;
    }

    // CodedInputStream memakai int di seluruh API-nya -- tolak sebelum overflow.
    if (codeSize > static_cast<size_t>(std::numeric_limits<int>::max())) {
        result.errorMessage = "Code buffer too large for protobuf decoder";
        return result;
    }

    CodedInputStream cis(code, static_cast<int>(codeSize));
    std::vector<Instruction> fields;
    const bool ok = walkFields(cis, baseAddr, fields, "", 0);

    result.totalBytes = static_cast<size_t>(cis.CurrentPosition());

    if (fields.empty()) {
        result.errorMessage = ok ? "No fields decoded" : "Malformed protobuf wire format";
        return result;
    }

    // ponytail: `count` dipotong SETELAH walk penuh, bukan di dalam loop.
    // Semua caller produksi memakai count=0 (unlimited). Kalau early-exit
    // ternyata perlu (stream besar + count kecil), thread `count` ke walkFields.
    if (count > 0 && fields.size() > count) {
        fields.resize(count);
    }

    result.instructions = std::move(fields);
    result.success = true;

    if (!ok) {
        result.success = false;
        result.errorMessage = "Truncated protobuf stream; partial fields returned";
    }

    return result;
}

std::unique_ptr<HydraDisassembler> createProtobufDisassembler(DisassemblerArch) {
    return std::make_unique<ProtobufAdapter>();
}

} // namespace omnibyte::hydradis

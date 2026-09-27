#include "Disassembler/backends/WasmAdapter/WasmAdapter.h"

#include <cstdio>
#include <cstring>
#include <limits>
#include <sstream>
#include <string>

#include "wabt/binary-reader.h"
#include "wabt/binary-reader-nop.h"

namespace omnibyte::hydradis {
namespace {

// f32/f64 -> hexfloat (0x1.91eb86p+1) ala objdump. Bit-cast sendiri
// supaya tidak perlu include literal.h milik wabt.
std::string hexFloat32(uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    std::ostringstream os;
    os << std::hexfloat << value;
    return os.str();
}

std::string hexFloat64(uint64_t bits) {
    double value;
    std::memcpy(&value, &bits, sizeof(value));
    std::ostringstream os;
    os << std::hexfloat << value;
    return os.str();
}

// BlockSigToString() di binary-reader-objdump.cc:569.
std::string blockSigToString(const wabt::Type& type) {
    if (type.IsIndex()) {
        return "type[" + std::to_string(type.GetIndex()) + "]";
    }
    if (type == wabt::Type::Void) {
        return "";
    }
    return type.GetName();
}

// Streaming model ala wasm-objdump: OnOpcode dipanggil SEBELUM operand
// dibaca (binary-reader.cc:767), jadi instruksi ke-N di-flush begitu
// OnOpcode ke-N+1 tiba -- ukurannya = jarak antar titik mulai. Instruksi
// terakhir tiap function body ditutup di EndFunctionBody, saat
// state->offset sudah menunjuk akhir body.
class WasmDelegate final : public wabt::BinaryReaderNop {
public:
    WasmDelegate(const uint8_t* code,
                 uint64_t baseAddr,
                 size_t count,
                 DisassemblyResult& out)
        : code_(code), baseAddr_(baseAddr), count_(count), out_(out) {}

    const std::string& error() const { return error_; }

    // Simpan pesan pertama, lalu return true = "sudah ditangani" supaya
    // wabt tidak ikut nulis ke stderr (binary-reader.cc:244).
    bool OnError(const wabt::Error& error) override {
        if (error_.empty()) {
            error_ = error.message;
        }
        return true;
    }

    wabt::Result BeginFunctionBody(wabt::Index, wabt::Offset) override {
        inFunctionBody_ = true;
        pending_ = false;
        return wabt::Result::Ok;
    }

    wabt::Result EndFunctionBody(wabt::Index) override {
        wabt::Result res = wabt::Result::Ok;
        if (pending_) {
            res = flush(state->offset);
        }
        inFunctionBody_ = false;
        pending_ = false;
        return res;
    }

    wabt::Result OnOpcode(wabt::Opcode opcode) override {
        // Init expression (elem/data/global section) dan opcode sintetis
        // dari binary-reader.cc:3019 berada di luar function body.
        if (!inFunctionBody_) {
            return wabt::Result::Ok;
        }

        // state->offset sudah melewati byte opcode; mundur sepanjang opcode.
        const wabt::Offset start = state->offset - opcode.GetLength();
        if (pending_) {
            const wabt::Result res = flush(start);
            if (wabt::Failed(res)) {
                return res;
            }
        }

        pendingStart_ = start;
        pending_ = true;
        mnemonic_ = opcode.GetName();
        opStr_.clear();
        return wabt::Result::Ok;
    }

    wabt::Result OnOpcodeIndex(wabt::Index value) override {
        return append(std::to_string(value));
    }

    wabt::Result OnOpcodeIndexIndex(wabt::Index value,
                                    wabt::Index value2) override {
        return append(std::to_string(value) + " " + std::to_string(value2));
    }

    wabt::Result OnOpcodeUint32(uint32_t value) override {
        return append(std::to_string(value));
    }

    wabt::Result OnOpcodeUint32Uint32(uint32_t value, uint32_t value2) override {
        return append(std::to_string(value) + " " + std::to_string(value2));
    }

    wabt::Result OnOpcodeUint32Uint32Uint32(uint32_t value,
                                             uint32_t value2,
                                             uint32_t value3) override {
        return append(std::to_string(value) + " " + std::to_string(value2) +
                      " " + std::to_string(value3));
    }

    wabt::Result OnOpcodeUint32Uint32Uint32Uint32(uint32_t value,
                                                   uint32_t value2,
                                                   uint32_t value3,
                                                   uint32_t value4) override {
        return append(std::to_string(value) + " " + std::to_string(value2) +
                      " " + std::to_string(value3) + " " +
                      std::to_string(value4));
    }

    wabt::Result OnOpcodeUint64(uint64_t value) override {
        return append(std::to_string(value));
    }

    wabt::Result OnOpcodeF32(uint32_t value) override {
        return append(hexFloat32(value));
    }

    wabt::Result OnOpcodeF64(uint64_t value) override {
        return append(hexFloat64(value));
    }

    // ::v128 (bukan wabt::v128): wabt/common.h men-declare struct v128 di
    // global scope, di luar namespace wabt.
    wabt::Result OnOpcodeV128(::v128 value) override {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "0x%08x 0x%08x 0x%08x 0x%08x",
                      value.u32(0), value.u32(1), value.u32(2), value.u32(3));
        return append(buffer);
    }

    wabt::Result OnOpcodeBlockSig(wabt::Type sig_type) override {
        if (sig_type == wabt::Type::Void) {
            return wabt::Result::Ok;
        }
        return append(blockSigToString(sig_type));
    }

    wabt::Result OnOpcodeType(wabt::Type type) override {
        // objdump memakai GetRefKindName() kecuali untuk select/call_ref;
        // GetName() selalu menghasilkan teks valid tanpa harus melacak
        // opcode sebelumnya.
        return append(type.GetName());
    }

    wabt::Result OnBrTableExpr(wabt::Index num_targets,
                               wabt::Index* target_depths,
                               wabt::Index default_target_depth) override {
        std::string text;
        for (wabt::Index i = 0; i < num_targets; ++i) {
            text += std::to_string(target_depths[i]);
            text += ' ';
        }
        text += std::to_string(default_target_depth);
        return append(text);
    }

private:
    wabt::Result append(const std::string& operand) {
        if (!opStr_.empty()) {
            opStr_ += ' ';
        }
        opStr_ += operand;
        return wabt::Result::Ok;
    }

    // pending_ hanya berlaku di dalam function body, jadi flush terakhir
    // selalu dipanggil dari EndFunctionBody.
    // ponytail: guard non-canonical-LEB128 ala objdump
    // (binary-reader-objdump.cc:596) dilewati -- hanya memindah batas
    // size antara dua instruksi berdampingan pada input malformed.
    wabt::Result flush(wabt::Offset end) {
        pending_ = false;

        const size_t size = static_cast<size_t>(end - pendingStart_);
        if (size == 0 || size > std::numeric_limits<uint16_t>::max()) {
            error_ = "Instruction size out of range";
            return wabt::Result::Error;
        }

        // ponytail: `count` dipotong SAAT menyimpan, bukan di akhir. Wabt
        // tetap dijalankan sampai selesai (tidak ada abort murah), jadi hanya
        // instruksi di bawah `count` yang masuk ke vector dan dihitung.
        if (count_ == 0 || out_.instructions.size() < count_) {
            Instruction instr;
            instr.address = baseAddr_ + pendingStart_;
            instr.size = static_cast<uint16_t>(size);
            instr.mnemonic = mnemonic_;
            instr.opStr = opStr_;
            instr.bytes.assign(code_ + pendingStart_, code_ + end);

            out_.totalBytes += size;
            out_.instructions.push_back(std::move(instr));
        }

        return wabt::Result::Ok;
    }

    const uint8_t* code_;
    uint64_t baseAddr_;
    size_t count_;
    DisassemblyResult& out_;

    bool inFunctionBody_ = false;
    bool pending_ = false;
    wabt::Offset pendingStart_ = 0;
    std::string mnemonic_;
    std::string opStr_;
    std::string error_;
};

} // namespace

DisassemblyResult WasmAdapter::disassemble(
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

    WasmDelegate delegate(code, baseAddr, count, result);

    wabt::ReadBinaryOptions options;
    // Semua proposal aktif -- modul modern (SIMD, threads, GC, ...) tidak
    // boleh ditolak hanya karena default features-nya konservatif.
    options.features.EnableAll();
    options.stop_on_first_error = true;

    const wabt::Result res =
        wabt::ReadBinary(wabt::ByteSpan(code, codeSize), &delegate, options);

    if (wabt::Failed(res)) {
        result.success = false;
        result.errorMessage = delegate.error().empty()
            ? std::string("Failed to parse WebAssembly module")
            : delegate.error();
        return result;
    }

    result.success = true;
    return result;
}

std::unique_ptr<HydraDisassembler> createWasmDisassembler(DisassemblerArch) {
    return std::make_unique<WasmAdapter>();
}

} // namespace omnibyte::hydradis

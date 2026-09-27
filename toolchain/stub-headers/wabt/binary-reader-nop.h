#pragma once
// ── wabt/binary-reader-nop.h (stub) ────────────────────────────────────
// Stub untuk syntax check lokal saja — lihat stub-headers/README.md.
// Kelas nyata (binary-reader-nop.h:21) menurunkan SEMUA virtual delegate
// jadi no-op supaya subclass boleh override pilihan. Stub hanya memuat 18
// virtual yang ada di stub binary-reader.h — cukup supaya WasmDelegate
// jadi class konkret.

#include "wabt/binary-reader.h"

namespace wabt {

class BinaryReaderNop : public BinaryReaderDelegate {
public:
    bool OnError(const Error&) override { return false; }

    Result BeginFunctionBody(Index, Offset) override { return Result::Ok; }
    Result EndFunctionBody(Index) override { return Result::Ok; }

    Result OnOpcode(Opcode) override { return Result::Ok; }
    Result OnOpcodeIndex(Index) override { return Result::Ok; }
    Result OnOpcodeIndexIndex(Index, Index) override { return Result::Ok; }
    Result OnOpcodeUint32(uint32_t) override { return Result::Ok; }
    Result OnOpcodeUint32Uint32(uint32_t, uint32_t) override {
        return Result::Ok;
    }
    Result OnOpcodeUint32Uint32Uint32(uint32_t, uint32_t, uint32_t) override {
        return Result::Ok;
    }
    Result OnOpcodeUint32Uint32Uint32Uint32(uint32_t,
                                             uint32_t,
                                             uint32_t,
                                             uint32_t) override {
        return Result::Ok;
    }
    Result OnOpcodeUint64(uint64_t) override { return Result::Ok; }
    Result OnOpcodeF32(uint32_t) override { return Result::Ok; }
    Result OnOpcodeF64(uint64_t) override { return Result::Ok; }
    Result OnOpcodeV128(v128) override { return Result::Ok; }
    Result OnOpcodeBlockSig(Type) override { return Result::Ok; }
    Result OnOpcodeType(Type) override { return Result::Ok; }
    Result OnBrTableExpr(Index, Index*, Index) override { return Result::Ok; }
};

}  // namespace wabt

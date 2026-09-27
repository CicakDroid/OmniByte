#pragma once
// ── WasmAdapter.h ──────────────────────────────────────────────────────
// Public API backend WebAssembly (modul .wasm berformat binary).
//
// Decoder penuh (wabt) ada di .cpp -- header ini bebas include wabt,
// jadi konsumen tidak perlu punya wabt di include path-nya.

#include "Disassembler/HydraDisassembler.h"

namespace omnibyte::hydradis {

/// HydraDisassembler untuk modul WebAssembly binary.
///
/// Tiap instruksi dalam tiap function body jadi satu Instruction:
/// address = baseAddr + offset byte instruksi di buffer, size = panjang
/// byte instruksi (opcode + operand), mnemonic/opStr dari wabt, bytes =
/// potongan buffer mentahnya.
///
/// Tidak ada state lintas-panggilan: aman untuk sharing satu instance.
class WasmAdapter final : public HydraDisassembler {
public:
    WasmAdapter() = default;

    WasmAdapter(const WasmAdapter&) = delete;
    WasmAdapter& operator=(const WasmAdapter&) = delete;

    std::string name() const override { return "wasm"; }

    /// Backend ini punya ISA tetap: WebAssembly.
    DisassemblerArch arch() const override { return DisassemblerArch::WASM; }

    /// Walk semua function body modul jadi list instruksi.
    ///
    /// Instruksi di luar function body (init expression di elem/data/global
    /// section) sengaja tidak diikutkan -- hanya kode yang dieksekusi di
    /// runtime yang dianggap instruksi.
    ///
    /// `count` dipotong setelah walk penuh (wabt tidak punya mekanisme
    /// abort murah), sama seperti ProtobufAdapter.
    DisassemblyResult disassemble(
        const uint8_t* code,
        size_t codeSize,
        uint64_t baseAddr,
        size_t count
    ) const override;
};

/// Buat HydraDisassembler untuk modul WebAssembly.
/// `arch` diabaikan -- backend ini selalu DisassemblerArch::WASM.
/// Selalu sukses: wabt lengkap di-link, tidak ada kondisi gagal di ctor.
std::unique_ptr<HydraDisassembler> createWasmDisassembler(DisassemblerArch arch);

} // namespace omnibyte::hydradis

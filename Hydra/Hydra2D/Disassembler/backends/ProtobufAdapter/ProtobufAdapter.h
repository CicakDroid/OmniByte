#pragma once
// ── ProtobufAdapter.h ──────────────────────────────────────────────
// Public API backend Protobuf (stream AST berformat protobuf wire).
//
// Stream ini arch-agnostic -- tidak punya ISA -- jadi arch() selalu
// DisassemblerArch::None. Decoder dibuat baru tiap panggilan disassemble(),
// tidak ada handle yang perlu di-RAII, jadi header ini bebas include
// protobuf (stub/real sekalipun).

#include "Disassembler/HydraDisassembler.h"

namespace omnibyte::hydradis {

/// HydraDisassembler untuk stream berformat protobuf wire.
///
/// Karena tidak ada schema, decoder hanya me-walk tag + payload tiap field
/// (varint / fixed32 / fixed64 / length-delimited). Field length-delimited
/// di-rekursi sebagai sub-message sampai depth cap 10; payload yang ternyata
/// bukan message valid (mis. string/bytes) jatuh jadi satu field opaq.
///
/// Tidak ada state lintas-panggilan: aman untuk sharing satu instance.
class ProtobufAdapter final : public HydraDisassembler {
public:
    ProtobufAdapter() = default;

    ProtobufAdapter(const ProtobufAdapter&) = delete;
    ProtobufAdapter& operator=(const ProtobufAdapter&) = delete;

    std::string name() const override { return "protobuf"; }

    /// Arch-agnostic: stream AST tidak punya ISA, selalu None.
    DisassemblerArch arch() const override { return DisassemblerArch::None; }

    /// Walk wire format jadi list field. Tiap field leaf = satu Instruction;
    /// address = baseAddr + offset field di buffer, mnemonic = path field
    /// (mis. "f3.f7.varint"), opStr = nilai/payload-nya.
    ///
    /// Instruction::bytes sengaja dibiarkan kosong -- raw encoding tiap field
    /// tidak disimpan karena hanya informasi duplikat dari address+size.
    DisassemblyResult disassemble(
        const uint8_t* code,
        size_t codeSize,
        uint64_t baseAddr,
        size_t count
    ) const override;
};

/// Buat HydraDisassembler untuk stream protobuf.
/// `arch` diabaikan (stream arch-agnostic) -- dipertahankan supaya signature
/// seragam dengan createCapstoneDisassembler().
/// Selalu sukses: backend ini tidak bergantung pada arch target.
std::unique_ptr<HydraDisassembler> createProtobufDisassembler(DisassemblerArch arch);

} // namespace omnibyte::hydradis

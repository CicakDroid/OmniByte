#pragma once
// ── CapstoneAdapter.h ──────────────────────────────────────────────
// Public API backend Capstone.
//
// Class-nya ada di header ini (bukan disembunyikan di .cpp) supaya caller
// bisa construct langsung. Konsekuensinya header ini ikut menyertakan
// <capstone/capstone.h> karena `csh handle_` adalah member privat.

#include "Disassembler/HydraDisassembler.h"

#include <capstone/capstone.h>

namespace omnibyte::hydradis {

/// HydraDisassembler berbasis Capstone untuk satu arsitektur target.
///
/// Satu instance = satu arch + mode, dibuka lewat cs_open() di constructor
/// dan ditutup lewat cs_close() di destructor (RAII). Copy di-disable:
/// `csh` handle milik Capstone tidak boleh di-share dua instance.
class CapstoneAdapter final : public HydraDisassembler {
public:
    /// Buka handle Capstone untuk `arch`.
    /// Kalau arch tidak didukung Capstone / cs_open() gagal, instance tetap
    /// terbentuk tapi `valid()` bernilai false — panggilan selanjutnya ke
    /// disassemble() akan return errorMessage, bukan crash.
    explicit CapstoneAdapter(DisassemblerArch arch);

    ~CapstoneAdapter() override;

    CapstoneAdapter(const CapstoneAdapter&) = delete;
    CapstoneAdapter& operator=(const CapstoneAdapter&) = delete;

    /// True kalau cs_open() berhasil di constructor.
    bool valid() const { return valid_; }

    std::string name() const override { return "capstone"; }

    DisassemblerArch arch() const override { return arch_; }

    /// Decode buffer memakai cs_disasm_iter() — satu instruksi per iterasi,
    /// tanpa alokasi besar sekaligus (hemat memori untuk buffer chunk besar).
    DisassemblyResult disassemble(
        const uint8_t* code,
        size_t codeSize,
        uint64_t baseAddr,
        size_t count
    ) const override;

private:
    csh handle_ = 0;      // 0 = handle belum pernah dibuka
    bool valid_ = false;  // cs_open() sukses
    DisassemblerArch arch_;
};

/// Buat HydraDisassembler berbasis Capstone untuk `arch`.
/// Return nullptr kalau `arch` tidak didukung Capstone / cs_open() gagal.
std::unique_ptr<HydraDisassembler> createCapstoneDisassembler(DisassemblerArch arch);

} // namespace omnibyte::hydradis

#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <memory>
#include <unordered_map>

namespace omnibyte::dumper::symbolic {

enum class SolverResult {
    Sat,
    Unsat,
    Unknown   // timeout, atau solver tidak bisa memutuskan
};

// Hasil satu model/assignment kalau formula SAT.
// key = nama variable simbolik, value = nilai konkret (raw bytes) --
// sengaja solver-agnostic, caller yang interpretasi sesuai tipe aslinya
// (int, pointer, dst) supaya adapter Z3 dan CVC5 tidak perlu expose tipe
// internal masing-masing solver ke pemanggil.
struct SolverModel {
    std::unordered_map<std::string, std::vector<uint8_t>> assignments;
};

// Kontrak umum -- triton-adapter panggil lewat interface ini saja.
// Triton TIDAK tahu solver konkret apa yang jalan di baliknya (Z3 atau CVC5),
// jadi ganti solver = ganti implementasi ISolverBackend, tidak menyentuh
// logic SymbolicExecution/engine/triton-adapter sama sekali.
class ISolverBackend {
public:
    virtual ~ISolverBackend() = default;

    virtual std::string name() const = 0;

    // Hapus semua constraint yang sudah ditambahkan -- dipakai sebelum mulai
    // analisis path baru dari awal.
    virtual void reset() = 0;

    // Tambah satu constraint dalam format SMT-LIB2 string.
    // SMT-LIB2 dipilih sebagai lingua franca karena didukung penuh baik oleh
    // Z3 maupun CVC5 -- pemanggil (Triton) tidak perlu tahu API native tiap solver.
    virtual void addConstraint(const std::string& smtLib2Expr) = 0;

    // Cek satisfiability dari seluruh constraint yang sudah ditambahkan.
    // timeoutMs mencegah solver hang di path yang terlalu kompleks --
    // ambil dari config/Runtime, jangan hardcode di adapter.
    virtual SolverResult check(uint32_t timeoutMs = 5000) = 0;

    // Ambil model konkret setelah check() == Sat.
    // Return nullopt kalau belum dipanggil check() atau hasilnya bukan Sat --
    // caller (mis. Triton, saat generate test-case konkret dari path yang di-explore)
    // wajib cek ini sebelum pakai hasilnya.
    virtual std::optional<SolverModel> getModel() const = 0;

    // Push/pop scope constraint -- dipakai Triton saat fork ke percabangan path baru,
    // supaya bisa kembali ke state constraint sebelumnya tanpa reset total dan
    // tanpa perlu re-add semua constraint dari root path.
    virtual void push() = 0;
    virtual void pop() = 0;
};

// Factory sederhana -- baca config/Runtime (mis. field "symbolicSolverBackend":
// "z3" | "cvc5") untuk tentukan instance mana yang dibuat, tanpa mengubah kode
// pemanggil kalau user ganti pilihan solver.
class SolverBackendFactory {
public:
    enum class Backend { Z3, CVC5 };
    static std::unique_ptr<ISolverBackend> create(Backend backend);
};

} // namespace omnibyte::dumper::symbolic
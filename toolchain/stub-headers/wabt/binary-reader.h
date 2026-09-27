#pragma once
// ── wabt/binary-reader.h (stub) ────────────────────────────────────────
// Stub untuk syntax check lokal saja — lihat stub-headers/README.md.
// Isi: subset wabt 1.0.42 yang benar-benar dipakai WasmAdapter.cpp.
// Deklarasi TANPA definisi (stub tidak pernah di-link).
// Sumber resmi dicantumkan per blok.

#include <cstddef>
#include <cstdint>
#include <string>

// ── common.h (blok ini GLOBAL scope, di luar namespace wabt) ────────────
// wabt/common.h:107 — namespace wabt ditutup di baris 104/105, lalu
// `struct v128` dideklarasikan di global scope. Karena itu sebutan
// `wabt::v128` TIDAK ada; pemakaian di dalam namespace wabt memakai
// unqualified `v128` yang resolve ke `::v128`.
// Hanya u32(int) yang dipakai (OnOpcodeV128).
struct v128 {
    v128() = default;

    uint32_t u32(int lane) const;
};

namespace wabt {

// ── base-types.h ───────────────────────────────────────────────────────
using Index = uint32_t;
using Address = uint64_t;
using Offset = size_t;

// base-types.h: `using ByteSpan = std::span<const uint8_t>;` (C++20).
// Stub memakai struct biasa dengan konversi implisit (ptr, size) supaya
// header ini tetap C++17-clean untuk gate `-std=c++17`.
struct ByteSpan {
    ByteSpan() = default;
    ByteSpan(const uint8_t* ptr, size_t size) : ptr_(ptr), size_(size) {}

    const uint8_t* data() const { return ptr_; }
    size_t size() const { return size_; }

    const uint8_t* ptr_ = nullptr;
    size_t size_ = 0;
};

// ── result.h ───────────────────────────────────────────────────────────
// struct [[nodiscard]] Result { enum Enum { Ok, Error }; ... };
// Succeeded()/Failed() di result.h juga inline di header asli.
struct [[nodiscard]] Result {
    enum Enum { Ok, Error };

    Result() : enum_(Ok) {}
    Result(Enum e) : enum_(e) {}
    operator Enum() const { return enum_; }

    Enum enum_;
};

inline bool Succeeded(Result r) { return r == Result::Ok; }
inline bool Failed(Result r) { return r == Result::Error; }

// ── type.h ─────────────────────────────────────────────────────────────
// class Type; hanya subset yang dipakai oleh BlockSigToString + OnOpcodeType.
// Enum lengkap (I8/I16/I32/I64/F32/F64/V128/Func/Struct/Array/Ref/...
// FuncRef/ExternRef/ExnRef/...) tidak dideklarasikan di stub.
class Type {
public:
    enum Enum : int32_t {
        Void = -0x40,
    };

    constexpr Type() : enum_(Void) {}
    constexpr Type(Enum e) : enum_(e) {}
    constexpr operator Enum() const { return enum_; }

    std::string GetName() const;        // type.h — contoh: "i32", "void"
    const char* GetRefKindName() const; // type.h — contoh: "func"
    bool IsIndex() const;               // type.h — true kalau type index
    Index GetIndex() const;             // type.h — valid hanya kalau IsIndex()

    Enum enum_ = Void;
    Index type_index_ = 0;
};

// ── opcode.h ───────────────────────────────────────────────────────────
// struct Opcode; hanya GetName() + GetLength() yang dipakai (OnOpcode).
// enum Enum (ratusan opcode dari opcode.def) tidak dideklarasikan di stub.
struct Opcode {
    Opcode() = default;

    size_t GetLength() const;   // opcode.h — panjang byte opcode
    const char* GetName() const; // opcode.h — contoh: "i32.const"
};

// ── error.h ────────────────────────────────────────────────────────────
// class Error; hanya .message yang dibaca (OnError memindahkannya).
// error.h juga punya `Location loc` dan `std::string_view filename` —
// tidak dideklarasikan di stub (tidak dipakai adapter).
enum class ErrorLevel { Warning, Error };

class Error {
public:
    Error() = default;

    ErrorLevel error_level = ErrorLevel::Error;
    std::string message;
};

// ── feature.h ──────────────────────────────────────────────────────────
class Features {
public:
    Features() = default;

    void EnableAll();
};

// ── stream.h ───────────────────────────────────────────────────────────
class Stream;

// ── binary-reader.h: struct ReadBinaryOptions ──────────────────────────
// Constructor kenyamanan 5-arg ada di header asli; stub hanya menyediakan
// default ctor yang dipakai adapter.
struct ReadBinaryOptions {
    ReadBinaryOptions() = default;

    Features features;
    Stream* log_stream = nullptr;
    bool read_debug_names = false;
    bool stop_on_first_error = true;
    bool fail_on_custom_section_error = true;
    bool skip_function_bodies = false;
};

// ── binary-reader.h: class BinaryReaderDelegate ────────────────────────
// Delegate asli punya ~300 callback (custom section, elem/data/global,
// init expr, reloc, ...). Stub hanya memuat 17 virtual yang di-override
// WasmDelegate + OnError + OnSetState — cukup supaya WasmDelegate jadi
// class konkret.
class BinaryReaderDelegate {
public:
    // binary-reader.h: State — pointer `state` valid HANYA di dalam
    // callback; jangan disimpan lintas-panggilan.
    struct State {
        explicit State(ByteSpan data) : data(data), offset(0) {}

        ByteSpan data;
        Offset offset;
    };

    virtual ~BinaryReaderDelegate() {}

    virtual bool OnError(const Error&) = 0;
    virtual void OnSetState(const State* s) { state = s; }

    // Function body (dipakai untuk batas flush instruksi).
    virtual Result BeginFunctionBody(Index index, Offset size) = 0;
    virtual Result EndFunctionBody(Index index) = 0;

    // Instruksi + operand — OnOpcode dipanggil SEBELUM operand dibaca.
    virtual Result OnOpcode(Opcode Opcode) = 0;
    virtual Result OnOpcodeIndex(Index value) = 0;
    virtual Result OnOpcodeIndexIndex(Index value, Index value2) = 0;
    virtual Result OnOpcodeUint32(uint32_t value) = 0;
    virtual Result OnOpcodeUint32Uint32(uint32_t value, uint32_t value2) = 0;
    virtual Result OnOpcodeUint32Uint32Uint32(uint32_t value,
                                               uint32_t value2,
                                               uint32_t value3) = 0;
    virtual Result OnOpcodeUint32Uint32Uint32Uint32(uint32_t value,
                                                     uint32_t value2,
                                                     uint32_t value3,
                                                     uint32_t value4) = 0;
    virtual Result OnOpcodeUint64(uint64_t value) = 0;
    virtual Result OnOpcodeF32(uint32_t value) = 0;
    virtual Result OnOpcodeF64(uint64_t value) = 0;
    virtual Result OnOpcodeV128(v128 value) = 0;
    virtual Result OnOpcodeBlockSig(Type sig_type) = 0;
    virtual Result OnOpcodeType(Type type) = 0;
    virtual Result OnBrTableExpr(Index num_targets,
                                 Index* target_depths,
                                 Index default_target_depth) = 0;

    const State* state = nullptr;
};

// binary-reader.h:518 — deklarasi saja; stub tidak menyediakan definisi
// (syntax check tidak men-link).
Result ReadBinary(ByteSpan data,
                  BinaryReaderDelegate* reader,
                  const ReadBinaryOptions& options);

}  // namespace wabt

// STUB HEADER -- untuk g++ -fsyntax-only lokal SAJA.
//  BUKAN library asli. Build production pakai header dari
//  FetchContent (lihat CMakeLists.txt backend terkait).
//  Signature di sini mengikuti dokumentasi resmi versi 1.0 --
//  jangan percaya buta, verifikasi ulang saat library asli di-install.
//
// Sources:
//   - JonathanSalwan/Triton: src/libtriton/includes/triton/context.hpp (API is typedef for Context)
//   - JonathanSalwan/Triton: src/libtriton/includes/triton/instruction.hpp
//   - JonathanSalwan/Triton: src/libtriton/includes/triton/symbolicExpression.hpp
//   - JonathanSalwan/Triton: src/libtriton/includes/triton/ast.hpp
//   - https://github.com/JonathanSalwan/Triton/blob/master/src/libtriton/includes/triton/

#ifndef TRITON_API_STUB_H
#define TRITON_API_STUB_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <set>
#include <sstream>

#include <triton/archEnums.hpp>
#include <triton/dllexport.hpp>

namespace triton {

// ── Basic types (from tritonTypes.hpp) ──────────────────────────
typedef std::uint8_t  uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;
typedef std::size_t   usize;

// ── MODE enum ───────────────────────────────────────────────────
// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/context.hpp — class Context
// enum MODE { ALIGNED_MEMORY, OPTIMIZE粒度, ... }
// unverified signature, inferred from usage: triton::MODE::ALIGNED_MEMORY
enum MODE {
    ALIGNED_MEMORY = 0,
};

// ── AstNode (must be defined before SymbolicExpression uses it) ─
/// AST node.
/// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/ast.hpp
class AstNode {
public:
    virtual ~AstNode() = default;
    /// Convert the AST to SMT-LIB2 string.
    /// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/ast.hpp
    /// In real Triton: std::string convertToSmtLib2(void) const
    std::string toSMTLib2String() const;
};

// ── Forward declarations ────────────────────────────────────────
namespace arch {
    class Instruction;
}

namespace engines {
namespace symbolic {

/// SymbolicExpression.
/// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/symbolicExpression.hpp
class SymbolicExpression {
public:
    virtual ~SymbolicExpression() = default;
    /// Get the AST node of this symbolic expression.
    /// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/ast.hpp
    /// In real Triton, returns std::shared_ptr<AbstractNode>
    std::shared_ptr<triton::AstNode> getAst() const;
};

typedef std::shared_ptr<SymbolicExpression> SharedSymbolicExpression;

/// SymbolicEngine.
/// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/symbolicEngine.hpp
class SymbolicEngine {
public:
    virtual ~SymbolicEngine() = default;
    /// Get all symbolic expressions.
    /// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/symbolicEngine.hpp
    /// Returns map of id → SharedSymbolicExpression
    std::map<uint64, SharedSymbolicExpression> getSymbolicExpressions() const;
};

} // namespace symbolic
} // namespace engines

// ── Instruction ─────────────────────────────────────────────────
/// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/instruction.hpp
namespace arch {

class Instruction {
public:
    /// Default constructor.
    /// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/instruction.hpp
    Instruction();

    /// Constructor with address, opcode, and size.
    /// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/instruction.hpp
    /// Instruction(triton::uint64 addr, const void* opcode, triton::uint32 opSize)
    Instruction(uint64 addr, const void* opcode, uint32 opSize);

    /// Copy constructor.
    Instruction(const Instruction& other) = default;
    Instruction& operator=(const Instruction& other) = default;
    ~Instruction() = default;

    /// The semantics set of the instruction.
    /// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/instruction.hpp
    std::vector<engines::symbolic::SharedSymbolicExpression> symbolicExpressions;

    /// Returns the address.
    uint64 getAddress() const;

    /// Returns the size.
    uint32 getSize() const;
};

} // namespace arch

// ── API (Context) ───────────────────────────────────────────────
/// The main Triton API class.
/// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/context.hpp
/// In Triton, API is a typedef for Context<...>.
/// unverified signature, inferred from usage in symbolicexec_triton.cpp
class API {
public:
    API() = default;
    ~API() = default;

    /// Set the execution mode.
    /// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/context.hpp
    void setMode(MODE mode);

    /// Set the architecture.
    /// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/context.hpp
    void setArchitecture(arch::architecture_e arch);

    /// Process an instruction (disassemble + semantics).
    /// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/context.hpp
    void processing(arch::Instruction& inst);

    /// Get the symbolic engine.
    /// Source: JonathanSalwan/Triton/src/libtriton/includes/triton/context.hpp
    engines::symbolic::SymbolicEngine* getSymbolicEngine() const;
};

} // namespace triton

#endif // TRITON_API_STUB_H

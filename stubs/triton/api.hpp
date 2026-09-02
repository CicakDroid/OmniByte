#pragma once
// ── triton/api.hpp ───────────────────────────────────────────────────
// STUB HEADER for compile-checking only.
// Based on Triton library API: https://triton-library.github.io/documentation/doxygen/classtriton_1_1Context.html
// Types and methods used by symbolicexec_triton.cpp.

#include "archEnums.hpp"
#include <cstdint>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace triton {

namespace arch {

class OperandWrapper {
public:
    OperandWrapper() = default;
};

class Register {
public:
    Register() = default;
};

class MemoryAccess {
public:
    MemoryAccess() = default;
};

} // namespace arch

namespace ast {
class SharedAbstractNode {
public:
    virtual ~SharedAbstractNode() = default;
    std::string toSMTLib2String() const { return ""; }
};
using SharedAstNode = std::shared_ptr<SharedAbstractNode>;
} // namespace ast

namespace engines {
namespace symbolic {

class SharedSymbolicExpression {
public:
    SharedSymbolicExpression() = default;
    ast::SharedAstNode getAst() const { return std::make_shared<ast::SharedAbstractNode>(); }
    bool isSymbolized() const { return false; }
    bool isTainted = false;
};

using SharedSymbolicExpressionPtr = std::shared_ptr<SharedSymbolicExpression>;

class SharedSymbolicVariable {
public:
    SharedSymbolicVariable() = default;
};

class SymbolicEngine {
public:
    std::unordered_map<uint64_t, SharedSymbolicExpressionPtr> getSymbolicExpressions() const {
        return {};
    }
};

} // namespace symbolic

namespace solver {
class SolverEngine {
public:
};
} // namespace solver

} // namespace engines

namespace arch {

class Instruction {
public:
    Instruction() : address_(0), size_(0) {}
    Instruction(uint64_t addr, const void* opcode, uint32_t opSize)
        : address_(addr), size_(opSize) {}

    void setArchitecture(architecture_e arch) {}
    bool isTainted() const { return false; }
    bool isSymbolized() const { return false; }

    std::vector<engines::symbolic::SharedSymbolicExpressionPtr> symbolicExpressions;
    std::vector<OperandWrapper> operands;

private:
    uint64_t address_;
    uint32_t size_;
};

} // namespace arch

class API {
public:
    API() = default;

    void setMode(MODE mode) {}
    void setArchitecture(arch::architecture_e arch) {}
    arch::exception_e processing(arch::Instruction& inst) { return arch::NO_FAULT; }

    engines::symbolic::SymbolicEngine* getSymbolicEngine() { return &symbolicEngine_; }
    const engines::symbolic::SymbolicEngine* getSymbolicEngine() const { return &symbolicEngine_; }

private:
    engines::symbolic::SymbolicEngine symbolicEngine_;
};

} // namespace triton

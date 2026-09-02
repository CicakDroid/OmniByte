// STUB HEADER -- untuk g++ -fsyntax-only lokal SAJA.
//  BUKAN library asli. Build production pakai header dari
//  FetchContent (lihat CMakeLists.txt backend terkait).
//  Signature di sini mengikuti dokumentasi resmi versi 4.13.0 --
//  jangan percaya buta, verifikasi ulang saat library asli di-install.
//
// Sources:
//   - Z3Prover/z3: src/api/c++/z3++.h (Z3 C++ API wrapper)
//   - https://github.com/Z3Prover/z3/blob/master/src/api/c++/z3++.h

#ifndef Z3_STUB_H
#define Z3_STUB_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <iterator>

// ── Forward declarations ─────────────────────────────────────────
// These allow the stub to compile without pulling in the full Z3 API.

namespace z3 {

// ── check_result enum ────────────────────────────────────────────
// Source: Z3Prover/z3/src/api/z3_types.h → Z3_lbool / z3++.h enum check_result
enum check_result {
    unsat   = -1,  // Z3_L_FALSE  = -1
    unknown = 0,   // Z3_L_UNDEF  =  0
    sat     = 1    // Z3_L_TRUE   =  1
};

// ── Forward declarations ─────────────────────────────────────────
class context;
class solver;
class expr;
class expr_vector;
class model;
class func_decl;
class sort;
class ast;

// ── context ──────────────────────────────────────────────────────
// Source: Z3Prover/z3/src/api/c++/z3++.h — class context
// Constructor: context();  context(config const& c);
// Method: expr_vector parse_string(char const* s);
class context {
public:
    context() = default;
    context(const context&) = default;
    context(context&&) = default;
    context& operator=(const context&) = default;
    context& operator=(context&&) = default;
    ~context() = default;

    /// Parse a string as an SMT-LIB2 set of assertions.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_parse_string
    expr_vector parse_string(const char* s);
};

// ── expr ─────────────────────────────────────────────────────────
// Source: Z3Prover/z3/src/api/c++/z3++.h — class expr
class expr {
public:
    expr() = default;
    expr(const expr&) = default;
    expr(expr&&) = default;
    expr& operator=(const expr&) = default;
    expr& operator=(expr&&) = default;
    ~expr() = default;

    /// Check if this expression is a numeral constant.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_is_numeral
    bool is_numeral() const;

    /// Get the numeral value as a uint64.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_get_numeral_uint64
    uint64_t get_numeral_uint64() const;
};

// ── func_decl ────────────────────────────────────────────────────
// Source: Z3Prover/z3/src/api/c++/z3++.h — class func_decl
class func_decl {
public:
    func_decl() = default;
    func_decl(const func_decl&) = default;
    func_decl(func_decl&&) = default;
    func_decl& operator=(const func_decl&) = default;
    func_decl& operator=(func_decl&&) = default;
    ~func_decl() = default;

    /// Get the name of this function declaration.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_get_decl_name
    /// Returns a Z3_symbol whose str() gives the name string.
    struct symbol {
        std::string str_val;
        const char* str() const { return str_val.c_str(); }
    };

    /// Get the name of this function declaration as a symbol.
    /// In the real Z3 API this returns z3::symbol, but for the stub
    /// we inline a minimal compatible type.
    symbol name() const;
};

// ── expr_vector ──────────────────────────────────────────────────
// Source: Z3Prover/z3/src/api/c++/z3++.h — class expr_vector
class expr_vector {
public:
    expr_vector() = default;
    expr_vector(const expr_vector&) = default;
    expr_vector(expr_vector&&) = default;
    expr_vector& operator=(const expr_vector&) = default;
    expr_vector& operator=(expr_vector&&) = default;
    ~expr_vector() = default;

    /// Begin iterator for range-based for loops.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_ast_vector_translate + iterator
    std::vector<expr>::iterator begin();
    std::vector<expr>::iterator end();
    std::vector<expr>::const_iterator begin() const;
    std::vector<expr>::const_iterator end() const;

private:
    std::vector<expr> data_;
};

// ── model ────────────────────────────────────────────────────────
// Source: Z3Prover/z3/src/api/c++/z3++.h — class model
class model {
public:
    model() = default;
    model(const model&) = default;
    model(model&&) = default;
    model& operator=(const model&) = default;
    model& operator=(model&&) = default;
    ~model() = default;

    /// Number of constants/functions in the model.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_model_get_num_consts
    unsigned size() const;

    /// Get the i-th function declaration in the model.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_model_get_const_decl
    func_decl operator[](unsigned i) const;

    /// Get the interpretation of a constant in the model.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_model_get_const_interp
    expr get_const_interp(const func_decl& f) const;
};

// ── solver ───────────────────────────────────────────────────────
// Source: Z3Prover/z3/src/api/c++/z3++.h — class solver
class solver {
public:
    /// Constructor from context.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_mk_solver
    solver(context& c);

    solver(const solver&) = default;
    solver(solver&&) = default;
    solver& operator=(const solver&) = default;
    solver& operator=(solver&&) = default;
    ~solver() = default;

    /// Set a solver parameter.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_solver_set_params
    void set(const char* param, unsigned value);
    void set(const char* param, uint64_t value);

    /// Add a formula as a constraint.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_solver_assert
    void add(const expr& e);

    /// Check satisfiability.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_solver_check
    check_result check();

    /// Get the model after a satisfiable check.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_solver_get_model
    model get_model();

    /// Push a scope.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_solver_push
    void push();

    /// Pop a scope.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_solver_pop
    void pop();

    /// Reset all solver state.
    /// Source: Z3Prover/z3/src/api/c++/z3++.h — Z3_solver_reset
    void reset();
};

} // namespace z3

#endif // Z3_STUB_H

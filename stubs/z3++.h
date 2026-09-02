#pragma once
// ── z3++.h ───────────────────────────────────────────────────────────
// STUB HEADER for compile-checking only.
// Based on Z3 4.x C++ API: https://z3prover.github.io/api/html/
// Types and methods used by solvers_z3.cpp.

#include <cstdint>
#include <string>
#include <vector>

namespace z3 {

enum check_result { unsat = -1, unknown = 0, sat = 1 };

// Forward declaration
class expr_vector;

class context {
public:
    context() = default;

    expr_vector parse_string(const char* s);
};

class expr {
public:
    expr() = default;

    bool is_numeral() const { return false; }
    uint64_t get_numeral_uint64() const { return 0; }
    std::string str() const { return ""; }

    expr operator==(const expr& o) const { return {}; }
    expr operator!=(const expr& o) const { return {}; }
};

class expr_vector {
public:
    expr_vector() = default;
    explicit expr_vector(context& c) {}

    using iterator = std::vector<expr>::iterator;
    using const_iterator = std::vector<expr>::const_iterator;
    iterator begin() { return exprs_.begin(); }
    iterator end() { return exprs_.end(); }
    const_iterator begin() const { return exprs_.begin(); }
    const_iterator end() const { return exprs_.end(); }
    size_t size() const { return exprs_.size(); }

private:
    std::vector<expr> exprs_;
};

class func_decl {
public:
    func_decl() = default;

    struct name_proxy {
        std::string str() const { return ""; }
    };
    name_proxy name() const { return {}; }

    unsigned arity() const { return 0; }
};

class model {
public:
    model() = default;

    unsigned size() const { return 0; }
    func_decl operator[](unsigned i) const { return {}; }
    expr get_const_interp(const func_decl& d) const { return {}; }
    expr eval(const expr& e) const { return {}; }
};

class solver {
public:
    explicit solver(context& c) : ctx_(c) {}

    void set(const char* key, unsigned val) {}
    void add(const expr& e) {}
    check_result check() { return unknown; }
    model get_model() { return model(); }
    void reset() {}
    void push() {}
    void pop() {}

    expr_vector parse_string(const char* s) { return expr_vector(ctx_); }

private:
    context& ctx_;
};

// context::parse_string out-of-line definition (needs expr_vector complete)
inline expr_vector context::parse_string(const char* s) { return expr_vector(*this); }

} // namespace z3

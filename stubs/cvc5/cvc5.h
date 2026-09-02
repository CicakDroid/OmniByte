#pragma once
// ── cvc5/cvc5.h ──────────────────────────────────────────────────────
// STUB HEADER for compile-checking only.
// Based on cvc5 1.1.x API: https://cvc5.github.io/docs/cvc5-1.1.1/api/cpp/
// Types and methods used by solvers_cvc5.cpp.

#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

namespace cvc5 {

class CVC5ApiException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class TermManager {
public:
    TermManager() = default;
};

class Term {
public:
    Term() = default;

    size_t getNumChildren() const { return 0; }
    std::string getSymbol() const { return ""; }
    std::string toString() const { return ""; }

    using const_iterator = std::vector<Term>::const_iterator;
    const_iterator begin() const { return children_.begin(); }
    const_iterator end() const { return children_.end(); }

private:
    std::vector<Term> children_;
};

class Result {
public:
    Result() = default;
    bool isSat() const { return false; }
    bool isUnsat() const { return false; }
};

class Solver {
public:
    explicit Solver(TermManager& tm) : tm_(tm) {}

    void setLogic(const std::string& logic) {}
    void setOption(const std::string& opt, const std::string& val) {}
    void setTimeLimit(uint64_t ms) {}

    void resetAssertions() {}
    void assertFormula(const Term& t) {}
    Result checkSat() { return Result(); }

    std::vector<Term> getInputFormula(const std::string& smtLib2) { return {}; }
    std::vector<Term> getAssertions() { return {}; }
    Term getValue(const Term& t) { return Term(); }

    void push() {}
    void pop() {}

private:
    TermManager& tm_;
};

} // namespace cvc5

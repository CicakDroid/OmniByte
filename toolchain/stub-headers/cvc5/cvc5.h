// STUB HEADER -- untuk g++ -fsyntax-only lokal SAJA.
//  BUKAN library asli. Build production pakai header dari
//  FetchContent (lihat CMakeLists.txt backend terkait).
//  Signature di sini mengikuti dokumentasi resmi versi 1.2.0 --
//  jangan percaya buta, verifikasi ulang saat library asli di-install.
//
// Sources:
//   - cvc5/cvc5: include/cvc5/cvc5.h (CVC5 C++ API)
//   - https://github.com/cvc5/cvc5/blob/main/include/cvc5/cvc5.h

#ifndef CVC5_STUB_H
#define CVC5_STUB_H

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace cvc5 {

// ── Forward declarations ─────────────────────────────────────────
class TermManager;
class Solver;
class Term;
class Result;
class Sort;
class Op;

// ── CVC5ApiException ─────────────────────────────────────────────
// Source: cvc5/cvc5/include/cvc5/cvc5.h — class CVC5ApiException
class CVC5ApiException : public std::exception {
public:
    CVC5ApiException(const std::string& str) : d_msg(str) {}
    CVC5ApiException(const std::stringstream& stream) : d_msg(stream.str()) {}
    const std::string& getMessage() const { return d_msg; }
    const char* what() const noexcept override { return d_msg.c_str(); }
private:
    std::string d_msg;
};

// ── Result ───────────────────────────────────────────────────────
// Source: cvc5/cvc5/include/cvc5/cvc5.h — class Result
class Result {
public:
    Result() = default;
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Result::isSat()
    bool isSat() const;
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Result::isUnsat()
    bool isUnsat() const;
    bool isUnknown() const;
    bool isNull() const;
    std::string toString() const;
};

// ── Term ─────────────────────────────────────────────────────────
// Source: cvc5/cvc5/include/cvc5/cvc5.h — class Term
class Term {
public:
    Term() = default;
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Term::getNumChildren()
    size_t getNumChildren() const;
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Term::getSymbol()
    std::string getSymbol() const;
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Term::toString()
    std::string toString() const;
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Term::operator[]
    Term operator[](size_t index) const;
    bool isNull() const;
    uint64_t getId() const;
    // Iterator support for range-based for over children
    // unverified signature, inferred from usage in solvers_cvc5.cpp
    using iterator = Term*;  // simplified; real impl uses internal iterator
    iterator begin() const;
    iterator end() const;
};

// ── TermManager ──────────────────────────────────────────────────
// Source: cvc5/cvc5/include/cvc5/cvc5.h — class TermManager
class TermManager {
public:
    TermManager() = default;
    ~TermManager() = default;
};

// ── Solver ───────────────────────────────────────────────────────
// Source: cvc5/cvc5/include/cvc5/cvc5.h — class Solver
class Solver {
public:
    /// Constructor from TermManager.
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Solver(TermManager&)
    Solver(TermManager& tm);

    Solver(const Solver&) = default;
    Solver(Solver&&) = default;
    Solver& operator=(const Solver&) = default;
    Solver& operator=(Solver&&) = default;
    ~Solver() = default;

    /// Set the logic.
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Solver::setLogic()
    void setLogic(const std::string& logic);

    /// Set an option.
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Solver::setOption()
    void setOption(const std::string& option, const std::string& value);

    /// Reset all assertions.
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Solver::resetAssertions()
    void resetAssertions();

    /// Parse an SMT-LIB2 string and return the asserted terms.
    /// unverified signature, inferred from usage in solvers_cvc5.cpp
    /// (no direct equivalent in public cvc5 API; may use internal parser)
    std::vector<Term> getInputFormula(const std::string& smtLib2Str);

    /// Assert a formula.
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Solver::assertFormula()
    void assertFormula(const Term& t);

    /// Set a time limit in milliseconds.
    /// unverified signature, inferred from usage in solvers_cvc5.cpp
    /// (real cvc5 may use setOption("tlimit", ...) instead)
    void setTimeLimit(uint64_t limit);

    /// Check satisfiability.
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Solver::checkSat()
    Result checkSat();

    /// Get all current assertions.
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Solver::getAssertions()
    std::vector<Term> getAssertions();

    /// Get the value of a term in the current model.
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Solver::getValue()
    Term getValue(const Term& t);

    /// Push a scope.
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Solver::push()
    void push();

    /// Pop a scope.
    /// Source: cvc5/cvc5/include/cvc5/cvc5.h — Solver::pop()
    void pop();
};

} // namespace cvc5

#endif // CVC5_STUB_H

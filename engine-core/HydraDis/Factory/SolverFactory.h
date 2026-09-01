#pragma once
// ── SolverFactory.h ───────────────────────────────────────────────
// Factory wrapper untuk ISolverBackend.
// Delegates ke SolverBackendFactory yang sudah ada di ISolverBackend.h,
// tapi menambah convenience methods dan layer abstraksi untuk orchestrator.

#include "Plugin/Enhanced/SymbolicExecution/ISolverBackend.h"
#include <memory>
#include <string>

namespace omnibyte::hydradis {

/// Factory untuk ISolverBackend (Z3/CVC5).
///
/// Usage:
///   auto solver = SolverFactory::create(SolverBackend::Z3);
///   solver->addConstraint("(assert (= x #x10))");
///   auto result = solver->check(5000);
class SolverFactory {
public:
    using Backend = omnibyte::dumper::symbolic::SolverBackendFactory::Backend;

    /// Buat ISolverBackend instance.
    ///
    /// @param backend  Z3 atau CVC5 (default: Z3)
    /// @return unique_ptr ke ISolverBackend
    static std::unique_ptr<omnibyte::dumper::symbolic::ISolverBackend> create(
        Backend backend = Backend::Z3
    );

    /// Convenience: create dari string name ("z3" atau "cvc5").
    /// Return nullptr kalau nama tidak dikenal.
    static std::unique_ptr<omnibyte::dumper::symbolic::ISolverBackend> createByName(
        const std::string& name
    );
};

} // namespace omnibyte::hydradis

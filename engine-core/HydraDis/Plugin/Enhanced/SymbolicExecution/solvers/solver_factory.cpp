#include "Plugin/Enhanced/SymbolicExecution/ISolverBackend.h"
#include <memory>
#include <stdexcept>

// Forward declarations — each adapter has a class defined in its own .cpp file.
// We include them here to construct both Z3 and CVC5 solvers from one factory.
namespace omnibyte::dumper::symbolic {

// Defined in z3-adapter/solvers_z3.cpp
std::unique_ptr<ISolverBackend> createZ3Solver();

// Defined in cvc5-adapter/solvers_cvc5.cpp
std::unique_ptr<ISolverBackend> createCVC5Solver();

} // namespace omnibyte::dumper::symbolic

namespace omnibyte::dumper::symbolic {

std::unique_ptr<ISolverBackend> SolverBackendFactory::create(Backend backend) {
    switch (backend) {
        case Backend::Z3:
            return createZ3Solver();
        case Backend::CVC5:
            return createCVC5Solver();
    }
    return nullptr;
}

} // namespace omnibyte::dumper::symbolic

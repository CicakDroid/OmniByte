#include "Factory/SolverFactory.h"

namespace omnibyte::hydradis {

std::unique_ptr<omnibyte::dumper::symbolic::ISolverBackend> SolverFactory::create(
    Backend backend
) {
    return omnibyte::dumper::symbolic::SolverBackendFactory::create(backend);
}

std::unique_ptr<omnibyte::dumper::symbolic::ISolverBackend> SolverFactory::createByName(
    const std::string& name
) {
    if (name == "z3" || name == "Z3") {
        return create(Backend::Z3);
    }
    if (name == "cvc5" || name == "CVC5") {
        return create(Backend::CVC5);
    }
    return nullptr;
}

} // namespace omnibyte::hydradis

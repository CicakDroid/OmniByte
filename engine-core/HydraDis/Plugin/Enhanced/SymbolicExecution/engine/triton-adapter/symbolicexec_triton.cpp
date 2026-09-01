#include "Plugin/Enhanced/SymbolicExecution/ISolverBackend.h"
#include <triton/api.hpp>
#include <triton/x8664Specifications.hpp>
#include <triton/archEnums.hpp>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::symbolic {

class TritonSymbolicExec {
public:
    TritonSymbolicExec() {
        triton_.setMode(triton::MODE::ALIGNED_MEMORY);
    }

    void setArchitecture(triton::arch::architecture_e arch) {
        triton_.setArchitecture(arch);
    }

    bool liftInstruction(const uint8_t* bytes, size_t size, uint64_t address) {
        triton::Context::Instruction inst(address, bytes, size);
        triton_.processing(inst);
        return inst.isTainted() || true;
    }

    std::string getSymbolicExpression(size_t index) const {
        auto expr = triton_.getSymbolicEngine()->getSymbolicExpressions()[index];
        return expr->getAst()->toSMTLib2String();
    }

    std::string getFullSMTLib2() const {
        std::string smt;
        auto exprs = triton_.getSymbolicEngine()->getSymbolicExpressions();
        for (const auto& [id, expr] : exprs) {
            smt += expr->getAst()->toSMTLib2String() + "\n";
        }
        return smt;
    }

    triton::API& api() { return triton_; }
    const triton::API& api() const { return triton_; }

private:
    triton::API triton_;
};

} // namespace omnibyte::dumper::symbolic

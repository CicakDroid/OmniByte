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
    TritonSymbolicExec() : solver_(nullptr) {
        triton_.setMode(triton::MODE::ALIGNED_MEMORY);
    }

    explicit TritonSymbolicExec(std::unique_ptr<ISolverBackend> solver)
        : solver_(std::move(solver)) {
        triton_.setMode(triton::MODE::ALIGNED_MEMORY);
    }

    void setArchitecture(triton::arch::architecture_e arch) {
        triton_.setArchitecture(arch);
    }

    void setSolver(std::unique_ptr<ISolverBackend> solver) {
        solver_ = std::move(solver);
    }

    // Process an instruction and return true if any symbolic expression was generated
    bool liftInstruction(const uint8_t* bytes, size_t size, uint64_t address) {
        // Correct namespace: triton::arch::Instruction (not triton::Context::Instruction)
        triton::arch::Instruction inst(address, bytes, size);
        triton_.processing(inst);
        // Return whether any symbolic expressions were created for this instruction
        return !inst.symbolicExpressions.empty();
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

    // Send all symbolic expressions to the solver, check satisfiability,
    // and return the model if SAT.
    std::optional<SolverModel> solveWithSolver(uint32_t timeoutMs = 5000) {
        if (!solver_) {
            return std::nullopt;
        }

        // Reset solver state before new query
        solver_->reset();

        // Add each symbolic expression as a constraint
        auto exprs = triton_.getSymbolicEngine()->getSymbolicExpressions();
        for (const auto& [id, expr] : exprs) {
            std::string smtLib2 = expr->getAst()->toSMTLib2String();
            if (!smtLib2.empty()) {
                solver_->addConstraint(smtLib2);
            }
        }

        // Check satisfiability
        SolverResult checkResult = solver_->check(timeoutMs);
        if (checkResult != SolverResult::Sat) {
            return std::nullopt;
        }

        // Get model from solver
        return solver_->getModel();
    }

    triton::API& api() { return triton_; }
    const triton::API& api() const { return triton_; }

private:
    triton::API triton_;
    std::unique_ptr<ISolverBackend> solver_;
};

} // namespace omnibyte::dumper::symbolic

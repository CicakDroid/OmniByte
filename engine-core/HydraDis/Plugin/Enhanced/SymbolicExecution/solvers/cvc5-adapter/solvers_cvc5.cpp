#include "Plugin/Enhanced/SymbolicExecution/ISolverBackend.h"
#include <cvc5/cvc5.h>
#include <memory>
#include <string>
#include <vector>

namespace omnibyte::dumper::symbolic {

class CVC5Solver final : public ISolverBackend {
public:
    CVC5Solver() : tm_(), solver_(tm_) {
        solver_.setLogic("QF_BV");
        solver_.setOption("produce-models", "true");
    }

    std::string name() const override {
        return "cvc5";
    }

    void reset() override {
        solver_.resetAssertions();
        lastCheckWasSat_ = false;
    }

    void addConstraint(const std::string& smtLib2Expr) override {
        try {
            std::vector<cvc5::Term> formulas = solver_.getInputFormula(smtLib2Expr);
            for (const auto& f : formulas) {
                solver_.assertFormula(f);
            }
        } catch (const cvc5::CVC5ApiException& e) {
            throw std::runtime_error(std::string("CVC5 parse error: ") + e.what());
        }
    }

    SolverResult check(uint32_t timeoutMs) override {
        solver_.setTimeLimit(static_cast<uint64_t>(timeoutMs));
        cvc5::Result result = solver_.checkSat();
        if (result.isSat()) {
            lastCheckWasSat_ = true;
            return SolverResult::Sat;
        }
        lastCheckWasSat_ = false;
        if (result.isUnsat()) return SolverResult::Unsat;
        return SolverResult::Unknown;
    }

    std::optional<SolverModel> getModel() const override {
        if (!lastCheckWasSat_) {
            return std::nullopt;
        }

        SolverModel model;

        // Get all asserted formulas and iterate to find variable terms (arity 0)
        std::vector<cvc5::Term> assertions = solver_.getAssertions();
        for (const cvc5::Term& assertion : assertions) {
            collectVariables(assertion, model);
        }

        return model;
    }

    void push() override {
        solver_.push();
    }

    void pop() override {
        solver_.pop();
    }

private:
    void collectVariables(const cvc5::Term& term, SolverModel& model) const {
        // A variable/constant has no children (arity 0)
        if (term.getNumChildren() == 0) {
            try {
                cvc5::Term value = solver_.getValue(term);
                std::string varName = term.getSymbol().c_str();
                std::string valStr = value.toString();
                model.assignments[varName] = std::vector<uint8_t>(
                    valStr.begin(), valStr.end()
                );
            } catch (const cvc5::CVC5ApiException&) {
                // Skip terms that cannot be evaluated
            }
            return;
        }
        // Recurse into children
        for (const cvc5::Term& child : term) {
            collectVariables(child, model);
        }
    }

    cvc5::TermManager tm_;
    mutable cvc5::Solver solver_;
    bool lastCheckWasSat_ = false;
};

std::unique_ptr<ISolverBackend> createCVC5Solver() {
    return std::make_unique<CVC5Solver>();
}

} // namespace omnibyte::dumper::symbolic

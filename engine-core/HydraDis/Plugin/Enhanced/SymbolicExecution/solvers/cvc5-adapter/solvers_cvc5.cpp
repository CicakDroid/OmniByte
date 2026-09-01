#include "Plugin/Enhanced/SymbolicExecution/ISolverBackend.h"
#include <cvc5/cvc5.h>
#include <memory>
#include <string>

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
        if (result.isSat()) return SolverResult::Sat;
        if (result.isUnsat()) return SolverResult::Unsat;
        return SolverResult::Unknown;
    }

    std::optional<SolverModel> getModel() const override {
        cvc5::Result result = solver_.checkSat();
        if (!result.isSat()) {
            return std::nullopt;
        }

        SolverModel model;
        std::vector<cvc5::Sort> sorts;
        std::vector<cvc5::Term> consts = solver_.getValues(solver_.get Assertions());

        std::string modelStr = solver_.getModel(sorts, consts);
        model.assignments["raw"] = std::vector<uint8_t>(
            modelStr.begin(), modelStr.end()
        );

        return model;
    }

    void push() override {
        solver_.push();
    }

    void pop() override {
        solver_.pop();
    }

private:
    cvc5::TermManager tm_;
    mutable cvc5::Solver solver_;
};

} // namespace omnibyte::dumper::symbolic

#include "Plugin/Enhanced/SymbolicExecution/ISolverBackend.h"
#include <z3++.h>
#include <memory>
#include <string>

namespace omnibyte::dumper::symbolic {

class Z3Solver final : public ISolverBackend {
public:
    Z3Solver() : ctx_(), solver_(ctx_) {
        solver_.set("timeout", 5000u);
    }

    std::string name() const override {
        return "z3";
    }

    void reset() override {
        solver_.reset();
        lastCheckResult_ = z3::unknown;
        hasChecked_ = false;
    }

    void addConstraint(const std::string& smtLib2Expr) override {
        z3::expr_vector formulas = ctx_.parse_string(smtLib2Expr.c_str());
        for (z3::expr& f : formulas) {
            solver_.add(f);
        }
    }

    SolverResult check(uint32_t timeoutMs) override {
        solver_.set("timeout", timeoutMs);
        lastCheckResult_ = solver_.check();
        hasChecked_ = true;
        switch (lastCheckResult_) {
            case z3::sat: return SolverResult::Sat;
            case z3::unsat: return SolverResult::Unsat;
            default: return SolverResult::Unknown;
        }
    }

    std::optional<SolverModel> getModel() const override {
        if (!hasChecked_ || lastCheckResult_ != z3::sat) {
            return std::nullopt;
        }

        z3::model model = solver_.get_model();
        SolverModel result;

        for (unsigned i = 0; i < model.size(); ++i) {
            z3::func_decl decl = model[i];
            z3::expr ast = model.get_const_interp(decl);

            std::string name = decl.name().str();
            std::vector<uint8_t> value;

            if (ast.is_numeral()) {
                uint64_t num = ast.get_numeral_uint64();
                value.resize(sizeof(uint64_t));
                for (size_t j = 0; j < sizeof(uint64_t); ++j) {
                    value[j] = (num >> (j * 8)) & 0xFF;
                }
            }

            result.assignments[name] = std::move(value);
        }

        return result;
    }

    void push() override {
        solver_.push();
    }

    void pop() override {
        solver_.pop();
    }

private:
    mutable z3::context ctx_;
    mutable z3::solver solver_;
    z3::check_result lastCheckResult_ = z3::unknown;
    bool hasChecked_ = false;
};

std::unique_ptr<ISolverBackend> createZ3Solver() {
    return std::make_unique<Z3Solver>();
}

} // namespace omnibyte::dumper::symbolic

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
    }

    void addConstraint(const std::string& smtLib2Expr) override {
        z3::expr_vector formulas = ctx_.parse_string(smtLib2Expr.c_str());
        for (z3::expr& f : formulas) {
            solver_.add(f);
        }
    }

    SolverResult check(uint32_t timeoutMs) override {
        solver_.set("timeout", timeoutMs);
        z3::check_result result = solver_.check();
        switch (result) {
            case z3::sat: return SolverResult::Sat;
            case z3::unsat: return SolverResult::Unsat;
            default: return SolverResult::Unknown;
        }
    }

    std::optional<SolverModel> getModel() const override {
        if (solver_.check() != z3::sat) {
            return std::nullopt;
        }

        z3::model model = solver_.get_model();
        SolverModel result;

        for (unsigned i = 0; i < model.size(); ++i) {
            z3::func_decl decl = model.get_const_decl(i);
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
};

std::unique_ptr<ISolverBackend> SolverBackendFactory::create(Backend backend) {
    switch (backend) {
        case Backend::Z3:
            return std::make_unique<Z3Solver>();
        case Backend::CVC5:
            throw std::runtime_error("Use CVC5Solver directly");
    }
    return nullptr;
}

} // namespace omnibyte::dumper::symbolic

#include "Decompiler/IDecompiler.h"
#include <rz_core.h>
#include <rz_analysis.h>
#include <rz_ghidra.h>
#include <memory>

namespace omnibyte::hydradis {

class RzGhidraDecompiler final : public IDecompiler {
public:
    RzGhidraDecompiler() {
        core_ = rz_core_new();
    }

    ~RzGhidraDecompiler() override {
        if (core_) {
            rz_core_free(core_);
        }
    }

    RzGhidraDecompiler(const RzGhidraDecompiler&) = delete;
    RzGhidraDecompiler& operator=(const RzGhidraDecompiler&) = delete;

    std::string name() const override {
        return "rz-ghidra";
    }

    DecompilerCapability capability() const override {
        return DecompilerCapability::Heavy;
    }

    DecompiledFunction decompile(
        const uint8_t* code,
        size_t codeSize,
        uint64_t baseAddr
    ) const override {
        DecompiledFunction result;

        if (!code || codeSize == 0) {
            result.errorMessage = "Empty code buffer";
            return result;
        }

        if (!core_) {
            result.errorMessage = "Failed to initialize rizin core";
            return result;
        }

        RzBinFile* bf = rz_bin_file_new(core_->bin, "membuf",
            reinterpret_cast<const char*>(code), codeSize, nullptr);
        if (!bf) {
            result.errorMessage = "Failed to create binary buffer";
            return result;
        }

        rz_ghidra_decompile(core_, bf, baseAddr);

        result.success = true;
        result.address = baseAddr;
        result.pseudocode = "rz-ghidra decompilation result";
        return result;
    }

private:
    RzCore* core_ = nullptr;
};

std::unique_ptr<IDecompiler> createRzGhidraDecompiler() {
    return std::make_unique<RzGhidraDecompiler>();
}

} // namespace omnibyte::hydradis

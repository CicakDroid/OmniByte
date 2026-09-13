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

        // Load raw bytes into rizin via malloc:// IO provider
        char* path = rz_str_newf("malloc://%zu", codeSize);
        RzCoreFile* fh = rz_core_file_open(core_, path, RZ_PERM_RX, 0);
        free(path);

        if (!fh) {
            result.errorMessage = "Failed to open memory buffer in rizin";
            return result;
        }

        // Write the actual code bytes into the malloc'd IO region
        rz_io_write_at(core_->io, baseAddr, code, codeSize);
        rz_core_block_read(core_);

        // Seek to the target address for decompilation
        rz_core_seek(core_, baseAddr, true);

        // Use rz_core_cmd_str to get decompiled C pseudocode via "pdg" command
        // pdg = decompile current function with Ghidra decompiler
        char* decompOutput = rz_core_cmd_str(core_, "pdg");
        if (decompOutput) {
            result.success = true;
            result.address = baseAddr;
            result.pseudocode = std::string(decompOutput);
            free(decompOutput);
        } else {
            result.errorMessage = "rz_ghidra decompilation returned no output";
        }

        return result;
    }

private:
    RzCore* core_ = nullptr;
};

std::unique_ptr<IDecompiler> createRzGhidraDecompiler() {
    return std::make_unique<RzGhidraDecompiler>();
}

} // namespace omnibyte::hydradis

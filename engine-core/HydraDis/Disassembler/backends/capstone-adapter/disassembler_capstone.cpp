#include "Disassembler/IDisassembler.h"
#include <capstone/capstone.h>
#include <stdexcept>

namespace omnibyte::hydradis {

class CapstoneDisassembler final : public IDisassembler {
public:
    CapstoneDisassembler(DisassemblerArch arch) : arch_(arch) {
        cs_arch csArch;
        cs_mode csMode;

        switch (arch) {
            case DisassemblerArch::ARM:
                csArch = CS_ARCH_ARM;
                csMode = CS_MODE_ARM;
                break;
            case DisassemblerArch::ARM_Thumb:
                csArch = CS_ARCH_ARM;
                csMode = CS_MODE_THUMB;
                break;
            case DisassemblerArch::ARM64:
                csArch = CS_ARCH_ARM64;
                csMode = CS_MODE_ARM;
                break;
            case DisassemblerArch::x86:
                csArch = CS_ARCH_X86;
                csMode = CS_MODE_32;
                break;
            case DisassemblerArch::x86_64:
                csArch = CS_ARCH_X86;
                csMode = CS_MODE_64;
                break;
            case DisassemblerArch::MIPS:
                csArch = CS_ARCH_MIPS;
                csMode = CS_MODE_MIPS32;
                break;
            case DisassemblerArch::PPC:
                csArch = CS_ARCH_PPC;
                csMode = CS_MODE_32;
                break;
            case DisassemblerArch::SPARC:
                csArch = CS_ARCH_SPARC;
                csMode = CS_MODE_V9;
                break;
            case DisassemblerArch::SystemZ:
                csArch = CS_ARCH_SYSZ;
                csMode = CS_MODE_LITTLE_ENDIAN;
                break;
            case DisassemblerArch::XCore:
                csArch = CS_ARCH_XCORE;
                csMode = CS_MODE_LITTLE_ENDIAN;
                break;
            case DisassemblerArch::M68K:
                csArch = CS_ARCH_M68K;
                csMode = CS_MODE_BIG_ENDIAN;
                break;
            case DisassemblerArch::TMS320C64X:
                csArch = CS_ARCH_TMS320C64X;
                csMode = CS_MODE_LITTLE_ENDIAN;
                break;
            case DisassemblerArch::M680X:
                csArch = CS_ARCH_M680X;
                csMode = CS_MODE_LITTLE_ENDIAN;
                break;
            case DisassemblerArch::EVM:
                csArch = CS_ARCH_EVM;
                csMode = CS_MODE_LITTLE_ENDIAN;
                break;
            default:
                throw std::runtime_error("Unsupported architecture");
        }

        cs_err err = cs_open(csArch, csMode, &handle_);
        if (err != CS_ERR_OK) {
            throw std::runtime_error(std::string("cs_open failed: ") + cs_strerror(err));
        }
    }

    ~CapstoneDisassembler() override {
        cs_close(&handle_);
    }

    CapstoneDisassembler(const CapstoneDisassembler&) = delete;
    CapstoneDisassembler& operator=(const CapstoneDisassembler&) = delete;

    CapstoneDisassembler(CapstoneDisassembler&& other) noexcept
        : handle_(other.handle_), arch_(other.arch_) {
        other.handle_ = 0;
    }

    CapstoneDisassembler& operator=(CapstoneDisassembler&& other) noexcept {
        if (this != &other) {
            cs_close(&handle_);
            handle_ = other.handle_;
            arch_ = other.arch_;
            other.handle_ = 0;
        }
        return *this;
    }

    std::string name() const override {
        return "capstone";
    }

    DisassemblerArch arch() const override {
        return arch_;
    }

    DisassemblyResult disassemble(
        const uint8_t* code,
        size_t codeSize,
        uint64_t baseAddr,
        size_t count
    ) const override {
        DisassemblyResult result;

        if (!code || codeSize == 0) {
            result.errorMessage = "Empty code buffer";
            return result;
        }

        cs_insn* insn = nullptr;
        size_t decoded = cs_disasm(handle_, code, codeSize, baseAddr, count, &insn);

        if (decoded == 0) {
            cs_err err = cs_errno(handle_);
            if (err != CS_ERR_OK) {
                result.errorMessage = std::string("cs_disasm failed: ") + cs_strerror(err);
            } else {
                result.errorMessage = "No instructions decoded";
            }
            return result;
        }

        result.success = true;
        result.instructions.reserve(decoded);
        result.totalBytes = 0;

        for (size_t i = 0; i < decoded; ++i) {
            Instruction instr;
            instr.address = insn[i].address;
            instr.size = static_cast<uint16_t>(insn[i].size);
            instr.mnemonic = insn[i].mnemonic;
            instr.opStr = insn[i].op_str;
            instr.bytes.assign(insn[i].bytes, insn[i].bytes + insn[i].size);
            result.totalBytes += insn[i].size;
            result.instructions.push_back(std::move(instr));
        }

        cs_free(insn, decoded);
        return result;
    }

private:
    csh handle_ = 0;
    DisassemblerArch arch_;
};

std::unique_ptr<IDisassembler> createCapstoneDisassembler(DisassemblerArch arch) {
    return std::make_unique<CapstoneDisassembler>(arch);
}

} // namespace omnibyte::hydradis

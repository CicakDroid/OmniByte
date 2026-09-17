#pragma once

#include "RootFileAccess.h"
#include "BinaryInstaller.h"
#include <LIEF/ELF.hpp>
#include <LIEF/Parser.hpp>
#include <string>
#include <memory>
#include <functional>

namespace omnibyte::runtime {

/**
 * BinaryInstaller + LIEF integration.
 * Validates and optionally patches .so files before deployment.
 *
 * Usage:
 *   auto result = ValidatedBinaryInstaller::installValidated(
 *       "com.target.app", "/data/local/tmp/libhook.so", "arm64"
 *   );
 *   if (!result.ok) { /* handle error */ }
 */
class ValidatedBinaryInstaller {
public:
    struct ValidationResult {
        bool ok = false;
        bool isElf = false;
        bool is64Bit = false;
        uint16_t machine = 0;
        std::string format;       // "ELF64", "ELF32", "unknown"
        std::string arch;         // "ARM64", "ARM32", "unknown"
        std::string errorMessage;
    };

    struct InstallResult {
        bool ok = false;
        ValidationResult validation;
        std::string destPath;
        std::string errorMessage;
    };

    /**
     * Validate .so file before install.
     * Checks: valid ELF, correct architecture, not corrupted.
     */
    static ValidationResult validate(const std::string& soPath,
                                    const std::string& targetArch = "arm64") {
        ValidationResult result;

        try {
            auto binary = LIEF::Parser::parse(soPath);
            if (!binary) {
                result.errorMessage = "Failed to parse: " + soPath;
                return result;
            }

            auto* elf = dynamic_cast<const LIEF::ELF::Binary*>(binary.get());
            if (!elf) {
                result.errorMessage = "Not an ELF binary";
                return result;
            }

            result.isElf = true;
            result.ok = true;

            result.is64Bit = (elf->header().identity_class() ==
                              LIEF::ELF::Elf64_Class::ELFCLASS64);
            result.format = result.is64Bit ? "ELF64" : "ELF32";
            result.machine = static_cast<uint16_t>(elf->header().machine_type());

            switch (result.machine) {
                case 0xB7:  // EM_AARCH64
                    result.arch = "ARM64";
                    break;
                case 0x28:  // EM_ARM
                    result.arch = "ARM32";
                    break;
                case 0x03:  // EM_386
                    result.arch = "x86";
                    break;
                case 0x3E:  // EM_X86_64
                    result.arch = "x86_64";
                    break;
                default:
                    result.arch = "unknown";
                    break;
            }

            if (targetArch == "arm64" && result.arch != "ARM64") {
                result.ok = false;
                result.errorMessage = "Arch mismatch: expected ARM64, got " + result.arch;
            } else if (targetArch == "arm" && result.arch != "ARM32") {
                result.ok = false;
                result.errorMessage = "Arch mismatch: expected ARM32, got " + result.arch;
            }

        } catch (const std::exception& e) {
            result.errorMessage = std::string("Exception: ") + e.what();
        }

        return result;
    }

    /**
     * Validate + install with optional pre-deploy patch.
     * The patchFn receives the LIEF ELF binary and can modify it before write.
     */
    static InstallResult installValidated(
        const std::string& packageName,
        const std::string& soPath,
        const std::string& arch = "arm64",
        std::function<bool(LIEF::ELF::Binary&)> patchFn = nullptr
    ) {
        InstallResult result;

        result.validation = validate(soPath, arch);
        if (!result.validation.ok) {
            result.errorMessage = result.validation.errorMessage;
            return result;
        }

        std::string installPath = soPath;

        if (patchFn) {
            try {
                auto binary = LIEF::Parser::parse(soPath);
                if (!binary) {
                    result.errorMessage = "Failed to parse for patching";
                    return result;
                }

                auto* elf = dynamic_cast<LIEF::ELF::Binary*>(binary.get());
                if (!elf) {
                    result.errorMessage = "Not an ELF binary for patching";
                    return result;
                }

                if (!patchFn(*elf)) {
                    result.errorMessage = "Patch function returned false";
                    return result;
                }

                installPath = soPath + ".patched";
                binary->write(installPath);

            } catch (const std::exception& e) {
                result.errorMessage = std::string("Patch failed: ") + e.what();
                return result;
            }
        }

        bool installed = BinaryInstaller::install(packageName, installPath, arch);
        if (!installed) {
            result.errorMessage = "BinaryInstaller::install failed";
            return result;
        }

        std::string destDir = BinaryInstaller::getLibDir(packageName, arch);
        std::string fileName = soPath.substr(soPath.find_last_of('/') + 1);
        result.destPath = destDir + "/" + fileName;
        result.ok = true;

        return result;
    }

    /**
     * Common patch: inject entry into .init_array.
     * Used for auto-loading a hook library when app starts.
     */
    static bool patchInitArray(LIEF::ELF::Binary& elf, uint64_t hookFuncRVA) {
        auto* initArray = elf.get_section(".init_array");
        if (!initArray) {
            return false;
        }

        std::vector<uint8_t> entry(8, 0);
        uint64_t addr = initArray->virtual_address() + initArray->content().size();
        entry[0] = static_cast<uint8_t>(hookFuncRVA & 0xFF);
        entry[1] = static_cast<uint8_t>((hookFuncRVA >> 8) & 0xFF);
        entry[2] = static_cast<uint8_t>((hookFuncRVA >> 16) & 0xFF);
        entry[3] = static_cast<uint8_t>((hookFuncRVA >> 24) & 0xFF);
        entry[4] = static_cast<uint8_t>((hookFuncRVA >> 32) & 0xFF);
        entry[5] = static_cast<uint8_t>((hookFuncRVA >> 40) & 0xFF);
        entry[6] = static_cast<uint8_t>((hookFuncRVA >> 48) & 0xFF);
        entry[7] = static_cast<uint8_t>((hookFuncRVA >> 56) & 0xFF);

        std::vector<uint8_t> content = initArray->content();
        content.insert(content.end(), entry.begin(), entry.end());
        initArray->content(content);

        return true;
    }
};

} // namespace omnibyte::runtime

#ifdef VALIDATEDBINARYINSTALLER_TEST
#include <cassert>
int main() {
    printf("ValidatedBinaryInstaller: compile-check passed\n");
    return 0;
}
#endif

#include "Parser/IParser.h"
#include <LIEF/ELF.hpp>
#include <LIEF/Parser.hpp>
#include <memory>

namespace omnibyte::hydradis {

class LiefParser final : public IParser {
public:
    std::string name() const override {
        return "lief";
    }

    ParsedBinary parseFile(const std::string& filePath) const override {
        ParsedBinary result;

        try {
            auto binary = LIEF::Parser::parse(filePath);
            if (!binary) {
                result.errorMessage = "Failed to parse file: " + filePath;
                return result;
            }

            populateResult(binary.get(), result);
        } catch (const std::exception& e) {
            result.errorMessage = std::string("Exception: ") + e.what();
        }

        return result;
    }

    ParsedBinary parseBuffer(const uint8_t* data, size_t dataSize) const override {
        ParsedBinary result;

        try {
            auto binary = LIEF::Parser::parse(
                std::vector<uint8_t>(data, data + dataSize)
            );
            if (!binary) {
                result.errorMessage = "Failed to parse buffer";
                return result;
            }

            populateResult(binary.get(), result);
        } catch (const std::exception& e) {
            result.errorMessage = std::string("Exception: ") + e.what();
        }

        return result;
    }

private:
    void populateResult(const LIEF::Binary* binary, ParsedBinary& result) const {
        result.success = true;

        auto* elf = dynamic_cast<const LIEF::ELF::Binary*>(binary);
        if (!elf) {
            result.header.format = BinaryFormat::Unknown;
            return;
        }

        result.header.format = BinaryFormat::ELF;
        result.header.is64Bit = (elf->header().identity_class() ==
                                 LIEF::ELF::Elf64_Class::ELFCLASS64);
        result.header.isEndianLittle = (elf->header().identity_data() ==
                                       LIEF::ELF::Elf_Data::ELFDATA2LSB);
        result.header.entryPoint = elf->header().entrypoint();
        result.header.machine = static_cast<uint16_t>(elf->header().machine_type());

        for (const auto& section : elf->sections()) {
            SectionInfo si;
            si.name = section.name();
            si.virtualAddress = section.virtual_address();
            si.fileOffset = section.offset();
            si.size = section.size();
            si.flags = static_cast<uint32_t>(section.flags());
            result.sections.push_back(std::move(si));
        }

        for (const auto& sym : elf->symbols()) {
            SymbolInfo si;
            si.name = sym.name();
            si.value = sym.value();
            si.size = sym.size();
            si.type = static_cast<uint32_t>(sym.type());
            si.binding = static_cast<uint32_t>(sym.binding());
            si.sectionIndex = sym.shndx();
            result.symbols.push_back(std::move(si));
        }
    }
};

std::unique_ptr<IParser> createLiefParser() {
    return std::make_unique<LiefParser>();
}

} // namespace omnibyte::hydradis

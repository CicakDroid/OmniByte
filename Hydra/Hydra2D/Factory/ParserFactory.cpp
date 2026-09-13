#include "Factory/ParserFactory.h"

// Forward declaration — LiefParser free function from parser_lief.cpp
namespace omnibyte::hydradis {
    std::unique_ptr<IParser> createLiefParser();
}

namespace omnibyte::hydradis {

std::unique_ptr<IParser> ParserFactory::create(ParserBackend backend) {
    switch (backend) {
        case ParserBackend::Lief:
            return createLiefParser();

        default:
            return nullptr;
    }
}

} // namespace omnibyte::hydradis

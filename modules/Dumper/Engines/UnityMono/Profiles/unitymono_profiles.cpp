#include "Mono2022_3Profile.h"
#include "Mono6000_0Profile.h"
#include "Mono6000_1Profile.h"
#include "Mono6000_2Profile.h"
#include "Mono6000_3Profile.h"
#include "Mono6000_4Profile.h"
#include <memory>
#include <string>

namespace omnibyte::dumper::unitymono {

std::shared_ptr<IEngineProfile> createMonoProfile(const std::string& version) {
    if (version == "2022.3") return std::make_shared<Mono2022_3Profile>();
    if (version == "6000.0") return std::make_shared<Mono6000_0Profile>();
    if (version == "6000.1") return std::make_shared<Mono6000_1Profile>();
    if (version == "6000.2") return std::make_shared<Mono6000_2Profile>();
    if (version == "6000.3") return std::make_shared<Mono6000_3Profile>();
    if (version == "6000.4") return std::make_shared<Mono6000_4Profile>();
    return nullptr;
}

} // namespace omnibyte::dumper::unitymono
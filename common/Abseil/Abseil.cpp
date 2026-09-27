#include "Abseil.h"

#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>

namespace omnibyte::common {

AbseilAdapter& AbseilAdapter::instance() {
    static AbseilAdapter s;
    return s;
}

std::vector<std::string> AbseilAdapter::StrSplit(const std::string& text, char delimiter) {
    return absl::StrSplit(text, delimiter);
}

std::string AbseilAdapter::StrJoin(const std::vector<std::string>& parts, const std::string& delimiter) {
    return absl::StrJoin(parts, delimiter);
}

} // namespace omnibyte::common

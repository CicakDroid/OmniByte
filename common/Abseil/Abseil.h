#ifndef OMNIBYTE_COMMON_ABSEIL_H
#define OMNIBYTE_COMMON_ABSEIL_H

#include <string>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_split.h>
#include <absl/strings/str_join.h>

namespace omnibyte::common {

template <typename K, typename V>
using FastMap = absl::flat_hash_map<K, V>;

template <typename T>
using FastSet = absl::flat_hash_set<T>;

class AbseilAdapter {
public:
    static AbseilAdapter& instance();

    std::vector<std::string> StrSplit(const std::string& text, char delimiter);
    std::string StrJoin(const std::vector<std::string>& parts, const std::string& delimiter);
};

} // namespace omnibyte::common

#endif // OMNIBYTE_COMMON_ABSEIL_H

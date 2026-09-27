#pragma once
// TaskFlow.h — thin include + type aliases for taskflow/taskflow.
// Source: https://github.com/taskflow/taskflow (MIT License)
// Version: 3.8.0
//
// No business logic — just brings tf:: into omnibyte::common.
//
// BUILD NOTE: requires headers from CMake FetchContent (or local checkout).
// Standalone -fsyntax-only fails until headers are on the include path.

#include <taskflow/taskflow.hpp>

namespace omnibyte::common {

using Executor = tf::Executor;
using Taskflow = tf::Taskflow;

} // namespace omnibyte::common

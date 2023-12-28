#pragma once

// use constexpr implementation of search
#define __OPTIMIZE__
#include "include/nonstd/string_view.hpp"
#undef __OPTIMIZE__

namespace skytest {
using ::nonstd::string_view;
}  // namespace skytest

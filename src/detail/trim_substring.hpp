#pragma once

#include "src/string_view.hpp"

namespace skytest::detail {

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
constexpr auto
trim_substring(string_view s, string_view left, string_view right)
// NOLINTEND(bugprone-easily-swappable-parameters)
{
  const auto lower = s.find(left) + left.size();
  const auto upper = s.find(right, lower);

  return string_view{s.begin() + lower, upper - lower};
}

}  // namespace skytest::detail

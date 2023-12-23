#pragma once

#include <string_view>

namespace skytest::detail {

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
constexpr auto trim_substring(
    std::string_view s, std::string_view left, std::string_view right)
// NOLINTEND(bugprone-easily-swappable-parameters)
{
  const auto lower = s.find(left) + left.size();
  const auto upper = s.find(right, lower);

  return std::string_view{s.begin() + lower, upper - lower};
}

}  // namespace skytest::detail

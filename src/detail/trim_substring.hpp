#pragma once

#include <string_view>

namespace skytest {
namespace detail {

constexpr auto trim_substring(
    std::string_view s, std::string_view left, std::string_view right)
{
  const auto lower = s.find(left) + left.size();
  const auto upper = s.find(right, lower);

  return std::string_view{s.begin() + lower, upper - lower};
}

}  // namespace detail
}  // namespace skytest

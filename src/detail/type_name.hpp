#pragma once

#include <string_view>

namespace skytest {
namespace detail {

template <class F>
constexpr auto type_name_impl()
{
  constexpr auto full = std::string_view{__PRETTY_FUNCTION__};

#ifdef __clang__
  constexpr auto prefix = std::string_view{"F = "};
  constexpr auto suffix = std::string_view{"]"};
#else
  constexpr auto prefix = std::string_view{"with F = "};
  constexpr auto suffix = std::string_view{"]"};
#endif

  const auto lower = full.find(prefix) + prefix.size();
  const auto upper = full.find(suffix, lower);

  return std::string_view{full.begin() + lower, upper - lower};
}

template <class F>
static constexpr auto type_name = type_name_impl<F>();

}  // namespace detail
}  // namespace skytest

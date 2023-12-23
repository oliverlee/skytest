#pragma once

#include "src/detail/trim_substring.hpp"

namespace skytest::detail {

template <class F>
constexpr auto type_name_impl()
{
  constexpr auto prefix =
#ifdef __clang__
      "F = ";
#else
      "with F = ";
#endif

  return trim_substring(__PRETTY_FUNCTION__, prefix, "]");
}

template <class F>
inline constexpr auto type_name = type_name_impl<F>();

}  // namespace skytest::detail

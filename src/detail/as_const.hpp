#pragma once

#include <type_traits>

// backport for C++14
// https://en.cppreference.com/w/cpp/utility/as_const

namespace skytest::detail {

constexpr struct
{
  template <class T>
  constexpr auto operator()(T& t) const noexcept -> std::add_const_t<T>&
  {
    return t;
  }
  template <class T>
  auto operator()(const T&&) const -> void = delete;
} as_const{};

}  // namespace skytest::detail

#pragma once

#include "src/detail/remove_cvref.hpp"

#include <type_traits>

namespace skytest::detail {

template <const auto& f, class F = remove_cvref_t<decltype(f)>>
struct static_closure : F
{
  constexpr static_closure() : F{f} {}
};

template <class T>
struct is_static_closure : std::false_type
{};
template <const auto& f, class F>
struct is_static_closure<static_closure<f, F>> : std::true_type
{};

template <class T>
constexpr auto is_static_closure_v = is_static_closure<T>::value;

template <class T>
constexpr auto is_static_closure_constructible_v =
    std::is_empty_v<T> and std::is_copy_constructible_v<T>;

}  // namespace skytest::detail

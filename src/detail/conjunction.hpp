#pragma once

#include <type_traits>

namespace skytest::detail {

template <class...>
struct conjunction : std::true_type
{};
template <class B1>
struct conjunction<B1> : B1
{};
template <class B1, class... Bn>
struct conjunction<B1, Bn...>
    : std::conditional_t<bool(B1::value), conjunction<Bn...>, B1>
{};

}  // namespace skytest::detail

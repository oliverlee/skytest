#pragma once

#include <type_traits>
#include <utility>

namespace skytest::detail {

template <class T, class = void>
struct is_range : std::false_type
{};
template <class T>
struct is_range<
    T,
    std::enable_if_t<
        decltype(std::declval<T&>().begin(),
                 std::declval<T&>().end(),
                 std::true_type{})::value>> : std::true_type
{};

template <class T>
inline constexpr auto is_range_v = is_range<T>::value;

}  // namespace skytest::detail

#pragma once

#include <type_traits>

namespace skytest {
namespace detail {

template <class T, class = void>
struct is_defined : std::false_type
{};
template <class T>
struct is_defined<T, std::void_t<decltype(sizeof(T))>> : std::true_type
{};
template <class T>
inline constexpr auto is_defined_v = is_defined<T>::value;

}  // namespace detail
}  // namespace skytest

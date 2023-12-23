#pragma once

#include <type_traits>

namespace skytest {
namespace detail {

template <class T, template <class...> class Primary>
struct is_specialization_of : std::false_type
{};

template <template <class...> class Primary, class... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type
{};

template <class T, template <class...> class Primary>
inline constexpr bool is_specialization_of_v =
    is_specialization_of<T, Primary>::value;

}  // namespace detail
}  // namespace skytest

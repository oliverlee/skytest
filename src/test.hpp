#pragma once

#include "src/result.hpp"

#include <cstddef>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

namespace skytest {
namespace detail {

template <class R, class M>
auto is_result_(result<R, M>) -> std::true_type;
template <class T>
auto is_result_(const T&) -> std::false_type;

template <class T>
constexpr auto is_result_v =
    (not std::is_reference_v<T>)and decltype(is_result_(
        std::declval<T>()))::value;

template <class F, class = void>
struct compile_time_result
{
  static constexpr auto value = std::optional<bool>{};
};
template <class F>
struct compile_time_result<F, std::enable_if_t<bool{F{}()} or true>>
{
  static constexpr auto value = std::optional<bool>{bool{F{}()}};
};

class test
{
  std::string_view name_;

public:
  constexpr explicit test(std::string_view name) : name_{name} {}

  template <
      class F,
      class Override = std::enable_if_t<
          is_result_v<decltype(std::declval<const F&>()())>,
          override>,
      class Pass = compile_time_result<F>>
  constexpr auto operator=(const F& func) -> void
  {
    auto result = func();
    result.name = name_;
    result.compile_time = Pass::value;

    cfg<Override>.report(result);
  }
};

}  // namespace detail

namespace literals {
constexpr auto operator""_test(const char* name, std::size_t len)
{
  return detail::test{{name, len}};
}
}  // namespace literals
}  // namespace skytest

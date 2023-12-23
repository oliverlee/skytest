#pragma once

#include "src/detail/is_specialization_of.hpp"
#include "src/detail/type_name.hpp"
#include "src/result.hpp"
#include "src/rope.hpp"
#include "src/version.hpp"

#include <cstddef>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

namespace skytest {
namespace detail {

template <class Params>
class parameterized_test;

struct runtime_only_result
{
  static constexpr auto value = std::optional<bool>{};
};
template <class F, class = void>
struct compile_time_result : runtime_only_result
{};
template <class F>
struct compile_time_result<F, std::enable_if_t<bool{F{}()} or true>>
{
  static constexpr auto value = std::optional<bool>{bool{F{}()}};
};

template <const auto& f, class F = remove_cvref_t<decltype(f)>>
struct static_closure : F
{
  constexpr static_closure() : F{f} {}
};

template <class T>
constexpr auto is_static_closure_constructible_v =
    std::is_empty_v<T> and std::is_copy_constructible_v<T>;

template <class F, class... Args>
struct returns_result
    : is_specialization_of<
          std::invoke_result_t<const remove_cvref_t<F>&, Args...>,
          result>
{};

template <class F, class... Args>
inline constexpr auto returns_result_v = returns_result<F, Args...>::value;

template <std::size_t N>
class test
{
  rope<N> name_;

  template <
      class F,
      class Pass = compile_time_result<F>,
      class Override = override>
  auto assign_impl(const F& func) -> void
  {
    auto result = func();
    result.name = name_;
    result.compile_time = Pass::value;

    cfg<Override>.report(result);
  }

public:
  constexpr explicit test(rope<N> name) : name_{name} {}

  template <
      class F,
      std::enable_if_t<
          returns_result_v<F> and not is_static_closure_constructible_v<F>,
          bool> = true>
  auto operator=(const F& func) && -> void
  {
    assign_impl<F, runtime_only_result>(func);
  }
  template <
      class F,
      std::enable_if_t<
          returns_result_v<F> and is_static_closure_constructible_v<F>,
          bool> = true>
  auto operator=(const F& func) && -> void
  {
    static const auto f = func;
    assign_impl(static_closure<f>{});
  }

  template <class Params>
  constexpr friend auto operator*(test&& t, const Params& params)
  {
    static_assert(
        N == 1,
        "parameterization of an already "
        "parameterized test");
    return detail::parameterized_test<Params>{t.name_, params};
  }
};

}  // namespace detail

namespace literals {
constexpr auto operator""_test(const char* name, std::size_t len)
{
  return detail::test{rope<1>{std::string_view{name, len}}};
}
}  // namespace literals
}  // namespace skytest

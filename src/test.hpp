#pragma once

#include "src/cfg.hpp"
#include "src/detail/is_specialization_of.hpp"
#include "src/detail/remove_cvref.hpp"
#include "src/detail/type_name.hpp"
#include "src/optional.hpp"
#include "src/result.hpp"
#include "src/rope.hpp"
#include "src/string_view.hpp"
#include "src/version.hpp"

#include <cstddef>
#include <type_traits>
#include <utility>

namespace skytest {
namespace detail {

template <class Params>
class parameterized_test;

struct runtime_only_result
{
  static constexpr auto value = optional<bool>{};
};

constexpr optional<bool> runtime_only_result::value;

template <class F, class = void>
struct compile_time_result : runtime_only_result
{};
template <class F>
struct compile_time_result<F, std::enable_if_t<bool(F{}()) or true>>
{
  static constexpr auto value = optional<bool>{bool{F{}()}};
};

template <class F, const F& f>
struct static_closure : F
{
  constexpr static_closure() : F{f} {}
};

template <class T>
constexpr auto is_static_closure_constructible_v =
    std::is_empty<T>::value and std::is_copy_constructible<T>::value and
    std::integral_constant<bool, SKYTEST_CXX17>::value;

template <class F, class... Args>
struct returns_result
    : is_specialization_of<
          decltype(std::declval<const remove_cvref_t<F>&>()(
              std::declval<Args>()...)),
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
    assign_impl(static_closure<remove_cvref_t<decltype(f)>, f>{});
  }

  template <class Params>
  constexpr friend auto operator*(const test& t, const Params& params)
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
  return detail::test<1>{rope<1>{string_view{name, len}}};
}
}  // namespace literals
}  // namespace skytest

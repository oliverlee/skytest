#pragma once

#include "src/cfg.hpp"
#include "src/detail/is_specialization_of.hpp"
#include "src/detail/remove_cvref.hpp"
#include "src/detail/static_closure.hpp"
#include "src/detail/test_style.hpp"
#include "src/detail/type_name.hpp"
#include "src/result.hpp"
#include "src/rope.hpp"
#include "src/version.hpp"

#include <cstddef>
#include <string_view>
#include <type_traits>
#include <utility>

namespace skytest {
namespace detail {

template <class Params, class Style>
class parameterized_test;

template <class F, class... Args>
struct returns_result
    : is_specialization_of<
          std::invoke_result_t<const remove_cvref_t<F>&, Args...>,
          result>
{};

template <class F, class... Args>
inline constexpr auto returns_result_v = returns_result<F, Args...>::value;

template <class Rope, class Style>
class test
{
  using rope_type = Rope;

  rope_type name_;

  template <class F, class S = Style, class Override = override>
  auto assign_impl(const F& func) -> void
  {
    auto result = func();
    result.name = name_;
    result.compile_time = S::template value<F>;

    cfg<Override>.report(result);
  }

public:
  constexpr explicit test(rope_type name) : name_{name} {}

  template <
      class F,
      std::enable_if_t<
          returns_result_v<F> and not is_static_closure_constructible_v<F>,
          bool> = true>
  auto operator=(const F& func) && -> void
  {
    assign_impl(func);
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
  constexpr friend auto operator*(const test& t, const Params& params)
  {
    static_assert(
        rope_type::size == 1,
        "parameterization of an already "
        "parameterized test");
    return detail::parameterized_test<Params, Style>{t.name_, params};
  }
};

}  // namespace detail

namespace literals {
constexpr auto operator""_test(const char* name, std::size_t len)
{
  using rope_type = rope<1>;
  using style_type = detail::test_style::compile_time_if_possible;
  return detail::test<rope_type, style_type>{
      rope_type{std::string_view{name, len}}};
}
constexpr auto operator""_ctest(const char* name, std::size_t len)
{
  using rope_type = rope<1>;
  using style_type = detail::test_style::compile_time;
  return detail::test<rope_type, style_type>{
      rope_type{std::string_view{name, len}}};
}
}  // namespace literals
}  // namespace skytest

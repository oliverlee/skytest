#pragma once

#include "src/detail/type_name.hpp"
#include "src/result.hpp"
#include "src/rope.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <optional>
#include <string_view>
#include <tuple>
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

template <class... Args>
class parameterized_test;

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
          std::is_empty_v<F> and
              is_result_v<decltype(std::declval<const F&>()())>,
          bool> = true>
  auto operator=(const F& func) && -> void
  {
    static const auto f = func;
    assign_impl(static_closure<f>{});
  }
  template <
      class F,
      std::enable_if_t<
          not std::is_empty_v<F> and
              is_result_v<decltype(std::declval<const F&>()())>,
          bool> = true>
  auto operator=(const F& func) && -> void
  {
    assign_impl<F, runtime_only_result>(func);
  }

  template <class... Args>
  constexpr friend auto operator*(test&& t, std::tuple<Args...> types)
  {
    static_assert(
        N == 1,
        "parameterization of an already "
        "parameterized test");
    return detail::parameterized_test<Args...>{t.name_, std::move(types)};
  }
};

template <class... Args>
class parameterized_test
{
  std::tuple<Args...> args_;
  rope<1> basename_;

  template <std::size_t I>
  using arg_type_t = std::tuple_element_t<I, std::tuple<Args...>>;

  template <std::size_t... Is, class F>
  constexpr auto assign_impl(std::index_sequence<Is...>, const F& func)
  {
    std::ignore = std::array<bool, sizeof...(Is)>{(
        test{rope<4>{basename_, " <", type_name<arg_type_t<Is>>, ">"}} =
            [&arg = std::get<Is>(args_), &func] { return func(arg); },
        true)...};
  }

public:
  constexpr explicit parameterized_test(
      rope<1> basename, std::tuple<Args...> args)
      : args_{std::move(args)}, basename_{basename}
  {}

  template <
      class F,
      class = std::enable_if_t<(
          is_result_v<decltype(std::declval<const F&>()(
              std::declval<const Args&>()))> and
          ...)>>
  constexpr auto operator=(const F& func) && -> void
  {
    assign_impl(std::index_sequence_for<Args...>{}, func);
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

#pragma once

#include "src/detail/type_name.hpp"
#include "src/result.hpp"
#include "src/rope.hpp"
#include "src/version.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace skytest {

#if SKYTEST_CXX20
using std::type_identity;
#else
template <class T>
struct type_identity
{
  using type = T;
};
#endif

struct param
{
  template <class... Ts>
  static constexpr auto types = std::tuple<type_identity<Ts>...>{};
};

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

template <class T>
constexpr auto is_static_closure_constructible_v =
    std::is_empty_v<T> and std::is_copy_constructible_v<T>;

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
          not is_static_closure_constructible_v<F> and
              is_result_v<decltype(std::declval<const F&>()())>,
          bool> = true>
  auto operator=(const F& func) && -> void
  {
    assign_impl<F, runtime_only_result>(func);
  }
  template <
      class F,
      std::enable_if_t<
          is_static_closure_constructible_v<F> and
              is_result_v<decltype(std::declval<const F&>()())>,
          bool> = true>
  auto operator=(const F& func) && -> void
  {
    static const auto f = func;
    assign_impl(static_closure<f>{});
  }

  template <class... Args>
  constexpr friend auto operator*(test&& t, const std::tuple<Args...>& types)
  {
    static_assert(
        N == 1,
        "parameterization of an already "
        "parameterized test");
    return detail::parameterized_test<Args...>{t.name_, types};
  }
};

template <std::size_t I>
using constant = std::integral_constant<std::size_t, I>;

template <class F, class Tuple, class = void>
struct bind_args
{
  const F& func;
  const Tuple& args;

  template <std::size_t I>
  constexpr auto operator[](constant<I>) const
  {
    return [this] { return func(std::get<I>(args)); };
  }
};

template <const auto& f, class... Args>
struct bind_args<
    static_closure<f>,
    std::tuple<Args...>,
    std::enable_if_t<(std::is_empty_v<Args> and ...)>>
{
  template <std::size_t I>
  constexpr auto operator[](constant<I>) const
  {
    return [] {
      const auto args = std::tuple<Args...>{};
      return static_closure<f>{}(std::get<I>(args));
    };
  }
};

template <class... Args>
class parameterized_test
{
  using args_type = std::tuple<Args...>;
  const args_type& args_;
  rope<1> basename_;

  template <std::size_t I>
  using arg_type_t = std::tuple_element_t<I, args_type>;

  template <std::size_t... Is, class G>
  auto assign_impl(std::index_sequence<Is...>, const G& g)
  {
    std::ignore =
        ((test{rope<4>{basename_, " <", type_name<arg_type_t<Is>>, ">"}} =
              g[constant<Is>{}],
          true) and
         ...);
  }

public:
  constexpr explicit parameterized_test(
      rope<1> basename, const std::tuple<Args...>& args)
      : args_{std::move(args)}, basename_{basename}
  {}

  template <
      class F,
      std::enable_if_t<
          not(is_static_closure_constructible_v<F> and
              (std::is_empty_v<Args> and ...)) and
              (is_result_v<decltype(std::declval<const F&>()(
                   std::declval<const Args&>()))> and
               ...),
          bool> = true>
  auto operator=(const F& func) && -> void
  {
    assign_impl(
        std::index_sequence_for<Args...>{},
        bind_args<F, args_type>{func, args_});
  }
  template <
      class F,
      std::enable_if_t<
          is_static_closure_constructible_v<F> and
              (std::is_empty_v<Args> and ...) and
              (is_result_v<decltype(std::declval<const F&>()(
                   std::declval<const Args&>()))> and
               ...),
          bool> = true>
  auto operator=(const F& func) && -> void
  {
    static const auto f = func;
    assign_impl(
        std::index_sequence_for<Args...>{},
        bind_args<static_closure<f>, args_type>{});
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

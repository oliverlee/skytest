#pragma once

#include "src/detail/remove_cvref.hpp"
#include "src/rope.hpp"
#include "src/test.hpp"

#include <cstddef>
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

namespace detail {

template <std::size_t I>
using constant = std::integral_constant<std::size_t, I>;

template <std::size_t>
auto get() -> void;

template <class F, class Params>
struct param_bound_closure
{
  const F& func;
  const Params& params;

  template <std::size_t I>
  constexpr auto operator[](constant<I>) const
  {
    return [this] { return func(get<I>(params)); };
  }
};

template <const auto& func, const auto& params>
struct param_bound_static_closure
{
  template <std::size_t I>
  constexpr auto operator[](constant<I>) const
  {
    return [] { return func(get<I>(params)); };
  }
};

template <std::size_t I, class Params>
using param_reference_t = decltype(get<I>(std::declval<const Params&>()));

template <class, class, class>
struct param_invocable_;

template <std::size_t... Is, class F, class Params>
struct param_invocable_<std::index_sequence<Is...>, F, Params>
    : std::conjunction<returns_result<F, param_reference_t<Is, Params>>...>
{};

template <class F, class Params>
struct param_invocable
    : param_invocable_<
          std::make_index_sequence<std::tuple_size_v<Params>>,
          F,
          Params>
{};

template <class F, class Params>
inline constexpr auto param_invocable_v = param_invocable<F, Params>::value;

template <class T, class = void>
struct has_static_value : std::false_type
{};
template <class T>
struct has_static_value<T, std::void_t<decltype(T::value)>> : std::true_type
{};
template <class T>
inline constexpr auto has_static_value_v = has_static_value<T>::value;

template <class Params>
class parameterized_test
{
public:
  using params_type = Params;

private:
  const params_type& params_;
  rope<1> basename_;

  template <std::size_t I>
  constexpr auto test_name() const
  {
    return rope<4>{
        basename_, " <", type_name<param_reference_t<I, params_type>>, ">"};
  }

  template <std::size_t... Is, class G>
  auto assign_impl(std::index_sequence<Is...>, const G& g)
  {
    std::ignore = ((test{test_name<Is>()} = g[constant<Is>{}], true) and ...);
  }

public:
  constexpr explicit parameterized_test(
      rope<1> basename, const params_type& params)
      : params_{params}, basename_{basename}
  {}

  template <
      class F,
      std::enable_if_t<
          param_invocable_v<F, Params> and
              not(is_static_closure_constructible_v<F> and
                  has_static_value_v<params_type>),
          bool> = true>
  auto operator=(const F& func) && -> void
  {

    assign_impl(
        std::make_index_sequence<std::tuple_size_v<params_type>>{},
        param_bound_closure<F, Params>{func, params_});
  }
  template <
      class F,
      std::enable_if_t<
          param_invocable_v<F, Params> and
              is_static_closure_constructible_v<F> and
              has_static_value_v<params_type>,
          bool> = true>
  auto operator=(const F& func) && -> void
  {
    static const auto f = func;
    static constexpr const auto& p = params_type::value;

    assign_impl(
        std::make_index_sequence<std::tuple_size_v<params_type>>{},
        param_bound_static_closure<f, p>{});
  }
};

template <std::size_t I, class T>
struct type_leaf
{
  template <std::size_t N>
  friend constexpr auto get(type_leaf) -> std::enable_if_t<I == N, T>
  {
    return {};
  }
};

template <class, class... Args>
struct type_list_impl;
template <std::size_t... Is, class... Args>
struct type_list_impl<std::index_sequence<Is...>, Args...>
    : type_leaf<Is, Args>...
{};

template <class... Args>
struct type_list : type_list_impl<std::index_sequence_for<Args...>, Args...>
{};

template <class... Ts>
inline constexpr auto type_params = type_list<type_identity<Ts>...>{};

}  // namespace detail

template <const auto& Tuple>
struct param_ref_t
{
  static constexpr const auto& value = Tuple;

  template <std::size_t I>
  friend constexpr decltype(auto) get(param_ref_t)
  {
    using detail::get;
    return get<I>(value);
  }
};
template <auto Tuple>
struct param_t : param_ref_t<Tuple>
{};

template <const auto& Tuple>
inline constexpr auto param_ref = param_ref_t<Tuple>{};
template <auto Tuple>
inline constexpr auto param = param_t<Tuple>{};

template <class... Ts>
inline constexpr auto types = param_ref<detail::type_params<Ts...>>;

}  // namespace skytest

template <class... Args>
struct std::tuple_size<::skytest::detail::type_list<Args...>>
    : ::std::integral_constant<std::size_t, sizeof...(Args)>
{};

template <const auto& Tuple>
struct std::tuple_size<::skytest::param_ref_t<Tuple>>
    : ::std::tuple_size<::skytest::detail::remove_cvref_t<decltype(Tuple)>>
{};
template <auto Tuple>
struct std::tuple_size<::skytest::param_t<Tuple>>
    : ::std::tuple_size<::skytest::detail::remove_cvref_t<decltype(Tuple)>>
{};

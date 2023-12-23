#pragma once

#include "src/detail/is_defined.hpp"
#include "src/detail/priority.hpp"
#include "src/detail/remove_cvref.hpp"
#include "src/detail/trim_substring.hpp"
#include "src/detail/type_name.hpp"
#include "src/rope.hpp"
#include "src/test.hpp"

#include <cstddef>
#include <cstdio>
#include <limits>
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

template <class T, class = void>
struct has_static_value : std::false_type
{};
template <class T>
struct has_static_value<T, std::void_t<decltype(T::value)>> : std::true_type
{};
template <class T>
inline constexpr auto has_static_value_v = has_static_value<T>::value;

template <class T>
struct static_size
{
  template <class U, class = std::enable_if_t<is_range_v<U>>>
  static constexpr auto impl(priority<1>)
  {
    return T::value.size();
  }

  template <class U, class = std::enable_if_t<is_defined_v<std::tuple_size<U>>>>
  static constexpr auto impl(priority<0>)
  {
    return std::tuple_size_v<U>;
  }

  static constexpr auto value =
      impl<remove_cvref_t<decltype(T::value)>>(priority<1>{});
};
template <class T>
inline constexpr auto static_size_v = static_size<T>::value;

struct value_sequence
{};

template <class T>
struct param_resolve
{
  template <class U = T, class = std::enable_if_t<is_range_v<U>>>
  static constexpr auto impl(priority<2>) -> U;

  template <
      class U = T,
      class = std::enable_if_t<is_defined_v<std::tuple_size<U>>>>
  static constexpr auto impl(priority<1>) -> U;

  template <class U = T, class = std::enable_if_t<has_static_value_v<U>>>
  static constexpr auto impl(priority<0>)
      -> decltype(impl<remove_cvref_t<decltype(U::value)>>(priority<2>{}));

  using type = decltype(impl(priority<2>{}));
};

template <class T>
using param_resolve_t = typename param_resolve<T>::type;

template <class T>
struct param_sequence
{
  template <class U, class = std::enable_if_t<is_range_v<U>>>
  static constexpr auto impl(priority<1>) -> value_sequence;

  template <class U, class = std::enable_if_t<is_defined_v<std::tuple_size<U>>>>
  static constexpr auto
      impl(priority<0>) -> std::make_index_sequence<std::tuple_size<U>::value>;

  using type = decltype(impl<param_resolve_t<T>>(priority<1>{}));
};

template <class T>
using param_sequence_t = typename param_sequence<T>::type;

template <std::size_t>
auto get() -> void;

template <std::size_t I, class Params>
using param_reference_t = decltype(get<I>(std::declval<const Params&>()));

template <class, class, class>
struct param_invocable_;

template <std::size_t... Is, class F, class Params>
struct param_invocable_<std::index_sequence<Is...>, F, Params>
    : std::conjunction<returns_result<F, param_reference_t<Is, Params>>...>
{};

template <class F, class Params>
struct param_invocable_<value_sequence, F, Params>
    : returns_result<F, decltype(*std::declval<Params&>().begin())>
{};

template <class F, class Params>
struct param_invocable
    : param_invocable_<param_sequence_t<Params>, F, param_resolve_t<Params>>
{};

template <class F, class Params>
inline constexpr auto param_invocable_v = param_invocable<F, Params>::value;

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

  template <class Iter>
  constexpr auto operator[](Iter it) const
  {
    return [this, it] { return func(*it); };
  }
};

template <const auto& func, const auto& params>
struct param_bound_static_closure
{
  template <
      std::size_t I,
      class P = remove_cvref_t<decltype(params)>,
      std::enable_if_t<not is_range_v<P>, bool> = true>
  constexpr auto operator[](constant<I>) const
  {
    return [] { return func(get<I>(params)); };
  }
  template <
      std::size_t I,
      class P = remove_cvref_t<decltype(params)>,
      std::enable_if_t<is_range_v<P>, bool> = true>
  constexpr auto operator[](constant<I>) const
  {
    return [] { return func(params.begin()[I]); };
  }
};

template <class Params>
class parameterized_test
{
public:
  using params_type = Params;

private:
  const params_type& params_;
  rope<1> basename_;

  constexpr auto value_param_name(std::string_view s) const
  {
    return rope<4>{basename_, " [", s, "]"};
  }
  template <std::size_t I>
  constexpr auto type_param_name() const
  {
    return rope<4>{
        basename_, " <", type_name<param_reference_t<I, params_type>>, ">"};
  }

  template <std::size_t I>
  constexpr auto param_name(std::false_type) const
  {
    return type_param_name<I>();
  }
  template <std::size_t I>
  constexpr auto param_name(std::true_type) const
  {
    constexpr auto suffix =
#ifdef __clang__
        "UL";
#else
        ";";
#endif
    constexpr auto s = trim_substring(__PRETTY_FUNCTION__, "I = ", suffix);
    return value_param_name(s);
  }

  template <class G>
  auto assign_impl(value_sequence, const G& g)
  {
    static constexpr auto N =
        std::size_t{std::numeric_limits<std::size_t>::digits10 + 2};

    auto s = std::array<char, N>{};
    auto i = std::size_t{};
    for (auto it = params_.begin(); it != params_.end(); ++it) {
      std::sprintf(s.data(), "%zu", i++);
      test{value_param_name(s.data())} = g[it];
    }
  }
  template <std::size_t... Is, class G>
  auto assign_impl(std::index_sequence<Is...>, const G& g)
  {
    static constexpr auto name_kind = is_range<param_resolve_t<params_type>>{};

    std::ignore =
        ((test{param_name<Is>(name_kind)} = g[constant<Is>{}], true) and ...);
  }

public:
  constexpr explicit parameterized_test(
      rope<1> basename, const params_type& params)
      : params_{params}, basename_{basename}
  {}

  template <
      class F,
      std::enable_if_t<
          param_invocable_v<F, params_type> and
              not(is_static_closure_constructible_v<F> and
                  has_static_value_v<params_type>),
          bool> = true>
  auto operator=(const F& func) && -> void
  {
    assign_impl(
        param_sequence_t<params_type>{},
        param_bound_closure<F, params_type>{func, params_});
  }
  template <
      class F,
      std::enable_if_t<
          param_invocable_v<F, params_type> and
              is_static_closure_constructible_v<F> and
              has_static_value_v<params_type>,
          bool> = true>
  auto operator=(const F& func) && -> void
  {
    static const auto f = func;
    static constexpr const auto& p = params_type::value;

    assign_impl(
        std::make_index_sequence<static_size_v<params_type>>{},
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

template <class P, P p>
struct param_impl
{
  static constexpr P value = p;

  template <std::size_t I>
  friend constexpr decltype(auto) get(param_impl)
  {
    using detail::get;
    return get<I>(value);
  }

  template <class T = P, class = std::enable_if_t<detail::is_range_v<T>>>
  constexpr auto begin() const
  {
    return value.begin();
  }
  template <class T = P, class = std::enable_if_t<detail::is_range_v<T>>>
  constexpr auto end() const
  {
    return value.end();
  }
};

template <const auto& Params>
struct param_ref_t : param_impl<decltype(Params), Params>
{};
template <auto Params>
struct param_t : param_impl<decltype(Params), Params>
{};

template <const auto& Params>
inline constexpr auto param_ref = param_ref_t<Params>{};
template <auto Params>
inline constexpr auto param = param_t<Params>{};

template <class... Ts>
inline constexpr auto types = param_ref<detail::type_params<Ts...>>;

}  // namespace skytest

template <class... Args>
struct std::tuple_size<::skytest::detail::type_list<Args...>>
    : ::std::integral_constant<std::size_t, sizeof...(Args)>
{};

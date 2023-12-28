#pragma once

#include "src/detail/as_const.hpp"
#include "src/detail/relation.hpp"
#include "src/detail/remove_cvref.hpp"
#include "src/detail/type_name.hpp"
#include "src/string_view.hpp"
#include "src/utility.hpp"

#include <tuple>
#include <type_traits>
#include <utility>

namespace skytest::detail {

template <class F, const string_view& Name, class Format>
struct predicate : F
{
  static constexpr auto name = Name;
  using notation_type = typename Format::notation_type;
  static constexpr decltype(Format::symbol) symbol = Format::symbol;

  template <
      class T = F,
      class = std::enable_if_t<std::is_default_constructible<T>::value>>
  constexpr predicate()
  {}

  template <
      class T,
      class = std::enable_if_t<std::is_constructible<F, T&&>::value>>
  constexpr predicate(T&& f) : F{std::forward<T>(f)}
  {}

  template <
      class... Ts,
      class = std::enable_if_t<std::is_convertible<
          decltype(std::declval<const F&>()(std::declval<const Ts&>()...)),
          bool>::value>>
  constexpr auto operator()(Ts&&... args) const
  {
    const auto value = static_cast<const F&>(*this)(as_const(args)...);
    return relation<predicate, std::decay_t<Ts>...>{
        std::tuple<std::decay_t<Ts>...>{std::forward<Ts>(args)...}, value};
  }
};

template <class F, const string_view& Name, class Format>
constexpr string_view predicate<F, Name, Format>::name;
template <class F, const string_view& Name, class Format>
constexpr decltype(Format::symbol) predicate<F, Name, Format>::symbol;

}  // namespace skytest::detail

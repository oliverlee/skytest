#pragma once

#include "src/detail/relation.hpp"
#include "src/detail/remove_cvref.hpp"
#include "src/detail/type_name.hpp"
#include "src/utility.hpp"

#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace skytest::detail {

template <class F, const std::string_view& Name, class Format>
struct predicate : F
{
  static constexpr auto name = Name;
  using notation_type = typename Format::notation_type;
  static constexpr auto symbol = Format::symbol;

  template <
      class T = F,
      class = std::enable_if_t<std::is_default_constructible_v<T>>>
  constexpr predicate()
  {}

  template <class T, class = std::enable_if_t<std::is_constructible_v<F, T&&>>>
  constexpr predicate(T&& f) : F{std::forward<T>(f)}
  {}

  template <class... Ts>
  constexpr auto operator()(Ts&&... args) const
  {
    auto value = static_cast<const F&>(*this)(std::as_const(args)...);

    return relation<predicate, std::decay_t<Ts>...>{
        std::tuple<std::decay_t<Ts>...>{std::forward<Ts>(args)...},
        std::move(value)};
  }
};

}  // namespace skytest::detail

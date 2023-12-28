#pragma once

#include "src/detail/arg_fmt.hpp"
#include "src/detail/as_const.hpp"
#include "src/detail/type_name.hpp"
#include "src/string_view.hpp"
#include "src/utility.hpp"

#include <functional>
#include <ostream>
#include <tuple>
#include <utility>

namespace skytest::detail {

struct and_
{
  static constexpr auto name = type_name<and_>;
  using notation_type = notation::infix;
  static constexpr auto symbol = string_view{"and"};
};

struct or_
{
  static constexpr auto name = type_name<or_>;
  using notation_type = notation::infix;
  static constexpr auto symbol = string_view{"or"};
};

struct not_
{
  static constexpr auto name = type_name<not_>;
  using notation_type = notation::prefix;
  static constexpr auto symbol = string_view{"not"};
};

template <class F, class... Ts>
struct relation
{
  using predicate_type = F;
  using arguments_type = std::tuple<Ts...>;

  arguments_type args;
  bool value;

  constexpr operator bool() const { return value; }

  static constexpr auto&
  print(notation::function, std::ostream& os, const relation& r)
  {
    if (const auto symbol = predicate_type::symbol; not symbol.empty()) {
      os << symbol;
    } else {
      os << predicate_type::name << "{}";
    }

    os << "(" << arg_fmt(r.args) << ")";
    return os;
  }
  static auto& print(notation::prefix, std::ostream& os, const relation& r)
  {
    static_assert(sizeof...(Ts) != 0);

    os << "(" << predicate_type::symbol << " " << arg_fmt(r.args) << ")";
    return os;
  }
  static auto& print(notation::infix, std::ostream& os, const relation& r)
  {
    static_assert(sizeof...(Ts) == 2);

    os << "(" << arg_fmt(std::get<0>(r.args)) << " " << predicate_type::symbol
       << " " << arg_fmt(std::get<1>(r.args)) << ")";
    return os;
  }

  friend auto& operator<<(std::ostream& os, const relation& r)
  {
    return print(typename relation::predicate_type::notation_type{}, os, r);
  }

  template <class G, class... Us>
  constexpr friend auto operator and(relation&& lhs, relation<G, Us...>&& rhs)
  {
    const auto result = as_const(lhs) and as_const(rhs);

    return relation<and_, relation, relation<G, Us...>>{
        {std::move(lhs), std::move(rhs)}, result};
  }
  template <class G, class... Us>
  constexpr friend auto operator or(relation&& lhs, relation<G, Us...>&& rhs)
  {
    const auto result = as_const(lhs) or as_const(rhs);

    return relation<or_, relation, relation<G, Us...>>{
        {std::move(lhs), std::move(rhs)}, result};
  }
  constexpr friend auto operator not(relation&& r)
  {
    const auto result = not as_const(r);

    return relation<not_, relation>{{std::move(r)}, result};
  }
};

}  // namespace skytest::detail

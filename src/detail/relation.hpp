#pragma once

#include "src/detail/tuple_fmt.hpp"
#include "src/detail/type_name.hpp"
#include "src/utility.hpp"

#include <ostream>
#include <tuple>
#include <utility>

namespace skytest {
namespace detail {

template <class F, class G>
struct and_ : predicate_format<>
{
  static constexpr auto name = type_name<and_>;
};
template <class F, class G>
struct or_ : predicate_format<>
{
  static constexpr auto name = type_name<or_>;
};
template <class F>
struct not_ : predicate_format<>
{
  static constexpr auto name = type_name<not_>;
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

    os << "(" << fmt(r.args) << ")";
    return os;
  }
  static auto& print(notation::infix, std::ostream& os, const relation& r)
  {
    static_assert(sizeof...(Ts) == 2);

    os << "(" << std::get<0>(r.args) << " " << predicate_type::symbol << " "
       << std::get<1>(r.args) << ")";
    return os;
  }

  friend auto& operator<<(std::ostream& os, const relation& r)
  {
    return print(typename relation::predicate_type::notation_type{}, os, r);
  }

  template <class G, class... Us>
  constexpr friend auto operator and(relation&& lhs, relation<G, Us...>&& rhs)
  {
    return relation<and_<F, G>, Ts..., Us...>{
        std::tuple_cat(std::move(lhs.args), std::move(rhs.args)),
        lhs.value and rhs.value};
  }
  template <class G, class... Us>
  constexpr friend auto operator or(relation&& lhs, relation<G, Us...>&& rhs)
  {
    return relation<or_<F, G>, Ts..., Us...>{
        std::tuple_cat(std::move(lhs.args), std::move(rhs.args)),
        lhs.value or rhs.value};
  }
  constexpr friend auto operator not(relation&& r)
  {
    return relation<not_<F>, Ts...>{std::move(r.args), not r.value};
  }
};

}  // namespace detail
}  // namespace skytest

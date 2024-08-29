#pragma once

#include "src/detail/is_range.hpp"
#include "src/detail/is_specialization_of.hpp"
#include "src/detail/priority.hpp"
#include "src/detail/type_name.hpp"
#include "src/rope.hpp"

#include <cstddef>
#include <ostream>
#include <tuple>
#include <utility>

namespace skytest::detail {
template <class R>
struct range_fmt;
template <class... Ts>
struct tuple_fmt;

class arg_fmt_fn
{
  struct ostream_inserter
  {
    template <class T>
    auto operator()(const T& value)
        -> decltype(std::declval<std::ostream&>() << value);
  };

  template <class T>
  static constexpr auto is_ostreamable_v =
      std::is_invocable_r_v<std::ostream&, ostream_inserter, const T&>;

  template <
      class R,
      std::enable_if_t<is_range_v<R> and not is_ostreamable_v<R>, bool> = true>
  static constexpr auto impl(priority<3>, const R& arg)
  {
    return range_fmt<R>{arg};
  }

  template <
      class T,
      std::enable_if_t<
          is_specialization_of_v<T, std::tuple> and not is_ostreamable_v<T>,
          bool> = true>
  static constexpr auto impl(priority<2>, const T& arg)
  {
    return tuple_impl(arg);
  }
  template <class... Ts>
  static constexpr auto tuple_impl(const std::tuple<Ts...>& arg)
  {
    return tuple_fmt<Ts...>{arg};
  }

  template <class T, std::enable_if_t<is_ostreamable_v<T>, bool> = true>
  static constexpr auto& impl(priority<1>, const T& arg)
  {
    return arg;
  }

  template <class T>
  static constexpr auto impl(priority<0>, const T&)
  {
    return rope<2>{type_name<T>, "{...}"};
  }

public:
  template <class T>
  constexpr decltype(auto) operator()(const T & arg) const
  {
    return impl(priority<3>{}, arg);
  }
};
inline constexpr auto arg_fmt = arg_fmt_fn{};

template <class R>
struct range_fmt
{
  const R& range;

  friend auto& operator<<(std::ostream& os, const range_fmt& f)
  {
    auto first = true;

    os << "[";
    for (const auto& value : f.range) {
      os << (std::exchange(first, false) ? "" : ", ") << value;
    }
    os << "]";
    return os;
  }
};

template <class... Ts>
struct tuple_fmt
{
  const std::tuple<Ts...>& value;

  template <std::size_t... Is>
  auto& stream_insert(std::index_sequence<Is...>, std::ostream& os) const
  {
    std::ignore =
        ((os << (Is == 0 ? "" : ", ") << arg_fmt(std::get<Is>(value)), true) and
         ...);
    return os;
  }

  friend auto& operator<<(std::ostream& os, const tuple_fmt& tup)
  {
    return tup.stream_insert(std::index_sequence_for<Ts...>{}, os);
  }
};

}  // namespace skytest::detail

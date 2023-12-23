#pragma once

#include <array>
#include <ostream>
#include <tuple>
#include <utility>

namespace skytest::detail {

template <class... Ts>
struct tuple_fmt
{
  const std::tuple<Ts...>& value;

  template <std::size_t... Is>
  auto& stream_insert(std::index_sequence<Is...>, std::ostream& os) const
  {
    std::ignore = std::array<bool, sizeof...(Is)>{
        (os << (Is == 0 ? "" : ", ") << std::get<Is>(value), true)...};
    return os;
  }

  friend auto& operator<<(std::ostream& os, const tuple_fmt& tup)
  {
    return tup.stream_insert(std::index_sequence_for<Ts...>{}, os);
  }
};

template <class... Ts>
constexpr auto fmt(const std::tuple<Ts...>& tup)
{
  return tuple_fmt<Ts...>{tup};
}

}  // namespace skytest::detail

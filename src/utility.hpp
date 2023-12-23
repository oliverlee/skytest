#pragma once

#include <cstddef>
#include <string_view>

namespace skytest {

struct notation
{
  struct function
  {};
  struct prefix
  {};
  struct infix
  {};
};

struct empty_symbol
{
  static constexpr auto value = std::string_view{};
};
template <class Symbol = empty_symbol, class Notation = notation::function>
struct predicate_format
{
  using notation_type = Notation;
  static constexpr auto symbol = Symbol::value;
};

struct summary
{
  std::size_t pass{};
  std::size_t fail{};
};

}  // namespace skytest

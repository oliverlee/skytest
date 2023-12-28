#pragma once

#include "src/string_view.hpp"

#include <cstddef>

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
  static constexpr auto value = string_view{};
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

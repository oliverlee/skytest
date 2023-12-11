#pragma once

#include "src/rope.hpp"
#include "src/source_location.hpp"

#include <optional>
#include <string_view>

namespace skytest {

template <class Relation, class Message>
struct result
{
  Relation relation;
  Message msg;
  source_location source;
  rope_ref name{};
  std::optional<bool> compile_time{};

  constexpr explicit operator bool() const { return bool(relation); }
};

}  // namespace skytest

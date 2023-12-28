#pragma once

#include "src/optional.hpp"
#include "src/rope.hpp"
#include "src/source_location.hpp"
#include "src/string_view.hpp"

namespace skytest {

template <class Relation, class Message>
struct result
{
  Relation relation;
  Message msg;
  source_location source;
  rope_ref name{};
  optional<bool> compile_time{};

  constexpr explicit operator bool() const { return bool(relation); }
};

}  // namespace skytest

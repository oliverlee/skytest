#pragma once

#include "src/result.hpp"
#include "src/source_location.hpp"

#include <utility>

namespace skytest {

struct empty_message
{
  template <class T>
  constexpr auto operator()(T&) const -> void
  {}
};

template <class Relation, class Message = empty_message>
[[nodiscard]]
constexpr auto expect(
    Relation r,
    Message msg = {},
    source_location sl = source_location::current())
{
  return result<Relation, Message>{std::move(r), std::move(msg), sl};
}

}  // namespace skytest

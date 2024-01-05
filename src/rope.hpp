#pragma once

#include <array>
#include <cstddef>
#include <ostream>
#include <string_view>

namespace skytest {

template <std::size_t>
struct rope;

class rope_ref
{
  template <std::size_t>
  friend struct rope;

  const std::string_view* begin{nullptr};
  const std::string_view* end{nullptr};

public:
  rope_ref() = default;

  template <std::size_t N>
  constexpr rope_ref(const rope<N>& r)
      : begin{r.strings.cbegin()}, end{r.strings.cend()}
  {}

  friend auto& operator<<(std::ostream& os, const rope_ref& rr)
  {
    for (auto it = rr.begin; it != rr.end; ++it) {
      os << *it;
    }
    return os;
  }
};

template <std::size_t N>
struct rope
{
  static constexpr auto size = N;

  std::array<std::string_view, N> strings;

  template <std::size_t M = N, class = std::enable_if_t<M == 1>>
  constexpr operator std::string_view() const
  {
    return strings[0];
  }

  friend auto& operator<<(std::ostream& os, const rope& r)
  {
    os << rope_ref{r};
    return os;
  }
};

}  // namespace skytest

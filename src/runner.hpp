#pragma once

#include "src/result.hpp"

#include <cstdlib>
#include <type_traits>

namespace skytest {

template <class Printer>
class runner
{
  Printer printer_;
  summary summary_{};

public:
  template <
      class P = Printer,
      class = std::enable_if_t<std::is_default_constructible_v<P>>>
  runner() : printer_{}
  {}

  explicit runner(Printer p) : printer_{p} {}

  ~runner()
  {
    printer_ << summary_;
    std::exit(summary_.fail != 0);
  }

  template <class R, class M>
  auto report(const result<R, M>& r) & -> void
  {
    printer_ << r;
    ++(r ? summary_.pass : summary_.fail);
  }
};

}  // namespace skytest

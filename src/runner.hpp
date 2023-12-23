#pragma once

#include "src/result.hpp"
#include "src/utility.hpp"

#include <cstdlib>
#include <type_traits>

namespace skytest {

template <class Printer>
class runner
{
  mutable Printer printer_;
  mutable summary summary_{};

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

    // NOLINTNEXTLINE(concurrency-mt-unsafe)
    std::exit(static_cast<int>(summary_.fail != 0));
  }

  template <class R, class M>
  auto report(const result<R, M>& r) const -> void
  {
    printer_ << r;
    ++(r ? summary_.pass : summary_.fail);
  }
};

}  // namespace skytest

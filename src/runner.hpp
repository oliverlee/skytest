#pragma once

#include "src/default_printer.hpp"
#include "src/result.hpp"
#include "src/utility.hpp"

#include <cstdlib>
#include <type_traits>

namespace skytest {
namespace detail {
struct aborts_fn;
}

template <class Printer = default_printer>
class runner
{
  mutable Printer printer_;
  mutable summary summary_{};
  mutable bool silent_ = false;

  friend struct ::skytest::detail::aborts_fn;

  auto silence() const { silent_ = true; }

public:
  template <
      class P = Printer,
      class = std::enable_if_t<std::is_default_constructible_v<P>>>
  runner() : printer_{}
  {}

  explicit runner(Printer p) : printer_{p} {}

  template <
      class... Args,
      class = std::enable_if_t<std::is_constructible_v<Printer, Args...>>>
  runner(Args&&... args) : printer_{std::forward<Args>(args)...}
  {}

  ~runner()
  {
    if (silent_) {
      return;
    }

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

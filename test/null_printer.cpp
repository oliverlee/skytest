#include "skytest/skytest.hpp"

namespace {
struct null_printer
{
  template <class T>
  friend auto& operator<<(null_printer& p, const T&)
  {
    return p;
  }
};
}  // namespace

template <>
auto ::skytest::cfg<::skytest::override> = ::skytest::runner<::null_printer>{};

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::expect;

  "expect true"_test = [] { return expect(true); };
}

#include "skytest/skytest.hpp"

#include <cassert>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::aborts;
  using ::skytest::expect;

  static const auto not_zero = [](int value) { assert(value != 0); };

  "aborts"_test = [] { return expect(aborts([] { not_zero(0); })); };
}

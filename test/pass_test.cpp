#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::expect;

  "expect true"_test = [] { return expect(true); };
}

#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::expect;

  "expect true 1"_test = [] { return expect(true); };
  "expect true 2"_test = [] { return expect(true); };
}

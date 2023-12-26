#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace skytest::literals;
  using ::skytest::expect;

  "truthy"_test = [] { return expect(true); };
}

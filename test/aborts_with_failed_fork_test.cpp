#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::aborts;
  using ::skytest::expect;

  "fork fails"_test = [] { return expect(aborts([] {})); };
}

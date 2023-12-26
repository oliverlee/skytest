#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace skytest::literals;
  using ::skytest::expect;
  using ::skytest::lt;

  "and"_test = [] { return expect(lt(1, 2) and lt(2, 3)); };
  "or"_test = [] { return expect(lt(3, 2) or lt(2, 3)); };
  "not"_test = [] { return expect(not lt(1, 0)); };
}

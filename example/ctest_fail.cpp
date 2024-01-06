#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace skytest::literals;
  using ::skytest::expect;
  using ::skytest::lt;

  static auto n = 0;
  "read non-const"_ctest = [] { return expect(lt(0, n)); };
}

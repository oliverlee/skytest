#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::types;

  "test1"_ctest = [] { return expect(eq(1, 1)); };

  "test2"_ctest * types<int> = [](auto param) {
    using T = typename decltype(param)::type;
    return expect(eq(0, T{}));
  };
}

#include "skytest/skytest.hpp"

#include <type_traits>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::constexpr_params;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::types;

  "test1"_ctest = [] { return expect(eq(1, 1)); };

  "test2"_ctest * types<int> = [](auto param) {
    using T = typename decltype(param)::type;
    return expect(eq(0, T{}));
  };

  "test3"_ctest * constexpr_params<1, 2U> = [](auto value) {
    return expect(eq(std::is_same_v<int, decltype(value)> ? 1 : 2, value));
  };
}

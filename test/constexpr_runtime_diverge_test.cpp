#include "skytest/skytest.hpp"

#include <type_traits>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;

  "runtime pass, constexpr fail"_test = [] {
    if (std::is_constant_evaluated()) {
      return expect(eq(1, 0));
    }
    return expect(eq(1, 1));
  };
  "runtime fail, constexpr pass"_test = [] {
    if (std::is_constant_evaluated()) {
      return expect(eq(1, 1));
    }
    return expect(eq(1, 0));
  };
}

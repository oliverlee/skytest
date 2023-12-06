#include "skytest/skytest.hpp"

#include <type_traits>

auto only_runtime() {}

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;

  "constexpr pass"_test = [] { return expect(eq(1, 1)); };
  "constexpr fail"_test = [] { return expect(eq(1, 2)); };

  "runtime pass"_test = [] {
    only_runtime();
    return expect(eq(1, 1));
  };
  "runtime fail"_test = [] {
    only_runtime();
    return expect(eq(1, 2));
  };

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

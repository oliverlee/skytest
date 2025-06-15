#include "skytest/skytest.hpp"

using ::skytest::eq;
using ::skytest::expect;

namespace {
auto only_runtime() {}

struct equals_one
{
  int value;
  constexpr auto operator()() const { return expect(eq(1, value)); }
};
}  // namespace

auto main() -> int
{
  using namespace ::skytest::literals;

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

  "closure with capture"_test = [x = 1] { return expect(eq(1, x)); };

  "function object with member value 1"_test = equals_one{1};
  "function object with member value 2"_test = equals_one{2};

  {
    const auto f = [] { return expect(eq(0, 0)); };
    "repeated closure 1"_test = f;
    "repeated closure 2"_test = f;
  }
}

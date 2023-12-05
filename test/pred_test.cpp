#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::expect;
  using ::skytest::pred;

  "equal one"_test = [] {
    return expect(pred([](auto t) { return t == 1; })(1));
  };
  "equal two"_test = [] {
    return expect(pred([](auto t) { return t == 2; })(1));
  };
}

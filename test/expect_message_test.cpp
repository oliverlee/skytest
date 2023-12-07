#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::expect;

  "message closure invoked on failure"_test = [] {
    return expect(false, [](auto& p) { p << "hello, world!"; });
  };

  "stateful message closure"_test = [] {
    return expect(false, [i = 1](auto& p) { p << "got: " << i; });
  };

  "not invoked on success"_test = [] {
    return expect(true, [](auto&) { throw std::runtime_error{"should not be invoked"}; });
  };
}

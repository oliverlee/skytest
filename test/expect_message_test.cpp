#include "skytest/skytest.hpp"

#include <exception>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::expect;

  "message string-view invoked on failure"_test = [] {
    return expect(false, "hello, world!");
  };

  "message closure invoked on failure"_test = [] {
    return expect(false, [](auto& p) { p << "hello, again!"; });
  };

  "stateful message closure"_test = [] {
    return expect(false, [i = 1](auto& p) { p << "got: " << i; });
  };

  "not invoked on success"_test = [] {
    return expect(true, [](auto&) { std::terminate(); });
  };
}

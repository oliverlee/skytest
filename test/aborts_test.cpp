#include "skytest/skytest.hpp"

#include <csignal>
#include <cstdlib>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::aborts;
  using ::skytest::expect;

  "aborts"_test = [] { return expect(aborts([] { std::abort(); })); };

  "does not abort"_test = [] { return expect(aborts([] {})); };

  "terminates due to sigsegv"_test = [] {
    return expect(aborts([] { std::raise(SIGSEGV); }));
  };

  "exits without abort"_test = [] {
    return expect(aborts([] { std::_Exit(1); }));
  };
}

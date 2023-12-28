#include "skytest/skytest.hpp"

#include <array>
#include <tuple>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::aborts;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::lt;

  "expect true"_test = [] { return expect(true); };

  "type param"_test * std::tuple<int, float, double>{} = [](auto param) {
    return expect(eq(0, param));
  };

  "value param"_test * std::array<int, 3>{0, 1, 2} = [](auto param) {
    return expect(lt(param, 3));
  };

  "aborts"_test = [] { return expect(aborts([] { std::abort(); })); };

  "message 1"_test = [] { return expect(eq(0, 1), "message 1"); };

  "message 2"_test = [] {
    return expect(eq(0, 1), [](auto& os) { os << "message 2"; });
  };
}

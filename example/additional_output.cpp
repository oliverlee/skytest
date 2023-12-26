#include "skytest/skytest.hpp"

#include <utility>

auto main() -> int
{
  using namespace skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;

  "string-view-ish"_test = [] { return expect(eq(1, 2), "a message"); };

  "ostream invocable closure"_test = [] {
    const auto x = std::pair{1, 2};
    const auto y = std::pair{2, 1};
    return expect(eq(x, y), [=](auto& os) {
      os << x.first << ", " << y.first << "\n";
    });
  };
}

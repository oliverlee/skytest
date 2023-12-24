#include "skytest/skytest.hpp"

#include <array>

auto main() -> int
{
  using namespace skytest::literals;
  using ::skytest::expect;
  using ::skytest::function;
  using ::skytest::pred;

  static constexpr auto empty = pred(function<"∅">, std::ranges::empty);

  "empty array"_test = [] { return expect(empty(std::array{1, 2, 3})); };
}

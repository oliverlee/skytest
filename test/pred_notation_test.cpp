#include "skytest/skytest.hpp"

#include <array>
#include <functional>
#include <iterator>
#include <string_view>
#include <utility>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::expect;
  using ::skytest::function;
  using ::skytest::infix;
  using ::skytest::pred;

  "infix notation"_test = [] {
    return expect(pred(infix<"@">, [](auto a, auto b) {
      return a == b;
    })(1, 2));
  };

  "function notation"_test = [] {
    return expect(pred(function<"f">, std::equal_to<>{})(1, 2));
  };

  static constexpr auto empty = pred(function<"∅">, std::ranges::empty);

  "empty array"_test = [] { return expect(empty(std::array{1, 23})); };
  "empty string"_test = [] { return expect(empty("hello")); };
}

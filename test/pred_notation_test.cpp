#include <array>
#include <iterator>
#include <ostream>
#include <string_view>
#include <utility>

// TODO: default printer for ranges

template <std::size_t N>
auto& operator<<(std::ostream& os, const std::array<int, N>& arr)
{
  auto first = true;

  os << "[";
  for (auto value : arr) {
    os << (std::exchange(first, false) ? "" : ", ") << value;
  }
  os << "]";
  return os;
}

#include "skytest/skytest.hpp"

using ::skytest::function;
using ::skytest::pred;

static constexpr auto empty =  //
    pred(function<"∅">, [](const auto& range) {
      using T = std::conditional_t<
          std::is_convertible_v<decltype(range), std::string_view>,
          std::string_view,
          decltype(range)>;

      return std::empty(static_cast<T>(range));
    });

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::expect;
  using ::skytest::infix;

  "infix notation"_test = [] {
    return expect(pred(infix<"@">, [](auto a, auto b) {
      return a == b;
    })(1, 2));
  };

  "function notation"_test = [] {
    return expect(pred(function<"f">, [](auto a, auto b) {
      return a == b;
    })(1, 2));
  };

  "empty array"_test = [] { return expect(empty(std::array{1, 23})); };
  "empty string"_test = [] { return expect(empty("hello")); };
}

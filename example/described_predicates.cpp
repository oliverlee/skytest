
#include "skytest/skytest.hpp"

#include <array>
#include <string_view>

using namespace skytest::literals;
using ::skytest::expect;
using ::skytest::pred;

struct empty_desc
{
  using notation_type = skytest::notation::function;
  static constexpr auto symbol = std::string_view{"empty"};
};

static constexpr auto empty = pred(empty_desc{}, [](const auto& rng) {
  return std::empty(rng);
});

auto main() -> int
{

  "empty array"_test = [] { return expect(empty(std::array{1, 2, 3})); };
}

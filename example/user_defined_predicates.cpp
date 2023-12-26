#include "skytest/skytest.hpp"

#include <array>
#include <iterator>

auto main() -> int
{
  using namespace skytest::literals;
  using ::skytest::expect;
  using ::skytest::pred;

  static constexpr auto empty = pred([](const auto& rng) {
    return std::empty(rng);
  });

  "empty array"_test = [] { return expect(empty(std::array{1, 2, 3})); };
}

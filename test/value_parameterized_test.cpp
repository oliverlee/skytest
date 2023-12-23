#include "skytest/skytest.hpp"

#include <array>
#include <initializer_list>
#include <vector>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::lt;
  using ::skytest::param_ref;

  "vector values"_test * std::vector{0, 1, 2} =  //
      [](auto value) { return expect(lt(value, 3)); };

  {
    static constexpr auto test_values = std::array{0, 1, 2};
    "array values"_test * param_ref<test_values> =  //
        [](const auto& value) {
          return expect(eq(value, &value - &test_values.front()));
        };
  }

  {
    static constexpr auto test_values = std::initializer_list<int>{0, 1, 2};
    "initializer_list values"_test * param_ref<test_values> =  //
        [](auto value) { return expect(lt(value, 3)); };
  }
}

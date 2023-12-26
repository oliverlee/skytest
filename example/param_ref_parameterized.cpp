#include "skytest/skytest.hpp"

#include <array>
#include <tuple>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::lt;
  using ::skytest::param_ref;

  static constexpr auto type_params = std::tuple{1.0F, 1.0};

  "types with param_ref"_test * param_ref<type_params> =  //
      [](auto param) { return expect(eq(1, param)); };

  static constexpr auto value_params = std::array{1, 2, 3};

  "values with param_ref"_test * param_ref<value_params> =  //
      [](auto param) { return expect(lt(0, param)); };
}

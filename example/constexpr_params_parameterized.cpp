#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::constexpr_params;
  using ::skytest::eq;
  using ::skytest::expect;

  "types with param_ref"_ctest * constexpr_params<1, 2U> =  //
      [](auto param) {
        return expect(eq(std::is_same_v<int, decltype(param)> ? 1 : 2, param));
      };
}

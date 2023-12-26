#include "skytest/skytest.hpp"

#include <cmath>
#include <complex>
#include <tuple>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::types;

  "typed"_test * std::tuple<int, float, std::complex<double>>{} =  //
      [](auto param) { return expect(eq(0, std::abs(param))); };

  "with types param"_test * types<int, float> =  //
      [](auto type_id) {
        using T = typename decltype(type_id)::type;
        return expect(eq(T{}, T{} + T{}));
      };
}

#include "skytest/skytest.hpp"

#include <complex>
#include <tuple>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::ne;
  using ::skytest::param;

  "typed"_test *
      std::tuple<float, std::complex<double>, decltype(nullptr)>{} =  //
      [](auto type_param) {
        using T = decltype(type_param);
        return expect(ne(T{}, T{}));
      };

  "constexpr typed"_test *
      param::types<float, std::complex<double>, decltype(nullptr)> =  //
      [](auto type_param) {
        using T = typename decltype(type_param)::type;
        return expect(eq(T{}, T{}));
      };
}

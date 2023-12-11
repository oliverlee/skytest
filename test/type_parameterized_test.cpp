#include "skytest/skytest.hpp"

#include <complex>
#include <tuple>

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::expect;
  using ::skytest::ne;

  "typed"_test *
      std::tuple<float, std::complex<double>, decltype(nullptr)>{} =  //
      [](auto type_param) {
        using T = decltype(type_param);
        return expect(ne(T{}, T{}));
      };
}

#include "skytest/skytest.hpp"

#include <complex>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace {
// TODO use https://github.com/apolukhin/magic_get
struct tuple_like
{
  int x;
  float y;
  double z;

  template <std::size_t I>
  friend constexpr auto get(const tuple_like& self)
  {
    if constexpr (I == 0) {
      return self.x;
    }
    if constexpr (I == 1) {
      return self.y;
    }
    if constexpr (I == 2) {
      return self.z;
    }
  }
};
}  // namespace

template <>
struct std::tuple_size<::tuple_like> : std::integral_constant<std::size_t, 3>
{};

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::ne;
  using ::skytest::param_ref;
  using ::skytest::types;

  "typed"_test *
      std::tuple<float, std::complex<double>, decltype(nullptr)>{} =  //
      [](auto type_param) {
        using T = decltype(type_param);
        return expect(ne(T{}, T{}));
      };

  "constexpr typed"_test *
      types<float, std::complex<double>, decltype(nullptr)> =  //
      [](auto type_param) {
        using T = typename decltype(type_param)::type;
        return expect(eq(T{}, T{}));
      };

  "pair types"_test * std::pair<float, double>{} =  //
      [](auto type_param) {
        using T = decltype(type_param);
        return expect(eq(T{}, T{}));
      };

  "custom tuple-like types"_test * tuple_like{1, 1.0F, 1.0} =  //
      [](auto type_param) {
        using T = decltype(type_param);
        return expect(eq(T{1}, type_param));
      };

  static constexpr auto test_types = tuple_like{1, 1.0F, 1.0};

  "constexpr custom tuple-like types"_test * param_ref<test_types> =  //
      [](auto type_param) {
        using T = decltype(type_param);
        return expect(eq(T{1}, type_param));
      };
}

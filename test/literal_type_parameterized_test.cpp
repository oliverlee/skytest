#include "skytest/skytest.hpp"

#include <cassert>
#include <cstddef>
#include <ranges>
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

template <int start, int step, int size>
struct arithmetic_t
{
  static constexpr auto value =
      std::views::iota(0, size) |
      std::views::transform([](int i) { return start + i * step; });
};
template <auto start, auto step, std::size_t size>
inline constexpr auto arithmetic = arithmetic_t<start, step, size>{};

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::param;

  "literal constexpr custom tuple-like types"_test *
      param<tuple_like{1, 1.0F, 1.0}> =  //
      [](auto type_param) {
        using T = decltype(type_param);
        return expect(eq(T{1}, type_param));
      };

  "literal constexpr range values"_test * param<std::array{1, 2, 3}> =  //
      [](auto value) { return expect(eq(2, value)); };

  "custom arithmetic range"_test * arithmetic<1, 3, 5> =  //
      [](auto value) { return expect(eq(value % 2, 0)); };
}

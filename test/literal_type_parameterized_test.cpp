#include "skytest/skytest.hpp"

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
  using ::skytest::param;

  "literal constexpr custom tuple-like types"_test *
      param<tuple_like{1, 1.0F, 1.0}> =  //
      [](auto type_param) {
        using T = decltype(type_param);
        return expect(eq(T{1}, type_param));
      };
}

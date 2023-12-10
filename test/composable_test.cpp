#include "skytest/skytest.hpp"

auto main() -> int
{
  using namespace ::skytest::literals;
  using ::skytest::eq;
  using ::skytest::expect;
  using ::skytest::lt;

  static constexpr auto x = 1;
  static constexpr auto y = 2;
  static constexpr auto z = 3;

  "[and] first fails"_test = [] { return expect(eq(x, y) and lt(y, z)); };
  "[and] second fails"_test = [] { return expect(lt(x, y) and eq(y, z)); };
  "[and] both fail"_test = [] { return expect(eq(x, y) and eq(y, z)); };

  "[or] both fail"_test = [] { return expect(eq(x, y) or eq(y, z)); };

  "[not] fails"_test = [] { return expect(not eq(x, x)); };

  "[and+not] fails"_test = [] {
    return expect(not eq(x, x) and not eq(y, y) and not eq(z, z));
  };

  "[and+and] fails"_test = [] {
    return expect(eq(x, x) and (eq(y, z) and eq(z, z)));
  };

  "[or] first passes"_test = [] { return expect(eq(x, y) or lt(y, z)); };
  "[or] second passes"_test = [] { return expect(eq(x, y) or lt(y, z)); };
}
